#pragma once

// Knob + button input for the M2 velocity-nudge stage. Quadrature encoder on
// PIN_ROT_A / PIN_ROT_B, active-low button with internal pull-up on PIN_BTN.
// input_init() wires pinMode + attachInterrupt; input_task_start() spawns the
// 10 ms polling task that turns each detent into ±KNOB_STEP_RAD_PER_SEC on the
// motor's manual-velocity target via motor_run_at_velocity(), and zeros it on
// button press. Both calls run on CORE_OTHERS so the GPIO ISR registers on
// core 0 and never preempts the FOC loop on core 1.

void input_init();
void input_task_start();
