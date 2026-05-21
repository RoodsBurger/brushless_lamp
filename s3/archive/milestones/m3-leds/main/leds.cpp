// M3+ LED controller — CylinderLamp semantics on arduino-esp32 LEDC. Two channels
// (WW/CW) at 25 kHz / 8-bit. The fader task runs at LED_FADE_PERIOD_MS on CORE_OTHERS
// and steps current_ww/current_cw one LED_FADE_STEP at a time toward target_ww/cw,
// where the target is recomputed from (on, colortemp, max_duty) every tick.
//
// Pulse feedback (leds_pulse): spawns a one-shot task that raises s_pulse_active,
// waits for the fader to notice, then hand-writes 20-step fade-down/pause/fade-up/
// pause cycles via ledcWrite() directly. On exit it lowers the flag; the fader picks
// back up from whatever current_ww/cw the pulse left behind.

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>

#include "leds.h"
#include "motor.h"
#include "config.h"
#include "pins.h"

// Target state (writable from input task / Matter callback on CORE_OTHERS). s_on
// defaults true on M3 — the angle curve already does the visual off-at-zero work,
// so M3 never touches the flag. M4 will bind this to Matter's OnOff cluster.
static volatile bool     s_on           = true;
static volatile uint16_t s_colortemp    = COLORTEMP_DEFAULT;
static volatile uint8_t  s_max_duty     = LED_MAX_DUTY_DEFAULT;

// Fader state (read+written only from leds_fader_task).
static uint8_t  s_current_ww = 0;
static uint8_t  s_current_cw = 0;

// Pulse task coordination. While s_pulse_active the fader skips its ledcWrite so the
// pulse task owns the pins; both run on CORE_OTHERS so there's no memory-fence work.
static volatile bool s_pulse_active = false;

// Perceptual curve: treats max_duty as "how bright it should feel" and applies the
// gamma to produce an actual LEDC duty cycle. At LED_GAMMA = 2.2 a knob step from
// 64 → 72 (perceptual) raises actual duty ~4 units; at 200 → 208, it raises ~12.
// That matches Stevens' power law for visual brightness, so equal knob steps feel
// roughly equal visually.
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
}

void leds_set_on(bool on)                    { s_on = on; }
void leds_set_colortemp(uint16_t mireds)     { s_colortemp = mireds; }
void leds_set_max_duty(uint8_t duty)         { s_max_duty = duty; }
uint8_t leds_get_max_duty()                  { return s_max_duty; }
void leds_nudge_max_duty(int16_t delta) {
    int32_t nd = (int32_t)s_max_duty + delta;
    if (nd < 0)    nd = 0;
    if (nd > 255)  nd = 255;
    s_max_duty = (uint8_t)nd;
}

static void leds_fader_task(void *) {
    while (true) {
        if (!s_pulse_active) {
            // Target = (on ? peak_for_angle : 0) split by color temperature.
            // peak_for_angle folds in both the linear ramp from zero and the gamma
            // curve on max_duty; the 10 ms fader then glides current → target so
            // discontinuous changes (knob bumps max_duty, Matter toggles off) feel
            // smooth rather than stepped.
            uint8_t tgt_ww = 0, tgt_cw = 0;
            if (s_on) {
                uint8_t peak = peak_for_angle(motor_get_shaft_angle(), s_max_duty);
                ct_to_targets(s_colortemp, peak, &tgt_ww, &tgt_cw);
            }

            s_current_ww = step_toward(s_current_ww, tgt_ww);
            s_current_cw = step_toward(s_current_cw, tgt_cw);
            ledcWrite(PIN_LED_WW, s_current_ww);
            ledcWrite(PIN_LED_CW, s_current_cw);
        }

        vTaskDelay(pdMS_TO_TICKS(LED_FADE_PERIOD_MS));
    }
}

void leds_start_fader() {
    xTaskCreatePinnedToCore(leds_fader_task, "leds_fade", 3072, nullptr, 2, nullptr, CORE_OTHERS);
}

// Pulse: N cycles of fade-down-from-current → pause → fade-up-to-saved → pause. 20
// steps per fade × 8 ms = 160 ms fade + 80 ms pause. Matches the CylinderLamp feel
// (CylinderLamp.ino:119-137 in the sibling project).
static void leds_pulse_task(void *param) {
    uint8_t count = (uint8_t)(uintptr_t)param;
    uint8_t start_ww = s_current_ww;
    uint8_t start_cw = s_current_cw;
    s_pulse_active = true;

    for (uint8_t i = 0; i < count; i++) {
        for (int s = 20; s >= 0; s--) {
            ledcWrite(PIN_LED_WW, (uint8_t)((uint32_t)start_ww * (uint32_t)s / 20));
            ledcWrite(PIN_LED_CW, (uint8_t)((uint32_t)start_cw * (uint32_t)s / 20));
            vTaskDelay(pdMS_TO_TICKS(8));
        }
        vTaskDelay(pdMS_TO_TICKS(80));
        for (int s = 0; s <= 20; s++) {
            ledcWrite(PIN_LED_WW, (uint8_t)((uint32_t)start_ww * (uint32_t)s / 20));
            ledcWrite(PIN_LED_CW, (uint8_t)((uint32_t)start_cw * (uint32_t)s / 20));
            vTaskDelay(pdMS_TO_TICKS(8));
        }
        vTaskDelay(pdMS_TO_TICKS(80));
    }

    s_current_ww = start_ww;
    s_current_cw = start_cw;
    s_pulse_active = false;
    vTaskDelete(nullptr);
}

void leds_pulse(uint8_t count) {
    if (count == 0 || s_pulse_active) return;     // coalesce overlapping requests
    xTaskCreatePinnedToCore(leds_pulse_task, "leds_pulse", 3072,
                            (void *)(uintptr_t)count, 2, nullptr, CORE_OTHERS);
}
