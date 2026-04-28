// M4-matter: full stack. Motor + knob + LEDs + Matter ColorTemperatureLight
// endpoint. Commissions via Apple Home / Google Home over WiFi (BLE for pairing),
// persists fabric across reboots. Button: 1×=on/off, 2×=mode toggle, 3×=speed
// cycle, 9 s hold = esp_matter::factory_reset.

#include <Arduino.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "motor.h"
#include "input.h"
#include "leds.h"
#include "matter_app.h"
#include "config.h"
#include "pins.h"

// Save the current boot's reset reason to NVS so the NEXT boot can print the
// PREVIOUS boot's reason. RTC retention loses it across a full power-cycle
// (POR), but NVS persists in flash — the only way to know what killed a boot
// after the chip has been fully unplugged.
static void log_prev_reset_reason() {
    nvs_handle_t h;
    if (nvs_open("boot_dbg", NVS_READWRITE, &h) != ESP_OK) return;
    uint32_t prev_rst = 0xFF, boot_n = 0;
    nvs_get_u32(h, "last_rst", &prev_rst);
    nvs_get_u32(h, "boot_n",   &boot_n);
    uint32_t cur_rst = (uint32_t)esp_reset_reason();
    boot_n++;
    nvs_set_u32(h, "last_rst", cur_rst);
    nvs_set_u32(h, "boot_n",   boot_n);
    nvs_commit(h);
    nvs_close(h);
    printf("[boot_dbg] cur_reset=0x%02X prev_reset=0x%02X boot#=%u\n",
           (unsigned)cur_rst, (unsigned)prev_rst, (unsigned)boot_n);
}

extern "C" void app_main() {
    // Park the DRV8313 asleep before anything else runs — nSLEEP has an external
    // pullup on XIAO so without this the driver is awake from POR through the
    // bootloader, pulling quiescent current. motor_foc_task wakes it later.
    gpio_set_direction((gpio_num_t)PIN_NSP, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)PIN_NSP, 0);

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    log_prev_reset_reason();

    initArduino();

    printf("\n=== BrushlessLamp M4-matter (Teyleten S3 SuperMini) ===\n");

    // Wire the knob → Matter push BEFORE motor starts so the first settle fires a
    // clean update.
    motor_set_settle_callback(matter_push_level_from_angle);

    leds_init();
    leds_start_fader();
    motor_init_and_start();
    input_init();
    input_task_start();

    // Defer Matter bring-up by 5 s onto a low-priority core-0 task so motor,
    // knob, and LEDs are interactive immediately. The CPU lockup on external
    // 5 V (see s3/README.md § 7.1) lands here when it triggers, so local
    // control survives even when Matter init wedges the radios.
    xTaskCreatePinnedToCore([](void *) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        matter_app_init();
        vTaskDelete(nullptr);
    }, "matter_late", 6144, nullptr, 3, nullptr, CORE_OTHERS);

    printf("Ready. Knob = %.2f rad/detent | 1×=on/off | 2×=mode | 3×=speed | 9s=factory reset\n",
           KNOB_STEP_RAD);

    vTaskDelete(nullptr);
}
