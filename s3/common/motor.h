#pragma once

#include <stdint.h>

// Shared motor control API for all M-stages (m1-motor through m4-matter). The S3 port
// uses SimpleFOC v2.4.0 in velocity mode with M2-knob's proven-quiet PID, and closes
// the position loop in the app-layer task via a linear+physics-bounded trapezoidal
// profile. The FOC task is pinned to core 1 (CORE_MOTOR) and calls attachInterrupt
// from inside its own task body so the encoder ISR lives on the same core as the
// control loop — arduino-esp32's noInterrupts() is per-core only.

void  motor_init_and_start();

void  motor_set_target_angle(float rad);      // absolute target, clamped to [0, ANGLE_MAX]
void  motor_nudge_target_angle(float d_rad);  // knob-driven relative tweak
float motor_get_target_angle();
float motor_get_shaft_angle();
float motor_get_shaft_velocity();

// Optional settle callback — motor.cpp invokes it once per knob-initiated move as
// soon as the idle-disable fires (i.e. the rotor has come to rest at the target).
// M4 registers matter_push_level_from_angle here; M1–M3 leave it null, which means
// the same motor.cpp links cleanly for every stage without any Matter dependency.
void  motor_set_settle_callback(void (*cb)(float angle_rad));

// Called by input.cpp (M2+) right after a knob nudge, to arm the one-shot settle
// push. Without the arm flag, motor.cpp doesn't invoke the callback — this keeps
// Matter-slider-initiated moves from bouncing back through the callback path.
void  motor_request_matter_sync_on_settle();
