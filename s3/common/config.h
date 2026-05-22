#pragma once

#include <stdint.h>

// Tuning shared by every S3 stage. Values are the silent-baseline hardware
// combo (4015 gimbal / 11 PP / DRV8313 / 24 V / MT6701 @ 1024 CPR quadrature).

// Motor electrical.
constexpr uint32_t PWM_FREQ_HZ       = 25000;
constexpr int      POLE_PAIRS        = 11;
constexpr int      ENCODER_CPR       = 1024;
constexpr float    SUPPLY_VOLTAGE    = 24.0f;
constexpr float    PHASE_RESISTANCE  = 5.0f;
constexpr float    KV_RATING         = 100.0f;
constexpr float    CURRENT_LIMIT     = 0.5f;
constexpr float    VELOCITY_LIMIT    = 50.0f;       // SimpleFOC's internal velocity cap
// 6 V sensor-align drives enough current (~1.2 A / 5 Ω) for a clean direction
// sweep on first boot; sensor_direction is then cached in NVS.
constexpr float    VOLTAGE_SENSOR_ALIGN = 6.0f;
// Caps Uq in voltage-torque mode. At 40 rad/s the PID needs ~10 V (back-EMF +
// IR + headroom); 18 V keeps the output off the rail without losing torque,
// and stays above the DRV8313's 200 ns min-on-time at idle duty.
constexpr float    VOLTAGE_LIMIT     = 18.0f;

// Inner velocity PID.
constexpr float PID_P             = 0.2f;
constexpr float PID_I             = 20.0f;
constexpr float PID_D             = 0.0f;
constexpr float PID_OUTPUT_RAMP   = 150.0f;         // V/s cap on Uq slew
constexpr float LPF_TF            = 0.02f;          // velocity LPF time constant (s)
constexpr int   MOTION_DOWNSAMPLE = 5;              // move() runs every N loopFOC ticks
constexpr float SENSOR_MIN_ELAPSED_TIME = 0.0005f;  // encoder delta-t gate (s); filters 1-LSB quantization spikes without LPF lag

// App-level position loop. SimpleFOC runs velocity mode; the position loop
// here feeds a physics-bounded brake curve into move():
//     desired = min(MOTION_VELOCITY, sqrt(2 * MOTION_ACCEL * |pos_err|))
// Accel / cruise / decel profile lands exactly at target with v=0.
constexpr float    ANGLE_MAX          = 100.0f * 6.2831853f;  // 100 motor rotations — shared by knob nudge and Matter level scale.
constexpr float    MOTION_VELOCITY    = 15.0f;     // default cruise cap (rad/s); runtime override via motor_set_motion_velocity
constexpr float    MOTION_ACCEL       = 10.0f;      // rad/s² ramp + brake rate
constexpr float    MOTION_EPS         = 0.5f;       // brake-on-reverse sign-change eps (rad/s)
constexpr float    KNOB_STEP_RAD         = 6.2831853f;  // 1 motor rotation per detent

// Triple-click cycles MOTION_VELOCITY through these presets; index is also the LED-pulse blink count - 1.
constexpr float   MOTION_VELOCITY_PRESETS[]      = { 15.0f, 25.0f, 40.0f };
constexpr uint8_t MOTION_VELOCITY_PRESET_COUNT   = sizeof(MOTION_VELOCITY_PRESETS) / sizeof(MOTION_VELOCITY_PRESETS[0]);
constexpr uint8_t MOTION_VELOCITY_PRESET_DEFAULT = 0;

// 0.3-rad POS eps is ≈17° of shaft slack — wide so the motor doesn't hold torque indefinitely when parked near target.
constexpr uint32_t IDLE_DISABLE_MS  = 250;
constexpr float    VEL_AT_REST_EPS  = 0.2f;
constexpr float    POS_AT_REST_EPS  = 0.3f;

constexpr float    STALL_VEL_EPS    = 0.5f;   // shaft slowing past this is "frozen"
constexpr uint32_t STALL_WARMUP_MS  = 400;   // grace after engage so the trapezoidal ramp doesn't false-trip
constexpr uint32_t STALL_TIMEOUT_MS = 150;   // sustained-frozen window before stall fires

// LED driver (LEDC). 25 kHz keeps PWM well above the audible band; 8-bit duty
// gives 256 levels which is plenty once the gamma curve is applied.
constexpr uint32_t LED_PWM_FREQ_HZ     = 25000;
constexpr uint8_t  LED_PWM_RESOLUTION  = 8;

// LED brightness ramps 0 → max linearly across the first LED_FADE_ANGLE_RAD of shaft travel; fader smooths discontinuous target changes.
constexpr float    LED_FADE_ANGLE_RAD   = 3.0f;
constexpr uint8_t  LED_MAX_DUTY_DEFAULT = 200;      // 0..255 pre-gamma
constexpr uint8_t  LED_MAX_DUTY_MIN     = 16;       // floor on knob + NVS load — below this gamma squashes to ~0 and the lamp reads as broken
constexpr uint8_t  LED_MAX_DUTY_STEP    = 8;        // knob nudge in brightness mode
constexpr uint8_t  LED_FADE_STEP        = 1;        // duty units per fader tick (full 0..255 sweep ≈ 2.55 s)
constexpr uint32_t LED_FADE_PERIOD_MS   = 10;
constexpr float    LED_GAMMA            = 2.2f;     // perceptual curve so equal knob steps feel visually equal
// Color temperature blend: WW = (ct-MIN)*max/span, CW = (MAX-ct)*max/span.
// 153 (≈6500 K cool) and 454 (≈2200 K warm) match the CHIP reference lighting-app's
// ColorTempPhysicalMin/MaxMireds — Google Home's slider sweeps that exact range.
constexpr uint16_t COLORTEMP_DEFAULT = 250;   // ~4000 K boot value, matches lighting-app
constexpr uint16_t COLORTEMP_MIN     = 153;
constexpr uint16_t COLORTEMP_MAX     = 454;

// Button: 1 tap = Matter OnOff toggle | 2 taps = knob mode (angle/brightness) | 3 taps = speed preset | hold ≥9 s = factory reset.
constexpr uint32_t BTN_DEBOUNCE_MS           = 30;
constexpr uint32_t BTN_CLICK_MAX_MS          = 400;
constexpr uint32_t BTN_DOUBLE_CLICK_MS       = 400;
constexpr uint32_t BTN_LONG_PRESS_WARNING_MS = 5000;
constexpr uint32_t BTN_FACTORY_RESET_MS      = 9000;

// Dual-core layout: motor FOC on core 1 (no preemption from WiFi/BLE/CHIP),
// everything else on core 0.
constexpr int       CORE_MOTOR           = 1;
constexpr int       CORE_OTHERS          = 0;
constexpr unsigned  MOTOR_TASK_PRIORITY  = 20;      // above lwIP (18), below IDF timer (22)
