// M3 LED driver — 25 kHz LEDC on WW/CW, 10 Hz follower task that turns LEDs on
// whenever the lamp is lifted above LED_ON_ANGLE_THRESH. No color-temperature
// parameter in M3 (that's an M4 Matter attribute); we just blend 50/50 here and
// verify the follower doesn't affect motor audibility on the dual-core layout.

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "leds.h"
#include "motor.h"
#include "config.h"
#include "pins.h"

static constexpr uint32_t LED_PWM_FREQ_HZ      = 25000;
static constexpr uint8_t  LED_PWM_RESOLUTION   = 8;
static constexpr uint32_t LED_UPDATE_PERIOD_MS = 100;

void leds_init() {
    // arduino-esp32 3.x LEDC pin-based API — one call per pin, no channel bookkeeping.
    ledcAttach(PIN_LED_WW, LED_PWM_FREQ_HZ, LED_PWM_RESOLUTION);
    ledcAttach(PIN_LED_CW, LED_PWM_FREQ_HZ, LED_PWM_RESOLUTION);
    ledcWrite(PIN_LED_WW, 0);
    ledcWrite(PIN_LED_CW, 0);
}

void leds_set(uint8_t warm, uint8_t cool) {
    ledcWrite(PIN_LED_WW, warm);
    ledcWrite(PIN_LED_CW, cool);
}

static void leds_follower_task(void *) {
    uint8_t last_duty = 0xFF;
    while (true) {
        bool on = motor_get_shaft_angle() > LED_ON_ANGLE_THRESH;
        uint8_t duty = on ? (LED_DUTY / 2) : 0;     // 50/50 mix until M4 adds color temp
        if (duty != last_duty) {
            leds_set(duty, duty);
            last_duty = duty;
        }
        vTaskDelay(pdMS_TO_TICKS(LED_UPDATE_PERIOD_MS));
    }
}

void leds_start_angle_follower() {
    // Priority 2, CORE_OTHERS. Lowest-priority visible task; LED flicker is never
    // worth taking a cycle from input (3) or motor (MOTOR_TASK_PRIORITY = 20).
    xTaskCreatePinnedToCore(leds_follower_task, "leds", 3072, nullptr, 2, nullptr, CORE_OTHERS);
}
