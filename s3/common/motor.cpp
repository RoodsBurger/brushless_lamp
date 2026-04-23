// FOC controller shared by every S3 stage. Runs SimpleFOC in velocity mode on
// a dedicated core-1 task; the position loop is closed here in the app layer
// with a pure physics-bounded trapezoidal profile and an M2-proven idle-disable.
//
// Two quirks baked in:
//   - motor.disable() writes setPwm(0,0,0) once; the very next move() re-drives
//     phases. So while idle-disabled we skip loopFOC()/move() entirely — the
//     encoder ISR keeps shaft_angle fresh in the background, and the next
//     target change wakes the task.
//   - attachInterrupt() binds to the calling core. Calling
//     encoder.enableInterrupts() from inside the motor task keeps the GPIO ISR
//     co-resident with the FOC loop so noInterrupts() in Encoder::update()
//     actually blocks it.

#include <Arduino.h>
#include <SimpleFOC.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>
#include <nvs.h>
#include <math.h>

#include "motor.h"
#include "config.h"
#include "pins.h"

// Cached FOC calibration: sensor_direction only. zero_electric_angle is NOT
// cached — the MT6701 runs in incremental quadrature, so its counter starts at
// 0 on every boot regardless of physical rotor position and a stale offset
// would point Uq the wrong way. sensor_direction is a property of rotor
// magnets vs encoder and is valid across reboots; caching it skips the loud
// forward/reverse rotation sweep inside alignSensor().
static constexpr const char *NVS_NS            = "foc_cal";
static constexpr const char *NVS_KEY_DIRECTION = "dir";

// FOC loop pacing. SimpleFOC produces the quietest PID output on this motor
// around 5 kHz; the S3 FPU would otherwise run loopFOC at ~50 kHz. 200 µs
// busy-wait holds the target period jitter-free below the FreeRTOS tick.
static constexpr uint32_t FOC_PERIOD_US = 200;

static BLDCMotor       s_motor  (POLE_PAIRS);
static BLDCDriver3PWM  s_driver (PIN_PWM_A, PIN_PWM_B, PIN_PWM_C);
static Encoder         s_encoder(PIN_ENC_A, PIN_ENC_B, ENCODER_CPR);

static volatile float  s_target_angle          = 0.0f;
static volatile float  s_shaft_angle_cached    = 0.0f;
static volatile float  s_shaft_velocity_cached = 0.0f;
static volatile bool   s_sync_pending          = false;
static void          (*s_settle_cb)(float)     = nullptr;

// Direct velocity mode (M1 audibility sweep, M2 knob velocity-nudge). When
// s_manual_mode is true the position loop is bypassed and commanded_vel tracks
// s_manual_vel through the MOTION_ACCEL ramp.
static volatile float  s_manual_vel    = 0.0f;
static volatile bool   s_manual_mode   = false;

// Runtime cruise cap, cycled by double-click through MOTION_VELOCITY_PRESETS.
static volatile float  s_motion_velocity = MOTION_VELOCITY;

static void IRAM_ATTR doMotorA() { s_encoder.handleA(); }
static void IRAM_ATTR doMotorB() { s_encoder.handleB(); }

static float clamp_angle(float rad) {
    if (rad < 0.0f)      return 0.0f;
    if (rad > ANGLE_MAX) return ANGLE_MAX;
    return rad;
}

static void motor_foc_task(void *) {
    // Encoder ISRs must attach from this task so they register on CORE_MOTOR.
    s_encoder.quadrature       = Quadrature::ON;
    s_encoder.pullup           = Pullup::USE_INTERN;
    s_encoder.min_elapsed_time = SENSOR_MIN_ELAPSED_TIME;
    s_encoder.init();
    s_encoder.enableInterrupts(doMotorA, doMotorB);
    s_motor.linkSensor(&s_encoder);

    pinMode(PIN_NSP, OUTPUT);
    digitalWrite(PIN_NSP, HIGH);
    delay(2);

    s_driver.voltage_power_supply = SUPPLY_VOLTAGE;
    s_driver.voltage_limit        = VOLTAGE_LIMIT;
    s_driver.pwm_frequency        = PWM_FREQ_HZ;
    printf("[motor] driver.init()=%d\n", s_driver.init());
    s_motor.linkDriver(&s_driver);

    s_motor.phase_resistance     = PHASE_RESISTANCE;
    s_motor.KV_rating            = KV_RATING;
    s_motor.current_limit        = CURRENT_LIMIT;
    s_motor.velocity_limit       = VELOCITY_LIMIT;
    s_motor.voltage_limit        = VOLTAGE_LIMIT;
    s_motor.voltage_sensor_align = VOLTAGE_SENSOR_ALIGN;
    s_motor.controller           = MotionControlType::velocity;
    s_motor.torque_controller    = TorqueControlType::voltage;
    s_motor.foc_modulation       = FOCModulationType::SpaceVectorPWM;
    s_motor.motion_downsample    = MOTION_DOWNSAMPLE;
    s_motor.modulation_centered  = 1;                 // 50 % duty at Uq=0 → no-torque idle state

    s_motor.PID_velocity.P           = PID_P;
    s_motor.PID_velocity.I           = PID_I;
    s_motor.PID_velocity.D           = PID_D;
    s_motor.PID_velocity.output_ramp = PID_OUTPUT_RAMP;
    s_motor.LPF_velocity.Tf          = LPF_TF;

    printf("[motor] motor.init()=%d\n", s_motor.init());

    // Reuse the direction detected on a prior boot if available.
    int8_t  saved_dir = 0;
    {
        nvs_handle_t h;
        if (nvs_open(NVS_NS, NVS_READONLY, &h) == ESP_OK) {
            nvs_get_i8(h, NVS_KEY_DIRECTION, &saved_dir);
            nvs_close(h);
        }
    }
    if (saved_dir == (int8_t)Direction::CW || saved_dir == (int8_t)Direction::CCW) {
        s_motor.sensor_direction = (Direction)saved_dir;
    }

    int foc_ok = s_motor.initFOC();
    printf("[motor] initFOC()=%d zero_elec=%.4f dir=%d\n",
           foc_ok, s_motor.zero_electric_angle, (int)s_motor.sensor_direction);
    if (!foc_ok) {
        printf("[motor] FOC init FAILED\n");
    } else if (saved_dir == 0 &&
               (s_motor.sensor_direction == Direction::CW || s_motor.sensor_direction == Direction::CCW)) {
        // Cache the direction so future boots skip the rotation sweep.
        nvs_handle_t h;
        if (nvs_open(NVS_NS, NVS_READWRITE, &h) == ESP_OK) {
            int8_t d = (int8_t)s_motor.sensor_direction;
            nvs_set_i8(h, NVS_KEY_DIRECTION, d);
            nvs_commit(h);
            nvs_close(h);
        }
    }

    s_motor.disable();   // boot idle; first target change wakes the task

    float    commanded_vel     = 0.0f;
    uint32_t last_ramp_us      = 0;
    uint32_t last_active_ms    = 0;
    uint32_t next_foc_us       = micros();
    uint32_t motion_started_ms = 0;     // stall: 0 = not trying to move
    uint32_t stuck_since_ms    = 0;     // stall: 0 = shaft isn't frozen in this attempt

    // Dedicated core: the motor task starves IDLE, so drop IDLE's wdt check.
    esp_task_wdt_delete(xTaskGetIdleTaskHandleForCore(CORE_MOTOR));

    while (true) {
        // Busy-wait to the next 200 µs boundary; if we fell behind by more
        // than two periods (e.g. a long printf), resync to avoid a catch-up
        // burst of back-to-back iterations.
        uint32_t now_us = micros();
        while ((int32_t)(now_us - next_foc_us) < 0) {
            now_us = micros();
        }
        next_foc_us += FOC_PERIOD_US;
        if ((int32_t)(now_us - next_foc_us) > (int32_t)(2 * FOC_PERIOD_US)) {
            next_foc_us = now_us + FOC_PERIOD_US;
        }

        // Direct-velocity path (M1 sweep, M2 velocity-nudge).
        if (s_manual_mode) {
            float dt_m = (last_ramp_us == 0) ? 0.0f : (now_us - last_ramp_us) * 1e-6f;
            last_ramp_us = now_us;
            if (dt_m > 0.0f && dt_m < 0.1f) {
                float step = MOTION_ACCEL * dt_m;
                if      (commanded_vel <  s_manual_vel - step) commanded_vel += step;
                else if (commanded_vel >  s_manual_vel + step) commanded_vel -= step;
                else                                           commanded_vel  = s_manual_vel;
            } else {
                commanded_vel = s_manual_vel;
            }

            bool at_rest_m = fabsf(commanded_vel)         < VEL_AT_REST_EPS &&
                             fabsf(s_motor.shaft_velocity) < VEL_AT_REST_EPS;
            if (!at_rest_m) last_active_ms = millis();
            bool live = (millis() - last_active_ms) < IDLE_DISABLE_MS;

            if (live) {
                if (!s_motor.enabled) s_motor.enable();
                s_motor.loopFOC();
                s_motor.move(commanded_vel);
            } else if (s_motor.enabled) {
                s_motor.disable();
                s_motor.PID_velocity.reset();
                s_motor.shaft_velocity = 0.0f;
                commanded_vel = 0.0f;
            }

            s_shaft_angle_cached    = s_motor.shaft_angle;
            s_shaft_velocity_cached = s_motor.shaft_velocity;
            continue;
        }

        // Position loop: physics-bounded trapezoidal descent toward target.
        float pos_err     = s_target_angle - s_motor.shaft_angle;
        float brake_v     = sqrtf(2.0f * MOTION_ACCEL * fabsf(pos_err));
        float desired_mag = fminf(s_motion_velocity, brake_v);
        float desired_raw = (pos_err >= 0.0f) ? desired_mag : -desired_mag;

        // Brake-on-reverse: a target flip forces commanded_vel through zero
        // before accepting the new sign.
        bool cur_fwd  = commanded_vel >  MOTION_EPS;
        bool cur_rev  = commanded_vel < -MOTION_EPS;
        bool want_fwd = desired_raw   >  MOTION_EPS;
        bool want_rev = desired_raw   < -MOTION_EPS;
        bool reversing = (want_fwd && cur_rev) || (want_rev && cur_fwd);
        float desired_vel = reversing ? 0.0f : desired_raw;

        float dt = (last_ramp_us == 0) ? 0.0f : (now_us - last_ramp_us) * 1e-6f;
        last_ramp_us = now_us;
        if (dt > 0.0f && dt < 0.1f) {
            float step = MOTION_ACCEL * dt;
            if      (commanded_vel <  desired_vel - step) commanded_vel += step;
            else if (commanded_vel >  desired_vel + step) commanded_vel -= step;
            else                                          commanded_vel  = desired_vel;
        } else {
            commanded_vel = desired_vel;
        }

        // Stall detection: if commanded motion isn't reaching the shaft after
        // WARMUP + TIMEOUT, snap the internal target to the current shaft (so
        // pos_err→0 and at_rest idle-disables next tick) and tell Matter about
        // the new resting position. A fresh target command re-engages the
        // motor; if the block is still there, stall fires again.
        bool trying_to_move = fabsf(commanded_vel) > STALL_VEL_EPS;
        if (trying_to_move) {
            uint32_t now_ms = millis();
            if (motion_started_ms == 0) motion_started_ms = now_ms;
            bool past_warmup  = (now_ms - motion_started_ms) > STALL_WARMUP_MS;
            bool shaft_frozen = fabsf(s_motor.shaft_velocity) < STALL_VEL_EPS;
            if (past_warmup && shaft_frozen) {
                if (stuck_since_ms == 0) stuck_since_ms = now_ms;
                else if ((now_ms - stuck_since_ms) > STALL_TIMEOUT_MS) {
                    printf("[motor] STALL at %.2f rad\n", s_motor.shaft_angle);
                    commanded_vel     = 0.0f;
                    s_target_angle    = s_motor.shaft_angle;
                    if (s_settle_cb) s_settle_cb(clamp_angle(s_motor.shaft_angle));
                    motion_started_ms = 0;
                    stuck_since_ms    = 0;
                }
            } else {
                stuck_since_ms = 0;
            }
        } else {
            motion_started_ms = 0;
            stuck_since_ms    = 0;
        }

        // Idle-disable: all three (commanded, shaft, pos_err) at rest → start
        // the idle timer; once expired the driver cuts PWM and we stop calling
        // move() so setPwm(0,0,0) persists. Entry also fires the settle
        // callback so Matter-slider or knob-initiated moves push back once.
        bool at_rest = fabsf(commanded_vel)          < VEL_AT_REST_EPS &&
                       fabsf(s_motor.shaft_velocity) < VEL_AT_REST_EPS &&
                       fabsf(pos_err)                < POS_AT_REST_EPS;
        if (!at_rest) last_active_ms = millis();
        bool should_enable = (millis() - last_active_ms) < IDLE_DISABLE_MS;

        if (should_enable) {
            if (!s_motor.enabled) s_motor.enable();
            s_motor.loopFOC();
            s_motor.move(commanded_vel);
        } else if (s_motor.enabled) {
            s_motor.disable();
            s_motor.PID_velocity.reset();
            s_motor.shaft_velocity = 0.0f;
            commanded_vel = 0.0f;
            if (s_sync_pending && s_settle_cb) {
                s_settle_cb(s_motor.shaft_angle);
                s_sync_pending = false;
            }
        }

        s_shaft_angle_cached    = s_motor.shaft_angle;
        s_shaft_velocity_cached = s_motor.shaft_velocity;
    }
}

void motor_init_and_start() {
    xTaskCreatePinnedToCore(motor_foc_task, "motor_foc", 8192, nullptr,
                            MOTOR_TASK_PRIORITY, nullptr, CORE_MOTOR);
}

void motor_set_target_angle(float rad)     { s_target_angle = clamp_angle(rad); }
void motor_nudge_target_angle(float d_rad) { s_target_angle = clamp_angle(s_target_angle + d_rad); }
void motor_request_matter_sync_on_settle() { s_sync_pending = true; }
void motor_set_settle_callback(void (*cb)(float)) { s_settle_cb = cb; }

void motor_run_at_velocity(float rad_per_sec) {
    if (rad_per_sec == 0.0f) {
        // Hand control back to the position loop at the current shaft
        // position so pos_err=0 and idle-disable fires cleanly next tick.
        s_manual_mode  = false;
        s_manual_vel   = 0.0f;
        s_target_angle = s_shaft_angle_cached;
    } else {
        s_manual_vel  = rad_per_sec;
        s_manual_mode = true;
    }
}

float motor_get_target_angle()   { return s_target_angle; }
float motor_get_shaft_angle()    { return s_shaft_angle_cached; }
float motor_get_shaft_velocity() { return s_shaft_velocity_cached; }

void motor_set_motion_velocity(float rad_per_sec) {
    if (rad_per_sec <= 0.0f) { s_motion_velocity = MOTION_VELOCITY; return; }
    if (rad_per_sec >  VELOCITY_LIMIT) rad_per_sec = VELOCITY_LIMIT;
    s_motion_velocity = rad_per_sec;
}
float motor_get_motion_velocity() { return s_motion_velocity; }
