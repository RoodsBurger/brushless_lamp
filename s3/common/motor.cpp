// FOC + position loop on core 1; encoder ISR attaches inside the task so noInterrupts() (per-core) blocks it.
// loopFOC()/move() run even while disabled (they only refresh sensor state then) so the LED fade tracks the
// real shaft; back-drive never wakes the motor — only commands do (wake compares target vs parked snapshot).
// Idle parks the DRV8313 via nSLEEP: outputs Hi-Z so the shaft spins truly free (user wants zero torque/drag
// at idle — plain disable() would short the windings and feel damped), and any latched OCP/TSD fault clears.

#include <Arduino.h>
#include <SimpleFOC.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <nvs.h>
#include <math.h>

#include "motor.h"
#include "config.h"
#include "pins.h"

static const char *TAG = "motor";

// NVS keys (all wiped by matter_wipe_local_nvs() on factory reset).
static constexpr const char *NVS_NS            = "foc_cal";
static constexpr const char *NVS_KEY_DIRECTION = "dir";    // sensor_direction across boots; initFOC's sweep is unreliable at the off-stop
static constexpr const char *NVS_KEY_HOMED     = "homed";  // install-time homing done flag
static constexpr const char *NVS_KEY_POSITION  = "pos";    // last user_angle; restored as home_offset after the encoder resets on power-on
static constexpr float       POS_SAVE_EPS_RAD  = 0.1f;     // skip save if move was <0.1 rad — spares flash

// 200 µs busy-wait paces loopFOC at 5 kHz — quietest PID output on this motor; the
// S3 FPU would otherwise free-run at ~50 kHz and whine.
static constexpr uint32_t FOC_PERIOD_US = 200;

static BLDCMotor       s_motor  (POLE_PAIRS);
static BLDCDriver3PWM  s_driver (PIN_PWM_A, PIN_PWM_B, PIN_PWM_C);
static Encoder         s_encoder(PIN_ENC_A, PIN_ENC_B, ENCODER_PPR);

static volatile float  s_target_angle          = 0.0f;
static volatile float  s_shaft_angle_cached    = 0.0f;
static volatile bool   s_sync_pending          = false;
static volatile bool   s_sync_allow_on         = false;   // may the pending settle turn the lamp ON? (knob raise only)
static volatile bool   s_home_request          = false;   // set by motor_request_homing(), serviced on the FOC task
static volatile bool   s_fault                 = false;   // driver/motor/FOC init failure — surfaced by the status LED
static void          (*s_settle_cb)(float, bool) = nullptr;
// Encoder reading that maps to user_angle=0; loaded from NVS at boot so the lead screw's
// physical position carries across plug/unplug (incremental encoder otherwise resets to 0).
static float           s_home_offset           = 0.0f;
static float           s_last_saved_position   = NAN;
// True while the motor is parked (disabled, at rest, position saved). OTA waits on
// this so a reboot never interrupts mid-travel and corrupts the saved zero.
static volatile bool   s_idle                  = true;

// Runtime cruise cap, cycled by triple-click through MOTION_VELOCITY_PRESETS.
static volatile float  s_motion_velocity = MOTION_VELOCITY;

static void IRAM_ATTR doMotorA() { s_encoder.handleA(); }
static void IRAM_ATTR doMotorB() { s_encoder.handleB(); }

// portMUX guards target-angle read-modify-writes against the 5 kHz reader on CORE_MOTOR.
static portMUX_TYPE s_target_mux = portMUX_INITIALIZER_UNLOCKED;

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

static bool load_position(float *out) {
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK) return false;
    size_t sz = sizeof(float);
    float v = 0.0f;
    esp_err_t err = nvs_get_blob(h, NVS_KEY_POSITION, &v, &sz);
    nvs_close(h);
    if (err != ESP_OK || sz != sizeof(float)) return false;
    *out = v;
    return true;
}

static void save_position(float v) {
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    esp_err_t err = nvs_set_blob(h, NVS_KEY_POSITION, &v, sizeof(float));
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    if (err == ESP_OK) s_last_saved_position = v;   // failed saves retry on the next settle
}

static void save_homed_flag() {
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    uint8_t v = 1;
    nvs_set_u8(h, NVS_KEY_HOMED, v);
    nvs_commit(h);
    nvs_close(h);
}

// Drive lead screw against off-stop in closed-loop velocity until stall, then reboot to re-init FOC at the wedged rotor position. Returns false on timeout.
static bool run_first_boot_homing() {
    constexpr float    HOMING_VELOCITY   = -3.0f;   // slow approach so stall fires before the rotor wedges hard
    constexpr uint32_t HOMING_TIMEOUT_MS = 120000;  // worst-case travel at -3 rad/s ≈ 360 rad
    constexpr float    HOMING_VOLTAGE    = 1.0f;    // very low Uq cap — soft impact at the off-stop

    ESP_LOGI(TAG, "first-boot homing: driving lead screw toward off-stop");
    s_idle = false;                       // homing in progress — OTA must not reboot
    float saved_vlimit = s_motor.voltage_limit;
    s_motor.updateVoltageLimit(HOMING_VOLTAGE);   // also re-clamps PID anti-windup, unlike a raw field write
    s_motor.enable();
    bool stalled = false;

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
                stalled = true;
                break;
            }
        } else {
            stuck_since_ms = 0;
        }
    }

    s_motor.disable();
    s_motor.updateVoltageLimit(saved_vlimit);   // restore runtime torque headroom
    return stalled;
}

// Runtime re-home (knob-hold gesture). Drives to the off-stop at a soft Uq cap and
// makes that physical position the new logical 0. Unlike boot homing it does NOT
// reboot — FOC is already calibrated, so we only re-zero s_home_offset. Runs inline
// on the FOC task, blocking the position loop for the descent. Returns true if it
// reached the stop and re-zeroed; false on timeout (prior zero left intact).
static bool run_runtime_homing() {
    ESP_LOGW(TAG, "runtime homing: driving to off-stop");
    s_idle = false;                       // homing in progress — OTA must not reboot
    float saved_vlimit = s_motor.voltage_limit;
    s_motor.updateVoltageLimit(HOMING_VOLTAGE);
    digitalWrite(PIN_NSP, HIGH);
    delayMicroseconds(1100);          // DRV8313 tWAKE before driving
    s_motor.enable();

    uint32_t start_ms          = millis();
    uint32_t motion_started_ms = 0;
    uint32_t stuck_since_ms    = 0;
    uint32_t next_foc_us       = micros();
    uint32_t last_ramp_us      = micros();
    float    commanded_vel     = 0.0f;
    bool     stalled           = false;

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
        if (!ramp_done) { motion_started_ms = 0; stuck_since_ms = 0; continue; }

        uint32_t now_ms = millis();
        if (motion_started_ms == 0) motion_started_ms = now_ms;
        bool past_warmup  = (now_ms - motion_started_ms) > STALL_WARMUP_MS;
        bool shaft_frozen = fabsf(s_motor.shaft_velocity) < STALL_VEL_EPS;
        if (past_warmup && shaft_frozen) {
            if (stuck_since_ms == 0) stuck_since_ms = now_ms;
            else if ((now_ms - stuck_since_ms) > STALL_TIMEOUT_MS) { stalled = true; break; }
        } else {
            stuck_since_ms = 0;
        }
    }

    s_motor.disable();
    digitalWrite(PIN_NSP, LOW);
    s_motor.updateVoltageLimit(saved_vlimit);

    if (!stalled) {
        // Timed out without reaching the off-stop — the shaft is at an arbitrary mid-travel
        // position, so re-zeroing here would persist a wrong logical 0. Keep the prior zero.
        ESP_LOGW(TAG, "runtime homing: TIMEOUT — no stop found; keeping prior zero");
        s_idle = true;
        return false;
    }

    // The wedged physical position becomes logical 0; target follows so the lamp parks at the bottom.
    s_home_offset = s_motor.shaft_angle;
    portENTER_CRITICAL(&s_target_mux);
    s_target_angle = 0.0f;
    portEXIT_CRITICAL(&s_target_mux);
    s_shaft_angle_cached = 0.0f;
    save_position(0.0f);
    s_sync_allow_on = false;          // homing must not turn the lamp on
    s_sync_pending  = true;           // push the new 0/off state to Matter on the next idle
    ESP_LOGW(TAG, "runtime homing: stalled at stop; re-zeroed (shaft=%.3f)", s_home_offset);
    s_idle = true;                        // parked at 0 + saved — safe to reboot again
    return true;
}

static void motor_foc_task(void *) {
    // Encoder ISRs must attach from this task so they register on CORE_MOTOR.
    s_encoder.quadrature       = Quadrature::ON;
    s_encoder.pullup           = Pullup::USE_INTERN;
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
        s_fault = true;
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
    // Voltage mode is the silent baseline (docs recommend it for low-speed
    // gimbals). estimated_current would enforce CURRENT_LIMIT but its raw
    // back-EMF feed-forward injects 1 kHz velocity-jitter into Uq and cancels
    // the motor's EM damping, making it audibly louder.
    // Stall current is bounded instead by the runtime stall handler below.
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
        s_fault = true;
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
        s_fault = true;
        s_motor.disable();
        vTaskDelete(nullptr);
    }

    if (!dir_cached) {
        save_sensor_direction();
        ESP_LOGI(TAG, "sensor_direction saved to NVS");
    }

    // First-boot install homing; flag skips it on subsequent boots, factory reset re-arms.
    if (!load_homed_flag()) {
        if (run_first_boot_homing()) {
            ESP_LOGI(TAG, "first-boot homing: saving flag and rebooting");
            save_homed_flag();
            vTaskDelay(pdMS_TO_TICKS(200));   // drain the log line
            esp_restart();
        }
        // Timeout without a stall: run unhomed this boot; next reboot retries instead of trusting an arbitrary zero.
        ESP_LOGE(TAG, "first-boot homing: timed out without stalling — continuing unhomed");
    }

    s_motor.loopFOC();   // prime shaft_angle; alignment-sweep's last sample is stale otherwise

    // Restore last-known user_angle from NVS so plug/unplug retains physical position.
    float saved_pos = 0.0f;
    if (load_position(&saved_pos)) {
        s_home_offset = s_motor.shaft_angle - saved_pos;
        // Clamp: a firmware update may have shrunk ANGLE_MAX below the saved position;
        // the clamped target walks the lamp down to the new max on first wake.
        s_target_angle = clamp_angle(saved_pos);
        s_sync_allow_on = false;            // a restore reflects position but must not turn the lamp on
        s_sync_pending  = true;             // push current state to Matter on first idle-disable
        ESP_LOGI(TAG, "restored position: user_angle=%.3f (home_offset=%.3f)", saved_pos, s_home_offset);
    } else {
        s_home_offset = s_motor.shaft_angle;
        s_target_angle = 0.0f;
        ESP_LOGI(TAG, "no saved position; home_offset=%.3f", s_home_offset);
    }
    s_shaft_angle_cached = s_motor.shaft_angle - s_home_offset;
    s_motor.disable();   // boot idle; first target change wakes the task
    digitalWrite(PIN_NSP, LOW);   // sleep the driver: Hi-Z outputs = free shaft, not disable()'s shorted-winding brake
    s_idle = true;       // parked baseline regardless of the homing path above

    float    commanded_vel     = 0.0f;
    float    parked_angle      = s_shaft_angle_cached;   // wake reference while disabled
    uint32_t last_ramp_us      = 0;
    uint32_t last_active_ms    = 0;
    uint32_t motion_started_ms = 0;
    uint32_t stuck_since_ms    = 0;
    uint32_t next_foc_us       = micros();

    while (true) {
        // Knob-hold re-home: runs the descent inline, then re-zeros and parks idle at 0.
        if (s_home_request) {
            s_home_request = false;
            bool rezeroed = run_runtime_homing();
            commanded_vel     = 0.0f;
            last_ramp_us      = 0;
            motion_started_ms = 0;
            stuck_since_ms    = 0;
            // Snap the wake-check baseline to 0 only if homing actually re-zeroed; on a
            // timeout keep the current logical position so we don't wedge toward a false 0.
            parked_angle      = rezeroed ? 0.0f : (s_motor.shaft_angle - s_home_offset);
            last_active_ms    = 0;        // (now_ms - 0) >> IDLE_DISABLE_MS → stay idle, don't wake
            next_foc_us       = micros();
            continue;
        }

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

        // esp_timer wraps at 2^32 ms (~49.7 days); micros()/1000 would wrap every ~71.6 min and blip the idle timer.
        uint32_t now_ms   = (uint32_t)(esp_timer_get_time() / 1000);
        // Position loop: physics-bounded trapezoidal descent toward target.
        // user_angle = shaft_angle - home_offset so plug/unplug restores the same logical position.
        float user_angle  = s_motor.shaft_angle - s_home_offset;
        float pos_err     = s_target_angle - user_angle;
        float brake_v     = sqrtf(2.0f * MOTION_ACCEL * fabsf(pos_err));
        float desired_mag = fminf(s_motion_velocity, brake_v);
        // Deadband inside POS_AT_REST_EPS: the bare brake curve stays above
        // VEL_AT_REST_EPS until ~0.002 rad, which stiction can make unreachable —
        // the motor would hold torque forever instead of idling.
        if (fabsf(pos_err) < POS_AT_REST_EPS) desired_mag = 0.0f;
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
        }
        // dt ≥ 0.1 s (NVS commit, settle work): hold the previous commanded_vel for one tick instead of snapping.

        // Stall: shaft frozen while commanded to move → accept the physical position
        // as the new target, pulse nSLEEP to clear any latched DRV8313 OCP/TSD, and
        // let the idle path persist + sync the position to Matter.
        if (s_motor.enabled && fabsf(commanded_vel) > STALL_VEL_EPS) {
            if (motion_started_ms == 0) motion_started_ms = now_ms;
            bool past_warmup  = (now_ms - motion_started_ms) > STALL_WARMUP_MS;
            bool shaft_frozen = fabsf(s_motor.shaft_velocity) < STALL_VEL_EPS;
            if (past_warmup && shaft_frozen && stuck_since_ms == 0) {
                stuck_since_ms = now_ms;
            } else if (!(past_warmup && shaft_frozen)) {
                stuck_since_ms = 0;
            }
            if (stuck_since_ms != 0 && (now_ms - stuck_since_ms) > STALL_TIMEOUT_MS) {
                portENTER_CRITICAL(&s_target_mux);
                s_target_angle = clamp_angle(user_angle);
                portEXIT_CRITICAL(&s_target_mux);
                commanded_vel = 0.0f;
                s_motor.PID_velocity.reset();    // drop the wound-up integrator so torque stops now, not at idle-disable
                s_sync_allow_on = false;         // a stall must never turn the lamp on
                s_sync_pending  = true;
                digitalWrite(PIN_NSP, LOW);      // ≥20 µs low resets all latched faults
                delayMicroseconds(50);
                digitalWrite(PIN_NSP, HIGH);
                delayMicroseconds(1100);         // DRV8313 tWAKE ≈ 1 ms
                ESP_LOGW(TAG, "stall: accepting position %.3f as target", user_angle);
                motion_started_ms = 0;
                stuck_since_ms    = 0;
            }
        } else {
            motion_started_ms = 0;
            stuck_since_ms    = 0;
        }

        // Idle-disable when commanded, shaft, and pos_err all rest below their eps; entry
        // fires the settle callback so Matter learns knob-driven moves.
        // While parked, only a target moved away from the parked position wakes the
        // motor — physical back-drive (off-stop wedge unwinding, hand moves) must not
        // re-engage torque, or the lamp re-wedges into the stop in an endless cycle.
        bool at_rest;
        if (s_motor.enabled) {
            at_rest = fabsf(commanded_vel)          < VEL_AT_REST_EPS &&
                      fabsf(s_motor.shaft_velocity) < VEL_AT_REST_EPS &&
                      fabsf(pos_err)                < POS_AT_REST_EPS;
        } else {
            at_rest = fabsf(s_target_angle - parked_angle) < POS_AT_REST_EPS;
        }
        if (!at_rest) last_active_ms = now_ms;
        bool should_enable = (now_ms - last_active_ms) < IDLE_DISABLE_MS;

        if (should_enable && !s_motor.enabled) {
            digitalWrite(PIN_NSP, HIGH);     // wake from idle sleep
            delayMicroseconds(1100);         // DRV8313 tWAKE ≈ 1 ms before PWM resumes
            s_motor.enable();                // enable() also resets the PIDs
            s_idle = false;                  // moving — OTA must not reboot now
        } else if (!should_enable && s_motor.enabled) {
            s_motor.disable();
            // Sleep the driver: outputs Hi-Z (zero idle drag), ~0.5 mA vs 1-5 mA active, latched faults clear.
            digitalWrite(PIN_NSP, LOW);
            parked_angle  = user_angle;
            commanded_vel = 0.0f;
            // Persist resting position to NVS (debounced) so plug/unplug restores it.
            if (isnan(s_last_saved_position) || fabsf(user_angle - s_last_saved_position) > POS_SAVE_EPS_RAD) {
                save_position(user_angle);
            }
            if (s_sync_pending && s_settle_cb) {
                bool allow_on = s_sync_allow_on;
                s_sync_pending = false;   // clear first so a knob nudge during the callback re-arms it
                s_settle_cb(user_angle, allow_on);
            }
            s_idle = true;                   // parked + position saved — safe to reboot for OTA
        }

        // While disabled these only refresh sensor state — keeps user_angle live so
        // back-driven motion is visible to the wake check and the LED fade.
        s_motor.loopFOC();
        s_motor.move(commanded_vel);

        s_shaft_angle_cached    = user_angle;
    }
}

void motor_init_and_start() {
    xTaskCreatePinnedToCore(motor_foc_task, "motor_foc", 8192, nullptr,
                            MOTOR_TASK_PRIORITY, nullptr, CORE_MOTOR);
}

void motor_set_target_angle(float rad) {
    float v = clamp_angle(rad);
    portENTER_CRITICAL(&s_target_mux);
    s_target_angle = v;
    portEXIT_CRITICAL(&s_target_mux);
}
void motor_nudge_target_angle(float d_rad) {
    portENTER_CRITICAL(&s_target_mux);
    s_target_angle = clamp_angle(s_target_angle + d_rad);
    portEXIT_CRITICAL(&s_target_mux);
}
void motor_request_matter_sync_on_settle() { s_sync_allow_on = true; s_sync_pending = true; }
void motor_set_settle_callback(void (*cb)(float, bool)) { s_settle_cb = cb; }
void motor_request_homing() { s_home_request = true; }
float motor_get_shaft_angle()    { return s_shaft_angle_cached; }
bool  motor_is_idle()            { return s_idle; }
bool  motor_get_fault()          { return s_fault; }

void motor_set_motion_velocity(float rad_per_sec) {
    if (rad_per_sec <= 0.0f) { s_motion_velocity = MOTION_VELOCITY; return; }
    if (rad_per_sec >  VELOCITY_LIMIT) rad_per_sec = VELOCITY_LIMIT;
    s_motion_velocity = rad_per_sec;
}
