#include "app_driver.h"
#include "motor.h"
#include "leds.h"
#include "input.h"
#include "persistence.h"
#include "config.h"

#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_system.h>
#include <esp_matter.h>
#include <esp_matter_core.h>
#include <esp_matter_attribute_utils.h>
#include <app/server/Server.h>

#include <atomic>

static const char *TAG = "app_drv";

using namespace esp_matter;
using namespace chip::app::Clusters;

static uint16_t s_endpoint_id = 0;

// True for the brief window we're pushing state outbound to Matter, so the inbound
// PRE_UPDATE callback can ignore the echo. Same pattern as Arduino sketch.
static std::atomic<bool> s_matter_echo{false};

// Knob mode — CylinderLamp's single-click-toggles pattern. Starts in position mode
// after motor init. Only meaningful once motor_is_ready().
enum class KnobMode { Position, Brightness };
static KnobMode s_knob_mode = KnobMode::Position;

static float matter_brightness_to_cmd(uint8_t level) {
    return ((float)level * POS_MAX) / 254.0f;
}
static uint8_t cmd_to_matter_brightness(float cmd) {
    long v = (long)(cmd * 254.0f / POS_MAX + 0.5f);
    if (v < 0) v = 0;
    if (v > 254) v = 254;
    return (uint8_t)v;
}
static uint8_t mireds_to_cct(uint16_t mireds) {
    if (mireds < MATTER_CT_COOL) mireds = MATTER_CT_COOL;
    if (mireds > MATTER_CT_WARM) mireds = MATTER_CT_WARM;
    return (uint8_t)((MATTER_CT_WARM - mireds) * 100L / (MATTER_CT_WARM - MATTER_CT_COOL));
}
static uint16_t cct_to_mireds(uint8_t cct) {
    if (cct > 100) cct = 100;
    return (uint16_t)(MATTER_CT_WARM - (long)cct * (MATTER_CT_WARM - MATTER_CT_COOL) / 100L);
}

// ---------------- Input event handlers ----------------

static void on_knob(int detents) {
    if (!motor_is_ready()) return;   // knob does nothing until motor is up

    if (s_knob_mode == KnobMode::Position) {
        float cmd = motor_get_commanded() + (float)detents * RAD_PER_CLICK;
        if (cmd < POS_MIN) cmd = POS_MIN;
        if (cmd > POS_MAX) cmd = POS_MAX;
        motor_set_commanded(cmd);
        ESP_LOGI(TAG, "knob %+d → cmd=%.2f → push", detents, (double)cmd);
        app_driver_push_to_matter(s_endpoint_id);
    } else { // KnobMode::Brightness
        int bri = (int)leds_get_brightness() + detents;
        if (bri < 0)   bri = 0;
        if (bri > 100) bri = 100;
        leds_set_brightness((uint8_t)bri);
        // Brightness isn't a Matter attribute in our product — it's purely local;
        // Matter's LevelControl drives motor position, not LED intensity. So no push.
    }
}

static void on_short_click() {
    if (!motor_is_ready()) {
        ESP_LOGI(TAG, "short-click — first-time motor init (bench-mode)");
        motor_init_and_foc();
        s_knob_mode = KnobMode::Position;
        return;
    }
    s_knob_mode = (s_knob_mode == KnobMode::Position) ? KnobMode::Brightness : KnobMode::Position;
    ESP_LOGI(TAG, "knob mode: %s", s_knob_mode == KnobMode::Position ? "position" : "brightness");
}

static void on_double_click() {
    uint8_t idx = persistence_get_speed_idx();
    idx = (idx + 1) % VELOCITY_PRESET_N;
    persistence_set_speed_idx(idx);
    motor_set_velocity_preset(idx);
    leds_pulse((uint8_t)(idx + 1));
}

static void on_long_press_warn() {
    ESP_LOGI(TAG, "long-press warn (5 s) — pulse LEDs");
    leds_pulse(5);
}

static void on_long_press_reset() {
    ESP_LOGI(TAG, "long-press reset (9 s) — factory reset");
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() > 0) {
        // Decommission via Matter — handles fabric state cleanly. Reboots internally.
        esp_matter::factory_reset();
    } else {
        // Already uncommissioned; just wipe blamp prefs and reboot.
        nvs_flash_erase();
        esp_restart();
    }
}

// ---------------- App lifecycle ----------------

app_driver_handle_t app_driver_light_init(uint16_t light_endpoint_id) {
    s_endpoint_id = light_endpoint_id;
    persistence_init();
    leds_init();
    leds_set_brightness(50);   // initial defaults; persisted Matter values applied later
    leds_set_cct(50);
    motor_set_velocity_preset(persistence_get_speed_idx());

    InputCallbacks cbs{};
    cbs.on_knob              = on_knob;
    cbs.on_short_click       = on_short_click;
    cbs.on_double_click      = on_double_click;
    cbs.on_long_press_warn   = on_long_press_warn;
    cbs.on_long_press_reset  = on_long_press_reset;
    input_init(cbs);

    ESP_LOGI(TAG, "app_driver init complete; motor stays cold until commissioning or short-click");
    return reinterpret_cast<app_driver_handle_t>(0xBEEF);   // opaque, unused for now
}

esp_err_t app_driver_attribute_update(app_driver_handle_t,
                                      uint16_t endpoint_id,
                                      uint32_t cluster_id,
                                      uint32_t attribute_id,
                                      esp_matter_attr_val_t *val) {
    if (endpoint_id != s_endpoint_id) return ESP_OK;
    if (s_matter_echo.load()) return ESP_OK;

    if (cluster_id == OnOff::Id && attribute_id == OnOff::Attributes::OnOff::Id) {
        bool on = val->val.b;
        if (!on) {
            motor_set_commanded(POS_MIN);
        } else {
            // On → restore motor to the last CurrentLevel. Without this, Off→On leaves the motor at zero.
            attribute_t *lvl_attr = attribute::get(endpoint_id, LevelControl::Id,
                                                   LevelControl::Attributes::CurrentLevel::Id);
            esp_matter_attr_val_t lvl;
            if (lvl_attr && attribute::get_val(lvl_attr, &lvl) == ESP_OK && lvl.val.u8 >= 1) {
                motor_set_commanded(matter_brightness_to_cmd(lvl.val.u8));
            }
        }
    } else if (cluster_id == LevelControl::Id &&
               attribute_id == LevelControl::Attributes::CurrentLevel::Id) {
        uint8_t level = val->val.u8;
        if (level < 1) level = 1;
        motor_set_commanded(matter_brightness_to_cmd(level));
    } else if (cluster_id == ColorControl::Id &&
               attribute_id == ColorControl::Attributes::ColorTemperatureMireds::Id) {
        uint16_t mireds = val->val.u16;
        leds_set_cct(mireds_to_cct(mireds));
    }
    return ESP_OK;
}

esp_err_t app_driver_apply_persisted(uint16_t endpoint_id) {
    auto get = [&](uint32_t cluster, uint32_t attr, esp_matter_attr_val_t &out) -> bool {
        attribute_t *a = attribute::get(endpoint_id, cluster, attr);
        if (!a) return false;
        return attribute::get_val(a, &out) == ESP_OK;
    };

    esp_matter_attr_val_t v;
    bool is_on = true;
    if (get(OnOff::Id, OnOff::Attributes::OnOff::Id, v)) is_on = v.val.b;

    // OnOff=false must override CurrentLevel; otherwise the sequential writes race and the Level apply clobbers the off state.
    if (!is_on) {
        motor_set_commanded(POS_MIN);
    } else if (get(LevelControl::Id, LevelControl::Attributes::CurrentLevel::Id, v) && v.val.u8 >= 1) {
        motor_set_commanded(matter_brightness_to_cmd(v.val.u8));
    }
    if (get(ColorControl::Id, ColorControl::Attributes::ColorTemperatureMireds::Id, v)) {
        leds_set_cct(mireds_to_cct(v.val.u16));
    }
    return ESP_OK;
}

void app_driver_tick() {
    motor_tick();
}

void app_driver_push_to_matter(uint16_t endpoint_id) {
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
        ESP_LOGW(TAG, "push skipped: uncommissioned");
        return;
    }

    float cmd = motor_get_commanded();
    bool on = cmd > LEDS_OFF_THRESH;
    uint8_t bri = cmd_to_matter_brightness(cmd);
    if (on && bri == 0) bri = 1;
    uint16_t mireds = cct_to_mireds(leds_get_cct());

    s_matter_echo.store(true);
    esp_err_t r_on = ESP_OK, r_lvl = ESP_OK, r_ct = ESP_OK;
    {
        esp_matter::lock::ScopedChipStackLock lock(portMAX_DELAY);
        esp_matter_attr_val_t v_on  = esp_matter_bool(on);
        // CurrentLevel is nullable-uint8 in Matter; esp_matter_uint8() returns ESP_ERR_INVALID_ARG (258) at update time.
        esp_matter_attr_val_t v_lvl = esp_matter_nullable_uint8(bri);
        esp_matter_attr_val_t v_ct  = esp_matter_nullable_uint16(mireds);
        r_on = attribute::update(endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id, &v_on);
        if (on) {
            r_lvl = attribute::update(endpoint_id, LevelControl::Id,
                                      LevelControl::Attributes::CurrentLevel::Id, &v_lvl);
        }
        r_ct = attribute::update(endpoint_id, ColorControl::Id,
                                 ColorControl::Attributes::ColorTemperatureMireds::Id, &v_ct);
    }
    s_matter_echo.store(false);
    ESP_LOGI(TAG, "push: on=%d lvl=%u mireds=%u (r_on=%d r_lvl=%d r_ct=%d)",
             (int)on, bri, mireds, r_on, r_lvl, r_ct);
}
