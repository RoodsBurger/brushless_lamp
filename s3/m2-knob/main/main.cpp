// M2-knob: motor + knob + stop button. Confirms that knob quadrature decoding on
// core 0 coexists cleanly with the FOC task on core 1 — the whole point of moving
// to a dual-core chip. No LEDs, no Matter, no radios.

#include <Arduino.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "motor.h"
#include "input.h"
#include "config.h"

extern "C" void app_main() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    initArduino();
    delay(500);

    printf("\n=== BrushlessLamp M2-knob (S3) ===\n");
    printf("knob: ±%.2f rad per detent, button → target=0\n", KNOB_STEP_RAD);

    input_init();              // attach knob/button on core 0
    input_task_start();         // 10 Hz poll, CORE_OTHERS
    motor_init_and_start();     // FOC task on core 1

    vTaskDelete(nullptr);       // app_main's task itself isn't needed after setup
}
