#pragma once

#include <stdint.h>

// Motor control surface shared by every S3 stage. SimpleFOC v2.4.0 runs in
// velocity mode on a dedicated core-1 task; the position loop is closed in the
// app layer with a physics-bounded trapezoidal profile. The FOC task attaches
// its encoder ISR from within its own body so the ISR lives on CORE_MOTOR —
// arduino-esp32's noInterrupts() is per-core.

void  motor_init_and_start();

void  motor_set_target_angle(float rad);      // absolute target, clamped to [0, ANGLE_MAX]
void  motor_nudge_target_angle(float d_rad);  // knob-driven relative tweak
float motor_get_target_angle();
float motor_get_shaft_angle();
float motor_get_shaft_velocity();

// Settle callback fires once per move as soon as idle-disable latches at rest.
// M4 registers matter_push_level_from_angle here; other stages leave it null
// so motor.cpp links cleanly without a Matter dependency.
void  motor_set_settle_callback(void (*cb)(float angle_rad));

// Arms the settle callback for the next rest event. Knob nudges call this so
// Matter learns the new position; Matter-slider-driven moves don't, to avoid
// bouncing their own writes back through the callback.
void  motor_request_matter_sync_on_settle();

// Direct-velocity mode — bypasses the position loop and drives SimpleFOC's
// velocity target at rad_per_sec with the same MOTION_ACCEL ramp. Pass 0.0f
// to exit: manual mode clears and s_target_angle resyncs to the current
// shaft so the position loop idle-disables on its next tick.
void  motor_run_at_velocity(float rad_per_sec);

// Runtime cruise cap used by the position loop. Double-click cycling swaps
// between MOTION_VELOCITY_PRESETS[] without rebuild. rad_per_sec ≤ 0 restores
// the config.h default; otherwise clamped to (0, VELOCITY_LIMIT].
void  motor_set_motion_velocity(float rad_per_sec);
float motor_get_motion_velocity();
