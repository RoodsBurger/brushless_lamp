#pragma once

// esp-matter ColorTemperatureLight endpoint. matter_app_init() creates the node +
// endpoint and starts the Matter stack. Incoming attribute writes are marshalled
// into motor_set_target_angle() / leds_* via apply_state(). Outgoing writes (the
// knob-driven ones) come through matter_push_level_from_angle(), which marshals
// the attribute::update() onto the CHIP task via PlatformMgr::ScheduleWork.

void matter_app_init();

#ifdef __cplusplus
extern "C" {
#endif
// Consumed by leds.cpp's follower — Matter's current ColorTemperatureMireds and
// OnOff states, respectively.
unsigned short matter_get_color_temp_mireds(void);
bool           matter_get_on_off(void);

// Called by motor.cpp's settle callback (registered in app_main) whenever a knob-
// initiated move comes to rest. Rounds angle → Matter Level (1..254) and pushes
// LevelControl::CurrentLevel to all subscribers. Uses esp_matter_nullable_uint8:
// the attribute is declared nullable in esp_matter's data model and passing the
// non-nullable variant is silently rejected with ESP_ERR_INVALID_ARG (258).
void matter_push_level_from_angle(float angle_rad);

// Wipes foc_cal / leds / input NVS namespaces. esp_matter::factory_reset()
// only touches CHIP's own namespaces, so call this right before it (on
// button-hold factory reset) to guarantee the next boot runs a full motor
// re-calibration. Automatically invoked on remote decommission via the
// kFabricRemoved event.
void matter_wipe_local_nvs(void);
#ifdef __cplusplus
}
#endif
