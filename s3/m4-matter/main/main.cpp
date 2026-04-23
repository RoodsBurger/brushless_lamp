// M4-matter: full stack. Motor + knob + LEDs + Matter ColorTemperatureLight
// endpoint. Commissions via Apple Home / Google Home over WiFi (BLE for pairing),
// persists fabric across reboots, factory-resettable via 5 s knob-button hold.

#include <Arduino.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "motor.h"
#include "input.h"
#include "leds.h"
#include "matter_app.h"
#include "config.h"

extern "C" void app_main() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    initArduino();
    delay(500);

    printf("\n=== BrushlessLamp M4-matter (S3) ===\n");

    // Wire the knob → Matter push BEFORE motor starts so the first settle fires a
    // clean update.
    motor_set_settle_callback(matter_push_level_from_angle);

    input_init();
    input_task_start();
    motor_init_and_start();
    leds_init();
    matter_app_init();
    leds_start_angle_follower();

    printf("Ready. Knob = +/-%.2f rad/detent. Hold button 5s for factory reset.\n",
           KNOB_STEP_RAD);

    vTaskDelete(nullptr);
}
