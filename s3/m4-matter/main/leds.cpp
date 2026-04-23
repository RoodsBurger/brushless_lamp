// M4 LEDs — same 10 Hz follower as M3, but the warm/cool mix comes from Matter's
// ColorControl::ColorTemperatureMireds attribute (exposed via matter_app getter).
// On/off is still driven purely by motor.shaft_angle crossing LED_ON_ANGLE_THRESH.

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>

#include "leds.h"
#include "motor.h"
#include "matter_app.h"
#include "config.h"
#include "pins.h"

static constexpr uint32_t LED_PWM_FREQ_HZ      = 25000;
static constexpr uint8_t  LED_PWM_RESOLUTION   = 8;
static constexpr uint32_t LED_UPDATE_PERIOD_MS = 100;

void leds_init() {
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
    uint8_t last_warm = 0xFF, last_cool = 0xFF;
    while (true) {
        bool on = motor_get_shaft_angle() > LED_ON_ANGLE_THRESH;
        uint8_t warm = 0, cool = 0;
        if (on) {
            // Mireds → warm/cool fraction. 153 mireds = coolest (pure cool), 500 = warmest.
            float mir = (float)matter_get_color_temp_mireds();
            if (mir < (float)CT_MIN_MIREDS) mir = (float)CT_MIN_MIREDS;
            if (mir > (float)CT_MAX_MIREDS) mir = (float)CT_MAX_MIREDS;
            float warm_frac = (mir - (float)CT_MIN_MIREDS) /
                              (float)(CT_MAX_MIREDS - CT_MIN_MIREDS);
            warm = (uint8_t)(warm_frac          * (float)LED_DUTY);
            cool = (uint8_t)((1.0f - warm_frac) * (float)LED_DUTY);
        }
        if (warm != last_warm || cool != last_cool) {
            leds_set(warm, cool);
            last_warm = warm;
            last_cool = cool;
        }
        vTaskDelay(pdMS_TO_TICKS(LED_UPDATE_PERIOD_MS));
    }
}

void leds_start_angle_follower() {
    xTaskCreatePinnedToCore(leds_follower_task, "leds", 3072, nullptr, 2, nullptr, CORE_OTHERS);
}
