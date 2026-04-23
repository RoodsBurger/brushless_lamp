#pragma once

#include <stdint.h>

// M4 CCT LED driver — shares the exact module contract with M3. Continuous angle
// ramp (peak_for_angle), gamma-2.2 on max_duty, 10 ms fader on core 0, pulse task
// for feedback. Matter attribute writes come in via leds_set_on() and
// leds_set_colortemp() from matter_app.cpp; brightness-mode knob nudges via
// leds_nudge_max_duty() from input.cpp. Intentionally identical to s3/m3-leds
// so both stages stay in lockstep on LED behavior.

void leds_init();
void leds_start_fader();

void leds_set_on(bool on);
void leds_set_colortemp(uint16_t mireds);
void leds_set_max_duty(uint8_t duty);
uint8_t leds_get_max_duty();
void leds_nudge_max_duty(int16_t delta);

void leds_pulse(uint8_t count);
