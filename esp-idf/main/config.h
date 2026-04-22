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
constexpr float    KNOB_STEP_RAD_PER_SEC = 3.0f;   // legacy — unused in angle mode

// Phase 14c+: position (angle) control mirroring CylinderLamp's Matter mapping.
// Matter Level 0..254 → shaft angle 0..ANGLE_MAX; Matter OnOff → LED + motor enable;
// Matter ColorTemp → warm/cool mix at fixed LED_DUTY. LED brightness is *not* slaved
// to Matter Level — CylinderLamp learned the hard way that fading LEDs with the slider
// creates on/off weirdness near 0 %; the slider is lamp height, not lamp brightness.
// Test range — 10 motor revolutions. Full CylinderLamp-equivalent travel is ~360
// rotations; this is shrunk for bring-up so end-to-end slider drags finish in seconds.
constexpr float    ANGLE_MAX            = 10.0f * 6.2831853f;    // 10 motor rotations
constexpr float    KNOB_STEP_RAD        = 1.0f;     // each detent moves target by 1 rad

// Motion profile for the app-level position loop. SimpleFOC stays in its M2-proven
// velocity mode (quiet); we compute the commanded velocity here as
//     desired_vel = clamp(P_POSITION * (target_angle - shaft_angle), ±MOTION_VELOCITY)
// then ramp commanded_vel toward desired_vel at MOTION_ACCEL (M2's target_accel
// pattern). Final output feeds motor.move(vel). Near the target the error shrinks so
// velocity naturally decays → soft stop.
constexpr float    MOTION_VELOCITY      = 25.0f;    // rad/s top speed
constexpr float    MOTION_ACCEL         = 10.0f;    // rad/s² — M2-proven value, keeps audibility close to M2
constexpr float    P_POSITION           = 3.0f;     // position→velocity gain (err=8.3 rad → vel=MOTION_VELOCITY)
constexpr uint8_t  LED_DUTY             = 128;      // fixed LED brightness when on (0..255)
constexpr uint16_t CT_MIN_MIREDS        = 153;      // matches Matter ColorTemperatureLight
constexpr uint16_t CT_MAX_MIREDS        = 500;
// Must sit above ANGLE_AT_TARGET_EPS so a motor settled at target=0 (OnOff=off) is
// guaranteed to be below this, and LEDs turn cleanly off. With eps=2 rad settling,
// threshold=3 rad keeps the hysteresis comfortable.
constexpr float    LED_ON_ANGLE_THRESH  = 3.0f;

constexpr uint32_t BTN_DEBOUNCE_MS = 30;
