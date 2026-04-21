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

// Per-iteration tick: advances the trapezoidal ramp, calls motor.move(),
// runs stall detection, refreshes the LEDs from shaft_angle. Call from the main task.
void motor_tick();
