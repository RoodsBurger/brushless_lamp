#pragma once

#include <stdint.h>

// M4 LEDs — angle-driven on/off, warm/cool mix from Matter's ColorTemperatureMireds
// attribute (read from matter_app's matter_get_color_temp_mireds() getter). The
// follower task on core 0 polls motor_get_shaft_angle() at 10 Hz and writes LEDC
// only on change, so there's minimal pressure on arduino-esp32 LEDC internals.

void leds_init();
void leds_set(uint8_t warm, uint8_t cool);
void leds_start_angle_follower();
