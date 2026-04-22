#pragma once

// Phase 14a: knob + button wired into Arduino's attachInterrupt / pinMode. Talks to motor
// via motor_set_target_velocity / motor_stop. In 14c we'll add a "local knob priority"
// hook so a physical knob turn overrides a pending Matter slider value.

void input_init();
void input_task_start();
