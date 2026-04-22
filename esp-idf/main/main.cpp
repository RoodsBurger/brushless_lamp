// Phase 14a — arduino-as-component + SimpleFOC v2.4.0 on native ESP-IDF, no Matter.
// We own app_main directly (CONFIG_AUTOSTART_ARDUINO=n). initArduino() brings up the
// Arduino runtime (millis/micros, Serial via USB-CDC, attachInterrupt, etc.) without
// starting Arduino's setup()/loop() — SimpleFOC's loopFOC() runs in our own motor task.

#include <Arduino.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "motor.h"
#include "input.h"
#include "leds.h"

static const char *TAG = "app";

extern "C" void app_main() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    initArduino();
    delay(500);

    // IDF printf routes to USB_SERIAL_JTAG here (arduino-esp32 3.3.8 defaults Serial to
    // UART0 on C6, which on our board is the nSLEEP pin and goes nowhere).
    printf("\n=== BrushlessLamp Phase 14b: motor + LEDs (no Matter) ===\n");

    input_init();
    motor_init_and_start();
    input_task_start();
    // LEDs are init'd AFTER motor and the angle follower runs at 2 Hz with binary
    // on/off — a 30 Hz sinf() task on the FPU-less C6 starved the FOC loop enough to
    // drop torque; 14c's Matter slider will write LED duty < 1 Hz anyway.
    leds_init();
    leds_start_angle_follower();

    printf("Ready. Rotate knob to set target velocity (+/-3 rad/s per detent). Press button to stop.\n");

    vTaskDelete(nullptr);
}
