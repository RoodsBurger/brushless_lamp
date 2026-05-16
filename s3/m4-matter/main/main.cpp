// M4-matter: ColorTemperatureLight Matter endpoint + knob/button local control.

#include <Arduino.h>
#include <driver/gpio.h>
#include <esp_private/rtc_clk.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

#include "config.h"
#include "input.h"
#include "leds.h"
#include "matter_app.h"
#include "motor.h"
#include "pins.h"

extern "C" void app_main() {
    // nSLEEP has an external pullup on XIAO; park the DRV8313 asleep before the
    // bootloader release lets it pull quiescent current.
    gpio_set_direction((gpio_num_t)PIN_NSP, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)PIN_NSP, 0);

    // GPIO 21 (XIAO on-board user LED, active-LOW): drive HIGH so the LED stays off.
    gpio_set_direction((gpio_num_t)21, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)21, 1);

    // Keep BBPLL consumer count >= 1 against USB-Serial-JTAG SOF auto-removal on host-less boot.
    rtc_clk_bbpll_add_consumer();

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    initArduino();
    delay(500);

    printf("\n=== BrushlessLamp M4-matter (S3) ===\n");

    motor_set_settle_callback(matter_push_level_from_angle);

    // Order matters: Matter (BLE / Wi-Fi / CHIP) must be up before the FOC busy-wait
    // pegs core 1, otherwise CSRRequest times out with CHIP_ERROR_TIMEOUT (32).
    leds_init();
    matter_app_init();
    leds_start_fader();
    motor_init_and_start();
    input_init();
    input_task_start();

    printf("Ready. Knob = %.2f rad/detent | 1x=on/off | 2x=mode | 3x=speed | 9s=factory reset\n",
           KNOB_STEP_RAD);

    vTaskDelete(nullptr);
}
