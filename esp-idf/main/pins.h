#pragma once

#include <stdint.h>

// XIAO ESP32-C6 → SimpleFOC Mini (DRV8313) + MT6701 ABZ + knob + button + LEDs.
// XIAO D-labels shown in comments for soldering reference; values are chip GPIOs.
constexpr uint8_t PIN_PWM_A = 18;   // D10 — DRV8313 IN1
constexpr uint8_t PIN_PWM_B = 20;   // D9  — DRV8313 IN2
constexpr uint8_t PIN_PWM_C = 19;   // D8  — DRV8313 IN3
constexpr uint8_t PIN_NSP   = 17;   // D7  — DRV8313 nSLEEP (EN is hardwired high)

constexpr uint8_t PIN_ENC_A = 0;    // D0  — MT6701 A
constexpr uint8_t PIN_ENC_B = 1;    // D1  — MT6701 B

constexpr uint8_t PIN_ROT_A = 2;    // D2  — knob quadrature A
constexpr uint8_t PIN_ROT_B = 21;   // D3  — knob quadrature B
constexpr uint8_t PIN_BTN   = 22;   // D4  — knob push-button (active-low, internal pull-up)

constexpr uint8_t PIN_LED_WW = 23;  // D5  — warm-white LEDC channel (Phase 14b)
constexpr uint8_t PIN_LED_CW = 16;  // D6  — cool-white LEDC channel (Phase 14b)
