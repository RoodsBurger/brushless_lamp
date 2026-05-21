#pragma once

#include <stdint.h>

// CCT LED driver: angle-curve fade on a 10 ms task, gamma-2.2 brightness,
// pulse task for button feedback. Matter writes drive on / colortemp;
// knob nudges drive max_duty.

void leds_init();
void leds_start_fader();

void leds_set_colortemp(uint16_t mireds);
void leds_set_max_duty(uint8_t duty);
uint8_t leds_get_max_duty();
void leds_nudge_max_duty(int16_t delta);

void leds_pulse(uint8_t count);
