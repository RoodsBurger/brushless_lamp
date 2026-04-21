// Knob (PCNT-decoded quadrature) + push-button polling. Drives event callbacks
// registered by the application; nothing motor- or matter-specific here.
#pragma once

#include <stdint.h>

struct InputCallbacks {
    void (*on_knob)(int detents);   // signed delta (negative = CCW)
    void (*on_short_click)();
    void (*on_double_click)();
    void (*on_long_press_warn)();   // ~5 s held, warning fade pulse
    void (*on_long_press_reset)();  // ~9 s held, factory reset
};

void input_init(const InputCallbacks &cbs);
void input_poll();   // call from main loop
