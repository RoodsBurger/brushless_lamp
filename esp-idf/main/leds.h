// Two-channel LED PWM (warm + cool white). Brightness gated by motor shaft angle so
// the lit core fades on as it rises through 0.5..3 rad. Configured brightness/cct ramped
// at LED_RAMP_PER_SEC so step changes look smooth.
#pragma once

#include <stdint.h>

void leds_init();

// Configured brightness (0..100) and CCT mix (0..100, 0=WW, 100=CW). Updated by knob,
// matter callback, or serial command.
void leds_set_brightness(uint8_t bri);
void leds_set_cct(uint8_t cct);
uint8_t leds_get_brightness();
uint8_t leds_get_cct();

// Re-render PWM duties from current effective brightness/cct + given shaft angle.
// Call from the main loop / motor task.
void leds_update(float shaft_angle);

// Blocking N× warning pulse — smooth start, gamma curve, smooth end. Used for the
// 5 s long-press factory-reset warning.
void leds_pulse(uint8_t n);
