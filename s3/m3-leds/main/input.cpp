// M3 input — same behavior as M2 (knob nudges target, button resets to 0). Copied
// verbatim; each stage keeps its own copy so it stays self-contained for testing.

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "input.h"
#include "motor.h"
#include "config.h"
#include "pins.h"

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

static bool button_pressed() {
    static bool     last_reading = true;
    static bool     stable       = true;
    static uint32_t last_change  = 0;
    bool reading = digitalRead(PIN_BTN);
    if (reading != last_reading) {
        last_reading = reading;
        last_change  = millis();
    }
    if (millis() - last_change > BTN_DEBOUNCE_MS && reading != stable) {
        stable = reading;
        if (stable == LOW) return true;
    }
    return false;
}

static void input_task(void *) {
    while (true) {
        int32_t delta;
        portDISABLE_INTERRUPTS();
        delta        = s_knob_delta;
        s_knob_delta = 0;
        portENABLE_INTERRUPTS();

        if (delta) {
            motor_nudge_target_angle((float)delta * KNOB_STEP_RAD);
            printf("[input] knob %+ld -> target=%.2f rad\n",
                   (long)delta, motor_get_target_angle());
        }
        if (button_pressed()) {
            motor_set_target_angle(0.0f);
            printf("[input] button -> target=0 rad\n");
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
