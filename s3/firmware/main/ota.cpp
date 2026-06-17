// Self-hosted signed OTA over HTTPS. A low-priority task on CORE_OTHERS polls a
// small JSON manifest on GitHub Releases; when its version exceeds the running
// firmware it downloads the image via esp_https_ota (which verifies the RSA
// signature before writing the spare slot), then reboots once the lamp is idle.

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_http_client.h>
#include <esp_https_ota.h>
#include <esp_crt_bundle.h>
#include <cJSON.h>
#include <string.h>

#include "ota.h"
#include "config.h"
#include "matter_app.h"

static const char *TAG = "ota";

// Manifest is tiny ({"version":N,"url":"..."}); accumulate the HTTPS body here.
static char s_manifest[512];
static int  s_manifest_len = 0;

static esp_err_t manifest_evt(esp_http_client_event_t *e) {
    if (e->event_id == HTTP_EVENT_ON_DATA && e->data_len > 0) {
        int room = (int)sizeof(s_manifest) - 1 - s_manifest_len;
        int n = (e->data_len < room) ? e->data_len : room;
        if (n > 0) { memcpy(s_manifest + s_manifest_len, e->data, n); s_manifest_len += n; }
    }
    return ESP_OK;
}

// GET + parse the manifest. Returns true and fills out_version / out_url on success.
static bool fetch_manifest(uint32_t *out_version, char *out_url, size_t url_sz) {
    s_manifest_len = 0;
    esp_http_client_config_t cfg = {};
    cfg.url               = OTA_MANIFEST_URL;
    cfg.crt_bundle_attach = esp_crt_bundle_attach;   // validate TLS against Mozilla roots
    cfg.event_handler     = manifest_evt;
    cfg.timeout_ms        = 15000;
    cfg.keep_alive_enable = true;                    // GitHub redirects to a CDN host

    esp_http_client_handle_t c = esp_http_client_init(&cfg);
    if (!c) return false;
    esp_err_t err = esp_http_client_perform(c);
    int status = esp_http_client_get_status_code(c);
    esp_http_client_cleanup(c);
    if (err != ESP_OK || status != 200) {
        ESP_LOGW(TAG, "manifest fetch failed err=%s status=%d", esp_err_to_name(err), status);
        return false;
    }

    s_manifest[s_manifest_len] = '\0';
    cJSON *root = cJSON_Parse(s_manifest);
    if (!root) { ESP_LOGW(TAG, "manifest parse failed"); return false; }
    cJSON *v = cJSON_GetObjectItem(root, "version");
    cJSON *u = cJSON_GetObjectItem(root, "url");
    bool ok = cJSON_IsNumber(v) && cJSON_IsString(u) && u->valuestring;
    if (ok) {
        *out_version = (uint32_t)v->valueint;
        strncpy(out_url, u->valuestring, url_sz - 1);
        out_url[url_sz - 1] = '\0';
    }
    cJSON_Delete(root);
    return ok;
}

// Download + verify + stage the new image. esp_https_ota sets the boot partition
// on success; the caller reboots when the lamp is idle.
static bool download_and_stage(const char *url) {
    esp_http_client_config_t http = {};
    http.url               = url;
    http.crt_bundle_attach = esp_crt_bundle_attach;
    http.timeout_ms        = 30000;
    http.keep_alive_enable = true;

    esp_https_ota_config_t ota = {};
    ota.http_config = &http;

    ESP_LOGW(TAG, "OTA: downloading %s", url);
    esp_err_t err = esp_https_ota(&ota);
    if (err == ESP_OK) { ESP_LOGW(TAG, "OTA: image verified + staged"); return true; }
    ESP_LOGE(TAG, "OTA: failed: %s", esp_err_to_name(err));
    return false;
}

static void ota_task(void *) {
    vTaskDelay(pdMS_TO_TICKS(OTA_INITIAL_DELAY_MS));   // let WiFi/Matter settle first
    while (true) {
        uint32_t latest = 0;
        char url[256];
        if (fetch_manifest(&latest, url, sizeof(url))) {
            ESP_LOGI(TAG, "manifest version=%u, running=%u", latest, (unsigned)OTA_FW_VERSION);
            if (latest > OTA_FW_VERSION && download_and_stage(url)) {
                // Apply when idle: don't reboot mid-use. Off = safe.
                while (matter_get_on_off()) vTaskDelay(pdMS_TO_TICKS(5000));
                ESP_LOGW(TAG, "lamp idle — rebooting into updated firmware");
                vTaskDelay(pdMS_TO_TICKS(500));
                esp_restart();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(OTA_CHECK_INTERVAL_MS));
    }
}

void ota_start() {
    xTaskCreatePinnedToCore(ota_task, "ota", 8192, nullptr, 3, nullptr, CORE_OTHERS);
}
