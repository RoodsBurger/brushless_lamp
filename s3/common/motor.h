#pragma once

#include <stdint.h>

// Motor control surface. SimpleFOC v2.4.0 in velocity mode on a dedicated core-1 task; app-layer position loop with trapezoidal profile.

void  motor_init_and_start();

void  motor_set_target_angle(float rad);      // absolute target, clamped to [0, ANGLE_MAX]
void  motor_nudge_target_angle(float d_rad);  // knob-driven relative tweak
float motor_get_shaft_angle();

// True when the motor is parked (disabled, at rest, last position saved to NVS).
// OTA waits on this so a reboot never interrupts mid-travel and corrupts the zero.
bool  motor_is_idle();

// Fires on idle-disable entry, ON THE CORE-1 FOC TASK — keep it short, no blocking
// I/O or INFO logs (USB-CDC backpressure can stall the caller for seconds).
// allow_on = may this settle turn the lamp ON (a knob raise) — false for stalls,
// restores and homing, so the lamp can never autonomously power itself on.
// matter_app registers matter_push_level_from_angle here.
void  motor_set_settle_callback(void (*cb)(float angle_rad, bool allow_on));

// Knob nudges arm this so the next rest event pushes position to Matter (allow_on=true);
// Matter-driven moves don't, to avoid bouncing writes through the callback.
void  motor_request_matter_sync_on_settle();

// Re-home: drive to the off-stop and set it as logical 0 (knob-hold gesture). Runs on
// the FOC task, blocking it for the descent. Pushes the new 0/off state to Matter.
void  motor_request_homing();

// Triple-click cycles between MOTION_VELOCITY_PRESETS[]. rad_per_sec ≤ 0 restores the config.h default; otherwise clamped to (0, VELOCITY_LIMIT].
void  motor_set_motion_velocity(float rad_per_sec);
