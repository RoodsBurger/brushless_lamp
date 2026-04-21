#include "leds.h"
#include "pins.h"
#include "config.h"

#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

#include <math.h>
#include <atomic>

static const char *TAG = "leds";

// LEDC layout: timer 1 (timer 0 reserved for esp_simplefoc's BLDCDriver3PWM in LEDC fallback mode)
// Both channels on the only low-speed group available on C6.
constexpr ledc_timer_t   LED_TIMER       = LEDC_TIMER_1;
constexpr ledc_channel_t LED_CHAN_WW     = LEDC_CHANNEL_0;
constexpr ledc_channel_t LED_CHAN_CW     = LEDC_CHANNEL_1;
constexpr ledc_mode_t    LED_SPEED_MODE  = LEDC_LOW_SPEED_MODE;

static std::atomic<uint8_t> s_brightness{50};
static std::atomic<uint8_t> s_cct{50};

static float s_effective_bri  = 50.0f;
static float s_effective_cct  = 50.0f;
static float s_effective_fade = 0.0f;
static int64_t s_last_ramp_us = 0;
static uint16_t s_last_duty_ww = 0xFFFF;
static uint16_t s_last_duty_cw = 0xFFFF;

void leds_init() {
    ledc_timer_config_t t = {};
    t.speed_mode      = LED_SPEED_MODE;
    t.duty_resolution = (ledc_timer_bit_t)LED_PWM_RES_BIT;
    t.timer_num       = LED_TIMER;
    t.freq_hz         = LED_PWM_FREQ_HZ;
    t.clk_cfg         = LEDC_AUTO_CLK;
    ESP_ERROR_CHECK(ledc_timer_config(&t));

    auto attach = [](ledc_channel_t ch, int gpio) {
        ledc_channel_config_t c = {};
        c.gpio_num   = gpio;
        c.speed_mode = LED_SPEED_MODE;
        c.channel    = ch;
        c.timer_sel  = LED_TIMER;
        c.duty       = 0;
        c.hpoint     = 0;
        ESP_ERROR_CHECK(ledc_channel_config(&c));
    };
    attach(LED_CHAN_WW, PIN_LED_WW);
    attach(LED_CHAN_CW, PIN_LED_CW);

    ESP_LOGI(TAG, "LED PWM up: %u Hz, %u-bit, channels on GPIO %u/%u",
             (unsigned)LED_PWM_FREQ_HZ, (unsigned)LED_PWM_RES_BIT,
             (unsigned)PIN_LED_WW, (unsigned)PIN_LED_CW);
}

void leds_set_brightness(uint8_t bri) {
    if (bri > 100) bri = 100;
    s_brightness = bri;
}

void leds_set_cct(uint8_t cct) {
    if (cct > 100) cct = 100;
    s_cct = cct;
}

uint8_t leds_get_brightness() { return s_brightness.load(); }
uint8_t leds_get_cct()        { return s_cct.load(); }

static void update_ramp(float shaft_angle) {
    int64_t now_us = esp_timer_get_time();
    if (s_last_ramp_us == 0) { s_last_ramp_us = now_us; return; }
    int64_t dt_us = now_us - s_last_ramp_us;
    if (dt_us < 5000) return;  // 5 ms cadence
    s_last_ramp_us = now_us;
    float dt_s = (float)dt_us / 1e6f;

    float step = LED_RAMP_PER_SEC * dt_s;
    float tb = (float)s_brightness.load();
    if      (s_effective_bri < tb - step) s_effective_bri += step;
    else if (s_effective_bri > tb + step) s_effective_bri -= step;
    else                                   s_effective_bri  = tb;
    float tc = (float)s_cct.load();
    if      (s_effective_cct < tc - step) s_effective_cct += step;
    else if (s_effective_cct > tc + step) s_effective_cct -= step;
    else                                   s_effective_cct  = tc;

    float target_fade = (shaft_angle > LEDS_OFF_THRESH) ? 1.0f : 0.0f;
    float fade_step = LED_FADE_PER_SEC * dt_s;
    if      (s_effective_fade < target_fade - fade_step) s_effective_fade += fade_step;
    else if (s_effective_fade > target_fade + fade_step) s_effective_fade -= fade_step;
    else                                                  s_effective_fade  = target_fade;
}

static inline void write_pwm(uint16_t want_ww, uint16_t want_cw) {
    if (want_ww != s_last_duty_ww) {
        ledc_set_duty(LED_SPEED_MODE, LED_CHAN_WW, want_ww);
        ledc_update_duty(LED_SPEED_MODE, LED_CHAN_WW);
        s_last_duty_ww = want_ww;
    }
    if (want_cw != s_last_duty_cw) {
        ledc_set_duty(LED_SPEED_MODE, LED_CHAN_CW, want_cw);
        ledc_update_duty(LED_SPEED_MODE, LED_CHAN_CW);
        s_last_duty_cw = want_cw;
    }
}

void leds_update(float shaft_angle) {
    update_ramp(shaft_angle);

    // Time-based fade at the threshold — LEDs ramp 0↔1 over ~250 ms when shaft crosses the
    // position threshold. Once faded to 1, position changes don't affect brightness.
    uint32_t base = (uint32_t)((s_effective_bri * (float)LED_PWM_MAX / 100.0f) * s_effective_fade);
    uint32_t ecct = (uint32_t)(s_effective_cct + 0.5f);
    uint16_t want_ww = (uint16_t)((base * (100 - ecct)) / 100);
    uint16_t want_cw = (uint16_t)((base * ecct)         / 100);
    write_pwm(want_ww, want_cw);
}

void leds_pulse(uint8_t n) {
    // Pulse peak comes from the configured brightness/cct so the warning is visible
    // even when the lamp is off (motor at position 0 → fade=0 → normal LEDs are dark).
    uint32_t base = (uint32_t)s_brightness.load() * LED_PWM_MAX / 100;
    uint8_t  ecct = s_cct.load();
    uint16_t peak_ww = (uint16_t)((base * (100 - ecct)) / 100);
    uint16_t peak_cw = (uint16_t)((base * ecct)         / 100);

    constexpr int FADE_STEPS   = 60;
    constexpr int FADE_STEP_MS = 3;
    constexpr int HOLD_MS      = 80;

    auto write_step = [&](int step) {
        uint32_t num = (uint32_t)step * (uint32_t)step;
        uint32_t den = (uint32_t)FADE_STEPS * (uint32_t)FADE_STEPS;
        write_pwm((uint16_t)((uint32_t)peak_ww * num / den),
                  (uint16_t)((uint32_t)peak_cw * num / den));
    };

    // Smooth fade-in entry
    for (int s = 0; s <= FADE_STEPS; s++) { write_step(s); vTaskDelay(pdMS_TO_TICKS(FADE_STEP_MS)); }
    vTaskDelay(pdMS_TO_TICKS(HOLD_MS));

    for (uint8_t i = 0; i < n; i++) {
        for (int s = FADE_STEPS; s >= 0; s--) { write_step(s); vTaskDelay(pdMS_TO_TICKS(FADE_STEP_MS)); }
        vTaskDelay(pdMS_TO_TICKS(HOLD_MS));
        if (i < (uint8_t)(n - 1)) {
            for (int s = 0; s <= FADE_STEPS; s++) { write_step(s); vTaskDelay(pdMS_TO_TICKS(FADE_STEP_MS)); }
            vTaskDelay(pdMS_TO_TICKS(HOLD_MS));
        }
    }
    // Ends at 0; force next leds_update() to re-write
    s_last_duty_ww = 0xFFFF;
    s_last_duty_cw = 0xFFFF;
}
