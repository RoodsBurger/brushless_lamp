#pragma once

#include <stdint.h>

// Tuning is carried over from `/tmp/m2_knob/BrushlessLamp/BrushlessLamp.ino` — the
// Arduino-platform sketch the user confirmed is silent + strong-torque on this exact
// hardware (4015 gimbal, 11 PP, DRV8313, 24 V, MT6701 1024 CPR). Phase 15's quietness
// target is to match this baseline on the S3, where a dedicated core + hardware FPU
// should remove the jitter sources that plagued the C6 build.

// Motor electrical.
constexpr uint32_t PWM_FREQ_HZ = 25000;
constexpr int   POLE_PAIRS     = 11;
constexpr int   ENCODER_CPR    = 1024;
constexpr float SUPPLY_VOLTAGE = 24.0f;

constexpr float PHASE_RESISTANCE     = 5.0f;
constexpr float KV_RATING            = 100.0f;
constexpr float CURRENT_LIMIT        = 0.5f;
constexpr float VELOCITY_LIMIT       = 30.0f;       // SimpleFOC's internal velocity cap
constexpr float VOLTAGE_SENSOR_ALIGN = 3.0f;

// Inner velocity PID — M2-knob values, do not tune unless you've verified audibility.
constexpr float PID_P             = 0.2f;
constexpr float PID_I             = 20.0f;
constexpr float PID_D             = 0.0f;
constexpr float PID_OUTPUT_RAMP   = 1000.0f;        // V/s, voltage slew cap
constexpr float LPF_TF            = 0.02f;          // velocity LPF time constant
constexpr int   MOTION_DOWNSAMPLE = 5;              // outer loop runs every N inner

// App-level position loop (phase 14c-derived). SimpleFOC stays in velocity mode; we
// close the position loop in the task and feed a ramped velocity target into move().
constexpr float ANGLE_MAX          = 10.0f * 6.2831853f;  // 10 motor rotations (test range)
constexpr float MOTION_VELOCITY    = 25.0f;         // rad/s top slew
constexpr float MOTION_ACCEL       = 10.0f;         // rad/s² — M2's TARGET_ACCEL_DEFAULT
constexpr float P_POSITION         = 3.0f;         // position→velocity gain (linear term)
constexpr float KNOB_STEP_RAD         = 1.0f;      // each detent nudges angle target by this (unused in M2 velocity-nudge)
constexpr float KNOB_STEP_RAD_PER_SEC = 10.0f;     // M2 velocity-nudge — each detent ±10 rad/s
constexpr float KNOB_VEL_MAX_RAD_PER_SEC = 50.0f;  // M2 velocity-nudge — clamp |target| to this

// Idle-disable: motor stays enabled while moving, disables after IDLE_DISABLE_MS of
// rest (commanded_vel + shaft_velocity + |pos_err| all near zero). While disabled we
// also *skip* motor.move() so the setPwm(0,0,0) in disable() actually persists — the
// velocity PID would otherwise re-drive phases via setPhaseVoltage on the next tick.
constexpr uint32_t IDLE_DISABLE_MS  = 300;
constexpr float    VEL_AT_REST_EPS  = 0.1f;         // rad/s
constexpr float    POS_AT_REST_EPS  = 0.1f;         // rad — target delta counts as "active"

// LED threshold: above this the lamp is "up" → LEDs on; below → off.
constexpr float    LED_ON_ANGLE_THRESH = 0.5f;
constexpr uint8_t  LED_DUTY            = 128;       // fixed brightness, 0..255
constexpr uint16_t CT_MIN_MIREDS       = 153;       // Matter ColorTemperatureLight range
constexpr uint16_t CT_MAX_MIREDS       = 500;

// Telemetry.
constexpr uint32_t PRINT_INTERVAL_MS   = 1000;

// Button handling.
constexpr uint32_t BTN_DEBOUNCE_MS       = 30;
constexpr uint32_t BTN_FACTORY_RESET_MS  = 5000;    // long-press → esp_matter::factory_reset() (M4)

// Dual-core task pinning (S3 only — C6 was UNICORE).
//   Core 0: main, WiFi, lwIP, CHIP, arduino events.
//   Core 1: motor FOC task (dedicated).
constexpr int CORE_MOTOR  = 1;
constexpr int CORE_OTHERS = 0;
constexpr unsigned MOTOR_TASK_PRIORITY = 20;        // above lwIP (18), below IDF timer (22)
