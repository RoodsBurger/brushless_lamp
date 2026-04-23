#pragma once

// M4 input: knob + button, same as M2/M3 BUT with a long-press factory-reset
// that calls esp_matter::factory_reset(). Holding the knob button for
// BTN_FACTORY_RESET_MS wipes the Matter fabric + KVS and reboots — the recovery
// hatch you reach for when commissioning state goes stale.

void input_init();
void input_task_start();
