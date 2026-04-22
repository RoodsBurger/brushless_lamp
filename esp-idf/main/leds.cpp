// Phase 14c — dual-channel CCT LED driver, angle-driven on/off.
// The light follows the physical lamp position, not the Matter Level: whenever the
// shaft angle is above LED_ON_ANGLE_THRESH the LEDs come on at LED_DUTY (warm/cool
// mix from Matter's ColorTemperatureMireds). Below threshold they're off. Matter's
// Level still drives the motor angle, so the chain is
//     Matter Level -> motor target angle -> shaft position -> LED on/off
// which matches a kinetic lamp's intuition: it's lit when raised, dark when stored.
//
// 10 Hz poll + write-on-change keeps FOC jitter minimal (the 30 Hz sinf() variant
// from 14b starved the motor; see memory/project_arduino_as_component_c6.md §6).

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
static constexpr uint8_t  LED_PWM_RESOLUTION   = 8;     // 0–255 duty
static constexpr uint32_t LED_UPDATE_PERIOD_MS = 100;   // 10 Hz

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
    uint8_t last_warm = 0xFF, last_cool = 0xFF;  // force first write
    while (true) {
        float ang = motor_get_shaft_angle();
        bool  on  = (ang > LED_ON_ANGLE_THRESH);

        uint8_t warm = 0, cool = 0;
        if (on) {
            // Map Matter's colorTemperatureMireds → warm/cool fraction at fixed LED_DUTY.
            float mir = (float)matter_get_color_temp_mireds();
            if (mir < (float)CT_MIN_MIREDS) mir = (float)CT_MIN_MIREDS;
            if (mir > (float)CT_MAX_MIREDS) mir = (float)CT_MAX_MIREDS;
            float warm_frac = (mir - (float)CT_MIN_MIREDS) /
                              (float)(CT_MAX_MIREDS - CT_MIN_MIREDS);
            warm = (uint8_t)(warm_frac         * (float)LED_DUTY);
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
    // Priority 2 — below motor FOC (5) and input (3); may be starved under Matter load,
    // which is fine since LED updates are visual-feedback-only.
    xTaskCreate(leds_follower_task, "leds", 3072, nullptr, 2, nullptr);
}
