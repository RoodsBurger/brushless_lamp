// test_ble + arduino-as-component init layer.
//
// Mirrors s3/test_ble/main/main.c but pulls in arduino-esp32 and calls
// initArduino() before any radio init. If THIS build wedges on external
// power (where test_ble alone does not), the bug is in arduino-as-component;
// if it doesn't, the bug is higher up in esp-matter / CHIP.

#include <Arduino.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"

static const char *TAG = "test_ble_arduino";

#define LED_GPIO 21

static void boot_dbg_set(const char *key, uint32_t val) {
    nvs_handle_t h;
    if (nvs_open("boot_dbg", NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_u32(h, key, val);
    nvs_commit(h);
    nvs_close(h);
}

static void log_prev_reset_reason(void) {
    nvs_handle_t h;
    if (nvs_open("boot_dbg", NVS_READWRITE, &h) != ESP_OK) return;
    uint32_t prev_rst = 0xFF, boot_n = 0, prev_ckpt = 0, prev_evt = 0;
    nvs_get_u32(h, "last_rst",  &prev_rst);
    nvs_get_u32(h, "boot_n",    &boot_n);
    nvs_get_u32(h, "last_ckpt", &prev_ckpt);
    nvs_get_u32(h, "last_evt",  &prev_evt);
    uint32_t cur_rst = (uint32_t)esp_reset_reason();
    boot_n++;
    nvs_set_u32(h, "last_rst",  cur_rst);
    nvs_set_u32(h, "boot_n",    boot_n);
    nvs_set_u32(h, "last_ckpt", 0);
    nvs_set_u32(h, "last_evt",  0);
    nvs_commit(h);
    nvs_close(h);
    printf("[boot_dbg] cur_reset=0x%02X prev_reset=0x%02X boot#=%u prev_ckpt=%u prev_evt=0x%X\n",
           (unsigned)cur_rst, (unsigned)prev_rst, (unsigned)boot_n,
           (unsigned)prev_ckpt, (unsigned)prev_evt);
}

static void beacon(uint8_t n) {
    for (uint8_t i = 0; i < n; i++) {
        gpio_set_level((gpio_num_t)LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(40));
        gpio_set_level((gpio_num_t)LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(60));
    }
    boot_dbg_set("last_ckpt", (uint32_t)n);
    vTaskDelay(pdMS_TO_TICKS(150));
}

static uint8_t own_addr_type;

static int gap_event(struct ble_gap_event *event, void *arg);

static void start_advertising(void) {
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof fields);
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    int rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) { ESP_LOGE(TAG, "set_fields rc=%d", rc); return; }
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, gap_event, NULL);
    if (rc != 0) ESP_LOGE(TAG, "adv_start rc=%d", rc);
    else printf("[test_ble_arduino] advertising started\n");
}

static int gap_event(struct ble_gap_event *event, void *arg) {
    boot_dbg_set("last_evt", (uint32_t)event->type);
    printf("[test_ble_arduino] gap event type=%d\n", event->type);
    if (event->type == BLE_GAP_EVENT_ADV_COMPLETE ||
        event->type == BLE_GAP_EVENT_DISCONNECT) {
        start_advertising();
    }
    return 0;
}

static void on_sync(void) {
    beacon(5);
    int rc = ble_hs_util_ensure_addr(0);
    if (rc != 0) { ESP_LOGE(TAG, "ensure_addr rc=%d", rc); return; }
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) { ESP_LOGE(TAG, "infer_auto rc=%d", rc); return; }
    beacon(6);
    start_advertising();
    beacon(7);
}

static void on_reset(int reason) {
    ESP_LOGE(TAG, "host reset reason=%d", reason);
}

static void host_task(void *param) {
    nimble_port_run();
    nimble_port_freertos_deinit();
}

static void heartbeat_task(void *arg) {
    while (1) {
        gpio_set_level((gpio_num_t)LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(20));
        gpio_set_level((gpio_num_t)LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(180));
    }
}

extern "C" void app_main(void) {
    gpio_set_direction((gpio_num_t)LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)LED_GPIO, 1);

    vTaskDelay(pdMS_TO_TICKS(2000));

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    log_prev_reset_reason();
    printf("\n=== BrushlessLamp test_ble_arduino ===\n");

    // Arduino-as-component init — sets up Serial (HWCDC over USB-Serial-JTAG),
    // releases unused BT controller memory if !btInUse(), runs init() and
    // initVariant() weak hooks.
    initArduino();
    printf("[test_ble_arduino] initArduino() done\n");

    xTaskCreatePinnedToCore(heartbeat_task, "hb", 2048, NULL, 1, NULL, 0);

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    printf("[test_ble_arduino] wifi started\n");

    beacon(1);
    err = nimble_port_init();
    if (err != ESP_OK) { ESP_LOGE(TAG, "nimble_port_init err=%d", err); return; }
    beacon(2);

    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.sync_cb  = on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
    beacon(3);

    int rc = ble_svc_gap_device_name_set("brushless-test-ble-arduino");
    if (rc != 0) ESP_LOGE(TAG, "name_set rc=%d", rc);
    beacon(4);

    nimble_port_freertos_init(host_task);
}

// arduino-esp32 calls esp_bt_controller_mem_release() unless btInUse() returns
// true. Our test uses NimBLE so we keep the controller memory alive.
extern "C" __attribute__((used)) bool btInUse(void) { return true; }
