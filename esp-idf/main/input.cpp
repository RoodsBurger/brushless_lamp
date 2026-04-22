// Phase 14a — knob quadrature + button debouncer, Arduino-API style.
// Knob ISR counts one delta per full detent (quadrature Gray-code state machine on CW-only
// transitions). A low-priority FreeRTOS task drains the delta into motor_set_target_velocity
// so the ISR stays pure and we don't block the FOC inner loop when the knob turns.

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

static void input_task(void *) {
    while (true) {
        int32_t delta;
        portDISABLE_INTERRUPTS();
        delta        = s_knob_delta;
        s_knob_delta = 0;
        portENABLE_INTERRUPTS();

        if (delta) {
            float t = motor_get_target_velocity() + (float)delta * KNOB_STEP_RAD_PER_SEC;
            motor_set_target_velocity(t);
            // printf (IDF stdout → USB_SERIAL_JTAG) instead of Serial.printf — see motor.cpp.
            printf("knob %+ld -> target=%.2f rad/s\n",
                   (long)delta, motor_get_target_velocity());
        }

        if (buttonPressed()) {
            motor_stop();
            printf("button -- target=0\n");
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
