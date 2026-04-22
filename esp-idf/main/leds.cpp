// Phase 14b — dual-channel CCT LED driver on arduino-esp32 3.x LEDC.
// LEDC runs independently of MCPWM (which drives the BLDC's 3-phase PWM), so there's no
// peripheral contention with the motor loop. 25 kHz is above audibility and gives 11-bit
// effective resolution on the 80 MHz LEDC clock — we just take 8 bits of it.

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>

#include "leds.h"
#include "motor.h"
#include "pins.h"

static constexpr uint32_t LED_PWM_FREQ_HZ      = 25000;
static constexpr uint8_t  LED_PWM_RESOLUTION   = 8;     // 0–255 duty matches setters
static constexpr uint32_t LED_UPDATE_PERIOD_MS = 500;   // 2 Hz — was 30 ms/33 Hz, too much
                                                        // jitter on the FOC loop on RISC-V.
static constexpr float    LED_ANGLE_THRESHOLD  = 5.0f;  // rad from origin
static constexpr uint8_t  LED_DUTY             = 32;    // 12.5 % both channels when "on"

void leds_init() {
    // ledcAttach() is the 3.x pin-based API — picks a free channel under the hood.
    ledcAttach(PIN_LED_WW, LED_PWM_FREQ_HZ, LED_PWM_RESOLUTION);
    ledcAttach(PIN_LED_CW, LED_PWM_FREQ_HZ, LED_PWM_RESOLUTION);
    ledcWrite(PIN_LED_WW, 0);
    ledcWrite(PIN_LED_CW, 0);
}

void leds_set(uint8_t warm, uint8_t cool) {
    ledcWrite(PIN_LED_WW, warm);
    ledcWrite(PIN_LED_CW, cool);
}

static void leds_angle_task(void *) {
    uint8_t last_duty = 0xFF;      // force first write
    while (true) {
        float a = motor_get_shaft_angle();
        // Binary: LEDs on while the rotor is away from origin, off near it. No sinf().
        // Only poke LEDC when the output actually changes — avoids needless bus traffic
        // that (we think) was stealing time from the FOC loop on RISC-V softfloat.
        uint8_t d = (fabsf(a) > LED_ANGLE_THRESHOLD) ? LED_DUTY : 0;
        if (d != last_duty) {
            leds_set(d, d);
            last_duty = d;
        }
        vTaskDelay(pdMS_TO_TICKS(LED_UPDATE_PERIOD_MS));
    }
}

void leds_start_angle_follower() {
    // Priority 2: below the motor FOC task (5) and the input task (3). Starvation is
    // acceptable for the LED loop; the motor must never be starved by it.
    xTaskCreate(leds_angle_task, "leds", 3072, nullptr, 2, nullptr);
}
