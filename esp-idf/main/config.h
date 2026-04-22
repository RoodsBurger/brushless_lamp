// Tunables — mirrors arduino sketch (commit 584deba) including the CylinderLamp-equivalent
// 2258 rad full-travel range. A full Matter slider drag at top preset (30 rad/s) takes
// ~75 s — accepted tradeoff for matching CylinderLamp's range. Earlier shrunken POS_MAX=60
// was dropped per user request (Phase 6e).
#pragma once

#include <stdint.h>

constexpr uint32_t PWM_FREQ_HZ      = 25000;
constexpr uint32_t FOC_ISR_HZ       = 1000;     // 1 kHz SVPWM — Arduino's proven-stable rate; higher rates starve Matter on the single-core C6
constexpr uint32_t LED_PWM_FREQ_HZ  = 30000;   // 30 kHz instead of 25 kHz to decouple from motor MCPWM (also 25 kHz). Same-frequency PWM on the same chip can produce beat/visual flicker even on independent peripherals due to shared clock domain — pushing LED above motor frequency removes the interaction.
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
constexpr float CURRENT_LIMIT        = 1.5f;   // 0.5 was M4 default but with phase_resistance=5 it caps PID-output voltage at 0.5×5=2.5 V (weak). 1.5 → 7.5 V max — strong enough for the lamp without unbounded wind-up.
constexpr float VOLTAGE_SENSOR_ALIGN = 8.0f;   // user-confirmed 8 V is good for alignment consistency (3 V was too low and produced wrong-direction runs).
constexpr float LPF_ANGLE_TF         = 0.0f;   // matches arduino-foc 2.4.0 which removed the shaft_angle LPF entirely (any Tf>0 adds phase lag that hurts tracking)

constexpr float PID_VEL_P           = 0.2f;
constexpr float PID_VEL_I           = 20.0f;   // M4-proven value. Tested 4 (louder) and 40 (also louder); 20 is the sweet spot for our setup.
constexpr float PID_VEL_D           = 0.0f;
constexpr float PID_VEL_OUTPUT_RAMP = 1000.0f;
constexpr float LPF_VEL_TF          = 0.05f;   // bumped from 0.02 — smoother velocity feedback into PID, reduces high-frequency voltage corrections that show up as audible whine
constexpr float PID_ANGLE_P         = 3.0f;
constexpr float ACCEL_DEFAULT       = 20.0f;   // matches Arduino M4 — low enough to stay smooth, high enough that long seeks finish before STALL_TIMEOUT_MS
constexpr int   MOTION_DOWNSAMPLE   = 5;        // Matches Arduino M4 exactly. With loopTask spinning at several kHz, move() body runs at ~400–2000 Hz — fast enough for smooth control, slow enough that the velocity estimator's dt isn't dominated by 1024-CPR quantization noise.

constexpr uint32_t IDLE_DISABLE_MS = 300;
constexpr float    IDLE_POS_THRESH = 0.2f;    // matches Arduino M4 — wider than the position loop's steady-state error so we don't chatter, narrow enough that we don't park visibly off-target

constexpr float    STALL_VEL_THRESHOLD = 5.0f;   // bumped from 2.5 — user reports stall still too hard to trigger. At 5.0, a grab that holds shaft below half the slowest preset (5 rad/s) fires; legitimate cruise tracks well above this even under load.
constexpr uint32_t STALL_TIMEOUT_MS    = 250;    // ~quarter-second hold and we declare stall — half of M4's 500 ms; user reports the previous setting felt too slow to react.
constexpr uint32_t STALL_WARMUP_MS     = 800;

constexpr float POS_MIN       = 0.0f;
constexpr float POS_MAX       = 2258.0f;        // matches CylinderLamp's stepper range (MAX_STEPS=1,150,000 ÷ 16 microsteps × NEMA-14 0.0025 rad/step = 2258 rad ≈ 359 motor revs); same as Arduino M4
constexpr float RAD_PER_CLICK = 56.5f;          // 40 knob detents across full travel (matches CylinderLamp feel: MAX_STEPS/STEPS_PER_CLICK = 1,150,000/28,750 = 40)

constexpr uint32_t BTN_DEBOUNCE_MS   = 30;
constexpr uint32_t BTN_DBL_GAP_MS    = 400;
constexpr uint32_t BTN_LONG_WARN_MS  = 5000;
constexpr uint32_t BTN_LONG_RESET_MS = 9000;

// Matches Arduino M4 exactly. Full travel at top preset (30 rad/s) takes ~75 s —
// long for a Matter slider drag but maps cleanly to the CylinderLamp-derived POS_MAX.
constexpr float   VELOCITY_PRESETS[] = {5.0f, 10.0f, 20.0f, 30.0f};
constexpr uint8_t VELOCITY_PRESET_N  = sizeof(VELOCITY_PRESETS) / sizeof(VELOCITY_PRESETS[0]);
constexpr uint8_t DEFAULT_SPEED_IDX  = 1;       // 10 rad/s

constexpr uint16_t MATTER_CT_WARM = 500;        // mireds
constexpr uint16_t MATTER_CT_COOL = 100;
