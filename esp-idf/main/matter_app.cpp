// Phase 14c: minimal Matter ColorTemperatureLight endpoint.
// On/Off toggles motor enable + LED master switch. LevelControl's CurrentLevel drives
// motor velocity (VELOCITY_LIMIT * level/254). ColorControl's ColorTemperatureMireds
// maps to a WW/CW balance: 153 mireds (coolest, ~6500 K) → full cool; 500 mireds
// (warmest, ~2000 K) → full warm.

#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_console.h>

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

#include "matter_app.h"
#include "motor.h"
#include "leds.h"
#include "config.h"

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

static const char *TAG = "matter";

static uint16_t s_light_endpoint_id = 0;

// Latest commanded Matter values — refreshed in the attribute callback and applied to
// the motor/LED drivers on every change. Persisted by the Matter stack across boots.
static bool    s_on_off         = false;
static uint8_t s_level          = 128;   // 0..254 (Matter convention)
static uint16_t s_color_temp_mireds = 250;  // neutral-ish daylight

static constexpr uint16_t MIN_MIREDS = 153;   // coolest — full cool white
static constexpr uint16_t MAX_MIREDS = 500;   // warmest — full warm white

// apply_state drives the motor only — LEDs are computed from the live shaft angle by
// leds.cpp's follower task so the light tracks physical lamp position, not Matter Level.
static void apply_state() {
    if (!s_on_off) {
        motor_disable();                 // want_motion=off — motor will idle once at 0
        motor_set_target_angle(0.0f);    // go to bottom; LED follower will dim when low
    } else {
        motor_enable();
        float angle = ((float)s_level / 254.0f) * ANGLE_MAX;
        motor_set_target_angle(angle);
    }
    ESP_LOGI(TAG, "apply: on=%d level=%u ct=%umir -> tgt_ang=%.2frad",
             (int)s_on_off, (unsigned)s_level, (unsigned)s_color_temp_mireds,
             motor_get_target_angle());
}

extern "C" unsigned short matter_get_color_temp_mireds(void) {
    return (unsigned short)s_color_temp_mireds;
}
extern "C" bool matter_get_on_off(void) {
    return s_on_off;
}

// Marshal a Matter attribute::update() call onto the CHIP task. Calling it from
// arbitrary FreeRTOS tasks (like input_task) silently drops the report-to-subscribers
// step — Google Home's slider stops tracking the physical knob.
namespace {
struct LevelPush { uint16_t ep; uint8_t level; bool also_on; };
}
static void matter_push_level_work(intptr_t arg) {
    LevelPush *p = reinterpret_cast<LevelPush *>(arg);
    // LevelControl::CurrentLevel is declared nullable in esp_matter's data model
    // (its bounds are registered with esp_matter_nullable_uint8). Passing the
    // non-nullable variant returns ESP_ERR_INVALID_ARG (258) silently — that's why
    // every knob push was dropped and Google Home never saw the update.
    esp_matter_attr_val_t v = esp_matter_nullable_uint8(p->level);
    esp_err_t r = attribute::update(p->ep, LevelControl::Id,
                                    LevelControl::Attributes::CurrentLevel::Id, &v);
    if (r != ESP_OK) ESP_LOGE(TAG, "level update failed: %d", r);
    if (p->also_on) {
        esp_matter_attr_val_t vo = esp_matter_bool(true);
        r = attribute::update(p->ep, OnOff::Id, OnOff::Attributes::OnOff::Id, &vo);
        if (r != ESP_OK) ESP_LOGE(TAG, "onoff update failed: %d", r);
    }
    delete p;
}

extern "C" void matter_push_level_from_angle(float angle_rad) {
    if (s_light_endpoint_id == 0) return;
    if (angle_rad < 0.0f) angle_rad = 0.0f;
    if (angle_rad > ANGLE_MAX) angle_rad = ANGLE_MAX;
    int lv = (int)((angle_rad / ANGLE_MAX) * 254.0f);
    // Matter LevelControl::CurrentLevel is constrained to [MinLevel=1, MaxLevel=254].
    // Pushing 0 comes back as ESP_ERR_INVALID_ARG (258) and the write is dropped —
    // which was exactly why Google Home never reflected the knob. Always clamp.
    if (lv < 1)   lv = 1;
    if (lv > 254) lv = 254;
    uint8_t new_level = (uint8_t)lv;
    if (new_level == s_level) return;

    bool also_on = !s_on_off;
    s_level  = new_level;
    if (also_on) s_on_off = true;

    ESP_LOGI(TAG, "push: knob angle=%.2f → level=%u (%s)",
             angle_rad, (unsigned)new_level, also_on ? "+OnOff=true" : "level-only");

    LevelPush *p = new LevelPush{ s_light_endpoint_id, new_level, also_on };
    chip::DeviceLayer::PlatformMgr().ScheduleWork(matter_push_level_work,
                                                  reinterpret_cast<intptr_t>(p));
}

static esp_err_t attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id,
                                     uint32_t cluster_id, uint32_t attribute_id,
                                     esp_matter_attr_val_t *val, void *priv_data) {
    // Only react after Matter has committed the new value; PRE_UPDATE can arrive
    // with a provisional payload that hasn't been range-checked yet.
    if (type != POST_UPDATE) return ESP_OK;
    if (endpoint_id != s_light_endpoint_id) return ESP_OK;

    // Echo suppression: matter_push_level_from_angle updates s_level before scheduling
    // the attribute::update that eventually triggers us. When that happens the incoming
    // value matches s_level exactly; re-running apply_state would kick the motor at a
    // round-tripped target, which felt like a bump after every knob detent.
    if (cluster_id == OnOff::Id && attribute_id == OnOff::Attributes::OnOff::Id) {
        if (val->val.b == s_on_off) return ESP_OK;
        s_on_off = val->val.b;
        apply_state();
    } else if (cluster_id == LevelControl::Id &&
               attribute_id == LevelControl::Attributes::CurrentLevel::Id) {
        if (val->val.u8 == s_level) return ESP_OK;
        s_level = val->val.u8;
        apply_state();
    } else if (cluster_id == ColorControl::Id &&
               attribute_id == ColorControl::Attributes::ColorTemperatureMireds::Id) {
        if (val->val.u16 == s_color_temp_mireds) return ESP_OK;
        s_color_temp_mireds = val->val.u16;
        apply_state();
    }
    return ESP_OK;
}

static esp_err_t identification_cb(identification::callback_type_t type, uint16_t endpoint_id,
                                   uint8_t effect_id, uint8_t effect_variant, void *priv_data) {
    ESP_LOGI(TAG, "identify: type=%u effect=%u variant=%u", type, effect_id, effect_variant);
    return ESP_OK;
}

static void event_cb(const ChipDeviceEvent *event, intptr_t arg) {
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "commissioning complete");
        break;
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "interface IP changed");
        break;
    case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
        ESP_LOGI(TAG, "fabric removed");
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
            auto &mgr = chip::Server::GetInstance().GetCommissioningWindowManager();
            if (!mgr.IsCommissioningWindowOpen()) {
                mgr.OpenBasicCommissioningWindow(chip::System::Clock::Seconds16(300),
                                                 chip::CommissioningWindowAdvertisement::kDnssdOnly);
            }
        }
        break;
    }
    default:
        break;
    }
}

void matter_app_init() {
    node::config_t node_config;
    node_t *node = node::create(&node_config, attribute_update_cb, identification_cb);
    if (!node) {
        ESP_LOGE(TAG, "node::create failed");
        return;
    }

    color_temperature_light::config_t light_config;
    light_config.on_off.on_off                   = false;
    light_config.on_off_lighting.start_up_on_off = nullptr;
    // First-boot default only (persists after first Matter write). 1 keeps a fresh
    // device at the bottom of travel until the user dials it up.
    light_config.level_control.current_level     = 1;
    // Deliberately leave `on_level` at its default (null) so off→on resumes the last
    // CurrentLevel instead of jumping to a hard 128. Same for start_up_current_level
    // which we let persist across power-cycles.
    light_config.level_control_lighting.start_up_current_level = nullptr;
    light_config.color_control.color_mode          = (uint8_t)ColorControl::ColorMode::kColorTemperature;
    light_config.color_control.enhanced_color_mode = (uint8_t)ColorControl::ColorMode::kColorTemperature;
    light_config.color_control_color_temperature.start_up_color_temperature_mireds = nullptr;

    endpoint_t *ep = color_temperature_light::create(node, &light_config, ENDPOINT_FLAG_NONE, nullptr);
    if (!ep) {
        ESP_LOGE(TAG, "color_temperature_light::create failed");
        return;
    }
    s_light_endpoint_id = endpoint::get_id(ep);
    ESP_LOGI(TAG, "color_temperature_light endpoint id=%u", s_light_endpoint_id);

    // Deferred persistence for rapid-change attributes — don't beat up NVS on slider drags.
    attribute_t *cl = attribute::get(s_light_endpoint_id, LevelControl::Id,
                                     LevelControl::Attributes::CurrentLevel::Id);
    if (cl) attribute::set_deferred_persistence(cl);
    attribute_t *ct = attribute::get(s_light_endpoint_id, ColorControl::Id,
                                     ColorControl::Attributes::ColorTemperatureMireds::Id);
    if (ct) attribute::set_deferred_persistence(ct);

    esp_err_t err = esp_matter::start(event_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_matter::start err=%d", err);
        return;
    }

    // Boot indicator: dim warm-white while uncommissioned. Matter callbacks take over
    // as soon as the first OnOff/Level write lands, so this only shows during the
    // "device booted, BLE advertising" window before the fabric joins.
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
        leds_set(16, 8);
    }

#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
    esp_matter::console::attribute_register_commands();
    esp_matter::console::init();
#endif
}
