// Pure-IDF blink test. No Arduino, no Matter, no SimpleFOC, no nothing.
// Drives PIN_LED_WW (GPIO 6) and PIN_LED_CW (GPIO 43) at 1 Hz alternation so
// the user can visibly verify the board is alive from inside the lamp housing
// (the XIAO's onboard CHG / user LEDs may not be visible there). Also toggles
// GPIO 21 (XIAO S3 user LED, active LOW) for direct chip-side proof.
//
// Purpose: power the board from external 5 V (industrial 24→5 V converter +
// bulk cap + Schottky → VBUS pad) with no USB attached. If the LEDs blink,
// hardware is fine and the M4 build's failure is a software regression. If
// the LEDs don't blink, the failure is unambiguously a power-rail problem.

#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

constexpr gpio_num_t PIN_LED_WW    = (gpio_num_t)6;     // CCT warm-white channel
constexpr gpio_num_t PIN_LED_CW    = (gpio_num_t)43;    // CCT cool-white channel (also UART0 TX)
constexpr gpio_num_t PIN_USER_LED  = (gpio_num_t)21;    // XIAO S3 onboard user LED (active LOW)
constexpr gpio_num_t PIN_NSP       = (gpio_num_t)44;    // DRV8313 nSLEEP — keep LOW so the driver stays asleep

extern "C" void app_main() {
    // Park the motor driver. Same rationale as the M4 build — without this the
    // external pullup keeps the DRV8313 awake at boot.
    gpio_set_direction(PIN_NSP, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NSP, 0);

    gpio_set_direction(PIN_LED_WW,   GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_LED_CW,   GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_USER_LED, GPIO_MODE_OUTPUT);

    printf("\n=== BrushlessLamp test blink ===\n");
    printf("If you see this, USB-CDC is up and the chip booted normally.\n");
    printf("Watch the lamp's LEDs alternating at 1 Hz.\n");

    bool phase = false;
    while (true) {
        gpio_set_level(PIN_LED_WW,    phase ? 1 : 0);
        gpio_set_level(PIN_LED_CW,    phase ? 0 : 1);
        gpio_set_level(PIN_USER_LED,  phase ? 0 : 1);   // active-LOW
        phase = !phase;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
