// BrushlessLamp firmware: ColorTemperatureLight Matter endpoint + knob/button local control.

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

static const char *reset_reason_str(esp_reset_reason_t r) {
    switch (r) {
    case ESP_RST_POWERON:    return "POWERON";
    case ESP_RST_EXT:        return "EXT";
    case ESP_RST_SW:         return "SW";
    case ESP_RST_PANIC:      return "PANIC";
    case ESP_RST_INT_WDT:    return "INT_WDT";
    case ESP_RST_TASK_WDT:   return "TASK_WDT";
    case ESP_RST_WDT:        return "WDT";
    case ESP_RST_DEEPSLEEP:  return "DEEPSLEEP";
    case ESP_RST_BROWNOUT:   return "BROWNOUT";
    case ESP_RST_SDIO:       return "SDIO";
    default:                 return "UNKNOWN";
    }
}

extern "C" void app_main() {
    // Park DRV8313 asleep before the bootloader release window so its 100 µA
    // quiescent draw doesn't blip 3V3 during external-power brown-in.
    gpio_set_direction((gpio_num_t)PIN_NSP, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)PIN_NSP, 0);

#ifndef BRUSHLESSLAMP_BOARD_TEYLETEN
    // XIAO on-board user LED is active-LOW on GPIO 21; drive HIGH = off.
    gpio_set_direction((gpio_num_t)PIN_XIAO_USER_LED, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)PIN_XIAO_USER_LED, 1);
#endif

    // Hold a BBPLL consumer so USB-Serial-JTAG SOF auto-removal doesn't park the PLL on host-less boot.
    rtc_clk_bbpll_add_consumer();

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    initArduino();
    delay(500);

    printf("\n=== BrushlessLamp (S3) ===\n");
    printf("[boot] reset_reason=%s (%d)\n",
           reset_reason_str(esp_reset_reason()), (int)esp_reset_reason());

    motor_set_settle_callback(matter_push_level_from_angle);

    // Matter (BLE/Wi-Fi/CHIP) must come up before the FOC busy-wait pegs core 1, or CSRRequest times out (CHIP_ERROR_TIMEOUT 32).
    leds_init();
    matter_app_init();
    leds_start_fader();
    motor_init_and_start();
    input_init();
    input_task_start();

    printf("Ready. Knob = %.2f rad/detent | 1x=on/off | 2x=cycle mode (motor/brightness/CT) | 3x=speed | 9s=factory reset\n",
           KNOB_STEP_RAD);

    vTaskDelete(nullptr);
}
