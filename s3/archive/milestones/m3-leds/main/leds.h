#pragma once

#include <stdint.h>

// CCT LED driver for M3+. Ported from CylinderLamp: linear time-based fade
// (LED_FADE_STEP / LED_FADE_PERIOD_MS) on both warm and cool channels, warm/cool
// blend controlled by a 100..500 color-temperature value, user-adjustable max
// duty ("brightness mode" via single click). Pulse feedback for button events
// runs in a dedicated short-lived task so it can't stall the fader or the
// input loop.
//
// Task pinning: fader on CORE_OTHERS @ 10 ms, pulse task also CORE_OTHERS and
// preempts the fader via a flag. All LEDC writes stay off the motor core.

void leds_init();
void leds_start_fader();           // 10 ms fader task; call once in app_main

// Target state — each setter updates a target; the fader ramps current → target.
void leds_set_on(bool on);         // drives both channels to 0 or (max_duty, ct)
void leds_set_colortemp(uint16_t mireds);  // 100..500 (clamped); M4 binds this to Matter
void leds_set_max_duty(uint8_t duty);       // 0..255 brightness cap
uint8_t leds_get_max_duty();
void leds_nudge_max_duty(int16_t delta);    // knob nudge in brightness mode (signed)

// Fire-and-forget feedback pulse (count = CylinderLamp's "speed index + 1" pattern).
// Spawns a one-shot task that briefly takes LEDC ownership; the fader resumes on return.
void leds_pulse(uint8_t count);
