// Bridge between Matter cluster callbacks (CHIP task) and BrushlessLamp hardware.
#pragma once

#include <esp_err.h>
#include <sdkconfig.h>

#if CONFIG_APP_MATTER_ENABLED
#include <esp_matter.h>
#endif

using app_driver_handle_t = void *;

app_driver_handle_t app_driver_light_init(uint16_t light_endpoint_id);
void app_driver_tick();

#if CONFIG_APP_MATTER_ENABLED
esp_err_t app_driver_attribute_update(app_driver_handle_t handle,
                                      uint16_t endpoint_id,
                                      uint32_t cluster_id,
                                      uint32_t attribute_id,
                                      esp_matter_attr_val_t *val);
esp_err_t app_driver_apply_persisted(uint16_t endpoint_id);
void app_driver_push_to_matter(uint16_t endpoint_id);
#endif
