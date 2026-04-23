// M1-motor: bare motor bring-up on XIAO S3. No knob, no LEDs, no Matter, no radios.
// Purpose: validate the whole stack (IDF 5.4.4 + arduino-as-component + SimpleFOC v2.4.0
// + XIAO S3 MCPWM + dual-core pinning) and confirm the motor sounds as quiet as the
// Arduino M2-knob reference sketch.
//
// Behavior: app_main initializes Arduino + the motor task (pinned to core 1), then
// slowly oscillates the target angle between 0 and ANGLE_MAX/2 every 15 s so we can
// listen for any audible artifacts as the rotor ramps up, cruises, decelerates, and
// idle-disables.

#include <Arduino.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>

#include "motor.h"
#include "config.h"

static constexpr uint32_t SWEEP_PERIOD_MS = 15000;    // one full oscillation
static constexpr float    SWEEP_AMPLITUDE = ANGLE_MAX * 0.5f;

extern "C" void app_main() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    initArduino();
    delay(500);

    printf("\n=== BrushlessLamp M1-motor (S3) ===\n");
    printf("target oscillates 0..%.2f rad every %u ms\n",
           SWEEP_AMPLITUDE, (unsigned)SWEEP_PERIOD_MS);

    motor_init_and_start();

    // Main task drives the sweep — deliberately low priority so we don't compete with
    // the FOC task on core 1 for wake-ups.
    uint32_t t0 = millis();
    while (true) {
        float phase = (float)((millis() - t0) % SWEEP_PERIOD_MS) / (float)SWEEP_PERIOD_MS;
        // Cosine ramp from 0 → +A → 0, so both the forward slew and the return brake
        // exercise the decel path. Never negative — motor.cpp clamps below zero.
        float tgt = SWEEP_AMPLITUDE * (1.0f - cosf(phase * 2.0f * (float)M_PI)) * 0.5f;
        motor_set_target_angle(tgt);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
