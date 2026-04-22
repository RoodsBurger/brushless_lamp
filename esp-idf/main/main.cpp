// Phase 14c — arduino-as-component + SimpleFOC v2.4.0 + esp-matter on native ESP-IDF.
// We own app_main (CONFIG_AUTOSTART_ARDUINO=n). initArduino() brings up the Arduino
// runtime without starting setup()/loop(); the motor FOC runs in motor_foc_task and
// Matter runs in the CHIP task started by esp_matter::start().

#include <Arduino.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "motor.h"
#include "input.h"
#include "leds.h"
#include "matter_app.h"

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
    printf("\n=== BrushlessLamp Phase 14c: motor + LEDs + Matter ===\n");

    input_init();
    motor_init_and_start();
    input_task_start();
    // LEDs init after MCPWM is up; the angle follower (10 Hz) drives on/off from the
    // live shaft angle + Matter's current ColorTemperature attribute.
    leds_init();
    matter_app_init();
    leds_start_angle_follower();

    printf("Ready. Knob = +/-1 rad per detent. Matter slider = 0..ANGLE_MAX. Hold button 5s to factory reset.\n");

    vTaskDelete(nullptr);
}
