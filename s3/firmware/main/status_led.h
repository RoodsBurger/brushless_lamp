#pragma once

// Board status LED state machine (custom PCB only; no-op on other boards).
// Patterns by priority: fault 5 Hz, boot solid, pairing 1 Hz, WiFi-down 2.5 Hz,
// motor-active solid, idle off.

void status_led_init();              // configures GPIO, starts the tick task, LED solid until Matter is up
void status_led_set_matter_ready();  // switches from boot-solid to state-driven patterns
