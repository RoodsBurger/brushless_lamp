// M4-matter: ColorTemperatureLight Matter endpoint + knob/button local control.

#include <Arduino.h>
#include <driver/gpio.h>
#include <driver/gptimer.h>
#include <esp_private/rtc_clk.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <soc/gpio_struct.h>

#include "config.h"
#include "input.h"
#include "leds.h"
#include "matter_app.h"
#include "motor.h"
#include "pins.h"

// 100 ms heartbeat from gptimer ISR; ISR-driven so a stuck LED means a hard
// CPU/IRQ wedge rather than just a starved task.
static bool IRAM_ATTR hb_isr_cb(gptimer_handle_t, const gptimer_alarm_event_data_t *, void *) {
    static uint32_t n = 0;
    if (++n & 1) GPIO.out_w1tc = (1U << 21);
    else         GPIO.out_w1ts = (1U << 21);
    return false;
}

static void hb_isr_init() {
    gpio_set_direction((gpio_num_t)21, GPIO_MODE_OUTPUT);
    GPIO.out_w1ts = (1U << 21);
    gptimer_handle_t t = nullptr;
    gptimer_config_t cfg = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1 * 1000 * 1000,
        .intr_priority = 0,
        .flags = {},
    };
    gptimer_new_timer(&cfg, &t);
    gptimer_event_callbacks_t cbs = { .on_alarm = hb_isr_cb };
    gptimer_register_event_callbacks(t, &cbs, nullptr);
    gptimer_enable(t);
    gptimer_alarm_config_t alarm = {
        .alarm_count = 100000,
        .reload_count = 0,
        .flags = { .auto_reload_on_alarm = true },
    };
    gptimer_set_alarm_action(t, &alarm);
    gptimer_start(t);
}

extern "C" void app_main() {
    // nSLEEP has an external pullup on XIAO; park the DRV8313 asleep before the
    // bootloader release lets it pull quiescent current.
    gpio_set_direction((gpio_num_t)PIN_NSP, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)PIN_NSP, 0);

    // Keep BBPLL consumer count >= 1 against USB-Serial-JTAG SOF auto-removal on host-less boot.
    rtc_clk_bbpll_add_consumer();

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    hb_isr_init();
    initArduino();

    printf("\n=== BrushlessLamp M4-matter (S3) ===\n");

    // Wire the settle callback before motor init so the first calibration push fires cleanly.
    motor_set_settle_callback(matter_push_level_from_angle);

    leds_init();
    leds_start_fader();
    motor_init_and_start();
    input_init();
    input_task_start();

    // Defer Matter init 5 s so motor / knob / LEDs are interactive immediately.
    xTaskCreatePinnedToCore([](void *) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        matter_app_init();
        vTaskDelete(nullptr);
    }, "matter_late", 6144, nullptr, 3, nullptr, CORE_OTHERS);

    printf("Ready. Knob = %.2f rad/detent | 1x=on/off | 2x=mode | 3x=speed | 9s=factory reset\n",
           KNOB_STEP_RAD);

    vTaskDelete(nullptr);
}
