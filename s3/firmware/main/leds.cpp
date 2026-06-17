// LEDC dual-channel WW/CW at 25 kHz / 10-bit (perceptual values stay 0..255; gamma
// expands to 0..1023 hardware duty). CW runs 180° out of phase via hpoint so the two
// MOSFET current pulses interleave on the shared 24 V rail. Fader task on CORE_OTHERS
// steps current → target one LED_FADE_STEP at a time. leds_pulse spawns a one-shot
// task that owns the pins during its blink cycle via s_pulse_active.

#include <Arduino.h>
#include <driver/ledc.h>
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
// CT has a separate fader-owned `current` value so the visible color slews even when
// the target snaps instantly (Google Home slider drags arrive that way).
static volatile uint16_t s_colortemp_target = COLORTEMP_DEFAULT;
static volatile uint8_t  s_max_duty         = LED_MAX_DUTY_DEFAULT;

// Fader state (read+written only from leds_fader_task). 10-bit hardware duty.
static uint16_t s_current_ww        = 0;
static uint16_t s_current_cw        = 0;
static uint16_t s_colortemp_current = COLORTEMP_DEFAULT;

// While s_pulse_active the fader skips the duty writes so the pulse task owns the pins.
static volatile bool s_pulse_active = false;

// Fixed channels (both land on LEDC timer 0) so CW can carry a phase offset.
static constexpr uint8_t  LEDC_CH_WW = 0;
static constexpr uint8_t  LEDC_CH_CW = 1;
static constexpr uint32_t CW_HPOINT  = (LED_HW_DUTY_MAX + 1) / 2;   // half period = 180°

// All duty writes go through here: WW switches at hpoint 0, CW half a period
// later, so their current pulses interleave on the shared 24 V rail.
static void led_write(uint8_t channel, uint16_t duty, uint32_t hpoint) {
    ledc_set_duty_with_hpoint(LEDC_LOW_SPEED_MODE, (ledc_channel_t)channel, duty, hpoint);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)channel);
}
static void led_write_ww(uint16_t duty) { led_write(LEDC_CH_WW, duty, 0); }
static void led_write_cw(uint16_t duty) { led_write(LEDC_CH_CW, duty, CW_HPOINT); }

// Apply gamma to perceptual duty so equal knob steps feel visually equal (Stevens' power law).
// 0..255 perceptual in → 0..1023 hardware duty out: the 10-bit range keeps the low end
// of the gamma curve from collapsing into a handful of duty codes.
static uint16_t gamma_correct(uint8_t perceptual) {
    if (perceptual == 0) return 0;
    float f = (float)perceptual * (1.0f / 255.0f);
    return (uint16_t)(powf(f, LED_GAMMA) * (float)LED_HW_DUTY_MAX + 0.5f);
}

// Continuous angle → peak-duty mapping. Below 0: off. 0..LED_FADE_ANGLE_RAD:
// linear ramp. Above: clamp at max_duty. Gamma is applied to the post-ramp value
// so brightness truly goes to 0 when the motor is at rest at angle 0.
static uint16_t peak_for_angle(float angle, uint8_t max_duty) {
    if (angle <= 0.0f) return 0;
    float frac = angle / LED_FADE_ANGLE_RAD;
    if (frac > 1.0f) frac = 1.0f;
    uint8_t perceptual = (uint8_t)(frac * (float)max_duty + 0.5f);
    return gamma_correct(perceptual);
}

static void ct_to_targets(uint16_t ct, uint16_t peak, uint16_t *ww, uint16_t *cw) {
    if (ct < COLORTEMP_MIN) ct = COLORTEMP_MIN;
    if (ct > COLORTEMP_MAX) ct = COLORTEMP_MAX;
    const uint16_t span = COLORTEMP_MAX - COLORTEMP_MIN;
    *ww = (uint16_t)(((uint32_t)(ct - COLORTEMP_MIN) * peak) / span);
    *cw = (uint16_t)(peak - *ww);   // exact complement — both-truncated split dipped total brightness mid-range
}

static uint16_t step_toward(uint16_t cur, uint16_t tgt) {
    if (cur == tgt) return cur;
    if (cur <  tgt) return (uint16_t)((cur + LED_FADE_STEP <= tgt) ? cur + LED_FADE_STEP : tgt);
    /* cur > tgt */  return (uint16_t)((cur > LED_FADE_STEP && (cur - LED_FADE_STEP) >= tgt) ? cur - LED_FADE_STEP : tgt);
}

static uint16_t step_ct_toward(uint16_t cur, uint16_t tgt) {
    if (cur == tgt) return cur;
    if (cur <  tgt) return (uint16_t)((tgt - cur <= CT_FADE_STEP_MIREDS) ? tgt : cur + CT_FADE_STEP_MIREDS);
    /* cur > tgt */  return (uint16_t)((cur - tgt <= CT_FADE_STEP_MIREDS) ? tgt : cur - CT_FADE_STEP_MIREDS);
}

void leds_init() {
    // Explicit channels (not auto-assign) so led_write_* can address them for the hpoint offset.
    ledcAttachChannel(PIN_LED_WW, LED_PWM_FREQ_HZ, LED_PWM_RESOLUTION, LEDC_CH_WW);
    ledcAttachChannel(PIN_LED_CW, LED_PWM_FREQ_HZ, LED_PWM_RESOLUTION, LEDC_CH_CW);
    led_write_ww(0);
    led_write_cw(0);

    // Floor at MIN so an accidentally-zeroed saved value can't leave the lamp permanently dark.
    nvs_handle_t h;
    if (nvs_open(LED_NVS_NS, NVS_READONLY, &h) == ESP_OK) {
        uint8_t saved_duty = 0;
        if (nvs_get_u8(h, LED_NVS_KEY_DUTY, &saved_duty) == ESP_OK) {
            s_max_duty = (saved_duty >= LED_MAX_DUTY_MIN) ? saved_duty : LED_MAX_DUTY_DEFAULT;
        }
        uint16_t saved_ct = 0;
        if (nvs_get_u16(h, LED_NVS_KEY_CT, &saved_ct) == ESP_OK &&
            saved_ct >= COLORTEMP_ADVERTISED_MIN && saved_ct <= COLORTEMP_ADVERTISED_MAX) {
            s_colortemp_target = saved_ct;
        }
        nvs_close(h);
    }
    s_colortemp_current = s_colortemp_target;   // no boot-time CT slew from default to persisted
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
    uint16_t v = s_colortemp_target;
    if (v == s_last_saved_ct) return;
    nvs_handle_t h;
    if (nvs_open(LED_NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_u16(h, LED_NVS_KEY_CT, v);
    nvs_commit(h);
    nvs_close(h);
    s_last_saved_ct = v;
}

void leds_set_colortemp(uint16_t mireds) {
    // Matter inbound: accept up to the advertised max so warm presets (candlelight)
    // stick; ct_to_targets renders anything past COLORTEMP_MAX as full warm-white.
    if (mireds < COLORTEMP_ADVERTISED_MIN)  mireds = COLORTEMP_ADVERTISED_MIN;
    if (mireds > COLORTEMP_ADVERTISED_MAX)  mireds = COLORTEMP_ADVERTISED_MAX;
    s_colortemp_target = mireds;
    s_ct_save_pending_ms = millis();
}
uint16_t leds_get_colortemp() { return s_colortemp_target; }
void leds_nudge_colortemp(int16_t delta_mireds) {
    int32_t nc = (int32_t)s_colortemp_target + delta_mireds;
    if (nc < (int32_t)COLORTEMP_MIN) nc = COLORTEMP_MIN;
    if (nc > (int32_t)COLORTEMP_MAX) nc = COLORTEMP_MAX;
    s_colortemp_target = (uint16_t)nc;
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
            // Smooth CT first so slider drags fade rather than step, then split current CT into WW/CW at the current peak; per-channel fader glides duty → target.
            s_colortemp_current = step_ct_toward(s_colortemp_current, s_colortemp_target);
            uint16_t peak = peak_for_angle(motor_get_shaft_angle(), s_max_duty);
            uint16_t tgt_ww = 0, tgt_cw = 0;
            ct_to_targets(s_colortemp_current, peak, &tgt_ww, &tgt_cw);

            s_current_ww = step_toward(s_current_ww, tgt_ww);
            s_current_cw = step_toward(s_current_cw, tgt_cw);
            led_write_ww(s_current_ww);
            led_write_cw(s_current_cw);
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
    uint16_t flash_ww = 0, flash_cw = 0;
    ct_to_targets(s_colortemp_target, gamma_correct(s_max_duty), &flash_ww, &flash_cw);

    // Lamp dark → flash up to peak; lamp lit → blink down to 0. Symmetric so end == start.
    uint16_t a_ww = s_current_ww;
    uint16_t a_cw = s_current_cw;
    uint16_t b_ww, b_cw;
    if (a_ww == 0 && a_cw == 0) {
        b_ww = flash_ww; b_cw = flash_cw;   // lamp off → flash to peak and back
    } else {
        b_ww = 0;        b_cw = 0;          // lamp on → blink off and back
    }

    for (uint8_t i = 0; i < count; i++) {
        for (int s = 0; s <= 20; s++) {                  // a → b
            uint32_t w = ((uint32_t)a_ww * (20 - s) + (uint32_t)b_ww * s) / 20;
            uint32_t c = ((uint32_t)a_cw * (20 - s) + (uint32_t)b_cw * s) / 20;
            led_write_ww((uint16_t)w);
            led_write_cw((uint16_t)c);
            vTaskDelay(pdMS_TO_TICKS(8));
        }
        vTaskDelay(pdMS_TO_TICKS(80));
        for (int s = 0; s <= 20; s++) {                  // b → a
            uint32_t w = ((uint32_t)b_ww * (20 - s) + (uint32_t)a_ww * s) / 20;
            uint32_t c = ((uint32_t)b_cw * (20 - s) + (uint32_t)a_cw * s) / 20;
            led_write_ww((uint16_t)w);
            led_write_cw((uint16_t)c);
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
    // Claim the pins before the task exists — two gestures inside one scheduling
    // window could otherwise spawn two pulse tasks fighting over the channels.
    s_pulse_active = true;
    if (xTaskCreatePinnedToCore(leds_pulse_task, "leds_pulse", 2048,
                                (void *)(uintptr_t)count, 2, nullptr, CORE_OTHERS) != pdPASS) {
        s_pulse_active = false;
    }
}
