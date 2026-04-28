// M4-matter: full stack. Motor + knob + LEDs + Matter ColorTemperatureLight
// endpoint. Commissions via Apple Home / Google Home over WiFi (BLE for pairing),
// persists fabric across reboots. Button: 1×=on/off, 2×=mode toggle, 3×=speed
// cycle, 9 s hold = esp_matter::factory_reset.

#include <Arduino.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_private/rtc_clk.h>
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

// Save the current boot's reset reason + last beacon checkpoint + last CHIP
// event to NVS so the NEXT boot can print the PREVIOUS boot's progress. RTC
// retention loses these across a full power-cycle (POR), but NVS persists in
// flash — the only way to know what killed a boot after the chip has been
// fully unplugged.
static void log_prev_reset_reason() {
    nvs_handle_t h;
    if (nvs_open("boot_dbg", NVS_READWRITE, &h) != ESP_OK) return;
    uint32_t prev_rst = 0xFF, boot_n = 0, prev_ckpt = 0;
    uint32_t prev_evt = 0, prev_evt_n = 0, prev_evt_ms = 0;
    nvs_get_u32(h, "last_rst",    &prev_rst);
    nvs_get_u32(h, "boot_n",      &boot_n);
    nvs_get_u32(h, "last_ckpt",   &prev_ckpt);
    nvs_get_u32(h, "last_evt",    &prev_evt);
    nvs_get_u32(h, "last_evt_n",  &prev_evt_n);
    nvs_get_u32(h, "last_evt_ms", &prev_evt_ms);
    uint32_t cur_rst = (uint32_t)esp_reset_reason();
    boot_n++;
    nvs_set_u32(h, "last_rst",    cur_rst);
    nvs_set_u32(h, "boot_n",      boot_n);
    nvs_set_u32(h, "last_ckpt",   0);   // beacons overwrite as they fire
    nvs_set_u32(h, "last_evt",    0);
    nvs_set_u32(h, "last_evt_n",  0);
    nvs_set_u32(h, "last_evt_ms", 0);
    nvs_commit(h);
    nvs_close(h);
    printf("[boot_dbg] cur_reset=0x%02X prev_reset=0x%02X boot#=%u prev_ckpt=%u prev_evt=0x%X (n=%u, t=%ums)\n",
           (unsigned)cur_rst, (unsigned)prev_rst, (unsigned)boot_n,
           (unsigned)prev_ckpt, (unsigned)prev_evt, (unsigned)prev_evt_n, (unsigned)prev_evt_ms);
}

extern "C" void app_main() {
    // Park the DRV8313 asleep before anything else runs — nSLEEP has an external
    // pullup on XIAO so without this the driver is awake from POR through the
    // bootloader, pulling quiescent current. motor_foc_task wakes it later.
    gpio_set_direction((gpio_num_t)PIN_NSP, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)PIN_NSP, 0);

    // Pin BBPLL alive against the USB-Serial-JTAG SOF auto-removal. With
    // CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y the IDF connection monitor adds
    // BBPLL as a consumer at boot, then drops it ~3 ms later if no SOF
    // packets are seen. On external power (no USB host) that drops the ref
    // count to 0, BBPLL gets disabled at the next CPU clock switch, and
    // Wi-Fi/BLE radio init crashes silently around uptime 9.65 s. Adding our
    // own consumer keeps the count ≥ 1 forever, regardless of host presence.
    rtc_clk_bbpll_add_consumer();

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // DIAG: let USB-CDC re-enumerate after reset (~600 ms on macOS) so the
    // boot_dbg line and banner aren't dropped before the host reconnects.
    vTaskDelay(pdMS_TO_TICKS(2000));

    log_prev_reset_reason();

    initArduino();

    // Per-tag runtime overrides for diagnostic. Global default stays WARN
    // (sdkconfig.defaults) so USB-CDC doesn't backpressure when no host reads.
    esp_log_level_set("phy",    ESP_LOG_DEBUG);
    esp_log_level_set("wifi",   ESP_LOG_DEBUG);
    esp_log_level_set("BT",     ESP_LOG_DEBUG);
    esp_log_level_set("NimBLE", ESP_LOG_DEBUG);
    esp_log_level_set("CHIP",   ESP_LOG_DEBUG);
    esp_log_level_set("matter", ESP_LOG_DEBUG);

    printf("\n=== BrushlessLamp M4-matter (S3) ===\n");

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

    // Core-0 liveness probe — toggles the on-board user LED (GPIO 21, active-LOW)
    // every 200 ms with a short 20 ms pulse. If GPIO 21 stops blinking and the
    // matter-init beacon trace also stalls, all of core 0 is wedged.
    xTaskCreatePinnedToCore([](void *) {
        gpio_set_direction((gpio_num_t)21, GPIO_MODE_OUTPUT);
        gpio_set_level((gpio_num_t)21, 1);
        while (true) {
            gpio_set_level((gpio_num_t)21, 0);
            vTaskDelay(pdMS_TO_TICKS(20));
            gpio_set_level((gpio_num_t)21, 1);
            vTaskDelay(pdMS_TO_TICKS(180));
        }
    }, "hb", 2048, nullptr, 1, nullptr, CORE_OTHERS);

    printf("Ready. Knob = %.2f rad/detent | 1×=on/off | 2×=mode | 3×=speed | 9s=factory reset\n",
           KNOB_STEP_RAD);

    vTaskDelete(nullptr);
}
