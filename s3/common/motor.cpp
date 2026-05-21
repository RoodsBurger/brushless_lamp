// FOC + position loop on core 1; encoder ISR attaches inside the task so noInterrupts() (per-core) blocks it. Idle-disable skips loopFOC()/move() so setPwm(0,0,0) holds.

#include <Arduino.h>
#include <SimpleFOC.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs.h>
#include <math.h>

#include "motor.h"
#include "config.h"
#include "pins.h"

static const char *TAG = "motor";

// NVS calibration: dir cached because initFOC's direction sweep is unreliable when
// the rotor is pinned at the off-stop. homed flag skips the install-time homing on
// subsequent boots. Both wiped by matter_wipe_local_nvs() on factory reset.
static constexpr const char *NVS_NS            = "foc_cal";
static constexpr const char *NVS_KEY_DIRECTION = "dir";
static constexpr const char *NVS_KEY_HOMED     = "homed";

// 200 µs busy-wait paces loopFOC at 5 kHz — quietest PID output on this motor; the
// S3 FPU would otherwise free-run at ~50 kHz and whine.
static constexpr uint32_t FOC_PERIOD_US = 200;

static BLDCMotor       s_motor  (POLE_PAIRS);
static BLDCDriver3PWM  s_driver (PIN_PWM_A, PIN_PWM_B, PIN_PWM_C);
static Encoder         s_encoder(PIN_ENC_A, PIN_ENC_B, ENCODER_CPR);

static volatile float  s_target_angle          = 0.0f;
static volatile float  s_shaft_angle_cached    = 0.0f;
static volatile float  s_shaft_velocity_cached = 0.0f;
static volatile bool   s_sync_pending          = false;
static void          (*s_settle_cb)(float)     = nullptr;

// Runtime cruise cap, cycled by triple-click through MOTION_VELOCITY_PRESETS.
static volatile float  s_motion_velocity = MOTION_VELOCITY;

static void IRAM_ATTR doMotorA() { s_encoder.handleA(); }
static void IRAM_ATTR doMotorB() { s_encoder.handleB(); }

static float clamp_angle(float rad) {
    if (rad < 0.0f)      return 0.0f;
    if (rad > ANGLE_MAX) return ANGLE_MAX;
    return rad;
}

static bool load_sensor_direction() {
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK) return false;
    int8_t v = 0;
    esp_err_t err = nvs_get_i8(h, NVS_KEY_DIRECTION, &v);
    nvs_close(h);
    if (err != ESP_OK) return false;
    s_motor.sensor_direction = (v > 0) ? Direction::CW : Direction::CCW;
    return true;
}

static void save_sensor_direction() {
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    int8_t v = (s_motor.sensor_direction == Direction::CW) ? 1 : -1;
    nvs_set_i8(h, NVS_KEY_DIRECTION, v);
    nvs_commit(h);
    nvs_close(h);
}

static bool load_homed_flag() {
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK) return false;
    uint8_t v = 0;
    esp_err_t err = nvs_get_u8(h, NVS_KEY_HOMED, &v);
    nvs_close(h);
    return (err == ESP_OK && v == 1);
}

static void save_homed_flag() {
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    uint8_t v = 1;
    nvs_set_u8(h, NVS_KEY_HOMED, v);
    nvs_commit(h);
    nvs_close(h);
}

// Drive lead screw against off-stop in closed-loop velocity until stall, then reboot to re-init FOC at the wedged rotor position.
static void run_first_boot_homing() {
    constexpr float    HOMING_VELOCITY   = -10.0f;
    constexpr uint32_t HOMING_TIMEOUT_MS = 60000;

    ESP_LOGI(TAG, "first-boot homing: driving lead screw toward off-stop");
    s_motor.enable();

    uint32_t start_ms          = millis();
    uint32_t motion_started_ms = 0;
    uint32_t stuck_since_ms    = 0;
    uint32_t next_foc_us       = micros();
    uint32_t last_ramp_us      = micros();
    float    commanded_vel     = 0.0f;

    while ((millis() - start_ms) < HOMING_TIMEOUT_MS) {
        uint32_t now_us = micros();
        while ((int32_t)(now_us - next_foc_us) < 0) now_us = micros();
        next_foc_us += FOC_PERIOD_US;
        if ((int32_t)(now_us - next_foc_us) > (int32_t)(2 * FOC_PERIOD_US)) {
            next_foc_us = now_us + FOC_PERIOD_US;
        }

        float dt = (now_us - last_ramp_us) * 1e-6f;
        last_ramp_us = now_us;
        if (dt > 0.0f && dt < 0.1f) {
            float step = MOTION_ACCEL * dt;
            if      (commanded_vel >  HOMING_VELOCITY + step) commanded_vel -= step;
            else if (commanded_vel <  HOMING_VELOCITY - step) commanded_vel += step;
            else                                              commanded_vel  = HOMING_VELOCITY;
        }

        s_motor.loopFOC();
        s_motor.move(commanded_vel);

        bool ramp_done = fabsf(commanded_vel - HOMING_VELOCITY) < 0.1f;
        if (!ramp_done) {
            motion_started_ms = 0;
            stuck_since_ms = 0;
            continue;
        }

        uint32_t now_ms = millis();
        if (motion_started_ms == 0) motion_started_ms = now_ms;
        bool past_warmup  = (now_ms - motion_started_ms) > STALL_WARMUP_MS;
        bool shaft_frozen = fabsf(s_motor.shaft_velocity) < STALL_VEL_EPS;

        if (past_warmup && shaft_frozen) {
            if (stuck_since_ms == 0) stuck_since_ms = now_ms;
            else if ((now_ms - stuck_since_ms) > STALL_TIMEOUT_MS) {
                ESP_LOGI(TAG, "first-boot homing: stalled at off-stop (shaft_angle=%.3f)",
                         s_motor.shaft_angle);
                break;
            }
        } else {
            stuck_since_ms = 0;
        }
    }

    s_motor.disable();
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
    if (PIN_DRV_EN >= 0) {
        // Teyleten board wires DRV8313 EN to a GPIO; XIAO has it jumpered to 3V3.
        pinMode((uint8_t)PIN_DRV_EN, OUTPUT);
        digitalWrite((uint8_t)PIN_DRV_EN, HIGH);
    }
    delay(2);

    s_driver.voltage_power_supply = SUPPLY_VOLTAGE;
    s_driver.voltage_limit        = VOLTAGE_LIMIT;
    s_driver.pwm_frequency        = PWM_FREQ_HZ;
    int drv_ok = s_driver.init();
    ESP_LOGI(TAG, "driver.init()=%d", drv_ok);
    if (!drv_ok) {
        ESP_LOGE(TAG, "driver init failed; halting motor task");
        vTaskDelete(nullptr);
    }
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

    int mot_ok = s_motor.init();
    ESP_LOGI(TAG, "motor.init()=%d", mot_ok);
    if (!mot_ok) {
        ESP_LOGE(TAG, "motor init failed; halting motor task");
        vTaskDelete(nullptr);
    }

    // NVS-cached direction skips initFOC's direction sweep — only zero_electric_angle is computed, which is the only sweep that works with the rotor pinned at the off-stop.
    bool dir_cached = load_sensor_direction();
    ESP_LOGI(TAG, "sensor_direction loaded from NVS: %s (dir=%d)",
             dir_cached ? "yes" : "no", (int)s_motor.sensor_direction);

    int foc_ok = s_motor.initFOC();
    ESP_LOGI(TAG, "initFOC()=%d zero_elec=%.4f dir=%d",
             foc_ok, s_motor.zero_electric_angle, (int)s_motor.sensor_direction);
    if (!foc_ok) {
        ESP_LOGE(TAG, "FOC init failed; halting motor task — check encoder + driver wiring");
        s_motor.disable();
        vTaskDelete(nullptr);
    }

    if (!dir_cached) {
        save_sensor_direction();
        ESP_LOGI(TAG, "sensor_direction saved to NVS");
    }

    // First-boot install homing; flag skips it on subsequent boots, factory reset re-arms.
    if (!load_homed_flag()) {
        run_first_boot_homing();
        ESP_LOGI(TAG, "first-boot homing: saving flag and rebooting");
        save_homed_flag();
        vTaskDelay(pdMS_TO_TICKS(200));   // drain the log line
        esp_restart();
    }

    // Prime shaft_angle by running one loopFOC before the position loop reads it; otherwise
    // pos_err is computed from the alignment-sweep's last sample (often a few rad off zero)
    // and the loop drives the motor toward whatever stale value sat in shaft_angle.
    s_motor.loopFOC();
    s_target_angle           = s_motor.shaft_angle;
    s_shaft_angle_cached     = s_motor.shaft_angle;
    s_shaft_velocity_cached  = 0.0f;

    s_motor.disable();   // boot idle; first target change wakes the task

    float    commanded_vel     = 0.0f;
    uint32_t last_ramp_us      = 0;
    uint32_t last_active_ms    = 0;
    uint32_t next_foc_us       = micros();
    uint32_t motion_started_ms = 0;     // stall: 0 = not trying to move
    uint32_t stuck_since_ms    = 0;     // stall: 0 = shaft isn't frozen in this attempt

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

        // Stall: if commanded |v| > eps but shaft frozen past WARMUP+TIMEOUT, snap target
        // to shaft so pos_err→0 and idle-disable fires; report new resting angle to Matter.
        bool trying_to_move = fabsf(commanded_vel) > STALL_VEL_EPS;
        if (trying_to_move) {
            uint32_t now_ms = millis();
            if (motion_started_ms == 0) motion_started_ms = now_ms;
            bool past_warmup  = (now_ms - motion_started_ms) > STALL_WARMUP_MS;
            bool shaft_frozen = fabsf(s_motor.shaft_velocity) < STALL_VEL_EPS;
            if (past_warmup && shaft_frozen) {
                if (stuck_since_ms == 0) stuck_since_ms = now_ms;
                else if ((now_ms - stuck_since_ms) > STALL_TIMEOUT_MS) {
                    ESP_LOGW(TAG, "STALL at %.2f rad", s_motor.shaft_angle);
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

        // Idle-disable when commanded, shaft, and pos_err all rest below their eps; entry
        // fires the settle callback so Matter learns knob-driven moves.
        bool at_rest = fabsf(commanded_vel)          < VEL_AT_REST_EPS &&
                       fabsf(s_motor.shaft_velocity) < VEL_AT_REST_EPS &&
                       fabsf(pos_err)                < POS_AT_REST_EPS;
        if (!at_rest) last_active_ms = millis();
        bool should_enable = (millis() - last_active_ms) < IDLE_DISABLE_MS;

        if (should_enable) {
            if (!s_motor.enabled) {
                s_motor.enable();
                // Reset ramp + stall trackers so stale state from before the disable window
                // can't bias commanded_vel or false-trip the stall detector on re-engage.
                commanded_vel     = 0.0f;
                last_ramp_us      = 0;
                motion_started_ms = 0;
                stuck_since_ms    = 0;
            }
            s_motor.loopFOC();
            s_motor.move(commanded_vel);
        } else if (s_motor.enabled) {
            s_motor.disable();
            s_motor.PID_velocity.reset();
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


float motor_get_target_angle()   { return s_target_angle; }
float motor_get_shaft_angle()    { return s_shaft_angle_cached; }
float motor_get_shaft_velocity() { return s_shaft_velocity_cached; }

void motor_set_motion_velocity(float rad_per_sec) {
    if (rad_per_sec <= 0.0f) { s_motion_velocity = MOTION_VELOCITY; return; }
    if (rad_per_sec >  VELOCITY_LIMIT) rad_per_sec = VELOCITY_LIMIT;
    s_motion_velocity = rad_per_sec;
}
float motor_get_motion_velocity() { return s_motion_velocity; }
