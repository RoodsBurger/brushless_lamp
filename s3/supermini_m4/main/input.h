#pragma once

// M4 input: knob + button. 1 click toggles Matter OnOff (off → 0, on → last
// level, Google-Home-style); 2 clicks toggles knob mode (motor angle ↔ LED
// brightness); 3 clicks cycles the MOTION_VELOCITY_PRESETS (LEDs pulse idx+1).
// Hold ≥ 5 s flashes 5 warning blinks; keep holding to 9 s to
// esp_matter::factory_reset() (wipes fabric + KVS + reboots).

void input_init();
void input_task_start();
