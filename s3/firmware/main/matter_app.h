#pragma once

// esp-matter ColorTemperatureLight endpoint. Incoming attribute writes route via
// apply_state() → motor_set_target_angle() / leds_*. Outgoing knob-driven writes
// go through matter_push_*(), which marshals attribute::update() onto the CHIP
// task via PlatformMgr::ScheduleWork.

void matter_app_init();

#ifdef __cplusplus
extern "C" {
#endif
// Current OnOff — consumed by the input task for the single-click toggle.
bool           matter_get_on_off(void);

// Connectivity state for the status LED: commissioned = ≥1 fabric; wifi_up = STA associated.
bool matter_is_commissioned(void);
bool matter_is_wifi_up(void);

// Push LevelControl::CurrentLevel from a settle (1..254). allow_on = the settle may
// turn the lamp on (a knob raise); false for stalls / restores / homing.
void matter_push_level_from_angle(float angle_rad, bool allow_on);

// Local OnOff toggle (button single-click). Off snaps motor to 0; on restores last level.
void matter_push_onoff(bool on);

// Push ColorControl::ColorTemperatureMireds from a knob-driven CT change.
void matter_push_colortemp(uint16_t mireds);

// Wipe foc_cal / leds / input NVS — esp_matter::factory_reset() only clears CHIP's own namespaces.
void matter_wipe_local_nvs(void);
#ifdef __cplusplus
}
#endif
