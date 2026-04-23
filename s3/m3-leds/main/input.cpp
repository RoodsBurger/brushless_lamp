// M3 input — quadrature knob + debounced button. Button state machine is the
// CylinderLamp pattern: short click → toggle brightness mode; double click (two
// clicks within BTN_DOUBLE_CLICK_MS) → cycle speed presets + pulse LEDs; 5 s hold
// → warning pulse; 9 s hold → factory reset (NVS erase + restart on M3, M4 calls
// esp_matter::factory_reset). Lives on core 0 alongside arduino-esp32 event tasks.

#include <Arduino.h>
#include <nvs_flash.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "input.h"
#include "motor.h"
#include "leds.h"
#include "config.h"
#include "pins.h"

static volatile int32_t  s_knob_delta = 0;
static volatile uint8_t  s_knob_prev  = 0;

static bool     s_brightness_mode = false;         // single-click toggles this
static uint8_t  s_speed_idx       = MOTION_VELOCITY_PRESET_DEFAULT;

static void IRAM_ATTR onKnobEdge() {
    uint8_t a    = digitalRead(PIN_ROT_A);
    uint8_t b    = digitalRead(PIN_ROT_B);
    uint8_t now  = (a << 1) | b;
    uint8_t prev = s_knob_prev;
    s_knob_prev  = now;
    if ((prev == 0b00 && now == 0b01) || (prev == 0b11 && now == 0b10)) s_knob_delta++;
    if ((prev == 0b01 && now == 0b00) || (prev == 0b10 && now == 0b11)) s_knob_delta--;
}

// Factory-reset stub for non-Matter stages: wipe the nvs partition and reboot.
// M4 replaces this with esp_matter::factory_reset() via a weak override.
static void factory_reset() {
    printf("[input] FACTORY RESET: erasing NVS and restarting\n");
    nvs_flash_erase();
    vTaskDelay(pdMS_TO_TICKS(200));
    esp_restart();
}

static void on_single_click() {
    s_brightness_mode = !s_brightness_mode;
    printf("[input] click → %s mode\n", s_brightness_mode ? "BRIGHTNESS" : "motor");
}

static void on_double_click() {
    constexpr uint8_t N = sizeof(MOTION_VELOCITY_PRESETS) / sizeof(MOTION_VELOCITY_PRESETS[0]);
    s_speed_idx = (uint8_t)((s_speed_idx + 1) % N);
    float new_v = MOTION_VELOCITY_PRESETS[s_speed_idx];
    motor_set_motion_velocity(new_v);
    leds_pulse((uint8_t)(s_speed_idx + 1));
    printf("[input] 2× → speed[%u] = %.1f rad/s\n", s_speed_idx, new_v);
}

// Combined button + click detector. Called every tick. Returns the decoded event
// type so callers can fire on it in the same tick (keeps the state local).
enum class BtnEvent : uint8_t { NONE, CLICK, DOUBLE_CLICK, HOLD_WARNING, FACTORY_RESET };

static BtnEvent poll_button() {
    static bool     stable        = true;             // true == released (INPUT_PULLUP)
    static bool     last_reading  = true;
    static uint32_t last_change   = 0;
    static uint32_t press_start   = 0;
    static uint32_t last_click_ms = 0;                // 0 means no pending click
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
        if (stable == LOW) {                          // just pressed
            press_start   = now;
            warning_fired = false;
        } else {                                      // just released
            uint32_t held = now - press_start;
            if (held < BTN_CLICK_MAX_MS) {
                // Valid click — decide single vs double based on pending state.
                if (pending_click && (now - last_click_ms) < BTN_DOUBLE_CLICK_MS) {
                    pending_click = false;
                    last_click_ms = 0;
                    ev = BtnEvent::DOUBLE_CLICK;
                } else {
                    pending_click = true;
                    last_click_ms = now;
                }
            }
            // Long-hold releases after warning but before factory reset → no event,
            // just reset the hold tracking. Factory reset itself fires from the
            // still-held branch below so releases there never reach this code.
        }
    }

    // Pending-single-click expiry: if no second click lands inside the window,
    // commit the single-click.
    if (pending_click && (now - last_click_ms) >= BTN_DOUBLE_CLICK_MS) {
        pending_click = false;
        last_click_ms = 0;
        ev = BtnEvent::CLICK;
    }

    // Long-hold ladder. Fires *while still held*.
    if (stable == LOW && press_start != 0) {
        uint32_t held = now - press_start;
        if (held >= BTN_FACTORY_RESET_MS && warning_fired) {
            ev = BtnEvent::FACTORY_RESET;
            press_start = 0;                          // arm once per press
        } else if (held >= BTN_LONG_PRESS_WARNING_MS && !warning_fired) {
            warning_fired = true;
            ev = BtnEvent::HOLD_WARNING;
        }
    }

    return ev;
}

static void input_task(void *) {
    while (true) {
        // Knob drain.
        int32_t delta;
        portDISABLE_INTERRUPTS();
        delta        = s_knob_delta;
        s_knob_delta = 0;
        portENABLE_INTERRUPTS();

        if (delta) {
            if (s_brightness_mode) {
                leds_nudge_max_duty((int16_t)(delta * LED_MAX_DUTY_STEP));
                printf("[input] knob %+ld -> max_duty=%u\n",
                       (long)delta, (unsigned)leds_get_max_duty());
            } else {
                motor_nudge_target_angle((float)delta * KNOB_STEP_RAD);
                printf("[input] knob %+ld -> target=%.2f rad\n",
                       (long)delta, motor_get_target_angle());
            }
        }

        // Button events.
        switch (poll_button()) {
        case BtnEvent::CLICK:          on_single_click(); break;
        case BtnEvent::DOUBLE_CLICK:   on_double_click(); break;
        case BtnEvent::HOLD_WARNING:   leds_pulse(5);
                                       printf("[input] 5 s hold — release soon or factory-reset at 9 s\n");
                                       break;
        case BtnEvent::FACTORY_RESET:  factory_reset(); break;
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
}

void input_task_start() {
    xTaskCreatePinnedToCore(input_task, "input", 4096, nullptr, 3, nullptr, CORE_OTHERS);
}
