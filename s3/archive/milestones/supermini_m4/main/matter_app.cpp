// Matter ColorTemperatureLight endpoint.
//
// Inbound: attribute_update_cb fires on OnOff / Level / ColorTemperature
// writes, stores the new value, and apply_state() pushes it into the motor
// target and leds_set_colortemp. Echo suppression (same-value short-circuit)
// keeps our own pushbacks from re-triggering motor motion.
//
// Outbound: motor.cpp's settle callback fires matter_push_level_from_angle()
// once per knob gesture (at rest). attribute::update() runs on the CHIP task
// via PlatformMgr::ScheduleWork, serialising data-model access.
//
// Note: LevelControl::CurrentLevel is nullable in esp_matter's data model;
// push with esp_matter_nullable_uint8() — the non-nullable variant is rejected
// as ESP_ERR_INVALID_ARG=258.

#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_console.h>

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <nvs.h>

#include "matter_app.h"
#include "motor.h"
#include "leds.h"
#include "config.h"

// esp_matter::factory_reset() only clears CHIP's namespaces (chip-config,
// chip-counters, KVS). Our motor / LED / input namespaces stay put across
// factory reset, which means the cached sensor_direction from a prior session
// survives — and initFOC then skips the direction sweep on the next boot,
// running only the 700 ms zero-angle pulse. On certain rotor resting positions
// that short pulse alone isn't enough to finish alignment cleanly, so the
// motor struggles post-reset. Wiping our NVS too forces a full re-calibration
// on the next boot, which is what the user expects after a factory reset.
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

// Strong override of arduino-esp32's weak btInUse() — prevents
// initArduino() from calling esp_bt_controller_mem_release(), which would
// crash Matter's BLE bring-up. __attribute__((used)) survives LTO.
extern "C" __attribute__((used)) bool btInUse(void) { return true; }

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

static const char *TAG = "matter";

static volatile uint16_t s_endpoint_id = 0;

// Cached attribute state. CHIP task writes via attribute_update_cb; motor/LEDs
// read from other FreeRTOS tasks. volatile so the reader doesn't hoist the load
// across loop iterations — float-tearing on a 16-bit/8-bit aligned word is
// benign on xtensa-esp32s3.
static volatile bool     s_on_off             = false;
static volatile uint8_t  s_level              = 1;    // Matter MinLevel
static volatile uint16_t s_color_temp_mireds  = 250;  // neutral daylight

static void apply_state() {
    // OnOff off → motor target 0 (LEDs fade with the physical travel via the
    // angle curve, not an instant snap). OnOff on → target = level/254 ×
    // ANGLE_MAX. ColorTemp mirrors the Matter attribute into the fader.
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
            // Remote decommission (Home app "Remove") — CHIP wipes its own
            // NVS but not ours, so clear foc_cal / leds / input too.
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

    // Log the live VID/PID/discriminator + onboarding payload. When fctry holds
    // real mfg-tool data the VID/PID/discriminator are per-device; the passcode
    // Matter prints in the QR is a fallback (we persist only the Spake2+
    // verifier, not the raw code), so the real QR to scan lives in the CSV
    // esp-matter-mfg-tool emits alongside the partition binaries.
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
        uint16_t vid = 0, pid = 0, discriminator = 0;
        auto *info = chip::DeviceLayer::GetDeviceInstanceInfoProvider();
        auto *cd   = chip::DeviceLayer::GetCommissionableDataProvider();
        if (info) { info->GetVendorId(vid); info->GetProductId(pid); }
        if (cd)   { cd->GetSetupDiscriminator(discriminator); }
        ESP_LOGI(TAG, "factory data: VID=0x%04X PID=0x%04X discriminator=%u",
                 vid, pid, (unsigned)discriminator);
        PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
    } else {
        ESP_LOGI(TAG, "already commissioned (%u fabrics)",
                 (unsigned)chip::Server::GetInstance().GetFabricTable().FabricCount());
    }

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

// Button-driven OnOff toggle. Mirrors what attribute_update_cb would do on a
// Matter-side write: updates local state, runs apply_state (which drives the
// motor target to 0 or to level/254*ANGLE_MAX), then pushes OnOff out to
// subscribers. Level stays put so off→on restores last brightness.
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

// Outbound push — marshals attribute::update() onto the CHIP task so arbitrary
// FreeRTOS threads can call us safely. `also_on` and `force_off` are mutually
// exclusive: knob motion up from rest implies OnOff=true, knob motion down to 0
// implies OnOff=false (otherwise the Home app stays at "On, 1%" because
// MinLevel=1 is the floor the LevelControl cluster can express).
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

    // Raw level before the MinLevel clamp. Rounded (CylinderLamp pattern) so max
    // knob position maps to 254 exactly — without +0.5 a final angle just shy of
    // ANGLE_MAX truncates to 253 and the Home app shows 99 %. want_off only
    // fires at a true 0 angle, so one detent off zero pushes level=1 + OnOff=true.
    int raw = (angle_rad >= ANGLE_MAX)
                ? 254
                : (int)((angle_rad / ANGLE_MAX) * 254.0f + 0.5f);
    bool want_off = (angle_rad <= 0.0f);
    int lv = raw;
    if (lv < 1)   lv = 1;
    if (lv > 254) lv = 254;
    uint8_t new_level = (uint8_t)lv;

    bool on_changed    = want_off ? s_on_off : !s_on_off;   // anything non-zero implies on
    bool level_changed = (new_level != s_level);
    if (!on_changed && !level_changed) return;              // already in sync

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
