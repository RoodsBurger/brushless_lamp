#pragma once

#include <stdint.h>

// Motor control surface. SimpleFOC v2.4.0 in velocity mode on a dedicated core-1 task; app-layer position loop with trapezoidal profile.

void  motor_init_and_start();

void  motor_set_target_angle(float rad);      // absolute target, clamped to [0, ANGLE_MAX]
void  motor_nudge_target_angle(float d_rad);  // knob-driven relative tweak
float motor_get_target_angle();
float motor_get_shaft_angle();
float motor_get_shaft_velocity();

// M4 registers matter_push_level_from_angle here; other stages leave it null so motor.cpp links without a Matter dependency.
void  motor_set_settle_callback(void (*cb)(float angle_rad));

// Knob nudges arm this so the next rest event pushes the position to Matter; Matter-driven moves don't, to avoid bouncing writes through the callback.
void  motor_request_matter_sync_on_settle();

// Triple-click cycles between MOTION_VELOCITY_PRESETS[]. rad_per_sec ≤ 0 restores the config.h default; otherwise clamped to (0, VELOCITY_LIMIT].
void  motor_set_motion_velocity(float rad_per_sec);
float motor_get_motion_velocity();
