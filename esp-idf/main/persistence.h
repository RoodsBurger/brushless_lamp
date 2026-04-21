// NVS for the few state items Matter doesn't persist for us (speed_idx, accel).
// Matter persists OnOff/CurrentLevel/ColorTemperatureMireds via its own attribute store.
#pragma once

#include <stdint.h>

void persistence_init();

uint8_t persistence_get_speed_idx();
void    persistence_set_speed_idx(uint8_t idx);

float persistence_get_accel();
void  persistence_set_accel(float accel);

// FOC calibration — zero_electric_angle and sensor_direction from initFOC's alignSensor.
// Persisting means we don't re-run the noisy single-point alignment on every boot; the
// saved offset is reused instead, which the user experiences as consistent motor feel
// across reboots. Returns true if a valid record was loaded.
bool persistence_get_foc_calibration(float *zero_elec, int8_t *sensor_dir);
void persistence_set_foc_calibration(float  zero_elec, int8_t  sensor_dir);
void persistence_clear_foc_calibration();
