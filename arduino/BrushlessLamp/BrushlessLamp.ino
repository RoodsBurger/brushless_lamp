// Brushless Lamp — closed-loop FOC position control with rotary-encoder user input
// Seeed XIAO ESP32-C6 + SimpleFOC Mini (DRV8313) + 4015 BLDC gimbal + MT6701 ABZ encoder

#include <SimpleFOC.h>
#include <Preferences.h>
#include "driver/pulse_cnt.h"

constexpr uint8_t PIN_PWM_A = 18;    // D10 — DRV8313 IN1
constexpr uint8_t PIN_PWM_B = 20;    // D9  — DRV8313 IN2
constexpr uint8_t PIN_PWM_C = 19;    // D8  — DRV8313 IN3
constexpr uint8_t PIN_NSP   = 17;    // D7  — DRV8313 nSLEEP (pulse LOW to clear latched fault)

constexpr uint8_t PIN_ENC_A = 0;     // D0  — MT6701 A
constexpr uint8_t PIN_ENC_B = 1;     // D1  — MT6701 B

constexpr uint8_t PIN_ROT_A       = 2;     // D2  — rotary knob A
constexpr uint8_t PIN_ROT_B       = 21;    // D3  — rotary knob B
constexpr uint8_t PIN_BTN         = 22;    // D4  — rotary push button
constexpr uint8_t PIN_LED_ONBOARD = 15;    // XIAO user LED — factory-reset warning

constexpr uint32_t PWM_FREQ_HZ = 25000;

constexpr int   POLE_PAIRS     = 11;
constexpr int   ENCODER_CPR    = 1024;
constexpr float SUPPLY_VOLTAGE = 24.0f;

constexpr float PHASE_RESISTANCE     = 5.0f;
constexpr float KV_RATING            = 100.0f;
constexpr float CURRENT_LIMIT        = 0.5f;
constexpr float VOLTAGE_SENSOR_ALIGN = 3.0f;

constexpr float PID_VEL_P           = 0.2f;
constexpr float PID_VEL_I           = 20.0f;
constexpr float PID_VEL_D           = 0.0f;
constexpr float PID_VEL_OUTPUT_RAMP = 1000.0f;
constexpr float LPF_VEL_TF          = 0.02f;
constexpr float PID_ANGLE_P         = 3.0f;
constexpr float ACCEL_DEFAULT       = 10.0f;   // rad/s² — trapezoidal accel/decel cap; tune via 'A'
constexpr int   MOTION_DOWNSAMPLE   = 5;

constexpr uint32_t PRINT_INTERVAL_MS = 1000;

constexpr uint32_t IDLE_DISABLE_MS = 300;
constexpr float    IDLE_POS_THRESH = 0.2f;     // rad — "arrived" deadband; wider than the position loop's steady-state error

constexpr float    STALL_VEL_THRESHOLD = 0.2f;
constexpr uint32_t STALL_TIMEOUT_MS    = 500;
constexpr uint32_t STALL_WARMUP_MS     = 800;

constexpr float POS_MIN       = 0.0f;          // rad — soft lower limit
constexpr float POS_MAX       = 1000.0f;       // rad — soft upper limit (testing placeholder; tighten for final mechanics)
constexpr float RAD_PER_CLICK = 1.0f;          // rad per encoder detent

constexpr uint32_t BTN_DEBOUNCE_MS   = 30;
constexpr uint32_t BTN_DBL_GAP_MS    = 400;
constexpr uint32_t BTN_LONG_WARN_MS  = 5000;
constexpr uint32_t BTN_LONG_RESET_MS = 9000;

constexpr float   VELOCITY_PRESETS[] = {10.0f, 15.0f, 20.0f, 25.0f};
constexpr uint8_t VELOCITY_PRESET_N  = sizeof(VELOCITY_PRESETS) / sizeof(VELOCITY_PRESETS[0]);
constexpr uint8_t DEFAULT_SPEED_IDX  = 1;

constexpr uint32_t PERSIST_DEBOUNCE_MS = 2000;    // wait this long after the last change before writing NVS

BLDCMotor       motor   = BLDCMotor(POLE_PAIRS);
BLDCDriver3PWM  driver  = BLDCDriver3PWM(PIN_PWM_A, PIN_PWM_B, PIN_PWM_C);
Encoder         encoder = Encoder(PIN_ENC_A, PIN_ENC_B, ENCODER_CPR);
Commander       command = Commander(Serial);
Preferences     prefs;

pcnt_unit_handle_t pcnt_unit = nullptr;

void IRAM_ATTR doA() { encoder.handleA(); }
void IRAM_ATTR doB() { encoder.handleB(); }

float   commanded      = 0.0f;             // user-commanded position, rad (set by knob / G)
float   target         = 0.0f;             // trapezoidal-profile position fed to motor.move(), rad
float   target_vel     = 0.0f;             // current velocity of target, rad/s (internal state)
float   accel          = ACCEL_DEFAULT;    // rad/s² — ramp rate for target_vel
uint8_t speed_idx      = DEFAULT_SPEED_IDX;

bool     dirty_pos      = false;
bool     dirty_spd      = false;
uint32_t last_change_ms = 0;

int      pcnt_prev       = 0;

enum ClickStage { CLK_IDLE, CLK_WAIT_DBL };
ClickStage click_stage     = CLK_IDLE;
bool       btn_raw         = false;
bool       btn_debounced   = false;
uint32_t   btn_last_edge   = 0;
uint32_t   press_start_ms  = 0;
uint32_t   first_release_ms = 0;
bool       long_warn_active = false;
bool       long_reset_done  = false;

// Fixed-point printf args — C6 has no FPU and %.2f costs ~300 µs per arg.
#define FX_HI(v, scale) ((long)((int32_t)((v) * (scale)) / (scale)))
#define FX_LO(v, scale) ((long)((((int32_t)((v) * (scale))) < 0 ? -((int32_t)((v) * (scale))) : ((int32_t)((v) * (scale)))) % (scale)))

// Zero PID + filtered velocity so the re-enabled driver doesn't fire a voltage spike.
void flushControlState() {
    motor.PID_velocity.reset();
    motor.P_angle.reset();
    motor.shaft_velocity = 0.0f;
}

constexpr uint32_t NSP_LOW_US   = 50;          // nSLEEP low pulse width
constexpr uint32_t NSP_WAKE_US  = 2050;         // total delay before re-enabling (pulse + ≥1 ms wake)

bool     reset_pending   = false;
uint32_t reset_start_us  = 0;

// Begin a non-blocking nSLEEP reset; resetDriverStep() completes it after the wake delay.
void resetDriverBegin() {
    motor.disable();
    flushControlState();
    digitalWrite(PIN_NSP, LOW);
    reset_start_us = micros();
    reset_pending  = true;
}

// Poll each loop iteration; completes a pending nSLEEP reset once the wake delay has elapsed.
void resetDriverStep() {
    if (!reset_pending) return;
    uint32_t elapsed = micros() - reset_start_us;
    if (elapsed < NSP_LOW_US) return;
    if (digitalRead(PIN_NSP) == LOW) digitalWrite(PIN_NSP, HIGH);
    if (elapsed >= NSP_WAKE_US) {
        motor.enable();
        reset_pending = false;
    }
}

// Emergency stop: disable driver and zero control state without moving target.
void haltMotor() {
    motor.disable();
    flushControlState();
}

// Mark the target-position NVS entry for a debounced write.
void markPosDirty() { dirty_pos = true; last_change_ms = millis(); }

// Write pending target / speed entries to NVS when settled.
void maybePersist() {
    if (dirty_pos && millis() - last_change_ms > PERSIST_DEBOUNCE_MS) {
        prefs.putFloat("pos", commanded);
        dirty_pos = false;
    }
    if (dirty_spd) {
        prefs.putUChar("spd", speed_idx);
        dirty_spd = false;
    }
}

void doCurrent (char* arg) { command.scalar(&motor.current_limit, arg); }
void doP       (char* arg) { command.scalar(&motor.PID_velocity.P, arg); }
void doI       (char* arg) { command.scalar(&motor.PID_velocity.I, arg); }
void doTf      (char* arg) { command.scalar(&motor.LPF_velocity.Tf, arg); }
void doR       (char* arg) { command.scalar(&motor.phase_resistance, arg); }
void doK       (char* arg) { command.scalar(&motor.KV_rating, arg); }
void doM       (char* arg) { float v; command.scalar(&v, arg); motor.motion_downsample = (unsigned int)v; }
void doAccel   (char* arg) { command.scalar(&accel, arg); }
void doReset   (char* arg) { (void)arg; resetDriverBegin(); Serial.println("driver reset"); }
void doVelLim  (char* arg) { command.scalar(&motor.velocity_limit, arg); }

// Set the commanded position; rampTarget drives `target` toward it trapezoidally.
void doGoto(char* arg) {
    float v; command.scalar(&v, arg);
    float clamped = constrain(v, POS_MIN, POS_MAX);
    if (clamped != commanded) {
        commanded = clamped;
        markPosDirty();
    }
}

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== BrushlessLamp: closed-loop FOC position ===");

    pinMode(PIN_NSP, OUTPUT);
    digitalWrite(PIN_NSP, HIGH);
    pinMode(PIN_BTN, INPUT_PULLUP);
    pinMode(PIN_LED_ONBOARD, OUTPUT);
    digitalWrite(PIN_LED_ONBOARD, LOW);
    delay(2);

    encoder.quadrature = Quadrature::ON;
    encoder.pullup     = Pullup::USE_INTERN;
    encoder.init();
    encoder.enableInterrupts(doA, doB);
    motor.linkSensor(&encoder);

    driver.voltage_power_supply = SUPPLY_VOLTAGE;
    driver.voltage_limit        = SUPPLY_VOLTAGE;
    driver.pwm_frequency        = PWM_FREQ_HZ;
    Serial.printf("driver.init()=%d\n", driver.init());
    motor.linkDriver(&driver);

    motor.phase_resistance     = PHASE_RESISTANCE;
    motor.KV_rating            = KV_RATING;
    motor.current_limit        = CURRENT_LIMIT;
    motor.voltage_sensor_align = VOLTAGE_SENSOR_ALIGN;
    motor.controller           = MotionControlType::angle;
    motor.torque_controller    = TorqueControlType::voltage;
    motor.foc_modulation       = FOCModulationType::SpaceVectorPWM;
    motor.motion_downsample    = MOTION_DOWNSAMPLE;

    motor.PID_velocity.P           = PID_VEL_P;
    motor.PID_velocity.I           = PID_VEL_I;
    motor.PID_velocity.D           = PID_VEL_D;
    motor.PID_velocity.output_ramp = PID_VEL_OUTPUT_RAMP;
    motor.LPF_velocity.Tf          = LPF_VEL_TF;
    motor.P_angle.P                = PID_ANGLE_P;

    motor.useMonitoring(Serial);
    Serial.printf("motor.init()=%d\n", motor.init());

    int foc_ok = motor.initFOC();
    Serial.printf("motor.initFOC()=%d  zero_elec=%.4f  dir=%d\n",
                  foc_ok, motor.zero_electric_angle, (int)motor.sensor_direction);
    if (!foc_ok) Serial.println("FOC init failed");

    prefs.begin("blamp", false);
    commanded = constrain(prefs.getFloat("pos", POS_MIN), POS_MIN, POS_MAX);
    target    = commanded;
    speed_idx = prefs.getUChar("spd", DEFAULT_SPEED_IDX);
    if (speed_idx >= VELOCITY_PRESET_N) speed_idx = DEFAULT_SPEED_IDX;
    motor.velocity_limit = VELOCITY_PRESETS[speed_idx];

    // Anchor shaft_angle to the persisted commanded so the motor doesn't seek on boot.
    motor.sensor_offset -= commanded;
    motor.loopFOC();
    Serial.printf("loaded: commanded=%.2f rad  speed_idx=%u (%.1f rad/s)\n",
                  commanded, speed_idx, motor.velocity_limit);

    pinMode(PIN_ROT_A, INPUT_PULLUP);
    pinMode(PIN_ROT_B, INPUT_PULLUP);

    pcnt_unit_config_t unit_cfg = { .low_limit = -32768, .high_limit = 32767 };
    pcnt_new_unit(&unit_cfg, &pcnt_unit);
    pcnt_glitch_filter_config_t filter_cfg = { .max_glitch_ns = 10000 };
    pcnt_unit_set_glitch_filter(pcnt_unit, &filter_cfg);

    pcnt_chan_config_t chan_a_cfg = { .edge_gpio_num = PIN_ROT_A, .level_gpio_num = PIN_ROT_B };
    pcnt_channel_handle_t chan_a;
    pcnt_new_channel(pcnt_unit, &chan_a_cfg, &chan_a);
    pcnt_channel_set_edge_action(chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE);
    pcnt_channel_set_level_action(chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);

    pcnt_chan_config_t chan_b_cfg = { .edge_gpio_num = PIN_ROT_B, .level_gpio_num = PIN_ROT_A };
    pcnt_channel_handle_t chan_b;
    pcnt_new_channel(pcnt_unit, &chan_b_cfg, &chan_b);
    pcnt_channel_set_edge_action(chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE);
    pcnt_channel_set_level_action(chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);

    pcnt_unit_enable(pcnt_unit);
    pcnt_unit_clear_count(pcnt_unit);
    pcnt_unit_start(pcnt_unit);
    pcnt_prev = 0;

    command.add('G', doGoto,    "goto position rad");
    command.add('T', doVelLim,  "velocity_limit rad/s (overrides preset)");
    command.add('C', doCurrent, "current limit");
    command.add('P', doP,       "PID_velocity P");
    command.add('I', doI,       "PID_velocity I");
    command.add('F', doTf,      "LPF_velocity Tf");
    command.add('R', doR,       "phase R");
    command.add('K', doK,       "KV rating");
    command.add('M', doM,       "motion downsample");
    command.add('A', doAccel,   "trapezoidal accel rad/s²");
    command.add('X', doReset,   "reset driver (clear latched fault)");
    Serial.println("Ready.");
}

// Translate PCNT count deltas into commanded-position adjustments in whole-detent steps.
void readRotary() {
    int now = 0;
    pcnt_unit_get_count(pcnt_unit, &now);
    int delta = now - pcnt_prev;
    if (delta == 0) return;
    int detents = delta / 4;
    if (detents == 0) return;
    pcnt_prev += detents * 4;
    float clamped = constrain(commanded + detents * RAD_PER_CLICK, POS_MIN, POS_MAX);
    if (clamped != commanded) {
        commanded = clamped;
        markPosDirty();
    }
}

// Drive target toward commanded with a trapezoidal velocity profile (accel → cruise → decel).
void rampTarget() {
    static uint32_t last_us = 0;
    uint32_t now = micros();
    if (last_us == 0) { last_us = now; return; }
    float dt = (now - last_us) * 1e-6f;
    last_us = now;
    if (dt <= 0 || dt > 0.1f) return;

    float err = commanded - target;
    float decel_dist = (target_vel * target_vel) / (2.0f * accel);
    float desired_vel = (fabs(err) <= decel_dist) ? 0.0f
                                                  : (err > 0 ? 1.0f : -1.0f) * motor.velocity_limit;

    float vel_step = accel * dt;
    if      (target_vel < desired_vel - vel_step) target_vel += vel_step;
    else if (target_vel > desired_vel + vel_step) target_vel -= vel_step;
    else                                           target_vel  = desired_vel;

    target += target_vel * dt;

    if ((err > 0 && target >= commanded) || (err < 0 && target <= commanded)) {
        target = commanded;
        target_vel = 0.0f;
    }
}

// Advance to the next speed preset and reflect the new velocity_limit.
void advanceSpeedPreset() {
    speed_idx = (speed_idx + 1) % VELOCITY_PRESET_N;
    motor.velocity_limit = VELOCITY_PRESETS[speed_idx];
    dirty_spd = true;
    Serial.printf("speed preset %u = %ld.%ld rad/s\n",
                  speed_idx,
                  FX_HI(motor.velocity_limit, 10),
                  FX_LO(motor.velocity_limit, 10));
}

// Placeholder single-click handler; becomes the brightness-mode toggle in M3b.
void onShortClick() {
    Serial.println("mode: brightness not implemented (M3b)");
}

// Blink the onboard LED N times as a visual confirmation.
void confirmBlink(uint8_t n) {
    for (uint8_t i = 0; i < n; i++) {
        digitalWrite(PIN_LED_ONBOARD, HIGH); delay(100);
        digitalWrite(PIN_LED_ONBOARD, LOW);  delay(100);
    }
}

// Wipe NVS, write defaults, confirm-blink, and reboot.
void factoryReset() {
    Serial.println("factory reset: preferences cleared, rebooting");
    prefs.clear();
    prefs.putFloat("pos", POS_MIN);
    prefs.putUChar("spd", DEFAULT_SPEED_IDX);
    confirmBlink(5);
    ESP.restart();
}

// Debounce the button and drive the click / double-click / long-hold state machine.
void pollButton() {
    uint32_t now = millis();
    bool raw = (digitalRead(PIN_BTN) == LOW);
    if (raw != btn_raw) { btn_raw = raw; btn_last_edge = now; }
    if (now - btn_last_edge > BTN_DEBOUNCE_MS && raw != btn_debounced) {
        btn_debounced = raw;
        if (btn_debounced) {
            press_start_ms   = now;
            long_warn_active = false;
            long_reset_done  = false;
        } else {
            uint32_t held = now - press_start_ms;
            if (long_reset_done) {
                // reset already fired
            } else if (held < BTN_LONG_WARN_MS) {
                if (click_stage == CLK_WAIT_DBL && now - first_release_ms < BTN_DBL_GAP_MS) {
                    advanceSpeedPreset();
                    click_stage = CLK_IDLE;
                } else {
                    first_release_ms = now;
                    click_stage = CLK_WAIT_DBL;
                }
            } else {
                long_warn_active = false;
                digitalWrite(PIN_LED_ONBOARD, LOW);
                click_stage = CLK_IDLE;
            }
        }
    }
    if (click_stage == CLK_WAIT_DBL && !btn_debounced && now - first_release_ms > BTN_DBL_GAP_MS) {
        onShortClick();
        click_stage = CLK_IDLE;
    }
    if (btn_debounced && !long_reset_done) {
        uint32_t held = now - press_start_ms;
        if (held >= BTN_LONG_RESET_MS) {
            long_reset_done = true;
            factoryReset();
        } else if (held >= BTN_LONG_WARN_MS && !long_warn_active) {
            long_warn_active = true;
        }
    }
}

// Blink the onboard LED at 5 Hz while the long-hold warning is active.
void updateWarnLED() {
    if (!long_warn_active) return;
    digitalWrite(PIN_LED_ONBOARD, ((millis() / 100) & 1) ? HIGH : LOW);
}

// Never returns — see README for loopTask preemption context.
void loop() {
    static uint32_t prev_loop_us = 0;
    static uint32_t max_loop_us  = 0;
    static uint32_t last_active_ms = 0;
    static uint32_t stall_start_ms = 0;
    static uint32_t enable_time_ms = 0;
    static uint32_t last_print_ms  = 0;

    while (true) {
        uint32_t now_us = micros();
        if (prev_loop_us != 0) {
            uint32_t dt = now_us - prev_loop_us;
            if (dt > max_loop_us) max_loop_us = dt;
        }
        prev_loop_us = now_us;

        // Sync commanded to the physical shaft position whenever the motor is idle, so user input is always relative to reality.
        if (!motor.enabled && !reset_pending) {
            commanded  = motor.shaft_angle;
            target     = commanded;
            target_vel = 0.0f;
        }

        command.run();
        pollButton();
        readRotary();
        rampTarget();
        resetDriverStep();

        bool ramping     = fabs(commanded - target) > 1e-4f || fabs(target_vel) > 1e-4f;
        bool pos_err_big = motor.enabled && fabs(target - motor.shaft_angle) > IDLE_POS_THRESH;
        bool want_motion = ramping || pos_err_big;

        if (want_motion) {
            last_active_ms = millis();
            if (!motor.enabled && !reset_pending) {
                // Begin nSLEEP reset; completes asynchronously via resetDriverStep().
                resetDriverBegin();
                enable_time_ms = millis();
                stall_start_ms = 0;
            }
            // Skip stall detection during the accelerate-from-stop warmup.
            bool in_warmup = (millis() - enable_time_ms) < STALL_WARMUP_MS;
            if (!in_warmup && fabs(motor.shaft_velocity) < STALL_VEL_THRESHOLD && pos_err_big) {
                if (stall_start_ms == 0) stall_start_ms = millis();
                else if (millis() - stall_start_ms > STALL_TIMEOUT_MS) {
                    Serial.printf("stall at ang=%ld.%ld seeking G=%ld.%02ld — halted.\n",
                                  FX_HI(motor.shaft_angle, 10), FX_LO(motor.shaft_angle, 10),
                                  FX_HI(commanded, 100), FX_LO(commanded, 100));
                    haltMotor();
                    stall_start_ms = 0;
                }
            } else {
                stall_start_ms = 0;
            }
        } else {
            stall_start_ms = 0;
            if (motor.enabled && millis() - last_active_ms > IDLE_DISABLE_MS) {
                motor.disable();
            }
        }

        motor.loopFOC();
        motor.move(target);

        updateWarnLED();
        maybePersist();

        if (millis() - last_print_ms >= PRINT_INTERVAL_MS) {
            last_print_ms = millis();
            // Skip the write if the TX ring can't take the whole line without blocking.
            if (Serial.availableForWrite() >= 144) {
                Serial.printf("cmd:%ld.%02ld tgt:%ld.%02ld ang:%ld.%ld vel:%ld.%02ld tvel:%ld.%02ld vlim:%ld.%ld spd:%u en:%d maxLp:%luus\n",
                              FX_HI(commanded, 100),            FX_LO(commanded, 100),
                              FX_HI(target, 100),               FX_LO(target, 100),
                              FX_HI(motor.shaft_angle, 10),     FX_LO(motor.shaft_angle, 10),
                              FX_HI(motor.shaft_velocity, 100), FX_LO(motor.shaft_velocity, 100),
                              FX_HI(target_vel, 100),           FX_LO(target_vel, 100),
                              FX_HI(motor.velocity_limit, 10),  FX_LO(motor.velocity_limit, 10),
                              speed_idx,
                              (int)motor.enabled,
                              (unsigned long)max_loop_us);
            }
            max_loop_us = 0;
        }
    }
}
