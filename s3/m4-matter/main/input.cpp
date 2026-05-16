// Knob + button: knob nudges motor target and arms a Matter settle push;
// 9 s button hold calls esp_matter::factory_reset.

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <nvs.h>

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

static bool     s_brightness_mode = false;
static uint8_t  s_speed_idx       = MOTION_VELOCITY_PRESET_DEFAULT;

static void input_load_speed_idx() {
    nvs_handle_t h;
    if (nvs_open(INPUT_NVS_NS, NVS_READONLY, &h) != ESP_OK) return;
    uint8_t saved = 0;
    if (nvs_get_u8(h, INPUT_NVS_KEY_SPD, &saved) == ESP_OK) {
        constexpr uint8_t N = sizeof(MOTION_VELOCITY_PRESETS) / sizeof(MOTION_VELOCITY_PRESETS[0]);
        if (saved < N) s_speed_idx = saved;
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
    if ((prev == 0b00 && now == 0b01) || (prev == 0b11 && now == 0b10)) s_knob_delta++;
    if ((prev == 0b01 && now == 0b00) || (prev == 0b10 && now == 0b11)) s_knob_delta--;
}

static void on_single_click() {
    // Google-Home-style on/off: off → motor target 0, on → last Matter level
    // (matter_push_onoff's apply_state drives the motor from the current level).
    bool new_on = !matter_get_on_off();
    matter_push_onoff(new_on);
    ESP_LOGI(TAG, "onoff → %s", new_on ? "on" : "off");
}

static void on_double_click() {
    s_brightness_mode = !s_brightness_mode;
    // 2 blinks so the mode toggle can't be mistaken for the idx-0 speed blink.
    leds_pulse(2);
    ESP_LOGI(TAG, "mode=%s", s_brightness_mode ? "brightness" : "motor");
}

static void on_triple_click() {
    constexpr uint8_t N = sizeof(MOTION_VELOCITY_PRESETS) / sizeof(MOTION_VELOCITY_PRESETS[0]);
    s_speed_idx = (uint8_t)((s_speed_idx + 1) % N);
    float new_v = MOTION_VELOCITY_PRESETS[s_speed_idx];
    motor_set_motion_velocity(new_v);
    leds_pulse((uint8_t)(s_speed_idx + 1));
    // Synchronous NVS commit (~10-30 ms); fine because triple-click is a rare discrete event.
    input_save_speed_idx();
    ESP_LOGI(TAG, "speed=%.1f rad/s", new_v);
}

enum class BtnEvent : uint8_t { NONE, CLICK, DOUBLE_CLICK, TRIPLE_CLICK, HOLD_WARNING, FACTORY_RESET };

static BtnEvent poll_button() {
    static bool     stable        = true;
    static bool     last_reading  = true;
    static uint32_t last_change   = 0;
    static uint32_t press_start   = 0;
    static uint32_t last_click_ms = 0;
    static uint8_t  click_count   = 0;
    static bool     warning_fired = false;

    bool reading = digitalRead(PIN_BTN);
    uint32_t now = millis();

    if (reading != last_reading) {
        last_reading = reading;
        last_change  = now;
    }

    BtnEvent ev = BtnEvent::NONE;

    if (now - last_change > BTN_DEBOUNCE_MS && reading != stable) {
        stable = reading;
        if (stable == LOW) {
            press_start   = now;
            warning_fired = false;
        } else {
            uint32_t held = now - press_start;
            if (held < BTN_CLICK_MAX_MS) {
                // Accumulate click count; each new click restarts the window
                // so N rapid taps in a row fire one CLICK_N event at the end.
                click_count++;
                last_click_ms = now;
            }
        }
    }

    // Window closed with N clicks pending → fire the matching event.
    if (click_count > 0 && (now - last_click_ms) >= BTN_DOUBLE_CLICK_MS) {
        if      (click_count == 1) ev = BtnEvent::CLICK;
        else if (click_count == 2) ev = BtnEvent::DOUBLE_CLICK;
        else                        ev = BtnEvent::TRIPLE_CLICK;   // 3 or more
        click_count   = 0;
        last_click_ms = 0;
    }

    if (stable == LOW && press_start != 0) {
        uint32_t held = now - press_start;
        if (held >= BTN_FACTORY_RESET_MS && warning_fired) {
            ev = BtnEvent::FACTORY_RESET;
            press_start = 0;
        } else if (held >= BTN_LONG_PRESS_WARNING_MS && !warning_fired) {
            warning_fired = true;
            ev = BtnEvent::HOLD_WARNING;
        }
    }

    return ev;
}

static void input_task(void *) {
    while (true) {
        int32_t delta;
        portDISABLE_INTERRUPTS();
        delta        = s_knob_delta;
        s_knob_delta = 0;
        portENABLE_INTERRUPTS();

        if (delta) {
            if (s_brightness_mode) {
                leds_nudge_max_duty((int16_t)(delta * LED_MAX_DUTY_STEP));
            } else {
                motor_nudge_target_angle((float)delta * KNOB_STEP_RAD);
                motor_request_matter_sync_on_settle();   // pushes level to Matter once at rest
            }
        }

        switch (poll_button()) {
        case BtnEvent::CLICK:          on_single_click(); break;
        case BtnEvent::DOUBLE_CLICK:   on_double_click(); break;
        case BtnEvent::TRIPLE_CLICK:   on_triple_click(); break;
        case BtnEvent::HOLD_WARNING:   leds_pulse(5);
                                       ESP_LOGW(TAG, "5 s hold — release to cancel, hold to 9 s for factory reset");
                                       break;
        case BtnEvent::FACTORY_RESET:  ESP_LOGW(TAG, "FACTORY RESET");
                                       matter_wipe_local_nvs();         // foc_cal / leds / input — not touched by esp_matter::factory_reset
                                       esp_matter::factory_reset();     // wipes CHIP fabric + KVS, then esp_restart
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

    // Restore speed preset from NVS and apply to the motor BEFORE the FOC task
    // starts picking up motion commands.
    input_load_speed_idx();
    motor_set_motion_velocity(MOTION_VELOCITY_PRESETS[s_speed_idx]);
}

void input_task_start() {
    xTaskCreatePinnedToCore(input_task, "input", 4096, nullptr, 3, nullptr, CORE_OTHERS);
}
