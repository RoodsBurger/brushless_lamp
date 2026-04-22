#pragma once

#include <stdint.h>

// Phase 14a public motor API. Knob/button input lives in input.cpp and calls these.
// The actual FOC loop runs in its own FreeRTOS task (motor_foc_task) at priority 5.

void  motor_init_and_start();
void  motor_set_target_velocity(float rad_per_sec);
float motor_get_target_velocity();
float motor_get_shaft_velocity();
float motor_get_shaft_angle();
void  motor_stop();              // target = 0, no hard-disable
