#pragma once

#include <stdint.h>

// Board pin map. Two supported targets — same chip (ESP32-S3), same firmware
// behavior; only the GPIO numbers used for each peripheral differ because the
// boards expose different pins on different physical positions.
//
//   Signal           XIAO GPIO     Teyleten GPIO
//   ----------       ---------     -------------
//   PIN_ENC_A          1               1
//   PIN_ENC_B          2               2
//   PIN_ROT_A          3               3       (JTAG strap; internal pull-up = OK)
//   PIN_ROT_B          4               4
//   PIN_BTN            5               5
//   PIN_LED_WW         6               6
//   PIN_LED_CW        43               7       (off UART0 TX strap on Teyleten)
//   PIN_DRV_EN        −1 (jumper)      8       (software-controlled on Teyleten)
//   PIN_NSP           44              10       (off UART0 RX strap on Teyleten)
//   PIN_PWM_C          7              11
//   PIN_PWM_B          8              12
//   PIN_PWM_A          9              13
//
// XIAO notes: GPIO 43/44 are UART0 TX/RX straps — bootloader briefly drives them
// (~30 ms TX flicker on LED_CW) and DRV8313 nSLEEP has an external pull-up that
// keeps the driver awake regardless. DRV8313 EN is jumpered to 3V3 on the
// SimpleFOC Mini breakout (no software control).
// Teyleten notes: more GPIOs broken out so we move off the strap pins and gain
// software EN control; GPIO 9 is unused / spare on this layout.
// Switch with `-DBRUSHLESSLAMP_BOARD_TEYLETEN=1` at project COMPILE_OPTIONS.

#ifdef BRUSHLESSLAMP_BOARD_TEYLETEN

constexpr uint8_t PIN_ENC_A   = 1;     // MT6701 A
constexpr uint8_t PIN_ENC_B   = 2;     // MT6701 B
constexpr uint8_t PIN_ROT_A   = 3;     // knob quadrature A (JTAG-select strap, internal pull-up at boot is fine)
constexpr uint8_t PIN_ROT_B   = 4;     // knob quadrature B
constexpr uint8_t PIN_BTN     = 5;     // knob push-button (active-low, INPUT_PULLUP)
constexpr uint8_t PIN_LED_WW  = 6;     // warm-white LEDC
constexpr uint8_t PIN_LED_CW  = 7;     // cool-white LEDC
constexpr int     PIN_DRV_EN  = 8;     // DRV8313 EN (HIGH = outputs enabled, LOW = tri-state)
constexpr uint8_t PIN_NSP     = 10;    // DRV8313 nSLEEP
constexpr uint8_t PIN_PWM_C   = 11;    // DRV8313 IN3
constexpr uint8_t PIN_PWM_B   = 12;    // DRV8313 IN2
constexpr uint8_t PIN_PWM_A   = 13;    // DRV8313 IN1

#else  // default: Seeed XIAO ESP32-S3

// D-label → chip GPIO map for the XIAO breakout.
//   D2 (GPIO 3) is a JTAG-select strap — knob ROT_A sits here; internal pull-up
//     at boot keeps JTAG in its default mode. Safe.
//   D6 (GPIO 43) is UART0 TX — the ROM bootloader prints for ~30 ms at boot,
//     causing a brief flicker on LED_CW. Cosmetic.
//   D7 (GPIO 44) is UART0 RX (input during ROM boot) — DRV8313 nSLEEP has an
//     external pull-up that keeps the driver awake regardless.
constexpr uint8_t PIN_ENC_A   = 1;     // D0 — MT6701 A
constexpr uint8_t PIN_ENC_B   = 2;     // D1 — MT6701 B
constexpr uint8_t PIN_ROT_A   = 3;     // D2 — knob quadrature A
constexpr uint8_t PIN_ROT_B   = 4;     // D3 — knob quadrature B
constexpr uint8_t PIN_BTN     = 5;     // D4 — knob push-button
constexpr uint8_t PIN_LED_WW  = 6;     // D5 — warm-white LEDC
constexpr uint8_t PIN_LED_CW  = 43;    // D6 — cool-white LEDC (UART0 TX strap)
constexpr int     PIN_DRV_EN  = -1;    // sentinel: EN jumpered to 3V3 on the breakout
constexpr uint8_t PIN_NSP     = 44;    // D7 — DRV8313 nSLEEP (UART0 RX strap, external pullup)
constexpr uint8_t PIN_PWM_C   = 7;     // D8 — DRV8313 IN3
constexpr uint8_t PIN_PWM_B   = 8;     // D9 — DRV8313 IN2
constexpr uint8_t PIN_PWM_A   = 9;     // D10 — DRV8313 IN1

#endif
