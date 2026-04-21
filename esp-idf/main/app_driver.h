// Bridge between Matter cluster callbacks (CHIP task) and BrushlessLamp hardware
// (motor / LEDs / persistence). Mirrors the esp-matter examples/light driver pattern.
#pragma once

#include <esp_err.h>
#include <esp_matter.h>

using app_driver_handle_t = void *;

// Wires up LEDs + input + persistence. Motor stays cold until commissioning completes
// or the user short-clicks the knob.
app_driver_handle_t app_driver_light_init(uint16_t light_endpoint_id);

// Called from the node-level attribute callback in app_main.cpp on PRE_UPDATE.
// Dispatches OnOff / LevelControl / ColorControl writes to motor/LED setters.
esp_err_t app_driver_attribute_update(app_driver_handle_t handle,
                                      uint16_t endpoint_id,
                                      uint32_t cluster_id,
                                      uint32_t attribute_id,
                                      esp_matter_attr_val_t *val);

// Read persisted Matter attribute values back into our hardware drivers after
// esp_matter::start() returns.
esp_err_t app_driver_apply_persisted(uint16_t endpoint_id);

// Per-iteration tick called from the main task — drives motor ramp + LED render.
void app_driver_tick();

// Push local state (motor commanded → Matter Level/OnOff/CT) back to the fabric.
// Called when the user changes state via the knob.
void app_driver_push_to_matter(uint16_t endpoint_id);
