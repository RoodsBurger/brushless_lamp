// Matter ColorTemperatureLight endpoint. Inbound writes drive motor + LEDs via
// apply_state(); knob settle pushes Level/OnOff back to subscribers via the
// CHIP task to serialise data-model access.

#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs.h>

#include "config.h"
#include "leds.h"
#include "matter_app.h"
#include "motor.h"

// factory_reset() only wipes CHIP NVS; clear our namespaces too so the next boot
// re-runs a full FOC calibration sweep.
static void wipe_local_nvs() {
    const char * const namespaces[] = { "foc_cal", "leds", "input" };
    for (const char *ns : namespaces) {
        nvs_handle_t h;
        if (nvs_open(ns, NVS_READWRITE, &h) != ESP_OK) continue;
        nvs_erase_all(h);
        nvs_commit(h);
        nvs_close(h);
    }
}

extern "C" void matter_wipe_local_nvs() { wipe_local_nvs(); }

// Strong override of arduino-esp32's weak btInUse so initArduino doesn't release
// the BT controller heap (which would crash Matter's BLE bring-up). __used survives LTO.
extern "C" __attribute__((used)) bool btInUse(void) { return true; }

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

static const char *TAG = "matter";

static volatile uint16_t s_endpoint_id = 0;

// Last-applied attribute values; written from CHIP task, read from motor / LED tasks.
static volatile bool     s_on_off             = false;
static volatile uint8_t  s_level              = 1;    // Matter MinLevel
static volatile uint16_t s_color_temp_mireds  = 250;

static void apply_state() {
    leds_set_colortemp(s_color_temp_mireds);
    float target = s_on_off ? ((float)s_level / 254.0f) * ANGLE_MAX : 0.0f;
    motor_set_target_angle(target);
    ESP_LOGD(TAG, "apply: on=%d level=%u ct=%umir -> tgt=%.2frad",
             (int)s_on_off, (unsigned)s_level, (unsigned)s_color_temp_mireds, target);
}

static esp_err_t attribute_update_cb(callback_type_t type, uint16_t endpoint_id,
                                     uint32_t cluster_id, uint32_t attribute_id,
                                     esp_matter_attr_val_t *val, void *priv_data) {
    if (type != POST_UPDATE || endpoint_id != s_endpoint_id) return ESP_OK;

    if (cluster_id == OnOff::Id && attribute_id == OnOff::Attributes::OnOff::Id) {
        if (val->val.b == s_on_off) return ESP_OK;     // echo
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
    ESP_LOGI(TAG, "identify type=%u effect=%u variant=%u", type, effect_id, effect_variant);
    return ESP_OK;
}

static void event_cb(const ChipDeviceEvent *event, intptr_t arg) {
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "commissioning complete"); break;
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "IP address changed"); break;
    case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
        ESP_LOGI(TAG, "fabric removed");
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
            // Remote decommission ("Remove" from Home app) — CHIP wipes its own NVS only.
            wipe_local_nvs();
            auto &mgr = chip::Server::GetInstance().GetCommissioningWindowManager();
            if (!mgr.IsCommissioningWindowOpen()) {
                mgr.OpenBasicCommissioningWindow(chip::System::Clock::Seconds16(300),
                                                 chip::CommissioningWindowAdvertisement::kDnssdOnly);
            }
        }
        break;
    }
    default: break;
    }
}

void matter_app_init() {
    node::config_t node_config;
    node_t *node = node::create(&node_config, attribute_update_cb, identification_cb);
    if (!node) { ESP_LOGE(TAG, "node::create failed"); return; }

    color_temperature_light::config_t lc;
    lc.on_off.on_off                         = false;
    lc.on_off_lighting.start_up_on_off       = nullptr;
    // MinLevel; freshly paired device starts at the bottom of travel.
    lc.level_control.current_level           = 1;
    // null => off→on resumes last persisted level (avoids the "always 50% on turn-on" default).
    lc.level_control_lighting.start_up_current_level = nullptr;
    lc.color_control.color_mode              = (uint8_t)ColorControl::ColorMode::kColorTemperature;
    lc.color_control.enhanced_color_mode     = (uint8_t)ColorControl::ColorMode::kColorTemperature;
    lc.color_control_color_temperature.start_up_color_temperature_mireds = nullptr;

    endpoint_t *ep = color_temperature_light::create(node, &lc, ENDPOINT_FLAG_NONE, nullptr);
    if (!ep) { ESP_LOGE(TAG, "endpoint create failed"); return; }
    s_endpoint_id = endpoint::get_id(ep);
    ESP_LOGI(TAG, "ColorTemperatureLight endpoint=%u", s_endpoint_id);

    // Defer Level / ColorTemp NVS writes so slider drags don't thrash flash.
    if (auto *a = attribute::get(s_endpoint_id, LevelControl::Id,
                                 LevelControl::Attributes::CurrentLevel::Id)) attribute::set_deferred_persistence(a);
    if (auto *a = attribute::get(s_endpoint_id, ColorControl::Id,
                                 ColorControl::Attributes::ColorTemperatureMireds::Id)) attribute::set_deferred_persistence(a);

    esp_err_t err = esp_matter::start(event_cb);
    if (err != ESP_OK) { ESP_LOGE(TAG, "esp_matter::start err=%d", err); return; }

    // Print VID/PID/discriminator at WARN log level so the user can match the
    // device against the per-device row in mfg_out/.../summary-*.csv. Direct
    // printf, not ChipLogProgress — INFO-level chatter on chip[*] tags during
    // PASE/AddNOC backpressures USB-CDC TX and times commissioning out.
    //
    // The CSV's `manualcode` and `qrcode` columns are the ones to commission
    // with: factory data stores only the Spake2+ verifier so on-device code
    // generation falls back to the default test passcode (20202021), which
    // would NOT match the verifier and would fail Spake2+.
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
        uint16_t vid = 0, pid = 0, discriminator = 0;
        if (auto *info = chip::DeviceLayer::GetDeviceInstanceInfoProvider()) {
            info->GetVendorId(vid); info->GetProductId(pid);
        }
        if (auto *cd = chip::DeviceLayer::GetCommissionableDataProvider()) {
            cd->GetSetupDiscriminator(discriminator);
        }
        printf("[matter] uncommissioned: VID=0x%04X PID=0x%04X discriminator=%u\n",
               vid, pid, discriminator);
        printf("[matter] commission with the QR / manual code in mfg_out/out/%04x_%04x/summary-*.csv\n",
               vid, pid);
    } else {
        printf("[matter] commissioned: %u fabric(s)\n",
               (unsigned)chip::Server::GetInstance().GetFabricTable().FabricCount());
    }
}

extern "C" unsigned short matter_get_color_temp_mireds(void) { return s_color_temp_mireds; }
extern "C" bool           matter_get_on_off(void)            { return s_on_off; }

// Button-driven OnOff toggle; mirrors what an attribute_update_cb write would do.
namespace { struct OnOffPush { uint16_t ep; bool on; }; }
static void matter_push_onoff_work(intptr_t arg) {
    OnOffPush *p = reinterpret_cast<OnOffPush *>(arg);
    esp_matter_attr_val_t vo = esp_matter_bool(p->on);
    esp_err_t r = attribute::update(p->ep, OnOff::Id, OnOff::Attributes::OnOff::Id, &vo);
    if (r != ESP_OK) ESP_LOGE(TAG, "onoff push failed: %d", r);
    delete p;
}
extern "C" void matter_push_onoff(bool on) {
    if (s_endpoint_id == 0) return;
    if (on == s_on_off)     return;
    s_on_off = on;
    apply_state();
    auto *p = new OnOffPush{ s_endpoint_id, on };
    chip::DeviceLayer::PlatformMgr().ScheduleWork(matter_push_onoff_work,
                                                  reinterpret_cast<intptr_t>(p));
}

// Knob-driven Level push; marshals attribute::update onto the CHIP task.
// also_on / force_off are mutually exclusive — knob motion up from rest implies
// OnOff=true (Home app can't show a non-zero brightness while Off), knob motion
// down to zero implies OnOff=false (else Home stays at "On, 1%" since MinLevel=1).
namespace {
struct LevelPush { uint16_t ep; uint8_t level; bool also_on; bool force_off; };
}
static void matter_push_level_work(intptr_t arg) {
    LevelPush *p = reinterpret_cast<LevelPush *>(arg);
    esp_matter_attr_val_t v = esp_matter_nullable_uint8(p->level);
    esp_err_t r = attribute::update(p->ep, LevelControl::Id,
                                    LevelControl::Attributes::CurrentLevel::Id, &v);
    if (r != ESP_OK) ESP_LOGE(TAG, "level update failed: %d", r);
    if (p->also_on || p->force_off) {
        esp_matter_attr_val_t vo = esp_matter_bool(p->also_on);
        r = attribute::update(p->ep, OnOff::Id, OnOff::Attributes::OnOff::Id, &vo);
        if (r != ESP_OK) ESP_LOGE(TAG, "onoff update failed: %d", r);
    }
    delete p;
}

extern "C" void matter_push_level_from_angle(float angle_rad) {
    if (s_endpoint_id == 0) return;
    if (angle_rad < 0.0f) angle_rad = 0.0f;
    if (angle_rad > ANGLE_MAX) angle_rad = ANGLE_MAX;

    // +0.5 rounding so a final knob click maps to 254 (else off-by-one shows 99% in Home).
    int raw = (angle_rad >= ANGLE_MAX)
                ? 254
                : (int)((angle_rad / ANGLE_MAX) * 254.0f + 0.5f);
    bool want_off = (angle_rad <= 0.0f);
    int lv = raw;
    if (lv < 1)   lv = 1;
    if (lv > 254) lv = 254;
    uint8_t new_level = (uint8_t)lv;

    bool on_changed    = want_off ? s_on_off : !s_on_off;
    bool level_changed = (new_level != s_level);
    if (!on_changed && !level_changed) return;

    bool also_on    = !want_off && !s_on_off;
    bool force_off  =  want_off &&  s_on_off;
    if (also_on)   s_on_off = true;
    if (force_off) s_on_off = false;
    s_level = new_level;

    ESP_LOGI(TAG, "push: knob angle=%.2f -> level=%u%s",
             angle_rad, (unsigned)new_level,
             also_on ? " +OnOff=true" : (force_off ? " +OnOff=false" : ""));

    auto *p = new LevelPush{ s_endpoint_id, new_level, also_on, force_off };
    chip::DeviceLayer::PlatformMgr().ScheduleWork(matter_push_level_work,
                                                  reinterpret_cast<intptr_t>(p));
}
