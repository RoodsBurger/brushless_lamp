// Pin map — XIAO ESP32-C6 ↔ peripherals (mirrors arduino sketch's PIN_* constants)
#pragma once

#include <stdint.h>

constexpr uint8_t PIN_PWM_A = 18;    // D10 — DRV8313 IN1 (MCPWM phase A)
constexpr uint8_t PIN_PWM_B = 20;    // D9  — DRV8313 IN2 (MCPWM phase B)
constexpr uint8_t PIN_PWM_C = 19;    // D8  — DRV8313 IN3 (MCPWM phase C)
constexpr uint8_t PIN_NSP   = 17;    // D7  — DRV8313 nSLEEP (pulse LOW ≥30 µs to clear latched fault)

constexpr uint8_t PIN_ENC_A = 0;     // D0  — MT6701 A (motor encoder)
constexpr uint8_t PIN_ENC_B = 1;     // D1  — MT6701 B

constexpr uint8_t PIN_ROT_A = 2;     // D2 — rotary knob A (PCNT)
constexpr uint8_t PIN_ROT_B = 21;    // D3 — rotary knob B (PCNT)
constexpr uint8_t PIN_BTN   = 22;    // D4 — knob push-button (active low, internal pull-up)

constexpr uint8_t PIN_LED_WW = 23;   // D5 — AO3400A gate, warm-white channel (LEDC)
constexpr uint8_t PIN_LED_CW = 16;   // D6 — AO3400A gate, cool-white channel (LEDC)
