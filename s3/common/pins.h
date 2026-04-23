#pragma once

#include <stdint.h>

// Seeed XIAO ESP32-S3 — D-label → chip GPIO map. Breakout is wired by D-label, so
// only the GPIO constants shift versus the C6 build; the mechanical harness stays the
// same. Research summary (see PHASE15-S3-MIGRATION.md §"Pin map"):
//   D2 (GPIO3) is a JTAG-select strap — knob ROT_A sits here and is pulled HIGH via
//     internal pull-up at boot, which leaves JTAG in its default "on internal pad"
//     mode. Safe but worth noting.
//   D6 (GPIO43) is UART0 TX; the ROM bootloader prints on it for ~30 ms at power-up
//     and then the pin is free. Cosmetic flicker on LED_CW only.
//   D7 (GPIO44) is UART0 RX; input during ROM boot → harmless. External pull-up on
//     DRV8313 nSLEEP keeps the driver awake regardless.
constexpr uint8_t PIN_ENC_A  = 1;    // D0 — MT6701 A
constexpr uint8_t PIN_ENC_B  = 2;    // D1 — MT6701 B

constexpr uint8_t PIN_ROT_A  = 3;    // D2 — knob quadrature A (JTAG strap, pullup OK)
constexpr uint8_t PIN_ROT_B  = 4;    // D3 — knob quadrature B
constexpr uint8_t PIN_BTN    = 5;    // D4 — knob push-button (active-low, pullup)

constexpr uint8_t PIN_LED_WW = 6;    // D5 — warm-white LEDC
constexpr uint8_t PIN_LED_CW = 43;   // D6 — cool-white LEDC (UART0 TX; harmless after boot)

constexpr uint8_t PIN_NSP    = 44;   // D7 — DRV8313 nSLEEP (UART0 RX; external pullup holds HIGH)

constexpr uint8_t PIN_PWM_C  = 7;    // D8 — DRV8313 IN3
constexpr uint8_t PIN_PWM_B  = 8;    // D9 — DRV8313 IN2
constexpr uint8_t PIN_PWM_A  = 9;    // D10 — DRV8313 IN1
