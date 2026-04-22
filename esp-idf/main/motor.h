#pragma once

#include <stdint.h>

// Phase 14c public motor API. Position-control mode: Matter Level / knob detents set a
// target shaft angle in radians; SimpleFOC's cascaded angle→velocity→voltage PIDs slew
// the rotor there. Knob/button live in input.cpp and call these; the FOC inner loop
// runs in motor_foc_task (FreeRTOS priority 5).

void  motor_init_and_start();

void  motor_enable();                        // OnOff=on:  enable motor, hold target
void  motor_disable();                       // OnOff=off: free-spin rotor

void  motor_set_target_angle(float rad);     // absolute target, clamped to [0, ANGLE_MAX]
void  motor_nudge_target_angle(float d_rad); // knob-driven relative tweak
float motor_get_target_angle();
float motor_get_shaft_angle();
float motor_get_shaft_velocity();
