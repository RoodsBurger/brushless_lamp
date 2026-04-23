#pragma once

// Identical to M2's input API — motor still takes the knob via
// motor_nudge_target_angle() / motor_set_target_angle(). Button short-press returns
// to origin. No factory-reset long-press here (that's an M4 feature, when there's
// actually Matter fabric state to wipe).

void input_init();
void input_task_start();
