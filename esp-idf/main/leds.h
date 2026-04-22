#pragma once

#include <stdint.h>

// Phase 14b: dual-channel CCT LED driver on LEDC.
// WW = warm white (D5/GPIO 23), CW = cool white (D6/GPIO 16). Duty 0–255.
// `leds_init()` sets up both channels; `leds_set(warm, cool)` updates duty.
// `leds_start_angle_follower()` spawns a low-priority 2 Hz task that turns the
// LEDs on/off based on shaft angle — a visual sanity check that LED writes
// don't perturb the FOC loop on the C6's softfloat CPU. Matter (14c) will
// drive the LED directly from attribute changes, displacing this follower.

void leds_init();
void leds_set(uint8_t warm, uint8_t cool);
void leds_start_angle_follower();
