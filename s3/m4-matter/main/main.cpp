// M4-matter: full stack. Motor + knob + LEDs + Matter ColorTemperatureLight
// endpoint. Commissions via Apple Home / Google Home over WiFi (BLE for pairing),
// persists fabric across reboots. Button: click → brightness mode, 2× → speed
// cycle, 9 s hold → esp_matter::factory_reset.

#include <Arduino.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "motor.h"
#include "input.h"
#include "leds.h"
#include "matter_app.h"
#include "config.h"
#include "pins.h"

extern "C" void app_main() {
    // Park the DRV8313 asleep before anything else runs. nSLEEP has an external
    // pullup, so without this the driver is AWAKE from POR through ROM +
    // bootloader + our init, pulling quiescent current from a rail that may
    // still be ramping. motor_foc_task drives HIGH when it's ready.
    gpio_set_direction((gpio_num_t)PIN_NSP, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)PIN_NSP, 0);

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    initArduino();
    // Power-rail stabilization window. Lets the buck converter output and the
    // VBUS bulk cap fully charge before Matter's BLE advertiser (300–500 mA
    // transient) and initFOC's sensor-align sweep (~1.2 A) hit the supply.
    delay(1500);

    printf("\n=== BrushlessLamp M4-matter (S3) ===\n");

    // Wire the knob → Matter push BEFORE motor starts so the first settle fires a
    // clean update.
    motor_set_settle_callback(matter_push_level_from_angle);

    leds_init();
    matter_app_init();
    leds_start_fader();
    motor_init_and_start();
    input_init();
    input_task_start();

    printf("Ready. Knob = %.2f rad/detent | 1×=on/off | 2×=mode | 3×=speed | 9s=factory reset\n",
           KNOB_STEP_RAD);

    vTaskDelete(nullptr);
}
