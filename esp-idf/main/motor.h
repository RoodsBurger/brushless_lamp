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

// Ask the motor task to push the post-settle shaft angle back to Matter the next time
// the deadband is entered. Called from input.cpp when the user turns the knob —
// pushing on every detent creates an echo loop via attribute_update_cb that the knob
// sees as mid-movement rewind. Deferring to settle avoids that round trip.
void  motor_request_matter_sync_on_settle();
