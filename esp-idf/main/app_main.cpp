// BrushlessLamp — native ESP-IDF entry. Matter stack is gated behind CONFIG_APP_MATTER_ENABLED
// so we can build a motor-only diagnostic firmware to isolate FOC behavior from Matter/WiFi ISR load.

#include <esp_err.h>
#include <esp_log.h>
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

// M3b pattern: spin freely, anchor → input → tick in that order. taskYIELD lets
// equal-priority tasks run; we don't block on a FreeRTOS tick so our effective
// loop rate matches what Arduino's loopTask() achieved (~5-10 kHz uncontested).
static void app_loop_task(void *) {
    for (;;) {
        motor_idle_anchor();
        input_poll();
        app_driver_tick();
        taskYIELD();
    }
}

extern "C" void app_main() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_LOGI(TAG, "boot — free heap: %lu", (unsigned long)esp_get_free_heap_size());

#if CONFIG_APP_MATTER_ENABLED
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
    } else {
        ESP_LOGI(TAG, "uncommissioned — short-click the knob to bench-test the motor");
        PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
    }
#else
    // Motor-only diagnostic: no Matter, no WiFi, no BLE. Motor inits immediately.
    ESP_LOGI(TAG, "motor-only build (CONFIG_APP_MATTER_ENABLED=n)");
    app_driver_light_init(0);
    motor_init_and_foc();
#endif

    xTaskCreate(app_loop_task, "blamp_loop", 4096, nullptr, tskIDLE_PRIORITY + 2, nullptr);

    ESP_LOGI(TAG, "app_main done — free heap: %lu",
             (unsigned long)esp_get_free_heap_size());
}
