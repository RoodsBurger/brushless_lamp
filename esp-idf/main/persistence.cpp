#include "persistence.h"
#include "config.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "persist";
static const char *NS  = "blamp";

static uint8_t s_speed_idx = DEFAULT_SPEED_IDX;
static float   s_accel     = ACCEL_DEFAULT;

void persistence_init() {
    nvs_handle_t h;
    esp_err_t err = nvs_open(NS, NVS_READONLY, &h);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "no namespace '%s' yet (err %d) — using defaults", NS, err);
        return;
    }

    uint8_t v8 = DEFAULT_SPEED_IDX;
    if (nvs_get_u8(h, "speed_idx", &v8) == ESP_OK && v8 < VELOCITY_PRESET_N) {
        s_speed_idx = v8;
    }

    size_t sz = sizeof(float);
    float vf;
    if (nvs_get_blob(h, "accel", &vf, &sz) == ESP_OK && sz == sizeof(float) && vf >= 0.1f) {
        s_accel = vf;
    }

    nvs_close(h);
    ESP_LOGI(TAG, "loaded: speed_idx=%u accel=%.2f", s_speed_idx, s_accel);
}

uint8_t persistence_get_speed_idx() { return s_speed_idx; }
float   persistence_get_accel()     { return s_accel; }

void persistence_set_speed_idx(uint8_t idx) {
    if (idx >= VELOCITY_PRESET_N) idx = DEFAULT_SPEED_IDX;
    if (idx == s_speed_idx) return;
    s_speed_idx = idx;
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_u8(h, "speed_idx", idx);
    nvs_commit(h);
    nvs_close(h);
}

void persistence_set_accel(float accel) {
    if (accel < 0.1f) accel = 0.1f;
    if (accel == s_accel) return;
    s_accel = accel;
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_blob(h, "accel", &accel, sizeof(accel));
    nvs_commit(h);
    nvs_close(h);
}

bool persistence_get_foc_calibration(float *zero_elec, int8_t *sensor_dir) {
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READONLY, &h) != ESP_OK) return false;
    size_t sz = sizeof(float);
    float  ze;
    int8_t dir;
    bool ok = (nvs_get_blob(h, "zero_elec", &ze, &sz) == ESP_OK && sz == sizeof(float))
           && (nvs_get_i8  (h, "sens_dir", &dir)       == ESP_OK);
    nvs_close(h);
    if (ok) {
        *zero_elec  = ze;
        *sensor_dir = dir;
    }
    return ok;
}

void persistence_set_foc_calibration(float zero_elec, int8_t sensor_dir) {
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_blob(h, "zero_elec", &zero_elec, sizeof(zero_elec));
    nvs_set_i8  (h, "sens_dir", sensor_dir);
    nvs_commit(h);
    nvs_close(h);
    ESP_LOGI(TAG, "saved calibration: zero_elec=%.4f dir=%d", (double)zero_elec, (int)sensor_dir);
}

void persistence_clear_foc_calibration() {
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_erase_key(h, "zero_elec");
    nvs_erase_key(h, "sens_dir");
    nvs_commit(h);
    nvs_close(h);
    ESP_LOGI(TAG, "cleared calibration — next boot will re-run alignment");
}
