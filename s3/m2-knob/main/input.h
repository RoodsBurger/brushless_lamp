#pragma once

// Knob + button input for the M2 stage. The knob is a quadrature encoder on
// PIN_ROT_A / PIN_ROT_B; button is active-low with internal pull-up on PIN_BTN.
// input_init() calls pinMode / attachInterrupt, and input_task_start() spawns the
// 10 ms polling task that drains knob detents into motor_nudge_target_angle() and
// debounces the button into motor_set_target_angle(0).
//
// Both calls live on the arduino-event core (CORE_OTHERS) — NOT the motor core.
// That's intentional: we want attachInterrupt registered on core 0 so it doesn't
// interrupt the FOC loop on core 1. The per-core interrupt register-bank means
// CORE_MOTOR's ISRs are completely untouched by the knob activity.

void input_init();
void input_task_start();
