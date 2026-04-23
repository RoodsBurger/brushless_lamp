#pragma once

#include <stdint.h>

// CCT LED driver for M3. WW = warm white, CW = cool white, both driven via LEDC
// at 25 kHz / 8-bit. The follower task reads motor_get_shaft_angle() at 10 Hz and
// turns LEDs on at a fixed 50/50 mix whenever the lamp is above the threshold —
// M3 has no color-temp input yet (that comes with Matter in M4).
//
// Task pinned to CORE_OTHERS so the ledcWrite() path can't steal from the motor
// FOC loop.

void leds_init();
void leds_set(uint8_t warm, uint8_t cool);
void leds_start_angle_follower();
