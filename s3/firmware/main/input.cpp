// Knob + button: knob acts on the current mode (motor / brightness / CT).
// Button: 1 tap = OnOff, 2 = cycle mode, 3 = speed preset; hold 5 s + release =
// re-home, hold 15 s = factory reset (wipes the whole NVS partition + restarts).

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_system.h>
#include <nvs.h>
#include <nvs_flash.h>

#include "input.h"
#include "motor.h"
#include "leds.h"
#include "matter_app.h"
#include "config.h"
#include "pins.h"

static constexpr const char *INPUT_NVS_NS       = "input";
static constexpr const char *INPUT_NVS_KEY_SPD  = "spd_idx";

static const char *TAG = "input";

static volatile int32_t  s_knob_delta = 0;
static volatile uint8_t  s_knob_prev  = 0;

// Knob mode cycles motor → brightness → CT on each double-click; index doubles as the LED blink count.
enum class KnobMode : uint8_t { MOTOR = 0, BRIGHTNESS = 1, CT = 2, COUNT = 3 };
static KnobMode s_knob_mode  = KnobMode::MOTOR;
static uint8_t  s_speed_idx  = MOTION_VELOCITY_PRESET_DEFAULT;

static void input_load_speed_idx() {
    nvs_handle_t h;
    if (nvs_open(INPUT_NVS_NS, NVS_READONLY, &h) != ESP_OK) return;
    uint8_t saved = 0;
    if (nvs_get_u8(h, INPUT_NVS_KEY_SPD, &saved) == ESP_OK) {
        if (saved < MOTION_VELOCITY_PRESET_COUNT) s_speed_idx = saved;
    }
    nvs_close(h);
}

static void input_save_speed_idx() {
    nvs_handle_t h;
    if (nvs_open(INPUT_NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_u8(h, INPUT_NVS_KEY_SPD, s_speed_idx);
    nvs_commit(h);
    nvs_close(h);
}

static void IRAM_ATTR onKnobEdge() {
    uint8_t a    = digitalRead(PIN_ROT_A);
    uint8_t b    = digitalRead(PIN_ROT_B);
    uint8_t now  = (a << 1) | b;
    uint8_t prev = s_knob_prev;
    s_knob_prev  = now;
    // ++/-- swapped vs. canonical quadrature decode so the knob feels right with the inverted motor.
    if ((prev == 0b00 && now == 0b01) || (prev == 0b11 && now == 0b10)) s_knob_delta--;
    if ((prev == 0b01 && now == 0b00) || (prev == 0b10 && now == 0b11)) s_knob_delta++;
}

static void on_single_click() {
    // Google-Home-style on/off; matter_push_onoff fires apply_state which drives the motor.
    bool new_on = !matter_get_on_off();
    matter_push_onoff(new_on);
    ESP_LOGI(TAG, "onoff → %s", new_on ? "on" : "off");
}

static void on_double_click() {
    s_knob_mode = (KnobMode)(((uint8_t)s_knob_mode + 1) % (uint8_t)KnobMode::COUNT);
    // 1/2/3 blinks → motor/brightness/CT. Triple-click reuses 1..3 for speed presets; user disambiguates by gesture.
    leds_pulse((uint8_t)s_knob_mode + 1);
    const char *name = (s_knob_mode == KnobMode::MOTOR)      ? "motor"
                     : (s_knob_mode == KnobMode::BRIGHTNESS) ? "brightness"
                     :                                          "colortemp";
    ESP_LOGI(TAG, "mode=%s", name);
}

static void on_triple_click() {
    s_speed_idx = (uint8_t)((s_speed_idx + 1) % MOTION_VELOCITY_PRESET_COUNT);
    float new_v = MOTION_VELOCITY_PRESETS[s_speed_idx];
    motor_set_motion_velocity(new_v);
    leds_pulse((uint8_t)(s_speed_idx + 1));
    // Synchronous NVS commit (~10-30 ms); fine because triple-click is a rare discrete event.
    input_save_speed_idx();
    ESP_LOGI(TAG, "speed=%.1f rad/s", new_v);
}

enum class BtnEvent : uint8_t { NONE, CLICK, DOUBLE_CLICK, TRIPLE_CLICK, HOME_ARMED, HOME_RUN, FACTORY_RESET };

static BtnEvent poll_button() {
    static bool     stable        = true;
    static bool     last_reading  = true;
    static uint32_t last_change   = 0;
    static uint32_t press_start   = 0;
    static uint32_t last_click_ms = 0;
    static uint8_t  click_count   = 0;
    static bool     home_armed    = false;   // crossed the 5 s arm threshold this press

    bool reading = digitalRead(PIN_BTN);
    uint32_t now = millis();

    if (reading != last_reading) {
        last_reading = reading;
        last_change  = now;
    }

    BtnEvent ev = BtnEvent::NONE;

    if (now - last_change > BTN_DEBOUNCE_MS && reading != stable) {
        stable = reading;
        if (stable == LOW) {                 // pressed
            press_start = now;
            home_armed  = false;
        } else {                             // released
            uint32_t held = now - press_start;
            if (press_start != 0 && home_armed) {
                // Released after the 5 s arm (a 15 s hold zeroes press_start first) → run homing.
                ev = BtnEvent::HOME_RUN;
            } else if (held < BTN_CLICK_MAX_MS) {
                // Each tap restarts the window; CLICK_N fires when the window closes with N pending.
                click_count++;
                last_click_ms = now;
            }
            home_armed = false;
        }
    }

    // Window closed with N clicks pending → fire the matching event. The stable==HIGH
    // gate keeps the window open while a follow-up tap is still held, so a slow
    // double-click can't fire as two single clicks.
    if (stable == HIGH && click_count > 0 && (now - last_click_ms) >= BTN_DOUBLE_CLICK_MS) {
        if      (click_count == 1) ev = BtnEvent::CLICK;
        else if (click_count == 2) ev = BtnEvent::DOUBLE_CLICK;
        else                        ev = BtnEvent::TRIPLE_CLICK;   // 3 or more
        click_count   = 0;
        last_click_ms = 0;
    }

    // Hold staging: 5 s arms re-homing (blink 3, runs on release); 15 s = factory reset (blink 5).
    if (stable == LOW && press_start != 0) {
        uint32_t held = now - press_start;
        if (held >= BTN_FACTORY_RESET_MS) {
            ev = BtnEvent::FACTORY_RESET;
            press_start = 0;                 // consumed; the release won't also run homing
        } else if (held >= BTN_HOME_ARM_MS && !home_armed) {
            home_armed    = true;
            click_count   = 0;   // this press is a hold, not a tap — drop any pending tap
            ev = BtnEvent::HOME_ARMED;
        }
    }

    return ev;
}

static void input_task(void *) {
    // CT turns mirror to Matter only after the knob has been quiet — one attribute
    // report per gesture instead of one per 10 ms tick.
    constexpr uint32_t CT_PUSH_QUIET_MS = 250;
    uint32_t ct_push_pending_ms = 0;

    while (true) {
        int32_t delta;
        // Per-core mask is sufficient: the knob ISRs attach from input_init on CORE_OTHERS, same core this task is pinned to.
        portDISABLE_INTERRUPTS();
        delta        = s_knob_delta;
        s_knob_delta = 0;
        portENABLE_INTERRUPTS();

        if (delta) {
            switch (s_knob_mode) {
            case KnobMode::MOTOR:
                motor_nudge_target_angle((float)delta * KNOB_STEP_RAD);
                motor_request_matter_sync_on_settle();   // pushes level to Matter once at rest
                break;
            case KnobMode::BRIGHTNESS:
                leds_nudge_max_duty((int16_t)(delta * LED_MAX_DUTY_STEP));
                break;
            case KnobMode::CT:
                leds_nudge_colortemp((int16_t)(delta * KNOB_CT_STEP_MIREDS));
                ct_push_pending_ms = millis();
                break;
            default: break;
            }
        }
        if (ct_push_pending_ms != 0 && (millis() - ct_push_pending_ms) >= CT_PUSH_QUIET_MS) {
            ct_push_pending_ms = 0;
            matter_push_colortemp(leds_get_colortemp());   // mirror to ColorControl so Home apps reflect the change
        }

        switch (poll_button()) {
        case BtnEvent::CLICK:          on_single_click(); break;
        case BtnEvent::DOUBLE_CLICK:   on_double_click(); break;
        case BtnEvent::TRIPLE_CLICK:   on_triple_click(); break;
        case BtnEvent::HOME_ARMED:     leds_pulse(3);
                                       ESP_LOGW(TAG, "5 s hold — release to re-home, keep holding to 15 s for factory reset");
                                       break;
        case BtnEvent::HOME_RUN:       ESP_LOGW(TAG, "re-homing: driving to off-stop + re-zeroing");
                                       motor_request_homing();
                                       break;
        case BtnEvent::FACTORY_RESET:  ESP_LOGW(TAG, "FACTORY RESET — wiping NVS + restarting");
                                       leds_pulse(5);
                                       vTaskDelay(pdMS_TO_TICKS(1500));   // let the 5 blinks show before the reboot
                                       // Nuclear wipe of the whole NVS partition. esp_matter::factory_reset()
                                       // leaves residual deferred-persistence attribute values (the LevelControl
                                       // CurrentLevel write that doesn't get cleared), which on the next-but-one
                                       // boot fires apply_state() with a stale level and drives the motor
                                       // unexpectedly. Matches the clean state produced by `esptool erase-region`.
                                       nvs_flash_deinit();   // unmount first so a concurrent commit can't interleave with the erase
                                       nvs_flash_erase();
                                       esp_restart();
                                       break;
        case BtnEvent::NONE:           break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void input_init() {
    pinMode(PIN_BTN,   INPUT_PULLUP);
    pinMode(PIN_ROT_A, INPUT_PULLUP);
    pinMode(PIN_ROT_B, INPUT_PULLUP);
    s_knob_prev = (digitalRead(PIN_ROT_A) << 1) | digitalRead(PIN_ROT_B);
    attachInterrupt(digitalPinToInterrupt(PIN_ROT_A), onKnobEdge, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ROT_B), onKnobEdge, CHANGE);

    // Apply persisted speed preset before the FOC task picks up motion commands.
    input_load_speed_idx();
    motor_set_motion_velocity(MOTION_VELOCITY_PRESETS[s_speed_idx]);
}

void input_task_start() {
    xTaskCreatePinnedToCore(input_task, "input", 4096, nullptr, 3, nullptr, CORE_OTHERS);
}
