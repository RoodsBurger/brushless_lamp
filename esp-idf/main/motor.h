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

// While motor is idle-disabled, snap commanded to shaft_angle so subsequent user input
// is relative to the physical shaft. MUST be called BEFORE input_poll each iteration —
// otherwise the re-anchor clobbers knob deltas that arrived this tick.
void motor_idle_anchor();

// Per-iteration tick: ramp_target, motor.loopFOC(), motor.move(), stall detection, LED update.
void motor_tick();
