#pragma once

#include <stdint.h>

// Tuning shared by every S3 stage. Values are the silent-baseline hardware
// combo (4015 gimbal / 11 PP / DRV8313 / 24 V / MT6701 @ 1024 CPR quadrature).

// Motor electrical.
constexpr uint32_t PWM_FREQ_HZ       = 25000;
constexpr int      POLE_PAIRS        = 11;
constexpr int      ENCODER_PPR       = 1024;        // MT6701 ABZ pulses/rev; SimpleFOC quadruples to 4096 CPR in quadrature
constexpr float    SUPPLY_VOLTAGE    = 24.0f;
constexpr float    PHASE_RESISTANCE  = 5.0f;
constexpr float    KV_RATING         = 100.0f;
// INERT in voltage torque mode (SimpleFOC 2.4.0 ignores it there). Stall
// protection is the runtime stall handler + nSLEEP fault-clear in motor.cpp.
// estimated_current mode would enforce this, but its unfiltered back-EMF
// feed-forward makes the motor audibly louder, so voltage mode is used instead.
constexpr float    CURRENT_LIMIT     = 0.5f;
constexpr float    VELOCITY_LIMIT    = 50.0f;       // SimpleFOC's internal velocity cap
// 6 V sensor-align drives enough current (~1.2 A / 5 Ω) for a clean direction
// sweep on first boot; sensor_direction is then cached in NVS.
constexpr float    VOLTAGE_SENSOR_ALIGN = 6.0f;
// Caps Uq in voltage-torque mode. At 40 rad/s the PID needs ~10 V (back-EMF +
// IR + headroom); 18 V keeps the output off the rail without losing torque,
// and stays above the DRV8313's 200 ns min-on-time at idle duty.
constexpr float    VOLTAGE_LIMIT     = 18.0f;

// Inner velocity PID. Output is Uq volts (voltage torque mode — the silent
// baseline; SimpleFOC docs recommend it for low-speed gimbal motors).
constexpr float PID_P             = 0.2f;
constexpr float PID_I             = 20.0f;
constexpr float PID_D             = 0.0f;
constexpr float PID_OUTPUT_RAMP   = 150.0f;         // V/s cap on Uq slew
constexpr float LPF_TF            = 0.02f;          // velocity LPF time constant (s)
constexpr int   MOTION_DOWNSAMPLE = 5;              // move() runs every N loopFOC ticks

// App-level position loop. SimpleFOC runs velocity mode; the position loop
// here feeds a physics-bounded brake curve into move():
//     desired = min(MOTION_VELOCITY, sqrt(2 * MOTION_ACCEL * |pos_err|))
// Accel / cruise / decel profile lands exactly at target with v=0.
constexpr float    ANGLE_MAX          = 77.0f * 6.2831853f;  // 77 motor rotations — reaches max lamp height at 100%; shared by knob nudge and Matter level scale.
constexpr float    MOTION_VELOCITY    = 15.0f;     // default cruise cap (rad/s); runtime override via motor_set_motion_velocity
constexpr float    MOTION_ACCEL       = 10.0f;      // rad/s² ramp + brake rate
constexpr float    MOTION_EPS         = 0.5f;       // brake-on-reverse sign-change eps (rad/s)
constexpr float    KNOB_STEP_RAD         = 6.2831853f;  // 1 motor rotation per detent

// Triple-click cycles MOTION_VELOCITY through these presets; index is also the LED-pulse blink count - 1.
constexpr float   MOTION_VELOCITY_PRESETS[]      = { 15.0f, 25.0f, 40.0f };
constexpr uint8_t MOTION_VELOCITY_PRESET_COUNT   = sizeof(MOTION_VELOCITY_PRESETS) / sizeof(MOTION_VELOCITY_PRESETS[0]);
constexpr uint8_t MOTION_VELOCITY_PRESET_DEFAULT = 0;

// 0.3-rad POS eps is ≈17° of shaft slack — the position loop deadbands inside it
// so the motor doesn't hold torque indefinitely when parked near target.
constexpr uint32_t IDLE_DISABLE_MS  = 250;
constexpr float    VEL_AT_REST_EPS  = 0.2f;
constexpr float    POS_AT_REST_EPS  = 0.3f;

constexpr float    STALL_VEL_EPS    = 0.5f;   // shaft slowing past this is "frozen"
constexpr uint32_t STALL_WARMUP_MS  = 400;   // grace after engage so the trapezoidal ramp doesn't false-trip
constexpr uint32_t STALL_TIMEOUT_MS = 150;   // sustained-frozen window before stall fires

// LED driver (LEDC). 25 kHz keeps PWM well above the audible band; 10-bit duty
// gives the gamma curve 4× finer low-end steps than 8-bit. 10 bits is the max at
// 25 kHz: arduino-esp32 clocks LEDC from the 40 MHz XTAL (40 MHz / 25 kHz = 1600
// counts < 2^11). Perceptual values stay 0..255; hardware duty is 0..1023.
constexpr uint32_t LED_PWM_FREQ_HZ     = 25000;
constexpr uint8_t  LED_PWM_RESOLUTION  = 10;
constexpr uint16_t LED_HW_DUTY_MAX     = 1023;

// LED brightness ramps 0 → max linearly across the first LED_FADE_ANGLE_RAD of shaft travel; fader smooths discontinuous target changes.
constexpr float    LED_FADE_ANGLE_RAD   = 3.0f;
constexpr uint8_t  LED_MAX_DUTY_DEFAULT = 200;      // 0..255 pre-gamma
constexpr uint8_t  LED_MAX_DUTY_MIN     = 16;       // floor on knob + NVS load — below this gamma squashes to ~0 and the lamp reads as broken
constexpr uint8_t  LED_MAX_DUTY_STEP    = 8;        // knob nudge in brightness mode
constexpr uint16_t LED_FADE_STEP        = 4;        // 10-bit duty units per fader tick (full 0..1023 sweep ≈ 2.56 s)
constexpr uint32_t LED_FADE_PERIOD_MS   = 10;
constexpr float    LED_GAMMA            = 2.2f;     // perceptual curve so equal knob steps feel visually equal
// Color temperature blend: WW = (ct-MIN)*max/span, CW = (MAX-ct)*max/span.
// 153 (≈6500 K cool) and 454 (≈2200 K warm) match the CHIP reference lighting-app's
// ColorTempPhysicalMin/MaxMireds — Google Home's slider sweeps that exact range.
constexpr uint16_t COLORTEMP_DEFAULT = 370;   // Soft White (~2700 K), the 2nd-warmest Google Home preset
constexpr uint16_t COLORTEMP_MIN     = 153;
constexpr uint16_t COLORTEMP_MAX     = 454;
constexpr int16_t  KNOB_CT_STEP_MIREDS = 10;  // CT mode: full warm↔cool sweep takes ~30 detents (≈1¼ knob rotations)
// Per-tick CT slew so Google Home slider drags (which arrive as instant attribute snaps with TransitionTime=0) fade visually over ~600 ms instead of stepping.
constexpr uint16_t CT_FADE_STEP_MIREDS = 5;

// Button: 1 tap = Matter OnOff toggle | 2 taps = cycle knob mode (motor → brightness → CT) | 3 taps = speed preset.
// Hold ≥5 s (3 blinks) then release = re-home; keep holding to ≥15 s (5 blinks) = factory reset.
constexpr uint32_t BTN_DEBOUNCE_MS           = 30;
constexpr uint32_t BTN_CLICK_MAX_MS          = 400;
constexpr uint32_t BTN_DOUBLE_CLICK_MS       = 400;
constexpr uint32_t BTN_HOME_ARM_MS           = 5000;    // hold this long → arm homing (3 blinks); homing runs on release
constexpr uint32_t BTN_FACTORY_RESET_MS      = 15000;   // keep holding to here → factory reset (5 blinks)

// Runtime re-home (knob-hold gesture): drive to the off-stop and set it as logical 0.
// Softer Uq cap than normal so the impact is gentle; faster than boot homing since FOC is already calibrated.
constexpr float    HOMING_VELOCITY   = -8.0f;     // rad/s toward the off-stop
constexpr float    HOMING_VOLTAGE    = 1.0f;      // Uq cap during homing (~0.2 A at 5 Ω — soft stall, no brownout)
constexpr uint32_t HOMING_TIMEOUT_MS = 90000;     // give up if no stop is found (≈full travel at 8 rad/s + margin)

// Dual-core layout: motor FOC on core 1 (no preemption from WiFi/BLE/CHIP),
// everything else on core 0.
constexpr int       CORE_MOTOR           = 1;
constexpr int       CORE_OTHERS          = 0;
constexpr unsigned  MOTOR_TASK_PRIORITY  = 20;      // above lwIP (18), below IDF timer (22)

// Self-hosted signed OTA. The task polls OTA_MANIFEST_URL and updates only when
// the manifest's integer version exceeds OTA_FW_VERSION. Bump OTA_FW_VERSION with
// PROJECT_VER(_NUMBER) in CMakeLists on every release (release.sh does both).
constexpr uint32_t  OTA_FW_VERSION        = 10003;                 // = PROJECT_VER 1.0.3
constexpr uint32_t  OTA_INITIAL_DELAY_MS  = 60000;                   // first check ~1 min after each boot (so a reboot = an update check)
constexpr uint32_t  OTA_CHECK_INTERVAL_MS = 5u * 24 * 60 * 60 * 1000; // then re-check every 5 days while running
constexpr const char *OTA_MANIFEST_URL =
    "https://github.com/RoodsBurger/brushless_lamp/releases/latest/download/manifest.json";

// WiFi/Matter self-heal. CHIP retries association every 100 ms but can wedge
// after AP loss (esp_wifi_connect → ESP_ERR_WIFI_CONN, "Failed to get configured
// network 0x0500300F"); a full reboot reinitializes the WiFi stack and clears it.
// Only fires while commissioned. Reboots at _TIMEOUT_ if the lamp is idle (so it
// never interrupts local knob/motor use, which works offline), or at _HARD_
// regardless for always-on lamps. Position restores from NVS, so a reboot is cheap.
constexpr uint32_t NET_WATCHDOG_POLL_MS    = 30000;            // connectivity check cadence
constexpr uint32_t NET_WATCHDOG_TIMEOUT_MS = 10u * 60 * 1000;  // disconnected + idle → reboot
constexpr uint32_t NET_WATCHDOG_HARD_MS    = 30u * 60 * 1000;  // disconnected this long → reboot even if on
