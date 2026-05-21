// M1-motor: S3 bring-up + audibility test. Runs 10 full motor rotations at each
// of +10, -10, +20, -20, +30, -30 rad/s, using the same trapezoidal accel/decel as
// the production position loop (MOTION_ACCEL = 10 rad/s², silent M2-knob PID
// tuning, center-aligned MCPWM). Between segments there's a 1 s dwell so the
// motor can settle and idle-disable, giving a clean audio break between speeds.
//
// Use `motor_run_at_velocity(v)` to set a target velocity that SimpleFOC's
// velocity PID tracks directly; the motor task still ramps commanded_vel at
// MOTION_ACCEL, so ramp-up and ramp-down are trapezoidal just like the position
// loop. `motor_run_at_velocity(0)` initiates ramp-down + idle-disable.

#include <Arduino.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>

#include "motor.h"
#include "config.h"

static constexpr float    TEST_ROTATIONS        = 10.0f;                            // motor rotations per segment
static constexpr float    TEST_TRAVEL_RAD       = TEST_ROTATIONS * 2.0f * (float)M_PI;
static constexpr uint32_t TEST_DWELL_MS         = 1000;                             // between segments
static constexpr float    TEST_SPEEDS[]         = { 10.0f, -10.0f, 20.0f, -20.0f, 30.0f, -30.0f };

// Cruise time at |v| rad/s for TEST_TRAVEL_RAD rotation, plus padding for the
// trapezoidal accel+decel ramps (v/MOTION_ACCEL each side).
static uint32_t cruise_ms_for(float v) {
    float mag = fabsf(v);
    float ramp_s   = mag / MOTION_ACCEL;
    float cruise_s = TEST_TRAVEL_RAD / mag;
    return (uint32_t)((cruise_s + 2.0f * ramp_s) * 1000.0f);
}

extern "C" void app_main() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    initArduino();
    delay(500);

    printf("\n=== BrushlessLamp M1-motor (S3) — audibility sweep ===\n");
    printf("segments: ");
    for (float v : TEST_SPEEDS) printf("%.0f ", v);
    printf("rad/s, %.0f rot each, accel=%.0f rad/s^2\n",
           TEST_ROTATIONS, MOTION_ACCEL);

    motor_init_and_start();

    // Give the FOC task a moment to boot + initFOC before we start hitting it.
    vTaskDelay(pdMS_TO_TICKS(2000));

    while (true) {
        for (float v : TEST_SPEEDS) {
            uint32_t dur = cruise_ms_for(v);
            printf("\n[test] -> %.0f rad/s for %u ms\n", v, (unsigned)dur);
            motor_run_at_velocity(v);
            vTaskDelay(pdMS_TO_TICKS(dur));
            motor_run_at_velocity(0.0f);         // decelerate + idle-disable
            vTaskDelay(pdMS_TO_TICKS(TEST_DWELL_MS));
        }
        printf("[test] --- sequence complete, looping ---\n");
    }
}
