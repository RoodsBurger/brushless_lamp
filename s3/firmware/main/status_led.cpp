// Status LED on the custom PCB (GPIO21, active-HIGH via 1 kΩ to GND). A 100 ms
// tick task on core 0 renders one pattern by priority:
//   fault (init failure or DRV8313 nFAULT while driving)  5 Hz blink
//   booting (Matter stack not up yet)                     solid on
//   uncommissioned (pairing window)                       1 Hz blink
//   commissioned but WiFi down                            2.5 Hz blink
//   motor actively moving                                 solid on
//   idle operational                                      off

#include "status_led.h"

#ifdef BRUSHLESSLAMP_BOARD_CUSTOM

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "matter_app.h"
#include "motor.h"
#include "pins.h"

static volatile bool s_matter_ready = false;

// nFAULT is open-drain active-low with an external pull-up (R_NFAULT). It is only
// meaningful while the driver is awake — nSLEEP low at idle leaves it indeterminate.
static bool nfault_active() {
    return !motor_is_idle() && gpio_get_level((gpio_num_t)PIN_DRV_NFAULT) == 0;
}

static void status_led_task(void *) {
    uint32_t tick = 0;
    for (;;) {
        bool on;
        if (motor_get_fault() || nfault_active()) on = tick & 1;         // 5 Hz: fault
        else if (!s_matter_ready)                 on = true;             // solid: booting
        else if (!matter_is_commissioned())       on = (tick % 10) < 5;  // 1 Hz: pairing
        else if (!matter_is_wifi_up())            on = (tick % 4) < 2;   // 2.5 Hz: WiFi down
        else if (!motor_is_idle())                on = true;             // solid: motor moving
        else                                      on = false;            // off: idle
        gpio_set_level((gpio_num_t)PIN_STATUS_LED, on);
        tick++;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void status_led_init() {
    gpio_set_direction((gpio_num_t)PIN_STATUS_LED, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)PIN_STATUS_LED, 1);   // proof of life from power-on
    gpio_set_direction((gpio_num_t)PIN_DRV_NFAULT, GPIO_MODE_INPUT);
    xTaskCreatePinnedToCore(status_led_task, "status_led", 2048, nullptr, 1, nullptr, 0);
}

void status_led_set_matter_ready() { s_matter_ready = true; }

#else  // XIAO / Teyleten: no managed status LED

void status_led_init() {}
void status_led_set_matter_ready() {}

#endif
