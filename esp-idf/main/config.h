// Tunables — mirrors arduino sketch's final Matter-tuned constexpr block (commit 584deba).
// Do NOT scale POS_MAX up "to match CylinderLamp steps" — the whole range is travelled on
// every Matter brightness slider drag, so bigger POS_MAX = longer seek = stall timeouts.
#pragma once

#include <stdint.h>

constexpr uint32_t PWM_FREQ_HZ      = 25000;
constexpr uint32_t FOC_ISR_HZ       = 1000;     // 1 kHz SVPWM — Arduino's proven-stable rate; higher rates starve Matter on the single-core C6
constexpr uint32_t LED_PWM_FREQ_HZ  = 25000;
constexpr uint8_t  LED_PWM_RES_BIT  = 10;
constexpr uint16_t LED_PWM_MAX      = (1u << LED_PWM_RES_BIT) - 1;

constexpr float    LEDS_OFF_THRESH  = 0.5f;
constexpr float    LEDS_FULL_THRESH = 3.0f;
constexpr float    LED_RAMP_PER_SEC = 60.0f;
constexpr float    LED_FADE_PER_SEC = 4.0f;    // on/off fade rate at the position threshold: 1.0 unit over 250 ms

constexpr int   POLE_PAIRS     = 11;
constexpr int   ENCODER_CPR    = 1024;
constexpr float SUPPLY_VOLTAGE = 24.0f;

constexpr float PHASE_RESISTANCE     = 5.0f;
constexpr float KV_RATING            = 100.0f;  // true motor spec — 2.3.0 bug patched directly in BLDCMotor.cpp (see esp-idf/patches/)
constexpr float CURRENT_LIMIT        = 0.5f;
constexpr float VOLTAGE_SENSOR_ALIGN = 3.0f;
constexpr float LPF_ANGLE_TF         = 0.0f;   // matches arduino-foc 2.4.0 which removed the shaft_angle LPF entirely (any Tf>0 adds phase lag that hurts tracking)

constexpr float PID_VEL_P           = 0.2f;
constexpr float PID_VEL_I           = 20.0f;
constexpr float PID_VEL_D           = 0.0f;
constexpr float PID_VEL_OUTPUT_RAMP = 1000.0f;
constexpr float LPF_VEL_TF          = 0.02f;
constexpr float PID_ANGLE_P         = 3.0f;
constexpr float ACCEL_DEFAULT       = 20.0f;   // matches Arduino M4 — low enough to stay smooth, high enough that long seeks finish before STALL_TIMEOUT_MS
constexpr int   MOTION_DOWNSAMPLE   = 5;        // Matches Arduino M4 exactly. With loopTask spinning at several kHz, move() body runs at ~400–2000 Hz — fast enough for smooth control, slow enough that the velocity estimator's dt isn't dominated by 1024-CPR quantization noise.

constexpr uint32_t IDLE_DISABLE_MS = 300;
constexpr float    IDLE_POS_THRESH = 0.2f;    // matches Arduino M4 — wider than the position loop's steady-state error so we don't chatter, narrow enough that we don't park visibly off-target

constexpr float    STALL_VEL_THRESHOLD = 0.2f;
constexpr uint32_t STALL_TIMEOUT_MS    = 500;
constexpr uint32_t STALL_WARMUP_MS     = 800;

constexpr float POS_MIN       = 0.0f;
constexpr float POS_MAX       = 60.0f;          // ~9.5 shaft revolutions — CylinderLamp-equivalent travel range
constexpr float RAD_PER_CLICK = 1.5f;           // 40 knob detents across full travel (matches CylinderLamp click-to-range feel: MAX_STEPS/STEPS_PER_CLICK = 1,150,000/28,750 = 40)

constexpr uint32_t BTN_DEBOUNCE_MS   = 30;
constexpr uint32_t BTN_DBL_GAP_MS    = 400;
constexpr uint32_t BTN_LONG_WARN_MS  = 5000;
constexpr uint32_t BTN_LONG_RESET_MS = 9000;

// Spread tuned for perceivable difference: at POS_MAX=30 and ACCEL=25, slowest preset
// reaches ~1.8 s end-to-end, fastest reaches ~1.1 s. 2× spread max→min is about as
// much as the encoder's edge-rate can handle without dropping counts at peak speed.
constexpr float   VELOCITY_PRESETS[] = {10.0f, 18.0f, 28.0f, 40.0f};
constexpr uint8_t VELOCITY_PRESET_N  = sizeof(VELOCITY_PRESETS) / sizeof(VELOCITY_PRESETS[0]);
constexpr uint8_t DEFAULT_SPEED_IDX  = 1;       // 18 rad/s

constexpr uint16_t MATTER_CT_WARM = 500;        // mireds
constexpr uint16_t MATTER_CT_COOL = 100;
