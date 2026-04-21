// BrushlessLamp — native ESP-IDF entry. Matter stack is gated behind CONFIG_APP_MATTER_ENABLED
// so we can build a motor-only diagnostic firmware to isolate FOC behavior from Matter/WiFi ISR load.

#include <esp_err.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_task_wdt.h>
#include <nvs_flash.h>
#include <sdkconfig.h>

#if CONFIG_APP_MATTER_ENABLED
#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>
#include <app/server/Server.h>
#include <setup_payload/OnboardingCodesUtil.h>
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_driver.h"
#include "input.h"
#include "motor.h"
#include "leds.h"

static const char *TAG = "blamp";

// Set in app_main when esp_reset_reason() reports the prior boot ended in a panic/WDT.
// Suppresses esp_matter::start() so a CHIP-stack crash can't boot-loop the device — the
// 9 s long-press path stays alive so the user can factory-reset out.
static bool s_safe_mode = false;

#if CONFIG_APP_MATTER_ENABLED
using namespace esp_matter;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

static uint16_t s_light_endpoint_id = 0;
static app_driver_handle_t s_drv = nullptr;

static esp_err_t app_attribute_update_cb(attribute::callback_type_t type,
                                         uint16_t endpoint_id,
                                         uint32_t cluster_id,
                                         uint32_t attribute_id,
                                         esp_matter_attr_val_t *val,
                                         void *priv_data) {
    if (type == attribute::PRE_UPDATE) {
        return app_driver_attribute_update(s_drv, endpoint_id, cluster_id, attribute_id, val);
    }
    return ESP_OK;
}

static esp_err_t app_identification_cb(identification::callback_type_t,
                                       uint16_t, uint8_t, uint8_t, void *) {
    return ESP_OK;
}

static void app_event_cb(const ChipDeviceEvent *event, intptr_t /*arg*/) {
    using namespace chip::DeviceLayer;
    switch (event->Type) {
        case DeviceEventType::kCommissioningComplete:
            ESP_LOGI(TAG, "event: CommissioningComplete — initializing motor + FOC");
            app_driver_apply_persisted(s_light_endpoint_id);
            motor_init_and_foc();
            motor_start_foc_timer();
            break;
        case DeviceEventType::kBLEDeinitialized:
            ESP_LOGI(TAG, "event: BLEDeinitialized — free heap: %lu",
                     (unsigned long)esp_get_free_heap_size());
            break;
        case DeviceEventType::kInterfaceIpAddressChanged:
            ESP_LOGI(TAG, "event: InterfaceIpAddressChanged");
            break;
        case DeviceEventType::kFabricRemoved:
            ESP_LOGI(TAG, "event: FabricRemoved");
            if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
                chip::Server::GetInstance().GetCommissioningWindowManager()
                    .OpenBasicCommissioningWindow(
                        chip::System::Clock::Seconds16(300),
                        chip::CommissioningWindowAdvertisement::kDnssdOnly);
            }
            break;
        default:
            break;
    }
}
#endif  // CONFIG_APP_MATTER_ENABLED

// Single loop matching Arduino M4's pattern: priority 1 (= CHIP), tight loop, no
// explicit taskYIELD. FreeRTOS time-slices both tasks at tick boundaries (1 ms each),
// so each gets ~50 % CPU when both are ready. This is what M4 does and the user reports
// is smooth there. Earlier experiments:
//  - taskYIELD per iteration → forces a context switch per iteration; if CHIP is ready
//    we switch immediately and don't come back until CHIP yields, dropping our effective
//    rate to ~hundreds of iters/sec (PID rate ~30–80 Hz with motion_downsample=5).
//    Result: PID oscillation, motor felt choppy.
//  - Two-task split (motor at priority 2 vTaskDelayUntil 1 kHz) → PID capped at exactly
//    200 Hz, even worse oscillation than the taskYIELD version.
// Without taskYIELD we get ~thousands of iters/sec → PID at 200–800 Hz, smooth.
static void app_loop_task(void *) {
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    for (;;) {
        input_poll();
        app_driver_motor_tick();
        app_driver_ui_tick();
        esp_task_wdt_reset();
    }
}

extern "C" void app_main() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Reconfigure the task watchdog (auto-inited by ESP-IDF) to a 10 s timeout matching
    // Arduino M4. Long enough to absorb Matter's commissioning crypto bursts; short enough
    // to recover from a real hang within a user-noticeable window.
    esp_task_wdt_config_t wdt_cfg = {};
    wdt_cfg.timeout_ms     = 10000;
    wdt_cfg.idle_core_mask = 0;
    wdt_cfg.trigger_panic  = true;
    esp_task_wdt_reconfigure(&wdt_cfg);

    esp_reset_reason_t reason = esp_reset_reason();
    s_safe_mode = (reason == ESP_RST_PANIC || reason == ESP_RST_TASK_WDT || reason == ESP_RST_INT_WDT);
    ESP_LOGI(TAG, "boot reason: %d  safe_mode=%d  free heap: %lu",
             (int)reason, (int)s_safe_mode, (unsigned long)esp_get_free_heap_size());

#if CONFIG_APP_MATTER_ENABLED
    if (!s_safe_mode) {
        node::config_t node_config;
        node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);

        color_temperature_light::config_t light_cfg;
        light_cfg.on_off.on_off                              = false;
        light_cfg.on_off_lighting.start_up_on_off            = nullptr;
        light_cfg.level_control.current_level                = 1;
        // on_level left at its null default — otherwise Matter forces CurrentLevel to this value on every Off→On.
        light_cfg.level_control_lighting.start_up_current_level = nullptr;
        light_cfg.color_control.color_mode                   = (uint8_t)ColorControl::ColorMode::kColorTemperature;
        light_cfg.color_control.enhanced_color_mode          = (uint8_t)ColorControl::ColorMode::kColorTemperature;
        light_cfg.color_control_color_temperature.start_up_color_temperature_mireds = nullptr;
        endpoint_t *ep = color_temperature_light::create(node, &light_cfg, ENDPOINT_FLAG_NONE, nullptr);
        s_light_endpoint_id = endpoint::get_id(ep);
        ESP_LOGI(TAG, "color_temperature_light endpoint id: %u", s_light_endpoint_id);

        attribute::set_deferred_persistence(attribute::get(
            s_light_endpoint_id, LevelControl::Id, LevelControl::Attributes::CurrentLevel::Id));
        attribute::set_deferred_persistence(attribute::get(
            s_light_endpoint_id, ColorControl::Id, ColorControl::Attributes::ColorTemperatureMireds::Id));

        s_drv = app_driver_light_init(s_light_endpoint_id);

        ESP_ERROR_CHECK(esp_matter::start(app_event_cb));

        if (chip::Server::GetInstance().GetFabricTable().FabricCount() > 0) {
            ESP_LOGI(TAG, "already commissioned — initializing motor + FOC");
            app_driver_apply_persisted(s_light_endpoint_id);
            motor_init_and_foc();
            motor_start_foc_timer();
        } else {
            ESP_LOGI(TAG, "uncommissioned — short-click the knob to bench-test the motor");
            PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
        }
    } else {
        // Skip Matter entirely after a panic boot. Bring the local UI up so the user can
        // 9-second long-press to factory-reset without re-triggering whatever crashed.
        ESP_LOGW(TAG, "SAFE MODE — last boot panicked; Matter is OFF. Long-press 9s to factory-reset.");
        s_drv = app_driver_light_init(0);
        leds_pulse(2);
    }
#else
    // Motor-only diagnostic: no Matter, no WiFi, no BLE. Motor inits immediately.
    (void)s_safe_mode;
    ESP_LOGI(TAG, "motor-only build (CONFIG_APP_MATTER_ENABLED=n)");
    app_driver_light_init(0);
    motor_init_and_foc();
    motor_start_foc_timer();
#endif

    xTaskCreate(app_loop_task, "blamp_loop", 4096, nullptr, tskIDLE_PRIORITY + 1, nullptr);

    ESP_LOGI(TAG, "app_main done — free heap: %lu",
             (unsigned long)esp_get_free_heap_size());
}
