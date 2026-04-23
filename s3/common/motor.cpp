// Shared motor controller for all S3 M-stages. SimpleFOC velocity mode (M2-proven
// quiet tuning) is the inner; the app-layer task closes the position loop via a
// linear + physics-bounded trapezoidal profile, plus brake-on-reverse and M2-style
// idle-disable. Pinned to core 1 — on the XIAO S3 this core is otherwise idle, so
// the FOC loop sees no preemption from WiFi/Matter/lwIP (they all live on core 0).
//
// Two production quirks worth preserving from the C6 phase 14c rounds:
//   1. SimpleFOC's motor.disable() only issues a single setPwm(0,0,0); the very next
//      motor.move() re-drives phases. So while idle-disabled we also *skip* the FOC
//      cycle entirely — the encoder ISR still updates shaft_angle in the background,
//      so the next target change wakes the task correctly.
//   2. attachInterrupt binds the ISR to the calling core. We call
//      encoder.enableInterrupts() from inside this task body, not from app_main, so
//      the GPIO ISR lives on CORE_MOTOR and noInterrupts() in Encoder::update()
//      actually blocks it.

#include <Arduino.h>
#include <SimpleFOC.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>
#include <math.h>

#include "motor.h"
#include "config.h"
#include "pins.h"

// Target FOC period. On C6 softfloat each loopFOC naturally took ~200 µs (→ ~5 kHz)
// and that was the silent M2-knob baseline. On S3's FPU a loopFOC is ~20 µs, so we
// busy-wait on micros() between iterations to hold the same 200 µs period jitter-
// free (no FreeRTOS tick granularity). This is the same cadence the Arduino
// reference ran at organically — exposed explicitly here because we have the CPU
// headroom to run faster and would otherwise.
static constexpr uint32_t FOC_PERIOD_US = 200;

static BLDCMotor       s_motor  (POLE_PAIRS);
static BLDCDriver3PWM  s_driver (PIN_PWM_A, PIN_PWM_B, PIN_PWM_C);
static Encoder         s_encoder(PIN_ENC_A, PIN_ENC_B, ENCODER_CPR);

static volatile float  s_target_angle  = 0.0f;
static volatile float  s_shaft_angle_cached    = 0.0f;
static volatile float  s_shaft_velocity_cached = 0.0f;
static volatile bool   s_sync_pending  = false;
static void          (*s_settle_cb)(float) = nullptr;

// Manual velocity override — M1's audibility test uses this to drive the motor at
// precise constant speeds, bypassing the position loop and its accel ramp. The
// position loop is skipped whenever s_manual_mode is true; a zero manual velocity
// explicitly clears the flag.
static volatile float  s_manual_vel    = 0.0f;
static volatile bool   s_manual_mode   = false;

// Runtime MOTION_VELOCITY (writable via double-click speed cycling). Initialised from
// config.h default; motor_set_motion_velocity() clamps to (0, VELOCITY_LIMIT].
static volatile float  s_motion_velocity = MOTION_VELOCITY;

static void IRAM_ATTR doMotorA() { s_encoder.handleA(); }
static void IRAM_ATTR doMotorB() { s_encoder.handleB(); }

static float clamp_angle(float rad) {
    if (rad < 0.0f)       return 0.0f;
    if (rad > ANGLE_MAX)  return ANGLE_MAX;
    return rad;
}

static float clampf(float v, float lo, float hi) {
    return (v < lo) ? lo : (v > hi ? hi : v);
}

// Fixed-point formatters — S3 has an FPU so %.2f is cheap, but keeping these keeps
// logs byte-identical to the M2-knob reference for easy diffing.
#define FX_HI(v, s) ((long)((int32_t)((v) * (s)) / (s)))
#define FX_LO(v, s) ((long)((((int32_t)((v) * (s))) < 0 ? -((int32_t)((v) * (s))) : ((int32_t)((v) * (s)))) % (s)))

static void motor_foc_task(void *) {
    // Encoder ISRs must attach from this task so they register on CORE_MOTOR.
    s_encoder.quadrature       = Quadrature::ON;
    s_encoder.pullup           = Pullup::USE_INTERN;
    s_encoder.min_elapsed_time = SENSOR_MIN_ELAPSED_TIME;   // gate velocity reads vs encoder quantization
    s_encoder.init();
    s_encoder.enableInterrupts(doMotorA, doMotorB);
    s_motor.linkSensor(&s_encoder);

    pinMode(PIN_NSP, OUTPUT);
    digitalWrite(PIN_NSP, HIGH);
    delay(2);

    s_driver.voltage_power_supply = SUPPLY_VOLTAGE;
    s_driver.voltage_limit        = VOLTAGE_LIMIT;    // hardware-specific cap (see config.h)
    s_driver.pwm_frequency        = PWM_FREQ_HZ;
    printf("[motor] driver.init()=%d\n", s_driver.init());
    s_motor.linkDriver(&s_driver);

    s_motor.phase_resistance     = PHASE_RESISTANCE;
    s_motor.KV_rating            = KV_RATING;
    s_motor.current_limit        = CURRENT_LIMIT;
    s_motor.velocity_limit       = VELOCITY_LIMIT;
    s_motor.voltage_limit        = VOLTAGE_LIMIT;       // Uq cap — PID_velocity.limit follows this
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

    int foc_ok = s_motor.initFOC();
    printf("[motor] initFOC()=%d zero_elec=%.4f dir=%d core=%d\n",
           foc_ok, s_motor.zero_electric_angle, (int)s_motor.sensor_direction, xPortGetCoreID());
    if (!foc_ok) printf("[motor] FOC init FAILED\n");

    s_motor.disable();   // boot idle — first target change wakes it

    float    commanded_vel = 0.0f;
    uint32_t last_ramp_us  = 0;
    uint32_t prev_loop_us  = 0;
    uint32_t max_loop_us   = 0;
    uint32_t last_print_ms = 0;
    uint32_t last_active_ms = 0;
    uint32_t iter          = 0;
    uint32_t next_foc_us   = micros();

    // On a dedicated core the motor task would starve IDLE and trip the IDLE
    // watchdog — don't check IDLE on this core. We still yield periodically via
    // the busy-wait fallback below, so the RTOS stays healthy.
    esp_task_wdt_delete(xTaskGetIdleTaskHandleForCore(CORE_MOTOR));

    while (true) {
        // Busy-wait until next 200 µs boundary. Matches Arduino M2's naturally-~5 kHz
        // softfloat loop rate, jitter-free below the FreeRTOS tick. Any spare time
        // after the iteration is spent in the wait — IDLE on this core won't run
        // (handled by the task_wdt_delete above), but the OTHER core isn't blocked.
        uint32_t now_us = micros();
        while ((int32_t)(now_us - next_foc_us) < 0) {
            now_us = micros();
        }
        next_foc_us += FOC_PERIOD_US;
        // If we're falling behind (took > FOC_PERIOD_US of processing), skip forward
        // so we don't try to "catch up" with a bunch of rapid-fire iterations.
        if ((int32_t)(now_us - next_foc_us) > (int32_t)(2 * FOC_PERIOD_US)) {
            next_foc_us = now_us + FOC_PERIOD_US;
        }
        if (prev_loop_us != 0) {
            uint32_t d = now_us - prev_loop_us;
            if (d > max_loop_us) max_loop_us = d;
        }
        prev_loop_us = now_us;

        // Manual-velocity mode — keeps the same trapezoidal ramp (MOTION_ACCEL) as the
        // production position loop, but takes the target velocity directly from the
        // app instead of deriving it from pos_err. Only enters on s_manual_mode: the
        // older "also run if commanded_vel hasn't fully decayed" fallthrough caused
        // angle-only stages (M3/M4) to ping-pong between branches whenever the
        // position loop was commanding a non-trivial velocity.
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

            bool at_rest_m = fabsf(commanded_vel) < VEL_AT_REST_EPS &&
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

            if (millis() - last_print_ms >= PRINT_INTERVAL_MS) {
                last_print_ms = millis();
                printf("[motor] manual target:%ld.%02ld cmd:%ld.%02ld shaft_vel:%ld.%02ld ang:%ld.%02ld en:%d maxLp:%luus\n",
                       FX_HI(s_manual_vel,            100), FX_LO(s_manual_vel,            100),
                       FX_HI(commanded_vel,           100), FX_LO(commanded_vel,           100),
                       FX_HI(s_motor.shaft_velocity,  100), FX_LO(s_motor.shaft_velocity,  100),
                       FX_HI(s_motor.shaft_angle,     100), FX_LO(s_motor.shaft_angle,     100),
                       (int)s_motor.enabled,
                       (unsigned long)max_loop_us);
                max_loop_us = 0;
            }
            ++iter;
            continue;
        }

        float pos_err = s_target_angle - s_motor.shaft_angle;

        // Pure physics-bounded trapezoidal — desired speed at distance `err` is the
        // highest value we can still brake to zero from, given MOTION_ACCEL. Capping
        // by MOTION_VELOCITY gives the textbook accel/cruise/decel profile; no linear
        // term near target, so motion stays fast until the last few ticks.
        float brake_v     = sqrtf(2.0f * MOTION_ACCEL * fabsf(pos_err));
        float desired_mag = fminf(s_motion_velocity, brake_v);
        float desired_raw = (pos_err >= 0.0f) ? desired_mag : -desired_mag;

        // Brake-on-reverse: opposite-direction target forces commanded through zero before
        // accepting the new sign. Same-direction updates flow through the ramp unchanged.
        const float MOTION_EPS = 0.5f;
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

        // Idle-disable: commanded + shaft + pos_err all at rest → start the idle timer.
        // After IDLE_DISABLE_MS the driver cuts PWM and we stop calling motor.move() so
        // setPwm(0,0,0) sticks. On entry: invoke the settle callback (M4 Matter push).
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

        if (millis() - last_print_ms >= PRINT_INTERVAL_MS) {
            last_print_ms = millis();
            printf("[motor] tgt:%ld.%02ld ang:%ld.%02ld vel:%ld.%02ld cmd:%ld.%02ld en:%d maxLp:%luus\n",
                   FX_HI(s_target_angle,         100), FX_LO(s_target_angle,         100),
                   FX_HI(s_motor.shaft_angle,    100), FX_LO(s_motor.shaft_angle,    100),
                   FX_HI(s_motor.shaft_velocity, 100), FX_LO(s_motor.shaft_velocity, 100),
                   FX_HI(commanded_vel,          100), FX_LO(commanded_vel,          100),
                   (int)s_motor.enabled,
                   (unsigned long)max_loop_us);
            max_loop_us = 0;
        }

        ++iter;
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
        s_manual_mode = false;
        s_manual_vel  = 0.0f;
        // Don't call disable here — the position loop's idle-disable will pick up
        // on the new at_rest state and cut PWM cleanly on its next tick.
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
