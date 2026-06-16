#pragma once

// Input: knob + button. 1 click toggles Matter OnOff (off → 0, on → last
// level, Google-Home-style); 2 clicks cycles knob mode (motor → brightness →
// CT); 3 clicks cycles the MOTION_VELOCITY_PRESETS (LEDs pulse idx+1).
// Hold ≥ 5 s → 3 blinks (homing armed); release to re-home. Keep holding to
// ≥ 15 s → 5 blinks → factory reset (full NVS partition erase + reboot).

void input_init();
void input_task_start();
