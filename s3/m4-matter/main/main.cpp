// M4-matter: full stack. Motor + knob + LEDs + Matter ColorTemperatureLight
// endpoint. Commissions via Apple Home / Google Home over WiFi (BLE for pairing),
// persists fabric across reboots. Button: click → brightness mode, 2× → speed
// cycle, 9 s hold → esp_matter::factory_reset.

#include <Arduino.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <esp_private/brownout.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "motor.h"
#include "input.h"
#include "leds.h"
#include "matter_app.h"
#include "config.h"
#include "pins.h"

// Diagnostic: save the current boot's reset reason to NVS so the NEXT boot can
// print the PREVIOUS boot's reason. RTC retention loses the reason across a
// full power-cycle (POR), but NVS persists in flash. Useful for "the chip
// died on external 5 V, plug USB back in to find out what killed it."
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
    // Reference: esp_reset_reason_t — 1=POWERON, 4=SW, 5=DEEPSLEEP,
    //            7=TASK_WDT, 8=WDT, 9=DEEPSLEEP, 10=BROWNOUT, 11=SDIO,
    //            12=USB, 13=JTAG, 14=EFUSE_ERR, 15=PWR_GLITCH, 16=CPU_LOCKUP
}

extern "C" void app_main() {
    // Brownout detector temporarily RE-ENABLED so the next boot's reset reason
    // surfaces "RST_BROWNOUT" if VBUS is dipping during PHY cal. Re-disable
    // once we've confirmed (or ruled out) brownout as the failure mechanism.
    // esp_brownout_disable();

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

    log_prev_reset_reason();

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
    leds_start_fader();
    motor_init_and_start();
    input_init();
    input_task_start();

    // Defer matter_app_init() — it kicks off BLE controller + WiFi PHY init,
    // which fires a ~300-500 mA cal spike on top of the chip baseline. On
    // marginal external 5 V supplies that spike collapsed VBUS during boot.
    // Letting motor / knob / LEDs settle first puts the spike onto a rail
    // that's already proven stable. ESP_PHY_MAX_WIFI_TX_POWER does NOT bound
    // the initial cal — only deferring it does.
    xTaskCreatePinnedToCore([](void *) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        matter_app_init();
        vTaskDelete(nullptr);
    }, "matter_late", 6144, nullptr, 3, nullptr, CORE_OTHERS);

    printf("Ready. Knob = %.2f rad/detent | 1×=on/off | 2×=mode | 3×=speed | 9s=factory reset\n",
           KNOB_STEP_RAD);

    vTaskDelete(nullptr);
}
