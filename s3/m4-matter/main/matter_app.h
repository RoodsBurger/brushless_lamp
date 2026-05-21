#pragma once

// esp-matter ColorTemperatureLight endpoint. Incoming attribute writes route via
// apply_state() → motor_set_target_angle() / leds_*. Outgoing knob-driven writes
// go through matter_push_*(), which marshals attribute::update() onto the CHIP
// task via PlatformMgr::ScheduleWork.

void matter_app_init();

#ifdef __cplusplus
extern "C" {
#endif
// Current ColorTemperatureMireds and OnOff — consumed by the LED fader.
unsigned short matter_get_color_temp_mireds(void);
bool           matter_get_on_off(void);

// Push LevelControl::CurrentLevel from a knob-driven settle (1..254).
void matter_push_level_from_angle(float angle_rad);

// Local OnOff toggle (button single-click). Off snaps motor to 0; on restores last level.
void matter_push_onoff(bool on);

// Wipe foc_cal / leds / input NVS — esp_matter::factory_reset() only clears CHIP's own namespaces.
void matter_wipe_local_nvs(void);
#ifdef __cplusplus
}
#endif
