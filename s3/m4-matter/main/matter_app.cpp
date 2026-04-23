// Matter ColorTemperatureLight endpoint — the full smart-home surface.
//
// Inbound (controller app → device): attribute_update_cb fires on OnOff / Level /
// ColorTemperature writes, stores the new value, and apply_state() routes it into
// motor_set_target_angle() and the leds.cpp follower sees the new color temp via
// matter_get_color_temp_mireds(). Echo suppression (same-value short-circuit) keeps
// our own push-backs from re-triggering motor motion.
//
// Outbound (knob → device → controller app): motor.cpp's settle callback fires
// matter_push_level_from_angle() once per knob gesture (after the lamp comes to
// rest). attribute::update() runs on the CHIP task via PlatformMgr::ScheduleWork
// so cross-task access to the data model is serialized.
//
// One landmine worth surfacing in the code: LevelControl::CurrentLevel is declared
// nullable in esp_matter's data-model; pushing esp_matter_uint8(v) (non-nullable)
// is silently rejected with ESP_ERR_INVALID_ARG=258. Use esp_matter_nullable_uint8.

#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_console.h>

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include "matter_app.h"
#include "motor.h"
#include "leds.h"
#include "config.h"

// Override arduino-esp32's weak btInUse() so initArduino() does NOT call
// esp_bt_controller_mem_release(ESP_BT_MODE_BTDM). Without this, Arduino frees
// the BT controller's own memory into the heap, then Matter's bt_controller_init
// malloc's from that same internal-RAM pool and reads a corrupted TLSF header —
// LoadProhibited during r_llm_env_init. Discovered via phase-15 research pass;
// same fix that ESP Rainmaker uses when co-existing with Matter+NimBLE on S3.
extern "C" bool btInUse(void) { return true; }

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

static const char *TAG = "matter";

static uint16_t s_endpoint_id = 0;

// Cached attribute state — motor/LEDs read this, attribute_update_cb writes it.
static bool     s_on_off             = false;
static uint8_t  s_level              = 1;    // Matter MinLevel
static uint16_t s_color_temp_mireds  = 250;  // neutral daylight

static void apply_state() {
    // Matter state → device state:
    //   OnOff=false: just set motor target to 0. The LEDs follow shaft_angle
    //     continuously via the angle curve, so they fade out WITH the physical
    //     motion (kinetic "closing" feel) instead of snapping dark the instant
    //     the Home app toggle flips.
    //   OnOff=true:  motor target = (level / 254) × ANGLE_MAX, LEDs light via
    //     angle curve as shaft climbs.
    //   ColorTemp always mirrors Matter's ColorTemperatureMireds attribute.
    // leds_set_on(true) is sticky (default) — we don't flip it here. The angle
    // curve reaches 0 duty at shaft_angle=0 naturally, so an explicit hard kill
    // isn't needed for the Matter off path.
    leds_set_colortemp(s_color_temp_mireds);
    float target = s_on_off ? ((float)s_level / 254.0f) * ANGLE_MAX : 0.0f;
    motor_set_target_angle(target);
    ESP_LOGI(TAG, "apply: on=%d level=%u ct=%umir -> tgt=%.2frad",
             (int)s_on_off, (unsigned)s_level, (unsigned)s_color_temp_mireds, target);
}

static esp_err_t attribute_update_cb(callback_type_t type, uint16_t endpoint_id,
                                     uint32_t cluster_id, uint32_t attribute_id,
                                     esp_matter_attr_val_t *val, void *priv_data) {
    if (type != POST_UPDATE || endpoint_id != s_endpoint_id) return ESP_OK;

    if (cluster_id == OnOff::Id && attribute_id == OnOff::Attributes::OnOff::Id) {
        if (val->val.b == s_on_off) return ESP_OK;     // echo — skip
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
    // First-boot default; persists after first Matter write. 1 keeps a freshly paired
    // device at the bottom of travel rather than jumping to mid.
    lc.level_control.current_level           = 1;
    // Leave on_level null so off→on resumes the last persisted level instead of
    // slamming to a hard-coded value (the classic "always 50% on turn-on" bug).
    lc.level_control_lighting.start_up_current_level = nullptr;
    lc.color_control.color_mode              = (uint8_t)ColorControl::ColorMode::kColorTemperature;
    lc.color_control.enhanced_color_mode     = (uint8_t)ColorControl::ColorMode::kColorTemperature;
    lc.color_control_color_temperature.start_up_color_temperature_mireds = nullptr;

    endpoint_t *ep = color_temperature_light::create(node, &lc, ENDPOINT_FLAG_NONE, nullptr);
    if (!ep) { ESP_LOGE(TAG, "endpoint create failed"); return; }
    s_endpoint_id = endpoint::get_id(ep);
    ESP_LOGI(TAG, "ColorTemperatureLight endpoint=%u", s_endpoint_id);

    // Deferred persistence on rapidly-changing attrs — slider drags don't thrash NVS.
    if (auto *a = attribute::get(s_endpoint_id, LevelControl::Id,
                                 LevelControl::Attributes::CurrentLevel::Id)) attribute::set_deferred_persistence(a);
    if (auto *a = attribute::get(s_endpoint_id, ColorControl::Id,
                                 ColorControl::Attributes::ColorTemperatureMireds::Id)) attribute::set_deferred_persistence(a);

    esp_err_t err = esp_matter::start(event_cb);
    if (err != ESP_OK) { ESP_LOGE(TAG, "esp_matter::start err=%d", err); return; }

    // Print the exact commissioning payload so the user can paste it into their
    // controller app. The chip's generator uses the factory-data-provider values,
    // which for our current build (no production certs flashed) are Matter's test
    // defaults — but we'd rather print the true values than assume them.
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
        PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
    } else {
        ESP_LOGI(TAG, "already commissioned (%u fabrics)",
                 (unsigned)chip::Server::GetInstance().GetFabricTable().FabricCount());
    }

    // Nothing special uncommissioned — the fader on its own will hold s_on=true
    // (default) and the LED curve goes to 0 at shaft_angle=0, so the lamp sits
    // dark until the user rotates the knob. That's a cleaner "ready to pair"
    // state than the M3-era hard-coded 16/8 dim glow.

#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
    esp_matter::console::attribute_register_commands();
    esp_matter::console::init();
#endif
}

extern "C" unsigned short matter_get_color_temp_mireds(void) { return s_color_temp_mireds; }
extern "C" bool           matter_get_on_off(void)            { return s_on_off; }

// Outbound push — marshals attribute::update() onto the CHIP task so arbitrary
// FreeRTOS threads can call us safely.
namespace {
struct LevelPush { uint16_t ep; uint8_t level; bool also_on; };
}
static void matter_push_level_work(intptr_t arg) {
    LevelPush *p = reinterpret_cast<LevelPush *>(arg);
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
    if (s_endpoint_id == 0) return;
    if (angle_rad < 0.0f) angle_rad = 0.0f;
    if (angle_rad > ANGLE_MAX) angle_rad = ANGLE_MAX;
    int lv = (int)((angle_rad / ANGLE_MAX) * 254.0f);
    if (lv < 1)   lv = 1;                // Matter MinLevel
    if (lv > 254) lv = 254;
    uint8_t new_level = (uint8_t)lv;
    if (new_level == s_level) return;    // already in sync

    bool also_on = !s_on_off;
    s_level  = new_level;
    if (also_on) s_on_off = true;

    ESP_LOGI(TAG, "push: knob angle=%.2f -> level=%u (%s)",
             angle_rad, (unsigned)new_level, also_on ? "+OnOff=true" : "level-only");

    auto *p = new LevelPush{ s_endpoint_id, new_level, also_on };
    chip::DeviceLayer::PlatformMgr().ScheduleWork(matter_push_level_work,
                                                  reinterpret_cast<intptr_t>(p));
}
