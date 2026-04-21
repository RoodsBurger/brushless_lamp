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

constexpr int   POLE_PAIRS     = 11;
constexpr int   ENCODER_CPR    = 1024;
constexpr float SUPPLY_VOLTAGE = 24.0f;

constexpr float PHASE_RESISTANCE     = 5.0f;
constexpr float KV_RATING            = 100.0f;
constexpr float CURRENT_LIMIT        = 0.5f;
constexpr float VOLTAGE_SENSOR_ALIGN = 3.0f;
constexpr float LPF_ANGLE_TF         = 0.001f;

constexpr float PID_VEL_P           = 0.2f;
constexpr float PID_VEL_I           = 20.0f;
constexpr float PID_VEL_D           = 0.0f;
constexpr float PID_VEL_OUTPUT_RAMP = 1000.0f;
constexpr float LPF_VEL_TF          = 0.02f;
constexpr float PID_ANGLE_P         = 3.0f;
constexpr float ACCEL_DEFAULT       = 10.0f;
constexpr int   MOTION_DOWNSAMPLE   = 0;        // SimpleFOC downsample semantics: `if (motion_cnt++ < N) return;` — 0 = run every call. Task is 1 kHz so PID runs at 1 kHz. LPF_velocity with Tf=0.02 handles the dt=1 ms encoder quantization noise.

constexpr uint32_t IDLE_DISABLE_MS = 300;
constexpr float    IDLE_POS_THRESH = 0.2f;

constexpr float    STALL_VEL_THRESHOLD = 0.2f;
constexpr uint32_t STALL_TIMEOUT_MS    = 500;
constexpr uint32_t STALL_WARMUP_MS     = 800;

constexpr float POS_MIN       = 0.0f;
constexpr float POS_MAX       = 30.0f;          // full Matter slider → ~1.2 s motor travel at 25 rad/s
constexpr float RAD_PER_CLICK = 0.3f;           // ~100 knob detents across full travel

constexpr uint32_t BTN_DEBOUNCE_MS   = 30;
constexpr uint32_t BTN_DBL_GAP_MS    = 400;
constexpr uint32_t BTN_LONG_WARN_MS  = 5000;
constexpr uint32_t BTN_LONG_RESET_MS = 9000;

constexpr float   VELOCITY_PRESETS[] = {10.0f, 15.0f, 20.0f, 25.0f};
constexpr uint8_t VELOCITY_PRESET_N  = sizeof(VELOCITY_PRESETS) / sizeof(VELOCITY_PRESETS[0]);
constexpr uint8_t DEFAULT_SPEED_IDX  = 3;       // 25 rad/s — Matter slider end-to-end in ~1.2 s

constexpr uint16_t MATTER_CT_WARM = 500;        // mireds
constexpr uint16_t MATTER_CT_COOL = 100;
