// M3-leds: motor + knob + LEDs. LED brightness comes from motor shaft angle (lamp
// raised → lit, lamp down → dark) at a fixed 50/50 warm/cool mix. No Matter yet.

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
    printf("LED on when shaft > %.1f rad, 50/50 warm/cool at duty=%u\n",
           LED_ON_ANGLE_THRESH, (unsigned)LED_DUTY);

    input_init();
    input_task_start();
    motor_init_and_start();
    leds_init();
    leds_start_angle_follower();

    vTaskDelete(nullptr);
}
