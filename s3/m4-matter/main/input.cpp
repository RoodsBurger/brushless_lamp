// M4 input — same quadrature + button state machine as M3, but the 9 s hold calls
// esp_matter::factory_reset() (wipes fabric + KVS + reboots) instead of a plain
// nvs_flash_erase(). Knob nudges also arm motor_request_matter_sync_on_settle so
// the matter_app's settle callback pushes the new level once the lamp comes to rest.

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <nvs.h>

#include "input.h"
#include "motor.h"
#include "leds.h"
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
    s_brightness_mode = !s_brightness_mode;
    ESP_LOGI(TAG, "mode=%s", s_brightness_mode ? "brightness" : "motor");
}

static void on_double_click() {
    constexpr uint8_t N = sizeof(MOTION_VELOCITY_PRESETS) / sizeof(MOTION_VELOCITY_PRESETS[0]);
    s_speed_idx = (uint8_t)((s_speed_idx + 1) % N);
    float new_v = MOTION_VELOCITY_PRESETS[s_speed_idx];
    motor_set_motion_velocity(new_v);
    leds_pulse((uint8_t)(s_speed_idx + 1));
    // Synchronous NVS commit (~10-30 ms) on the 10 ms input task; acceptable
    // because double-click is a rare discrete event, not a rapid knob gesture.
    input_save_speed_idx();
    ESP_LOGI(TAG, "speed=%.1f rad/s", new_v);
}

enum class BtnEvent : uint8_t { NONE, CLICK, DOUBLE_CLICK, HOLD_WARNING, FACTORY_RESET };

static BtnEvent poll_button() {
    static bool     stable        = true;
    static bool     last_reading  = true;
    static uint32_t last_change   = 0;
    static uint32_t press_start   = 0;
    static uint32_t last_click_ms = 0;
    static bool     pending_click = false;
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
                if (pending_click && (now - last_click_ms) < BTN_DOUBLE_CLICK_MS) {
                    pending_click = false;
                    last_click_ms = 0;
                    ev = BtnEvent::DOUBLE_CLICK;
                } else {
                    pending_click = true;
                    last_click_ms = now;
                }
            }
        }
    }

    if (pending_click && (now - last_click_ms) >= BTN_DOUBLE_CLICK_MS) {
        pending_click = false;
        last_click_ms = 0;
        ev = BtnEvent::CLICK;
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
        case BtnEvent::HOLD_WARNING:   leds_pulse(5);
                                       ESP_LOGW(TAG, "5 s hold — release to cancel, hold to 9 s for factory reset");
                                       break;
        case BtnEvent::FACTORY_RESET:  ESP_LOGW(TAG, "FACTORY RESET");
                                       esp_matter::factory_reset();     // wipes fabric + KVS + reboots
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
