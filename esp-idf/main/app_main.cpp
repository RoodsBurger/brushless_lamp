// BrushlessLamp — native ESP-IDF + esp-matter implementation
// Mirrors CylinderLamp's proven Matter pattern: lock-free, event-driven syncs,
// motor init deferred until commissioning completes (or short-click escape).

#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>

#include <app/server/Server.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_driver.h"
#include "input.h"
#include "motor.h"
#include "leds.h"

static const char *TAG = "blamp";

using namespace esp_matter;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

static uint16_t s_light_endpoint_id = 0;
static app_driver_handle_t s_drv = nullptr;

// Inbound: Matter controller writes an attribute → CHIP task fires this on PRE_UPDATE.
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

// Device-event hook: commissioning state changes, BLE deinit, fabric removal, etc.
static void app_event_cb(const ChipDeviceEvent *event, intptr_t /*arg*/) {
    using namespace chip::DeviceLayer;
    switch (event->Type) {
        case DeviceEventType::kCommissioningComplete:
            ESP_LOGI(TAG, "event: CommissioningComplete — initializing motor + FOC");
            // apply_persisted first so motor_init_and_foc's sensor_offset anchor
            // lands on the restored commanded value, not on zero.
            app_driver_apply_persisted(s_light_endpoint_id);
            motor_init_and_foc();
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

// Background task — drives motor ramp + LED render + input poll.
// Runs at slightly elevated priority so it gets consistent service from the scheduler.
static void app_loop_task(void *) {
    const TickType_t period = pdMS_TO_TICKS(1);   // 1 kHz — matches FreeRTOS tick; position PID runs every ms
    TickType_t next = xTaskGetTickCount();
    for (;;) {
        input_poll();
        app_driver_tick();
        vTaskDelayUntil(&next, period);
    }
}

extern "C" void app_main() {
    // NVS init (esp-matter & WiFi need this).
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_LOGI(TAG, "boot — free heap: %lu", (unsigned long)esp_get_free_heap_size());

    // Build the Matter data model: root node + ColorTemperatureLight on endpoint 1.
    node::config_t node_config;
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);

    color_temperature_light::config_t light_cfg;
    light_cfg.on_off.on_off                              = false;
    light_cfg.on_off_lighting.start_up_on_off            = nullptr;   // restore last value on power-up
    light_cfg.level_control.current_level                = 1;
    // Do not set on_level — leaving it at its null default is what keeps Matter from forcing CurrentLevel to ~50% on every Off→On transition.
    light_cfg.level_control_lighting.start_up_current_level = nullptr;
    light_cfg.color_control.color_mode                   = (uint8_t)ColorControl::ColorMode::kColorTemperature;
    light_cfg.color_control.enhanced_color_mode          = (uint8_t)ColorControl::ColorMode::kColorTemperature;
    light_cfg.color_control_color_temperature.start_up_color_temperature_mireds = nullptr;
    endpoint_t *ep = color_temperature_light::create(node, &light_cfg, ENDPOINT_FLAG_NONE, nullptr);
    s_light_endpoint_id = endpoint::get_id(ep);
    ESP_LOGI(TAG, "color_temperature_light endpoint id: %u", s_light_endpoint_id);

    // Tell Matter to batch-persist Level/CT to NVS so we don't burn flash on every
    // knob detent (the framework still persists eventually).
    attribute::set_deferred_persistence(attribute::get(
        s_light_endpoint_id, LevelControl::Id, LevelControl::Attributes::CurrentLevel::Id));
    attribute::set_deferred_persistence(attribute::get(
        s_light_endpoint_id, ColorControl::Id, ColorControl::Attributes::ColorTemperatureMireds::Id));

    // Bring up our hardware drivers. Motor stays cold; init is gated on commissioning
    // complete OR user short-click.
    s_drv = app_driver_light_init(s_light_endpoint_id);

    // Start the CHIP stack. Hands BLE/WiFi over to the Matter task.
    ESP_ERROR_CHECK(esp_matter::start(app_event_cb));

    // If we already have a fabric (post-reboot), bring up the motor immediately
    // and apply persisted attribute values to hardware.
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() > 0) {
        ESP_LOGI(TAG, "already commissioned — initializing motor + FOC");
        // Order matters: apply_persisted sets g_commanded from NVS-restored Matter
        // attribute values; motor_init_and_foc then anchors sensor_offset to that
        // commanded so the shaft doesn't seek at boot.
        app_driver_apply_persisted(s_light_endpoint_id);
        motor_init_and_foc();
    } else {
        ESP_LOGI(TAG, "uncommissioned — short-click the knob to bench-test the motor");
        // Print QR URL + manual pairing code for Google Home / Apple Home commissioning.
        PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
    }

    // Background loop task: motor + LED + input service.
    xTaskCreate(app_loop_task, "blamp_loop", 4096, nullptr, tskIDLE_PRIORITY + 2, nullptr);

    ESP_LOGI(TAG, "app_main done — free heap: %lu",
             (unsigned long)esp_get_free_heap_size());
}
