// Matter ColorTemperatureLight endpoint. Inbound writes drive motor + LEDs via
// apply_state(); knob settle pushes Level/OnOff back to subscribers via the
// CHIP task to serialise data-model access.

#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>

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

// Override arduino-esp32's weak btInUse so initArduino doesn't release the BT controller heap before Matter brings up BLE. __used survives LTO.
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
static volatile uint16_t s_color_temp_mireds  = COLORTEMP_DEFAULT;

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

static void reopen_commissioning_window() {
    auto &mgr = chip::Server::GetInstance().GetCommissioningWindowManager();
    if (mgr.IsCommissioningWindowOpen()) return;
    CHIP_ERROR err = mgr.OpenBasicCommissioningWindow(
        chip::System::Clock::Seconds16(300),
        chip::CommissioningWindowAdvertisement::kAllSupported);  // BLE + DNS-SD
    if (err != CHIP_NO_ERROR) {
        ESP_LOGE(TAG, "reopen commissioning window failed: %" CHIP_ERROR_FORMAT, err.Format());
    } else {
        ESP_LOGI(TAG, "commissioning window re-opened (300 s, BLE+DNS-SD)");
    }
}

static void event_cb(const ChipDeviceEvent *event, intptr_t arg) {
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "commissioning complete"); break;
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "IP address changed"); break;
    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        // NimBLE state machine often wedges after a partial commission (ble_gap_adv_set_data
        // returns 0x5B0001E); an in-place reopen leaves the wedge. Reboot clears it and
        // CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART re-arms commissioning on the next boot.
        ESP_LOGW(TAG, "failsafe timer expired — rebooting to clear BLE state");
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
            vTaskDelay(pdMS_TO_TICKS(200));
            esp_restart();
        }
        break;
    case chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished:
        ESP_LOGI(TAG, "BLE conn established"); break;
    case chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionClosed:
        ESP_LOGI(TAG, "BLE conn closed"); break;
    case chip::DeviceLayer::DeviceEventType::kFabricRemoved:
        ESP_LOGI(TAG, "fabric removed");
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
            wipe_local_nvs();
            reopen_commissioning_window();
        }
        break;
    default: break;
    }
}

void matter_app_init() {
    node::config_t node_config;
    node_t *node = node::create(&node_config, attribute_update_cb, identification_cb);
    if (!node) { ESP_LOGE(TAG, "node::create failed; restarting"); vTaskDelay(pdMS_TO_TICKS(200)); esp_restart(); }

    // esp-matter's root basic_information omits the optional SerialNumber attribute, so add it;
    // ATTRIBUTE_FLAG_MANAGED_INTERNALLY makes it read the chip-factory serial-num like vendor_name does.
    if (endpoint_t *root_ep = endpoint::get(node, 0)) {
        if (cluster_t *bi = cluster::get(root_ep, BasicInformation::Id)) {
            cluster::basic_information::attribute::create_serial_number(bi, nullptr, 0);
        }
    }

    color_temperature_light::config_t lc;
    lc.on_off.on_off                         = false;
    lc.on_off_lighting.start_up_on_off       = nullptr;
    lc.level_control.current_level           = 1;     // Matter MinLevel
    lc.level_control_lighting.start_up_current_level = nullptr;   // resume last persisted on power-up
    lc.color_control.color_mode              = (uint8_t)ColorControl::ColorMode::kColorTemperature;
    lc.color_control.enhanced_color_mode     = (uint8_t)ColorControl::ColorMode::kColorTemperature;
    // ColorCapabilities bit 4 = ColorTemperature supported; Google Home reads this + Physical Min/Max to render the slider.
    lc.color_control.color_capabilities                                  = 0x0010;
    lc.color_control_color_temperature.color_temperature_mireds          = COLORTEMP_DEFAULT;
    lc.color_control_color_temperature.color_temp_physical_min_mireds    = COLORTEMP_MIN;
    lc.color_control_color_temperature.color_temp_physical_max_mireds    = COLORTEMP_MAX;
    lc.color_control_color_temperature.couple_color_temp_to_level_min_mireds = COLORTEMP_MIN;
    lc.color_control_color_temperature.start_up_color_temperature_mireds = nullptr;   // resume last persisted

    // Single device type only. Also listing the Dimmable / On-Off subset types
    // made Google Home intermittently resolve the endpoint as a plain on/off
    // light (no brightness/CT slider, "something went wrong").
    endpoint_t *ep = color_temperature_light::create(node, &lc, ENDPOINT_FLAG_NONE, nullptr);
    if (!ep) { ESP_LOGE(TAG, "endpoint create failed; restarting"); vTaskDelay(pdMS_TO_TICKS(200)); esp_restart(); }
    s_endpoint_id = endpoint::get_id(ep);
    ESP_LOGI(TAG, "ColorTemperatureLight endpoint=%u", s_endpoint_id);

    // Defer Level / ColorTemp NVS writes so slider drags don't thrash flash.
    if (auto *a = attribute::get(s_endpoint_id, LevelControl::Id,
                                 LevelControl::Attributes::CurrentLevel::Id)) attribute::set_deferred_persistence(a);
    if (auto *a = attribute::get(s_endpoint_id, ColorControl::Id,
                                 ColorControl::Attributes::ColorTemperatureMireds::Id)) attribute::set_deferred_persistence(a);

    // Seed the shadow cache from the values the data model restored from NVS —
    // attribute restore doesn't fire attribute_update_cb, so without this an
    // Off→reboot→On round-trip applies level=1 (~0% height) instead of the saved level.
    {
        esp_matter_attr_val_t v = esp_matter_invalid(nullptr);
        if (attribute::get_val(s_endpoint_id, OnOff::Id,
                               OnOff::Attributes::OnOff::Id, &v) == ESP_OK) s_on_off = v.val.b;
        v = esp_matter_invalid(nullptr);
        if (attribute::get_val(s_endpoint_id, LevelControl::Id,
                               LevelControl::Attributes::CurrentLevel::Id, &v) == ESP_OK &&
            v.val.u8 >= 1 && v.val.u8 <= 254) s_level = v.val.u8;
    }

    esp_err_t err = esp_matter::start(event_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_matter::start err=%d; restarting", err);
        vTaskDelay(pdMS_TO_TICKS(200));
        esp_restart();
    }

    // Mains-powered: keep the modem awake so Matter commands aren't gated on the AP's DTIM interval.
    esp_wifi_set_ps(WIFI_PS_NONE);

    // Push the LED-side restored CT into Matter from the CHIP task — attribute::update
    // isn't safe from app_main once the stack is live.
    {
        uint16_t ct = leds_get_colortemp();
        s_color_temp_mireds = ct;
        chip::DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t arg) {
            esp_matter_attr_val_t v = esp_matter_uint16((uint16_t)arg);
            attribute::update(s_endpoint_id, ColorControl::Id,
                              ColorControl::Attributes::ColorTemperatureMireds::Id, &v);
        }, (intptr_t)ct);
    }

    // Use printf, not ChipLogProgress: chip[*] INFO chatter during PASE/AddNOC backpressures USB-CDC TX and times commissioning out.
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

extern "C" bool matter_get_on_off(void) { return s_on_off; }

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
    CHIP_ERROR err = chip::DeviceLayer::PlatformMgr().ScheduleWork(
        matter_push_onoff_work, reinterpret_cast<intptr_t>(p));
    if (err != CHIP_NO_ERROR) {
        ESP_LOGE(TAG, "ScheduleWork(onoff) failed: %" CHIP_ERROR_FORMAT, err.Format());
        delete p;
    }
}

// also_on / force_off are mutually exclusive: knob up from rest → OnOff=true;
// knob down to 0 → OnOff=false only — CurrentLevel keeps the pre-off value so
// the next On restores the remembered height (Home renders "Off" regardless).
namespace {
struct LevelPush { uint16_t ep; uint8_t level; bool also_on; bool force_off; };
}
static void matter_push_level_work(intptr_t arg) {
    LevelPush *p = reinterpret_cast<LevelPush *>(arg);
    if (!p->force_off) {
        // Must use the nullable variant — the non-nullable one is silently rejected with ESP_ERR_INVALID_ARG (258) because the cluster declares CurrentLevel nullable.
        esp_matter_attr_val_t v = esp_matter_nullable_uint8(p->level);
        esp_err_t r = attribute::update(p->ep, LevelControl::Id,
                                        LevelControl::Attributes::CurrentLevel::Id, &v);
        if (r != ESP_OK) ESP_LOGE(TAG, "level update failed: %d", r);
    }
    if (p->also_on || p->force_off) {
        esp_matter_attr_val_t vo = esp_matter_bool(p->also_on);
        esp_err_t r = attribute::update(p->ep, OnOff::Id, OnOff::Attributes::OnOff::Id, &vo);
        if (r != ESP_OK) ESP_LOGE(TAG, "onoff update failed: %d", r);
    }
    delete p;
}

// Knob-driven CT change in CT mode; push the new mireds back to ColorControl so Home apps see it.
namespace { struct CTPush { uint16_t ep; uint16_t mireds; }; }
static void matter_push_colortemp_work(intptr_t arg) {
    CTPush *p = reinterpret_cast<CTPush *>(arg);
    esp_matter_attr_val_t v = esp_matter_uint16(p->mireds);
    esp_err_t r = attribute::update(p->ep, ColorControl::Id,
                                    ColorControl::Attributes::ColorTemperatureMireds::Id, &v);
    if (r != ESP_OK) ESP_LOGE(TAG, "ct update failed: %d", r);
    delete p;
}
extern "C" void matter_push_colortemp(uint16_t mireds) {
    if (s_endpoint_id == 0) return;
    if (mireds == s_color_temp_mireds) return;
    s_color_temp_mireds = mireds;
    auto *p = new CTPush{ s_endpoint_id, mireds };
    CHIP_ERROR err = chip::DeviceLayer::PlatformMgr().ScheduleWork(
        matter_push_colortemp_work, reinterpret_cast<intptr_t>(p));
    if (err != CHIP_NO_ERROR) {
        ESP_LOGE(TAG, "ScheduleWork(ct) failed: %" CHIP_ERROR_FORMAT, err.Format());
        delete p;
    }
}

// Runs on the core-1 FOC task (settle callback) — DEBUG log only, no blocking work.
extern "C" void matter_push_level_from_angle(float angle_rad, bool allow_on) {
    if (s_endpoint_id == 0) return;
    if (angle_rad < 0.0f) angle_rad = 0.0f;
    if (angle_rad > ANGLE_MAX) angle_rad = ANGLE_MAX;

    // Half-a-level-step epsilon: the settle angle lands within encoder/NVS-save
    // noise of 0, so an exact <= 0 test made knob-to-zero a coin flip between
    // "Off" and "On, 1%" in Home.
    bool want_off = (angle_rad < 0.5f * ANGLE_MAX / 254.0f);

    bool also_on   = false;
    bool force_off = false;
    uint8_t new_level = s_level;
    if (want_off) {
        if (!s_on_off) return;     // already off; level untouched
        s_on_off  = false;
        force_off = true;
    } else {
        // Only a knob raise (allow_on) may power the lamp on from off. A stall, a
        // boot restore or a re-home that lands at a high angle must NOT turn it on,
        // or the lamp would refuse to stay off and keep coming back on.
        if (!s_on_off && !allow_on) return;
        // +0.5 rounding so a final knob click maps to 254 (else off-by-one shows 99% in Home).
        int lv = (angle_rad >= ANGLE_MAX)
                   ? 254
                   : (int)((angle_rad / ANGLE_MAX) * 254.0f + 0.5f);
        if (lv < 1)   lv = 1;
        if (lv > 254) lv = 254;
        new_level = (uint8_t)lv;
        also_on   = !s_on_off;
        if (!also_on && new_level == s_level) return;
        s_on_off = true;
        s_level  = new_level;
    }

    ESP_LOGD(TAG, "push: knob angle=%.2f -> level=%u%s",
             angle_rad, (unsigned)new_level,
             also_on ? " +OnOff=true" : (force_off ? " +OnOff=false" : ""));

    auto *p = new LevelPush{ s_endpoint_id, new_level, also_on, force_off };
    CHIP_ERROR err = chip::DeviceLayer::PlatformMgr().ScheduleWork(
        matter_push_level_work, reinterpret_cast<intptr_t>(p));
    if (err != CHIP_NO_ERROR) {
        ESP_LOGE(TAG, "ScheduleWork(level) failed: %" CHIP_ERROR_FORMAT, err.Format());
        delete p;
    }
}
