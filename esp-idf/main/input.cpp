// Phase 14a — knob quadrature + button debouncer, Arduino-API style.
// Knob ISR counts one delta per full detent (quadrature Gray-code state machine on CW-only
// transitions). A low-priority FreeRTOS task drains the delta into motor_set_target_velocity
// so the ISR stays pure and we don't block the FOC inner loop when the knob turns.
//
// Phase 14c adds a 5 s button-hold detector that triggers an esp_matter factory reset
// (wipes the Matter fabric/KVS partitions and reboots). Short clicks still stop motion.

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_matter.h>

#include "input.h"
#include "motor.h"
#include "matter_app.h"
#include "config.h"
#include "pins.h"

static constexpr uint32_t BTN_FACTORY_RESET_MS = 5000;  // hold this long → factory reset
static const char *TAG_BTN = "input";

static volatile int32_t  s_knob_delta = 0;
static volatile uint8_t  s_knob_prev  = 0;

static void IRAM_ATTR onKnobEdge() {
    uint8_t a    = digitalRead(PIN_ROT_A);
    uint8_t b    = digitalRead(PIN_ROT_B);
    uint8_t now  = (a << 1) | b;
    uint8_t prev = s_knob_prev;
    s_knob_prev  = now;
    if ((prev == 0b00 && now == 0b01) || (prev == 0b11 && now == 0b10)) s_knob_delta++;
    if ((prev == 0b01 && now == 0b00) || (prev == 0b10 && now == 0b11)) s_knob_delta--;
}

static bool buttonPressed() {
    static bool     s_last_reading = true;
    static bool     s_stable       = true;
    static uint32_t s_last_change  = 0;
    bool reading = digitalRead(PIN_BTN);
    if (reading != s_last_reading) {
        s_last_reading = reading;
        s_last_change  = millis();
    }
    if (millis() - s_last_change > BTN_DEBOUNCE_MS && reading != s_stable) {
        s_stable = reading;
        if (s_stable == LOW) return true;
    }
    return false;
}

// Track how long the button has been held. Reset to 0 on release.
static uint32_t s_btn_press_start = 0;

static void input_task(void *) {
    while (true) {
        int32_t delta;
        portDISABLE_INTERRUPTS();
        delta        = s_knob_delta;
        s_knob_delta = 0;
        portENABLE_INTERRUPTS();

        if (delta) {
            motor_nudge_target_angle((float)delta * KNOB_STEP_RAD);
            motor_enable();   // any knob activity implies "I want the lamp moving"
            // Don't push to Matter on every detent — the echo through
            // attribute_update_cb was the knob choppiness. Let motor_foc_task push
            // once on deadband entry (post-settle).
            motor_request_matter_sync_on_settle();
            printf("knob %+ld -> target=%.2f rad\n",
                   (long)delta, motor_get_target_angle());
        }

        if (buttonPressed()) {
            motor_set_target_angle(0.0f);
            printf("button -- target=0 rad\n");
        }

        // Long-press → factory reset: wipe Matter fabric/KVS and reboot. Useful when
        // commissioning gets wedged (stale fabric state, failed prior attempts, etc.).
        bool held = (digitalRead(PIN_BTN) == LOW);
        if (held) {
            if (s_btn_press_start == 0) s_btn_press_start = millis();
            else if (millis() - s_btn_press_start > BTN_FACTORY_RESET_MS) {
                ESP_LOGW(TAG_BTN, "button held %lu ms -> factory reset", (unsigned long)BTN_FACTORY_RESET_MS);
                esp_matter::factory_reset();  // erases Matter NVS + reboots
            }
        } else {
            s_btn_press_start = 0;
        }

        // 10 ms poll — matches the Arduino button period and leaves 99 %+ of the CPU for FOC.
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
    xTaskCreate(input_task, "input", 4096, nullptr, 3, nullptr);
}
