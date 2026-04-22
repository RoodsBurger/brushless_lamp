#pragma once

// Phase 14c: Matter ColorTemperatureLight endpoint.
// On/Off      → motor enable + LED master
// LevelControl → motor velocity target (0..254 maps to 0..VELOCITY_LIMIT rad/s)
// ColorControl (ColorTemperature) → LED warm/cool balance
// Must be called after initArduino() + motor_init_and_start() + leds_init() so the
// callbacks have live peripherals to drive.

void matter_app_init();

// Accessors used by leds.cpp's angle-follower to know which color temperature to mix
// the LEDs at when the shaft is lifted above LED_ON_ANGLE_THRESH.
#ifdef __cplusplus
extern "C" {
#endif
unsigned short matter_get_color_temp_mireds(void);
bool           matter_get_on_off(void);

// Push a local (knob-driven) angle change back to Matter so the controller app's
// slider position stays in sync with the physical lamp. Safe to call from a task
// context — goes through esp_matter::attribute::report().
void matter_push_level_from_angle(float angle_rad);
#ifdef __cplusplus
}
#endif
