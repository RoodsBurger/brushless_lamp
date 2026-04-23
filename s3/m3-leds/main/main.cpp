// M3-leds: motor + knob + LEDs. Adds CylinderLamp's control feel — single-click
// toggles brightness mode (knob → LED max duty), double-click cycles
// MOTION_VELOCITY presets with a feedback pulse, 9 s hold clears NVS + reboots.
// LEDs fade smoothly to/from the knob-driven on/off state.

#include <Arduino.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "motor.h"
#include "input.h"
#include "leds.h"
#include "config.h"

extern "C" void app_main() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    initArduino();
    delay(500);

    printf("\n=== BrushlessLamp M3-leds (S3) ===\n");
    printf("LED curve: 0..%.2f rad linear ramp → γ=%.1f perceptual; max_duty=%u; CT=%u\n",
           LED_FADE_ANGLE_RAD, LED_GAMMA, (unsigned)LED_MAX_DUTY_DEFAULT, (unsigned)COLORTEMP_DEFAULT);
    printf("Button: click → brightness mode; 2× → speed cycle; 9 s → factory reset\n");

    input_init();
    input_task_start();
    motor_init_and_start();
    leds_init();
    leds_start_fader();

    vTaskDelete(nullptr);
}
