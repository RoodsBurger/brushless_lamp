// Brushless Lamp — closed-loop FOC position control with rotary-encoder user input
// Seeed XIAO ESP32-C6 + SimpleFOC Mini (DRV8313) + 4015 BLDC gimbal + MT6701 ABZ encoder

#include <SimpleFOC.h>
#include <Preferences.h>
#include <Matter.h>
#include "driver/pulse_cnt.h"
#include "esp_bt.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"

constexpr uint8_t PIN_PWM_A = 18;    // D10 — DRV8313 IN1
constexpr uint8_t PIN_PWM_B = 20;    // D9  — DRV8313 IN2
constexpr uint8_t PIN_PWM_C = 19;    // D8  — DRV8313 IN3
constexpr uint8_t PIN_NSP   = 17;    // D7  — DRV8313 nSLEEP (pulse LOW to clear latched fault)

constexpr uint8_t PIN_ENC_A = 0;     // D0  — MT6701 A
constexpr uint8_t PIN_ENC_B = 1;     // D1  — MT6701 B

constexpr uint8_t PIN_ROT_A = 2;     // D2 — rotary knob A
constexpr uint8_t PIN_ROT_B = 21;    // D3 — rotary knob B
constexpr uint8_t PIN_BTN   = 22;    // D4 — rotary push button

constexpr uint8_t PIN_LED_WW = 23;   // D5 — AO3400A gate for warm-white channel
constexpr uint8_t PIN_LED_CW = 16;   // D6 — AO3400A gate for cool-white channel

constexpr uint32_t PWM_FREQ_HZ     = 25000;
constexpr uint32_t FOC_ISR_HZ      = 1000;     // hw-timer rate for motor.loopFOC(); 1 kHz gives enough SVPWM updates while leaving plenty of CPU for the Matter/WiFi init storm
constexpr uint32_t WDT_TIMEOUT_MS  = 10000;    // main-loop watchdog — reboots the chip if loop() hangs
constexpr uint32_t CRASH_CLEAR_MS  = 15000;    // clear the consecutive-crash counter once we've been alive this long
constexpr uint8_t  CRASH_WIPE_LIMIT = 3;       // after this many consecutive panic reboots, wipe NVS and start fresh
constexpr uint32_t LED_PWM_FREQ_HZ  = 25000;
constexpr uint8_t  LED_PWM_RES_BIT  = 10;
constexpr uint16_t LED_PWM_MAX      = (1u << LED_PWM_RES_BIT) - 1;
constexpr float    LEDS_OFF_THRESH  = 0.5f;  // rad — shaft_angle at or below this means lamp fully off
constexpr float    LEDS_FULL_THRESH = 3.0f;  // rad — shaft_angle at or above this means lamp at configured brightness
constexpr float    LED_RAMP_PER_SEC = 60.0f; // bri/cct units (0-100) per second — full swing takes ~1.7 s

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

constexpr uint32_t BLE_RELEASE_DELAY_MS = 3000;   // free the BLE controller N ms after commissioning completes
constexpr uint16_t MATTER_CT_WARM       = 500;    // mireds — Matter's "warm" end of the range
constexpr uint16_t MATTER_CT_COOL       = 100;    // mireds — Matter's "cool" end

BLDCMotor       motor   = BLDCMotor(POLE_PAIRS);
BLDCDriver3PWM  driver  = BLDCDriver3PWM(PIN_PWM_A, PIN_PWM_B, PIN_PWM_C);
Encoder         encoder = Encoder(PIN_ENC_A, PIN_ENC_B, ENCODER_CPR);
Commander       foc_cmd = Commander(Serial);
Preferences     prefs;

pcnt_unit_handle_t pcnt_unit = nullptr;
hw_timer_t*        foc_timer = nullptr;

MatterColorTemperatureLight matter_light;
volatile bool matter_echo = false;                // set while we push local state to Matter — prevents onMatterChange feedback
bool          matter_qr_printed = false;
bool          ble_release_pending = false;
bool          ble_released = false;
uint32_t      ble_release_start_ms = 0;

void IRAM_ATTR doA() { encoder.handleA(); }
void IRAM_ATTR doB() { encoder.handleB(); }

// Runs at FOC_ISR_HZ — keeps SVPWM locked to the rotor regardless of what the scheduler is doing.
void IRAM_ATTR onFocTick() { motor.loopFOC(); }

float   commanded      = 0.0f;             // user-commanded position, rad (set by knob / G)
float   target         = 0.0f;             // trapezoidal-profile position fed to motor.move(), rad
float   target_vel     = 0.0f;             // current velocity of target, rad/s (internal state)
float   accel          = ACCEL_DEFAULT;    // rad/s² — ramp rate for target_vel
uint8_t speed_idx      = DEFAULT_SPEED_IDX;
uint8_t brightness     = 50;               // 0..100, logarithmic via gamma LUT
uint8_t cct            = 50;               // 0..100, 0 = all WW, 100 = all CW

enum KnobMode { MODE_POSITION, MODE_BRIGHTNESS };
KnobMode knob_mode = MODE_POSITION;

float    effective_bri = 50.0f;            // bri with LED_RAMP_PER_SEC smoothing applied
float    effective_cct = 50.0f;            // cct with LED_RAMP_PER_SEC smoothing applied
uint32_t led_ramp_last_ms = 0;
uint16_t last_duty_ww = 0xFFFF;            // cached last-written LEDC values; 0xFFFF forces first update
uint16_t last_duty_cw = 0xFFFF;

bool     dirty_pos      = false;
bool     dirty_spd      = false;
bool     dirty_bri      = false;
bool     dirty_cct      = false;
uint32_t last_change_ms = 0;

int      pcnt_prev       = 0;

enum ClickStage { CLK_IDLE, CLK_WAIT_DBL };
ClickStage click_stage     = CLK_IDLE;
bool       btn_raw         = false;
bool       btn_debounced   = false;
uint32_t   btn_last_edge   = 0;
uint32_t   press_start_ms  = 0;
uint32_t   first_release_ms = 0;
bool       warn_fired      = false;
bool       long_reset_done = false;

// Fixed-point printf args — C6 has no FPU and %.2f costs ~300 µs per arg.
#define FX_HI(v, scale) ((long)((int32_t)((v) * (scale)) / (scale)))
#define FX_LO(v, scale) ((long)((((int32_t)((v) * (scale))) < 0 ? -((int32_t)((v) * (scale))) : ((int32_t)((v) * (scale)))) % (scale)))

// Ramp effective_bri / effective_cct toward the user/Matter-set values at LED_RAMP_PER_SEC so step changes fade.
void updateLedRamp() {
    uint32_t now = millis();
    if (led_ramp_last_ms == 0) { led_ramp_last_ms = now; return; }
    uint32_t dt_ms = now - led_ramp_last_ms;
    if (dt_ms < 5) return;
    led_ramp_last_ms = now;
    float step = LED_RAMP_PER_SEC * (float)dt_ms / 1000.0f;
    float tb = (float)brightness;
    if      (effective_bri < tb - step) effective_bri += step;
    else if (effective_bri > tb + step) effective_bri -= step;
    else                                 effective_bri  = tb;
    float tc = (float)cct;
    if      (effective_cct < tc - step) effective_cct += step;
    else if (effective_cct > tc + step) effective_cct -= step;
    else                                 effective_cct  = tc;
}

// Write the current effective brightness / cct to the two PWM channels; intensity ramps linearly with shaft_angle from LEDS_OFF_THRESH to LEDS_FULL_THRESH.
void updateLEDs() {
    updateLedRamp();

    float a = motor.shaft_angle;
    float fade;
    if      (a <= LEDS_OFF_THRESH)  fade = 0.0f;
    else if (a >= LEDS_FULL_THRESH) fade = 1.0f;
    else                             fade = (a - LEDS_OFF_THRESH) / (LEDS_FULL_THRESH - LEDS_OFF_THRESH);

    uint32_t base = (uint32_t)((effective_bri * LED_PWM_MAX / 100.0f) * fade);
    uint32_t ecct = (uint32_t)(effective_cct + 0.5f);
    uint16_t want_ww = (uint16_t)((base * (100 - ecct)) / 100);
    uint16_t want_cw = (uint16_t)((base * ecct)        / 100);
    if (want_ww != last_duty_ww) { ledcWrite(PIN_LED_WW, want_ww); last_duty_ww = want_ww; }
    if (want_cw != last_duty_cw) { ledcWrite(PIN_LED_CW, want_cw); last_duty_cw = want_cw; }
}

// Blocking 5x fade-out/fade-in on both LED channels as the factory-reset warning (~480 ms per pulse).
void pulseLEDs(uint8_t n) {
    uint32_t base = (uint32_t)((uint32_t)brightness * LED_PWM_MAX / 100);
    uint16_t base_ww = (uint16_t)((base * (100 - cct)) / 100);
    uint16_t base_cw = (uint16_t)((base * cct)        / 100);
    for (uint8_t i = 0; i < n; i++) {
        for (int step = 20; step >= 0; step--) {
            ledcWrite(PIN_LED_WW, (uint16_t)((uint32_t)base_ww * step / 20));
            ledcWrite(PIN_LED_CW, (uint16_t)((uint32_t)base_cw * step / 20));
            delay(8);
        }
        delay(80);
        for (int step = 0; step <= 20; step++) {
            ledcWrite(PIN_LED_WW, (uint16_t)((uint32_t)base_ww * step / 20));
            ledcWrite(PIN_LED_CW, (uint16_t)((uint32_t)base_cw * step / 20));
            delay(8);
        }
        delay(80);
    }
    last_duty_ww = 0xFFFF;
    last_duty_cw = 0xFFFF;
    updateLEDs();
}

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

// Write pending entries to NVS once user input has settled.
void maybePersist() {
    bool settled = millis() - last_change_ms > PERSIST_DEBOUNCE_MS;
    if (dirty_pos && settled) { prefs.putFloat("pos", commanded);   dirty_pos = false; }
    if (dirty_bri && settled) { prefs.putUChar("bri", brightness);  dirty_bri = false; }
    if (dirty_cct && settled) { prefs.putUChar("cct", cct);         dirty_cct = false; }
    if (dirty_spd)            { prefs.putUChar("spd", speed_idx);   dirty_spd = false; }
}

void doCurrent (char* arg) { foc_cmd.scalar(&motor.current_limit, arg); }
void doP       (char* arg) { foc_cmd.scalar(&motor.PID_velocity.P, arg); }
void doI       (char* arg) { foc_cmd.scalar(&motor.PID_velocity.I, arg); }
void doTf      (char* arg) { foc_cmd.scalar(&motor.LPF_velocity.Tf, arg); }
void doR       (char* arg) { foc_cmd.scalar(&motor.phase_resistance, arg); }
void doK       (char* arg) { foc_cmd.scalar(&motor.KV_rating, arg); }
void doM       (char* arg) { float v; foc_cmd.scalar(&v, arg); motor.motion_downsample = (unsigned int)v; }
void doAccel   (char* arg) { foc_cmd.scalar(&accel, arg); if (accel < 0.1f) accel = 0.1f; }
void doReset   (char* arg) { (void)arg; resetDriverBegin(); Serial.println("driver reset"); }
void doVelLim  (char* arg) { foc_cmd.scalar(&motor.velocity_limit, arg); }

// Set the commanded position; rampTarget drives `target` toward it trapezoidally.
void doGoto(char* arg) {
    float v; foc_cmd.scalar(&v, arg);
    float clamped = constrain(v, POS_MIN, POS_MAX);
    if (clamped != commanded) {
        commanded = clamped;
        markPosDirty();
    }
}

// Set LED brightness 0..100; scales both channels via gamma LUT.
void doBrightness(char* arg) {
    float v; foc_cmd.scalar(&v, arg);
    int clamped = constrain((int)v, 0, 100);
    if (clamped != (int)brightness) {
        brightness = (uint8_t)clamped;
        dirty_bri = true;
        last_change_ms = millis();
    }
}

// Set CCT mix 0..100; 0 = all warm white, 100 = all cool white.
void doCct(char* arg) {
    float v; foc_cmd.scalar(&v, arg);
    int clamped = constrain((int)v, 0, 100);
    if (clamped != (int)cct) {
        cct = (uint8_t)clamped;
        dirty_cct = true;
        last_change_ms = millis();
    }
}

// Map a Matter CurrentLevel 1..254 into a commanded motor position in radians.
float matterBrightnessToCmd(uint8_t level) {
    return ((float)level * POS_MAX) / 254.0f;
}

// Map the current commanded position to a Matter CurrentLevel 0..254.
uint8_t cmdToMatterBrightness(float cmd) {
    long v = (long)(cmd * 254.0f / POS_MAX + 0.5f);
    if (v < 0)   v = 0;
    if (v > 254) v = 254;
    return (uint8_t)v;
}

// Map Matter ColorTemperatureMireds (100..500) to our cct (0..100, 0=WW,100=CW).
uint8_t miredsToCct(uint16_t mireds) {
    long m = constrain((long)mireds, (long)MATTER_CT_COOL, (long)MATTER_CT_WARM);
    return (uint8_t)((MATTER_CT_WARM - m) * 100L / (MATTER_CT_WARM - MATTER_CT_COOL));
}

// Inverse of miredsToCct.
uint16_t cctToMireds(uint8_t cct_val) {
    long c = constrain((long)cct_val, 0L, 100L);
    return (uint16_t)(MATTER_CT_WARM - c * (MATTER_CT_WARM - MATTER_CT_COOL) / 100L);
}

// Matter pushed a new state — translate into local commanded/cct.
bool onMatterChange(bool state, uint8_t level, uint16_t mireds) {
    Serial.printf("matter->dev: state=%d level=%u mireds=%u echo=%d\n",
                  state ? 1 : 0, level, mireds, matter_echo ? 1 : 0);
    if (matter_echo) return true;
    float new_cmd = state ? constrain(matterBrightnessToCmd(level), POS_MIN, POS_MAX) : POS_MIN;
    if (fabs(new_cmd - commanded) > 1e-3f) {
        commanded = new_cmd;
        markPosDirty();
    }
    uint8_t new_cct = miredsToCct(mireds);
    if (new_cct != cct) {
        cct = new_cct;
        dirty_cct = true;
        last_change_ms = millis();
    }
    return true;
}

// Push local state back to Matter when it drifts meaningfully from the last-sent snapshot.
void syncMatterIfDirty() {
    if (!Matter.isDeviceCommissioned()) return;
    if (!Matter.isWiFiConnected()) return;   // don't poke the library while it's trying to recover the radio
    static uint32_t last_sync_ms = 0;
    static uint8_t  last_sent_bri = 255;
    static uint8_t  last_sent_cct = 255;
    static bool     last_sent_on  = false;
    uint32_t now = millis();
    if (now - last_sync_ms < 200) return;                        // rate-limit outbound chatter
    bool on_state = commanded > LEDS_OFF_THRESH;
    uint8_t bri = cmdToMatterBrightness(commanded);
    if (on_state && bri == 0) bri = 1;
    if (on_state == last_sent_on && bri == last_sent_bri && cct == last_sent_cct) return;
    matter_echo = true;
    matter_light.setOnOff(on_state);
    if (on_state) matter_light.setBrightness(bri);
    matter_light.setColorTemperature(cctToMireds(cct));
    matter_light.updateAccessory();
    matter_echo = false;
    last_sent_on = on_state;
    last_sent_bri = bri;
    last_sent_cct = cct;
    last_sync_ms = now;
}

// Print the QR-code URL and manual pairing code exactly once if the node isn't commissioned yet.
void printMatterPairing() {
    if (matter_qr_printed || Matter.isDeviceCommissioned()) return;
    Serial.println("=== Matter: uncommissioned ===");
    Serial.printf("QR code: %s\n", Matter.getOnboardingQRCodeUrl().c_str());
    Serial.printf("Manual pairing code: %s\n", Matter.getManualPairingCode().c_str());
    matter_qr_printed = true;
}

// Keep BLE resident — the arduino-esp32 Matter library uses it in the WiFi-reconnect recovery path and releasing it causes a use-after-free on Core 0 when WiFi flaps.
void handleBLERelease() {
    (void)ble_released; (void)ble_release_pending; (void)ble_release_start_ms;
}

// Wipe every NVS namespace so the next boot starts like a freshly-flashed device.
static void wipeAllNvsAndReboot(const char* reason) {
    Serial.printf("Auto-recovery: %s — wiping NVS and rebooting fresh\n", reason);
    Serial.flush();
    nvs_flash_erase();
    delay(500);
    ESP.restart();
}

// Consecutive panic-reboot counter. Reset once we've survived long enough (see CRASH_CLEAR_MS in loop()).
static Preferences boot_prefs;
static bool        crashes_cleared = false;

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== BrushlessLamp: closed-loop FOC position ===");

    esp_reset_reason_t reason = esp_reset_reason();
    boot_prefs.begin("boot", false);
    uint8_t crashes = boot_prefs.getUChar("crashes", 0);
    bool last_was_crash = (reason == ESP_RST_PANIC || reason == ESP_RST_TASK_WDT || reason == ESP_RST_INT_WDT);
    if (last_was_crash) crashes++; else crashes = 0;
    Serial.printf("boot: reset_reason=%d crashes=%u\n", (int)reason, crashes);
    if (crashes >= CRASH_WIPE_LIMIT) {
        boot_prefs.putUChar("crashes", 0);
        boot_prefs.end();
        wipeAllNvsAndReboot("consecutive crashes hit limit");
    }
    boot_prefs.putUChar("crashes", crashes);

    pinMode(PIN_NSP, OUTPUT);
    digitalWrite(PIN_NSP, HIGH);
    pinMode(PIN_BTN, INPUT_PULLUP);
    delay(2);

    ledcAttach(PIN_LED_WW, LED_PWM_FREQ_HZ, LED_PWM_RES_BIT);
    ledcAttach(PIN_LED_CW, LED_PWM_FREQ_HZ, LED_PWM_RES_BIT);
    ledcWrite(PIN_LED_WW, 0);
    ledcWrite(PIN_LED_CW, 0);

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
    commanded  = constrain(prefs.getFloat("pos", POS_MIN), POS_MIN, POS_MAX);
    target     = commanded;
    speed_idx  = prefs.getUChar("spd", DEFAULT_SPEED_IDX);
    brightness = prefs.getUChar("bri", 50);
    cct        = prefs.getUChar("cct", 50);
    if (speed_idx >= VELOCITY_PRESET_N) speed_idx = DEFAULT_SPEED_IDX;
    if (brightness > 100) brightness = 50;
    if (cct > 100)        cct        = 50;
    effective_bri = (float)brightness;
    effective_cct = (float)cct;
    motor.velocity_limit = VELOCITY_PRESETS[speed_idx];

    // Anchor shaft_angle to the persisted commanded so the motor doesn't seek on boot.
    motor.sensor_offset -= commanded;
    motor.loopFOC();
    updateLEDs();
    Serial.printf("loaded: commanded=%.2f rad  speed_idx=%u (%.1f rad/s)  bri=%u  cct=%u\n",
                  commanded, speed_idx, motor.velocity_limit, brightness, cct);

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

    foc_cmd.add('G', doGoto,    "goto position rad");
    foc_cmd.add('T', doVelLim,  "velocity_limit rad/s (overrides preset)");
    foc_cmd.add('C', doCurrent, "current limit");
    foc_cmd.add('P', doP,       "PID_velocity P");
    foc_cmd.add('I', doI,       "PID_velocity I");
    foc_cmd.add('F', doTf,      "LPF_velocity Tf");
    foc_cmd.add('R', doR,       "phase R");
    foc_cmd.add('K', doK,       "KV rating");
    foc_cmd.add('M', doM,       "motion downsample");
    foc_cmd.add('A', doAccel,   "trapezoidal accel rad/s²");
    foc_cmd.add('B', doBrightness, "LED brightness 0..100");
    foc_cmd.add('W', doCct,        "CCT mix 0..100 (0=WW,100=CW)");
    foc_cmd.add('X', doReset,   "reset driver (clear latched fault)");

    matter_light.begin(commanded > LEDS_OFF_THRESH, cmdToMatterBrightness(commanded), cctToMireds(cct));
    matter_light.onChange(onMatterChange);
    Matter.begin();
    Serial.printf("free heap after Matter.begin(): %lu\n", (unsigned long)ESP.getFreeHeap());

    // Start the FOC hw-timer AFTER Matter init so the commissioning storm doesn't compete with SVPWM interrupts.
    foc_timer = timerBegin(1000000);
    timerAttachInterrupt(foc_timer, &onFocTick);
    timerAlarm(foc_timer, 1000000 / FOC_ISR_HZ, true, 0);

    // Register the main loop task with the FreeRTOS watchdog so a library hang triggers a clean panic-reboot.
    esp_task_wdt_config_t wdt_cfg = { .timeout_ms = WDT_TIMEOUT_MS, .idle_core_mask = 0, .trigger_panic = true };
    esp_task_wdt_reconfigure(&wdt_cfg);
    esp_task_wdt_add(NULL);

    Serial.println("Ready.");
}

// Translate PCNT count deltas into either position or brightness adjustments depending on knob mode.
void readRotary() {
    int now = 0;
    pcnt_unit_get_count(pcnt_unit, &now);
    int delta = now - pcnt_prev;
    if (delta == 0) return;
    int detents = delta / 4;
    if (detents == 0) return;
    pcnt_prev += detents * 4;

    if (knob_mode == MODE_BRIGHTNESS) {
        int clamped = constrain((int)brightness + detents, 0, 100);
        if (clamped != (int)brightness) {
            brightness = (uint8_t)clamped;
            dirty_bri = true;
            last_change_ms = millis();
        }
    } else {
        float clamped = constrain(commanded + detents * RAD_PER_CLICK, POS_MIN, POS_MAX);
        if (clamped != commanded) {
            commanded = clamped;
            markPosDirty();
        }
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

// Toggle knob mode between position and brightness.
void onShortClick() {
    knob_mode = (knob_mode == MODE_POSITION) ? MODE_BRIGHTNESS : MODE_POSITION;
    Serial.printf("mode: %s\n", knob_mode == MODE_POSITION ? "position" : "brightness");
}

// Wipe NVS, decommission Matter, and reboot.
void factoryReset() {
    Serial.println("factory reset: preferences cleared, decommissioning Matter, rebooting");
    prefs.clear();
    prefs.putFloat("pos", POS_MIN);
    prefs.putUChar("spd", DEFAULT_SPEED_IDX);
    prefs.putUChar("bri", 50);
    prefs.putUChar("cct", 50);
    Matter.decommission();
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
            press_start_ms  = now;
            warn_fired      = false;
            long_reset_done = false;
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
        } else if (held >= BTN_LONG_WARN_MS && !warn_fired) {
            warn_fired = true;
            pulseLEDs(5);
        }
    }
}

// Returns each iteration so FreeRTOS can service Matter / Thread / BLE tasks; motor commutation is handled by onFocTick() from the hw-timer ISR.
void loop() {
    static uint32_t prev_loop_us = 0;
    static uint32_t max_loop_us  = 0;
    static uint32_t last_active_ms = 0;
    static uint32_t stall_start_ms = 0;
    static uint32_t enable_time_ms = 0;
    static uint32_t last_print_ms  = 0;

    uint32_t now_us = micros();
    if (prev_loop_us != 0) {
        uint32_t dt = now_us - prev_loop_us;
        if (dt > max_loop_us) max_loop_us = dt;
    }
    prev_loop_us = now_us;

    esp_task_wdt_reset();

    // Clear the consecutive-crash counter once we've survived long enough that the Matter/WiFi stack has finished its startup dance.
    if (!crashes_cleared && millis() > CRASH_CLEAR_MS) {
        boot_prefs.putUChar("crashes", 0);
        crashes_cleared = true;
    }

    // Sync commanded to the physical shaft position whenever the motor is idle, so user input is always relative to reality.
    if (!motor.enabled && !reset_pending) {
        commanded  = motor.shaft_angle;
        target     = commanded;
        target_vel = 0.0f;
    }

    foc_cmd.run();
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
            resetDriverBegin();
            enable_time_ms = millis();
            stall_start_ms = 0;
        }
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

    motor.move(target);

    updateLEDs();
    maybePersist();
    printMatterPairing();
    syncMatterIfDirty();
    handleBLERelease();

    if (millis() - last_print_ms >= PRINT_INTERVAL_MS) {
        last_print_ms = millis();
        if (Serial.availableForWrite() >= 160) {
            Serial.printf("cmd:%ld.%02ld tgt:%ld.%02ld ang:%ld.%ld vel:%ld.%02ld tvel:%ld.%02ld vlim:%ld.%ld spd:%u bri:%u cct:%u mode:%c en:%d maxLp:%luus\n",
                          FX_HI(commanded, 100),            FX_LO(commanded, 100),
                          FX_HI(target, 100),               FX_LO(target, 100),
                          FX_HI(motor.shaft_angle, 10),     FX_LO(motor.shaft_angle, 10),
                          FX_HI(motor.shaft_velocity, 100), FX_LO(motor.shaft_velocity, 100),
                          FX_HI(target_vel, 100),           FX_LO(target_vel, 100),
                          FX_HI(motor.velocity_limit, 10),  FX_LO(motor.velocity_limit, 10),
                          speed_idx,
                          brightness,
                          cct,
                          knob_mode == MODE_POSITION ? 'P' : 'B',
                          (int)motor.enabled,
                          (unsigned long)max_loop_us);
        }
        max_loop_us = 0;
    }
}
