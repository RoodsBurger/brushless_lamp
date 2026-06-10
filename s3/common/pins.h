#pragma once

#include <stdint.h>

// Board pin map. Three supported targets — same chip (ESP32-S3), same firmware
// behavior; only the GPIO numbers used for each peripheral differ because the
// boards expose different pins on different physical positions.
//
//   Signal           XIAO GPIO     Teyleten GPIO   Custom PCB GPIO
//   ----------       ---------     -------------   ---------------
//   PIN_ENC_A          1               1              17
//   PIN_ENC_B          2               2              18
//   PIN_ROT_A          3               3       (JTAG strap)  8
//   PIN_ROT_B          4               4               9
//   PIN_BTN            5               5              10
//   PIN_LED_WW        43               7              11
//   PIN_LED_CW         6               6              12
//   PIN_DRV_EN        −1 (jumper)      8              15      (software-controlled)
//   PIN_NSP           44              10               7
//   PIN_PWM_C          7              11               6
//   PIN_PWM_B          9              13               4     (A/B swapped vs DRV8313 IN1/IN2 to invert rotor direction)
//   PIN_PWM_A          8              12               5     (A/B swapped vs DRV8313 IN1/IN2 to invert rotor direction)
//
// XIAO notes: GPIO 43/44 are UART0 TX/RX straps — bootloader briefly drives them
// (~30 ms TX flicker on LED_WW) and DRV8313 nSLEEP has an external pull-up that
// keeps the driver awake regardless. DRV8313 EN is jumpered to 3V3 on the
// SimpleFOC Mini breakout (no software control).
// Teyleten notes: more GPIOs broken out so we move off the strap pins and gain
// software EN control; GPIO 9 is unused / spare on this layout.
// Custom-PCB notes: the all-in-one WROOM-1-N8R8 board (design package in PCB/).
// Adds nFAULT read-back (GPIO16) and a Z-index pin (GPIO14), both wired but unused
// by current firmware. Map mirrors PCB/01-architecture-and-pinmap.md §4 exactly.
// *** UNVERIFIED ON HARDWARE — the board hasn't been fabbed yet. This branch is
// compile-checked and matches the schematic net-for-net; confirm on the real
// board at bring-up before trusting it. ***
// Select a non-default board with `-DBRUSHLESSLAMP_BOARD_TEYLETEN=1` or
// `-DBRUSHLESSLAMP_BOARD_CUSTOM=1` at project COMPILE_OPTIONS.

#ifdef BRUSHLESSLAMP_BOARD_TEYLETEN

constexpr uint8_t PIN_ENC_A   = 1;     // MT6701 A
constexpr uint8_t PIN_ENC_B   = 2;     // MT6701 B
constexpr uint8_t PIN_ROT_A   = 3;     // knob quadrature A (JTAG-select strap, internal pull-up at boot is fine)
constexpr uint8_t PIN_ROT_B   = 4;     // knob quadrature B
constexpr uint8_t PIN_BTN     = 5;     // knob push-button (active-low, INPUT_PULLUP)
constexpr uint8_t PIN_LED_WW  = 7;     // warm-white LEDC
constexpr uint8_t PIN_LED_CW  = 6;     // cool-white LEDC
constexpr int     PIN_DRV_EN  = 8;     // DRV8313 EN (HIGH = outputs enabled, LOW = tri-state)
constexpr uint8_t PIN_NSP     = 10;    // DRV8313 nSLEEP
constexpr uint8_t PIN_PWM_C   = 11;    // DRV8313 IN3
constexpr uint8_t PIN_PWM_B   = 13;    // DRV8313 IN1 — swapped with A to invert rotor direction
constexpr uint8_t PIN_PWM_A   = 12;    // DRV8313 IN2 — swapped with B to invert rotor direction

#elif defined(BRUSHLESSLAMP_BOARD_CUSTOM)   // all-in-one WROOM-1-N8R8 board (see PCB/) — UNVERIFIED until fabbed

constexpr uint8_t PIN_ENC_A      = 17;   // MT6701 A (breakout via J_SENSOR)
constexpr uint8_t PIN_ENC_B      = 18;   // MT6701 B
constexpr uint8_t PIN_ENC_Z      = 14;   // MT6701 Z index — wired but unused by current firmware
constexpr uint8_t PIN_ROT_A      = 8;    // knob quadrature A
constexpr uint8_t PIN_ROT_B      = 9;    // knob quadrature B
constexpr uint8_t PIN_BTN        = 10;   // knob push-button (active-low, INPUT_PULLUP)
constexpr uint8_t PIN_LED_WW     = 11;   // warm-white LEDC
constexpr uint8_t PIN_LED_CW     = 12;   // cool-white LEDC
constexpr int     PIN_DRV_EN     = 15;   // DRV8313 EN (software-controlled; 0Ω-to-3V3 jumper option on board)
constexpr int     PIN_DRV_NFAULT = 16;   // DRV8313 nFAULT (read-only, active-low) — wired but unused by current firmware
constexpr uint8_t PIN_NSP        = 7;    // DRV8313 nSLEEP
constexpr uint8_t PIN_PWM_C      = 6;    // DRV8313 IN3
constexpr uint8_t PIN_PWM_B      = 4;    // DRV8313 IN1 — swapped with A to invert rotor direction
constexpr uint8_t PIN_PWM_A      = 5;    // DRV8313 IN2 — swapped with B to invert rotor direction
constexpr uint8_t PIN_STATUS_LED = 21;   // optional board status LED (active-HIGH) — unused by current firmware

#else  // default: Seeed XIAO ESP32-S3

constexpr uint8_t PIN_ENC_A         = 1;     // D0 — MT6701 A
constexpr uint8_t PIN_ENC_B         = 2;     // D1 — MT6701 B
constexpr uint8_t PIN_ROT_A         = 3;     // D2 — knob A (JTAG-select strap, INPUT_PULLUP safe)
constexpr uint8_t PIN_ROT_B         = 4;     // D3 — knob B
constexpr uint8_t PIN_BTN           = 5;     // D4 — knob push-button
constexpr uint8_t PIN_LED_WW        = 43;    // D6 — warm-white LEDC (UART0 TX strap; ~30 ms flicker at boot)
constexpr uint8_t PIN_LED_CW        = 6;     // D5 — cool-white LEDC
constexpr int     PIN_DRV_EN        = -1;    // sentinel: EN jumpered to 3V3 on the breakout
constexpr uint8_t PIN_NSP           = 44;    // D7 — DRV8313 nSLEEP (UART0 RX strap, external pullup)
constexpr uint8_t PIN_PWM_C         = 7;     // D8 — DRV8313 IN3
constexpr uint8_t PIN_PWM_B         = 9;     // D10 — DRV8313 IN1 (A/B swapped to invert rotor direction)
constexpr uint8_t PIN_PWM_A         = 8;     // D9 — DRV8313 IN2 (A/B swapped to invert rotor direction)
constexpr uint8_t PIN_XIAO_USER_LED = 21;    // XIAO on-board user LED (active-LOW)

#endif
