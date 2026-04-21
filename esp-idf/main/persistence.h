// NVS for the few state items Matter doesn't persist for us (speed_idx, accel).
// Matter persists OnOff/CurrentLevel/ColorTemperatureMireds via its own attribute store.
#pragma once

#include <stdint.h>

void persistence_init();

uint8_t persistence_get_speed_idx();
void    persistence_set_speed_idx(uint8_t idx);

float persistence_get_accel();
void  persistence_set_accel(float accel);
