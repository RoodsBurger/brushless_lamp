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
constexpr float VELOCITY_LIMIT       = 50.0f;       // SimpleFOC's internal velocity cap
constexpr float VOLTAGE_SENSOR_ALIGN = 6.0f;   // enough torque (~1.2 A on 5 Ω) for the first-boot / post-factory-reset direction sweep to move smoothly instead of choppy. Only runs at initFOC — sensor_direction is cached to NVS so it doesn't repeat on every boot.
// In TorqueControlType::voltage SimpleFOC caps Uq at VOLTAGE_LIMIT, not current_limit.
// Iteration: 6 V → audibly quiet at low speeds but PID railed at 40 rad/s (back-EMF
// ≈ 3.8 V + IR drop ≈ 2.5 V + headroom > 6). 12 V cleaned that up but at the top
// preset (40 rad/s) the audibility crept back — headroom too tight. 18 V gives the
// PID room to breathe at the top end (Uq needed ≈ 10 V ± margin) while still
// keeping duty well above the DRV8313's 200 ns min-on-time at idle.
constexpr float VOLTAGE_LIMIT        = 18.0f;

// Inner velocity PID. Second research pass (hardware-specific) pointed at three
// under-tuned knobs: VOLTAGE_LIMIT (see above), SENSOR_MIN_ELAPSED_TIME (proper
// quantization-noise filter without LPF's phase lag), and PID_OUTPUT_RAMP scaled
// to the new 6 V range (1000 V/s on 24 V was only 4% of range/tick — meaningless;
// 150 V/s on 6 V is 0.5%, enough to bite on audible transients).
constexpr float PID_P             = 0.2f;
constexpr float PID_I             = 20.0f;
constexpr float PID_D             = 0.0f;
constexpr float PID_OUTPUT_RAMP   = 150.0f;         // V/s, voltage slew cap
constexpr float LPF_TF            = 0.02f;          // velocity LPF time constant
constexpr int   MOTION_DOWNSAMPLE = 5;              // outer loop runs every N inner
constexpr float SENSOR_MIN_ELAPSED_TIME = 0.0005f;  // s — at 1024 CPR this drops single-LSB velocity spikes from 30 to 6 rad/s with no phase lag

// App-level position loop. SimpleFOC stays in velocity mode; we close the position
// loop here by feeding the physics-bounded brake curve into move(). Trapezoidal
// profile: accel at MOTION_ACCEL up to MOTION_VELOCITY, cruise, then decelerate
// along `sqrt(2 * MOTION_ACCEL * pos_err)` which lands exactly at target with
// velocity zero. No linear-approach term — it slowed the last rad of travel to
// a crawl and made the LED fade look like it was dragging behind the motor.
constexpr float ANGLE_MAX          = 100.0f * 6.2831853f; // 100 motor rotations
constexpr float MOTION_VELOCITY    = 8.0f;         // rad/s top slew (default = MOTION_VELOCITY_PRESETS[0])
constexpr float MOTION_ACCEL       = 10.0f;         // rad/s² — M2's TARGET_ACCEL_DEFAULT
constexpr float KNOB_STEP_RAD         = 6.2831853f;  // 1 full motor rotation per detent (angle-mode default; unused in M2 velocity-nudge)
constexpr float KNOB_STEP_RAD_PER_SEC = 10.0f;     // M2 velocity-nudge — each detent ±10 rad/s
constexpr float KNOB_VEL_MAX_RAD_PER_SEC = 50.0f;  // M2 velocity-nudge — clamp |target| to this

// Double-click cycles MOTION_VELOCITY through this set (rad/s). CylinderLamp uses
// 4 presets; we match the count so the LED-pulse feedback (count = idx+1) is 1..4.
// Default sits at index 0 = quietest; user bumps up with double-click.
constexpr float MOTION_VELOCITY_PRESETS[] = { 8.0f, 15.0f, 25.0f, 40.0f };
constexpr uint8_t MOTION_VELOCITY_PRESET_DEFAULT = 0;

// Idle-disable: motor stays enabled while moving, disables after IDLE_DISABLE_MS of
// rest (commanded_vel + shaft_velocity + |pos_err| all near zero). While disabled we
// also *skip* motor.move() so the setPwm(0,0,0) in disable() actually persists — the
// velocity PID would otherwise re-drive phases via setPhaseVoltage on the next tick.
// Idle-disable tolerances widened on S3 — the MT6701's tight quantization and the
// P_POSITION=1.0 linear-approach zone can leave commanded_vel hovering near 0.05
// rad/s indefinitely with the old 0.1-rad pos_err band; the motor would audibly
// hold position instead of cutting PWM. 0.3 rad corresponds to ~17° of shaft
// slack at rest, which is undetectable on a kinetic-lamp timescale.
constexpr uint32_t IDLE_DISABLE_MS  = 250;
constexpr float    VEL_AT_REST_EPS  = 0.2f;         // rad/s
constexpr float    POS_AT_REST_EPS  = 0.3f;         // rad

// Stall detection — when the app commands motion but the physical shaft doesn't
// follow (lamp hit its mechanical stop), accept the current position as the new
// target and push it to Matter so the controller-side slider mirrors reality.
// WARMUP skips the initial acceleration; TIMEOUT is how long shaft_vel must stay
// below VEL_EPS before we declare stall.
constexpr float    STALL_VEL_EPS    = 0.2f;         // rad/s
constexpr uint32_t STALL_WARMUP_MS  = 800;          // grace period after motion starts
constexpr uint32_t STALL_TIMEOUT_MS = 400;          // stall declared after this stuck time

// LED behavior. Brightness is a continuous function of shaft angle — off at 0,
// linearly ramping to full over the first LED_FADE_ANGLE_RAD of travel, then
// flat for the rest. The explicit "completely off when motor reaches 0" feel
// comes from zero being exactly zero duty (no threshold hysteresis).
// The 10 ms fader still runs so discontinuous target changes (knob-driven
// brightness nudge, color-temp shifts) glide instead of snapping.
constexpr float    LED_FADE_ANGLE_RAD      = 1.0f;  // angle over which LEDs ramp 0 → max
constexpr uint8_t  LED_MAX_DUTY_DEFAULT    = 200;   // 0..255 (single-click brightness-mode nudges this)
constexpr uint8_t  LED_MAX_DUTY_MIN        = 16;    // floor — below this the gamma curve effectively produces 0 duty and the lamp goes permanently dark; also lets the user recover via a single knob nudge after a factory reset if NVS ever saved something absurdly low
constexpr uint8_t  LED_MAX_DUTY_STEP       = 8;     // knob nudge size in brightness mode (perceptual, pre-gamma)
constexpr uint8_t  LED_FADE_STEP           = 3;     // duty units per fade tick — 1 was visually smoothest but brightness-mode knob changes took ~2.5 s to finish, 3 still looks smooth at ≈ 0.85 s for a full range sweep
constexpr uint32_t LED_FADE_PERIOD_MS      = 10;    // ms between fade ticks
constexpr float    LED_GAMMA               = 2.2f;  // perceptual curve on max_duty — evenly-spaced knob steps feel equal
constexpr uint16_t CT_MIN_MIREDS           = 153;   // Matter ColorTemperatureLight range
constexpr uint16_t CT_MAX_MIREDS           = 500;

// Color temperature blend — 100 (warm) → 500 (cool), scaled against max duty.
// CylinderLamp's linear mapping ported verbatim: WW = (ct-100) * max / 400;
// CW = (500-ct) * max / 400. Default sits mid-scale (300 → 50/50).
constexpr uint16_t COLORTEMP_DEFAULT       = 300;
constexpr uint16_t COLORTEMP_MIN           = 100;
constexpr uint16_t COLORTEMP_MAX           = 500;

// Telemetry.
constexpr uint32_t PRINT_INTERVAL_MS   = 1000;

// Button handling (CylinderLamp-derived single/double/long-press semantics):
//   short click  (<BTN_CLICK_MAX_MS, no second click within BTN_DOUBLE_CLICK_MS):
//       toggle brightness mode (knob → LED max duty instead of motor angle)
//   double click (two clicks within BTN_DOUBLE_CLICK_MS):
//       cycle MOTION_VELOCITY_PRESETS, pulse LEDs (count = preset index + 1)
//   long press BTN_LONG_PRESS_WARNING_MS: 5 warning pulses
//   long press BTN_FACTORY_RESET_MS: factory reset (M4: esp_matter::factory_reset;
//                                                   M3: NVS erase + restart)
constexpr uint32_t BTN_DEBOUNCE_MS              = 30;
constexpr uint32_t BTN_CLICK_MAX_MS             = 400;   // above this → hold, not click
constexpr uint32_t BTN_DOUBLE_CLICK_MS          = 400;   // inter-click window
constexpr uint32_t BTN_LONG_PRESS_WARNING_MS    = 5000;
constexpr uint32_t BTN_FACTORY_RESET_MS         = 9000;

// Dual-core task pinning (S3 only — C6 was UNICORE).
//   Core 0: main, WiFi, lwIP, CHIP, arduino events.
//   Core 1: motor FOC task (dedicated).
constexpr int CORE_MOTOR  = 1;
constexpr int CORE_OTHERS = 0;
constexpr unsigned MOTOR_TASK_PRIORITY = 20;        // above lwIP (18), below IDF timer (22)
