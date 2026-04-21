// SimpleFOC-based BLDC closed-loop position controller.
// Init is deferred (motor stays cold until the user short-clicks the knob OR
// commissioning completes) so the FOC ISR doesn't compete with SPAKE2+ for CPU.
#pragma once

#include <stdint.h>

void motor_init_and_foc();    // brings up SimpleFOC + 2 kHz GPTimer ISR. Idempotent.
bool motor_is_ready();        // true once init_and_foc has finished

// Commanded position (rad) — clamped to [POS_MIN, POS_MAX]. Settable from anywhere
// (Matter callback, knob, serial command). The motor loop task picks it up and ramps
// to it via the trapezoidal profile.
void  motor_set_commanded(float rad);
float motor_get_commanded();
float motor_get_shaft_angle();

// Velocity preset (cycles via double-click). Mirrors VELOCITY_PRESETS[].
void motor_set_velocity_preset(uint8_t idx);

// True when SimpleFOC has the motor in the enabled state (driver actively producing torque).
// Used by the knob handler to decide whether to anchor a detent to shaft_angle.
bool motor_is_enabled();

// One-shot flag: returns true iff a stall event occurred since the last call, and clears it.
// The app layer uses this to force-push the corrected position to Matter after fault_recover
// snaps g_commanded to shaft_angle.
bool motor_consume_stall_event();

// Per-iteration tick: ramp_target, motor.move(), stall detection, LED update.
// loopFOC() is called from the 1 kHz GPTimer ISR set up by motor_start_foc_timer().
void motor_tick();

// Start the 1 kHz GPTimer that drives motor.loopFOC() in ISR context. Call AFTER
// esp_matter::start() returns so commissioning interrupts get priority first.
void motor_start_foc_timer();
