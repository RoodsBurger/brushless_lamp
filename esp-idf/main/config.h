#pragma once

#include <stdint.h>

// Tuning taken verbatim from /tmp/m2_knob/BrushlessLamp/BrushlessLamp.ino — the
// Arduino-platform sketch on this exact hardware that the user confirmed is silent +
// strong-torque. Phase 14a's acceptance criterion is matching this behavior under IDF.

constexpr uint32_t PWM_FREQ_HZ = 25000;

constexpr int   POLE_PAIRS     = 11;
constexpr int   ENCODER_CPR    = 1024;
constexpr float SUPPLY_VOLTAGE = 24.0f;

constexpr float PHASE_RESISTANCE     = 5.0f;
constexpr float KV_RATING            = 100.0f;
constexpr float CURRENT_LIMIT        = 0.5f;
constexpr float VELOCITY_LIMIT       = 30.0f;
constexpr float VOLTAGE_SENSOR_ALIGN = 3.0f;

constexpr float PID_P               = 0.2f;
constexpr float PID_I               = 20.0f;
constexpr float PID_D               = 0.0f;
constexpr float PID_OUTPUT_RAMP     = 1000.0f;
constexpr float LPF_TF              = 0.02f;
constexpr int   MOTION_DOWNSAMPLE   = 5;

constexpr uint32_t PRINT_INTERVAL_MS   = 1000;
constexpr uint32_t IDLE_DISABLE_MS     = 300;
constexpr float    IDLE_TARGET_THRESH  = 0.01f;

constexpr float    STALL_VEL_THRESHOLD = 0.2f;
constexpr uint32_t STALL_TIMEOUT_MS    = 500;
constexpr uint32_t STALL_WARMUP_MS     = 800;

constexpr float    TARGET_ACCEL_DEFAULT  = 10.0f;
constexpr float    KNOB_STEP_RAD_PER_SEC = 3.0f;

constexpr uint32_t BTN_DEBOUNCE_MS = 30;
