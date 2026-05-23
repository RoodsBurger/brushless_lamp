// LEDC dual-channel WW/CW at 25 kHz / 8-bit. Fader task on CORE_OTHERS steps current
// → target one LED_FADE_STEP at a time. leds_pulse spawns a one-shot task that owns
// the pins during its blink cycle via s_pulse_active.

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs.h>
#include <math.h>

#include "leds.h"
#include "motor.h"
#include "config.h"
#include "pins.h"

// LED max duty + color temperature persist across boots; both writes debounced LED_NVS_DEBOUNCE ms after the last set to spare flash.
static constexpr const char    *LED_NVS_NS       = "leds";
static constexpr const char    *LED_NVS_KEY_DUTY = "maxduty";
static constexpr const char    *LED_NVS_KEY_CT   = "ct";
static constexpr uint32_t       LED_NVS_DEBOUNCE = 1000;

// Target state — written from input task / Matter callback on CORE_OTHERS.
// On/off is implicit: motor_get_shaft_angle()<=0 → peak_for_angle returns 0 → lamp dark.
static volatile uint16_t s_colortemp    = COLORTEMP_DEFAULT;
static volatile uint8_t  s_max_duty     = LED_MAX_DUTY_DEFAULT;

// Fader state (read+written only from leds_fader_task).
static uint8_t  s_current_ww = 0;
static uint8_t  s_current_cw = 0;

// While s_pulse_active the fader skips ledcWrite so the pulse task owns the pins.
static volatile bool s_pulse_active = false;

// Apply gamma to perceptual duty so equal knob steps feel visually equal (Stevens' power law).
static uint8_t gamma_correct(uint8_t perceptual) {
    if (perceptual == 0) return 0;
    float f = (float)perceptual * (1.0f / 255.0f);
    return (uint8_t)(powf(f, LED_GAMMA) * 255.0f + 0.5f);
}

// Continuous angle → peak-duty mapping. Below 0: off. 0..LED_FADE_ANGLE_RAD:
// linear ramp. Above: clamp at max_duty. Gamma is applied to the post-ramp value
// so brightness truly goes to 0 when the motor is at rest at angle 0.
static uint8_t peak_for_angle(float angle, uint8_t max_duty) {
    if (angle <= 0.0f) return 0;
    float frac = angle / LED_FADE_ANGLE_RAD;
    if (frac > 1.0f) frac = 1.0f;
    uint8_t perceptual = (uint8_t)(frac * (float)max_duty + 0.5f);
    return gamma_correct(perceptual);
}

static void ct_to_targets(uint16_t ct, uint8_t peak, uint8_t *ww, uint8_t *cw) {
    if (ct < COLORTEMP_MIN) ct = COLORTEMP_MIN;
    if (ct > COLORTEMP_MAX) ct = COLORTEMP_MAX;
    const uint16_t span = COLORTEMP_MAX - COLORTEMP_MIN;
    *ww = (uint8_t)(((uint32_t)(ct - COLORTEMP_MIN) * peak) / span);
    *cw = (uint8_t)(((uint32_t)(COLORTEMP_MAX - ct)  * peak) / span);
}

static uint8_t step_toward(uint8_t cur, uint8_t tgt) {
    if (cur == tgt) return cur;
    if (cur <  tgt) return (uint8_t)((cur + LED_FADE_STEP <= tgt) ? cur + LED_FADE_STEP : tgt);
    /* cur > tgt */  return (uint8_t)((cur > LED_FADE_STEP && (cur - LED_FADE_STEP) >= tgt) ? cur - LED_FADE_STEP : tgt);
}

void leds_init() {
    ledcAttach(PIN_LED_WW, LED_PWM_FREQ_HZ, LED_PWM_RESOLUTION);
    ledcAttach(PIN_LED_CW, LED_PWM_FREQ_HZ, LED_PWM_RESOLUTION);
    ledcWrite(PIN_LED_WW, 0);
    ledcWrite(PIN_LED_CW, 0);

    // Floor at MIN so an accidentally-zeroed saved value can't leave the lamp permanently dark.
    nvs_handle_t h;
    if (nvs_open(LED_NVS_NS, NVS_READONLY, &h) == ESP_OK) {
        uint8_t saved_duty = 0;
        if (nvs_get_u8(h, LED_NVS_KEY_DUTY, &saved_duty) == ESP_OK) {
            s_max_duty = (saved_duty >= LED_MAX_DUTY_MIN) ? saved_duty : LED_MAX_DUTY_DEFAULT;
        }
        uint16_t saved_ct = 0;
        if (nvs_get_u16(h, LED_NVS_KEY_CT, &saved_ct) == ESP_OK &&
            saved_ct >= COLORTEMP_MIN && saved_ct <= COLORTEMP_MAX) {
            s_colortemp = saved_ct;
        }
        nvs_close(h);
    }
}

// Debounced commits — fader task flushes a pending save once it has been quiet for LED_NVS_DEBOUNCE ms. Keeps NVS flash wear to one write per gesture.
static volatile uint32_t s_duty_save_pending_ms = 0;
static volatile uint32_t s_ct_save_pending_ms   = 0;
static uint8_t           s_last_saved_duty      = 0xFF;
static uint16_t          s_last_saved_ct        = 0xFFFF;
static void try_flush_duty_save(uint32_t now_ms) {
    if (s_duty_save_pending_ms == 0) return;
    if (now_ms - s_duty_save_pending_ms < LED_NVS_DEBOUNCE) return;
    s_duty_save_pending_ms = 0;
    uint8_t v = s_max_duty;
    if (v == s_last_saved_duty) return;
    nvs_handle_t h;
    if (nvs_open(LED_NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_u8(h, LED_NVS_KEY_DUTY, v);
    nvs_commit(h);
    nvs_close(h);
    s_last_saved_duty = v;
}
static void try_flush_ct_save(uint32_t now_ms) {
    if (s_ct_save_pending_ms == 0) return;
    if (now_ms - s_ct_save_pending_ms < LED_NVS_DEBOUNCE) return;
    s_ct_save_pending_ms = 0;
    uint16_t v = s_colortemp;
    if (v == s_last_saved_ct) return;
    nvs_handle_t h;
    if (nvs_open(LED_NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_u16(h, LED_NVS_KEY_CT, v);
    nvs_commit(h);
    nvs_close(h);
    s_last_saved_ct = v;
}

void leds_set_colortemp(uint16_t mireds) {
    s_colortemp = mireds;
    s_ct_save_pending_ms = millis();
}
uint16_t leds_get_colortemp() { return s_colortemp; }
void leds_nudge_colortemp(int16_t delta_mireds) {
    int32_t nc = (int32_t)s_colortemp + delta_mireds;
    if (nc < (int32_t)COLORTEMP_MIN) nc = COLORTEMP_MIN;
    if (nc > (int32_t)COLORTEMP_MAX) nc = COLORTEMP_MAX;
    s_colortemp = (uint16_t)nc;
    s_ct_save_pending_ms = millis();
}
void leds_nudge_max_duty(int16_t delta) {
    int32_t nd = (int32_t)s_max_duty + delta;
    if (nd < (int32_t)LED_MAX_DUTY_MIN) nd = LED_MAX_DUTY_MIN;  // floor, see config.h
    if (nd > 255)                       nd = 255;
    s_max_duty = (uint8_t)nd;
    s_duty_save_pending_ms = millis();
}

static void leds_fader_task(void *) {
    while (true) {
        if (!s_pulse_active) {
            // Target = gamma(peak_for_angle) split by color temperature; fader glides current → target.
            uint8_t peak = peak_for_angle(motor_get_shaft_angle(), s_max_duty);
            uint8_t tgt_ww = 0, tgt_cw = 0;
            ct_to_targets(s_colortemp, peak, &tgt_ww, &tgt_cw);

            s_current_ww = step_toward(s_current_ww, tgt_ww);
            s_current_cw = step_toward(s_current_cw, tgt_cw);
            ledcWrite(PIN_LED_WW, s_current_ww);
            ledcWrite(PIN_LED_CW, s_current_cw);
        }

        uint32_t now_ms = millis();
        try_flush_duty_save(now_ms);
        try_flush_ct_save(now_ms);
        vTaskDelay(pdMS_TO_TICKS(LED_FADE_PERIOD_MS));
    }
}

void leds_start_fader() {
    xTaskCreatePinnedToCore(leds_fader_task, "leds_fade", 2048, nullptr, 2, nullptr, CORE_OTHERS);
}

// N cycles of two-state alternation centered on the current visual state; ends at the original state so the fader has nothing to re-ramp.
static void leds_pulse_task(void *param) {
    uint8_t count = (uint8_t)(uintptr_t)param;
    uint8_t flash_ww = 0, flash_cw = 0;
    ct_to_targets(s_colortemp, gamma_correct(s_max_duty), &flash_ww, &flash_cw);

    // Lamp dark → flash up to peak; lamp lit → blink down to 0. Symmetric so end == start.
    uint8_t a_ww = s_current_ww;
    uint8_t a_cw = s_current_cw;
    uint8_t b_ww, b_cw;
    if (a_ww == 0 && a_cw == 0) {
        b_ww = flash_ww; b_cw = flash_cw;   // lamp off → flash to peak and back
    } else {
        b_ww = 0;        b_cw = 0;          // lamp on → blink off and back
    }

    s_pulse_active = true;

    for (uint8_t i = 0; i < count; i++) {
        for (int s = 0; s <= 20; s++) {                  // a → b
            uint32_t w = ((uint32_t)a_ww * (20 - s) + (uint32_t)b_ww * s) / 20;
            uint32_t c = ((uint32_t)a_cw * (20 - s) + (uint32_t)b_cw * s) / 20;
            ledcWrite(PIN_LED_WW, (uint8_t)w);
            ledcWrite(PIN_LED_CW, (uint8_t)c);
            vTaskDelay(pdMS_TO_TICKS(8));
        }
        vTaskDelay(pdMS_TO_TICKS(80));
        for (int s = 0; s <= 20; s++) {                  // b → a
            uint32_t w = ((uint32_t)b_ww * (20 - s) + (uint32_t)a_ww * s) / 20;
            uint32_t c = ((uint32_t)b_cw * (20 - s) + (uint32_t)a_cw * s) / 20;
            ledcWrite(PIN_LED_WW, (uint8_t)w);
            ledcWrite(PIN_LED_CW, (uint8_t)c);
            vTaskDelay(pdMS_TO_TICKS(8));
        }
        vTaskDelay(pdMS_TO_TICKS(80));
    }

    // End at the original state — no tail fade, no snap.
    s_current_ww = a_ww;
    s_current_cw = a_cw;
    s_pulse_active = false;
    vTaskDelete(nullptr);
}

void leds_pulse(uint8_t count) {
    if (count == 0 || s_pulse_active) return;     // coalesce overlapping requests
    xTaskCreatePinnedToCore(leds_pulse_task, "leds_pulse", 2048,
                            (void *)(uintptr_t)count, 2, nullptr, CORE_OTHERS);
}
