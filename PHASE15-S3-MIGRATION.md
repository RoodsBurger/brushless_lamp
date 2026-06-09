# BrushlessLamp — Phase 15: XIAO ESP32-C6 → XIAO ESP32-S3 Migration

> **Historical planning doc (pre-migration) — superseded by s3/README.md.** Versions, paths and pin tables below are stale.

## Why

After ~14 phases of fighting single-core + no-FPU limitations on the C6, moving to the XIAO S3 unlocks:

1. **Dedicated core for FOC.** The S3 is dual-core Xtensa LX7 @ 240 MHz. Pin the motor control loop to core 1, let Matter/CHIP/WiFi/lwIP/arduino-event live on core 0. Nothing preempts the FOC loop. Expected `maxLp` drops from 1–15 ms (C6 with Matter active) to sub-100 µs steady-state.
2. **Hardware FPU.** SimpleFOC's SVPWM (`setPhaseVoltage` → inverse Park/Clarke → 6 float muls + 4 float adds) runs in single-cycle FPU ops on S3 vs soft-float on C6. Rough estimate: 10× faster inner-loop math.
3. **MCPWM on S3** matches the Arduino-path "center-aligned UP_DOWN" peripheral exactly — 2 groups × 3 operators × 2 generators, no LEDC fallback.
4. **8 MB flash + 8 MB Quad PSRAM.** Comfortable OTA headroom (2 MB slots), and Matter's cert/cluster caches can spill to PSRAM.

The quietness target is to match or beat the Arduino M2-knob sketch on this same mechanical hardware.

---

## What carries over from the C6 build (unchanged)

These were painful to discover on the C6 — all still apply to S3, no re-research needed.

| Change | Purpose | Still required on S3? |
|---|---|---|
| `esp-idf v5.4.4` | esp-matter release/v1.5 pins 5.4.x | ✅ same |
| `esp-matter release/v1.5` | baseline | ✅ same |
| `arduino-esp32 ^3.2.0` via Component Manager | arduino runtime as IDF component | ✅ same (resolves to 3.3.8) |
| `SimpleFOC v2.4.0` vendored at `components/SimpleFOC/` | proven-quiet motor library | ✅ same — MCPWM path activates on S3 |
| `CONFIG_ARDUINO_SELECTIVE_COMPILATION=y` + `Matter=n, OpenThread=n, Insights=n, Zigbee=n` | avoid `Matter.h → esp_matter.h` link conflict | ✅ same (target-agnostic root cause) |
| chip manifest: `esp_insights ^1.2.4 → 1.2.2` | match arduino-esp32's exact pin | ✅ same |
| chip manifest: `esp_rcp_update ~1.3.0 → ~1.2.0` | match rainmaker's transitive dep | ✅ same |
| PR#42320 cherry-pick on chip | fix WiFi-reconnect Load-access-fault (C6) / LoadProhibited (S3) | ✅ same until esp-matter bumps chip past `c2ad2a5` |
| `-DARDUINO_USB_CDC_ON_BOOT=1 -DARDUINO_USB_MODE=1` in main/CMakeLists.txt | route Arduino `Serial` to HWCDC (USB_SERIAL_JTAG) | ✅ same |
| `CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y` | IDF `printf` to USB | ✅ same |
| `CONFIG_BT_NIMBLE_ENABLED=y / CONFIG_BT_BLUEDROID_ENABLED=n` | esp-matter requires NimBLE | ✅ same |
| `CONFIG_AUTOSTART_ARDUINO=n` | we own `app_main` | ✅ same |
| `printf()` instead of `Serial.printf()` | C6-specific: Serial defaulted to UART0 on GPIO 17 = PIN_NSP | ⚠️ less critical on S3 (HWCDC works) but harmless; keep for consistency |
| Build out-of-tree at `~/esp/brushlesslamp-idf` (no spaces) | libsodium's unquoted `-include` breaks on spaces | ✅ same |

---

## What changes for S3

### Pin map — XIAO D-label → chip GPIO

The BrushlessLamp breakout is wired by D-label. Only the GPIO numbers need updating in `pins.h`.

| D-label | XIAO C6 GPIO | XIAO S3 GPIO | Function | Notes |
|---|---|---|---|---|
| D0 | 0 | **1** | MT6701 ENC_A | |
| D1 | 1 | **2** | MT6701 ENC_B | |
| D2 | 2 | **3** | Knob ROT_A | ⚠️ S3 GPIO3 = JTAG strap; a knob in a deterministic state at boot is fine (internal pull-up puts it HIGH) |
| D3 | 21 | **4** | Knob ROT_B | |
| D4 | 22 | **5** | Knob button | |
| D5 | 23 | **6** | LED_WW | |
| D6 | 16 | **43** | LED_CW | ⚠️ S3 GPIO43 = UART0 TX. ROM bootloader prints on it until app starts (~30 ms blink cosmetic). Suppressed by `CONFIG_ESP_CONSOLE_SECONDARY_NONE=y`. |
| D7 | 17 | **44** | DRV8313 nSLEEP | ⚠️ S3 GPIO44 = UART0 RX. Input during ROM boot → harmless. External pullup on nSLEEP keeps driver awake. |
| D8 | 19 | **7** | DRV8313 IN3 (PWM C) | |
| D9 | 20 | **8** | DRV8313 IN2 (PWM B) | |
| D10 | 18 | **9** | DRV8313 IN1 (PWM A) | |

### sdkconfig — S3 additions / overrides

```kconfig
# --- base (shared between M1–M4) ---
CONFIG_IDF_TARGET="esp32s3"
CONFIG_AUTOSTART_ARDUINO=n
CONFIG_FREERTOS_HZ=1000
CONFIG_ESP_MAIN_TASK_STACK_SIZE=8192

# Console on USB_SERIAL_JTAG (D+/D- on the USB-C connector, not TinyUSB).
CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y
CONFIG_ESP_CONSOLE_SECONDARY_NONE=y

# --- dual-core task affinity ---
# Main IDF task, lwIP, WiFi, arduino event/UDP → all on core 0. Motor FOC task
# pinned to core 1 in code via xTaskCreatePinnedToCore.
CONFIG_ESP_MAIN_TASK_AFFINITY_CPU0=y
CONFIG_LWIP_TCPIP_TASK_AFFINITY_CPU0=y
CONFIG_ESP_WIFI_TASK_CORE_ID_0=y
CONFIG_ARDUINO_RUN_CORE0=y
CONFIG_ARDUINO_EVENT_RUN_CORE0=y
CONFIG_ARDUINO_SERIAL_EVENT_RUN_CORE0=y
CONFIG_ARDUINO_UDP_RUN_CORE0=y

# --- flash + PSRAM (M1–M3 don't strictly need these but they're cheap) ---
CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y
CONFIG_SPIRAM=y
CONFIG_SPIRAM_MODE_QUAD=y
CONFIG_SPIRAM_SPEED_80M=y

# --- arduino-esp32 surgery ---
# arduino-esp32 3.3.8's Matter library #includes esp_matter.h without declaring
# a dep, and its registry lookup maybe_add_component(espressif__esp_matter) would
# duplicate symbols. Suppress the Matter library + the other heavy ones we don't
# use — core (Serial, pinMode, millis, ledc, attachInterrupt) is always compiled.
CONFIG_ARDUINO_SELECTIVE_COMPILATION=y
CONFIG_ARDUINO_SELECTIVE_Matter=n
CONFIG_ARDUINO_SELECTIVE_OpenThread=n
CONFIG_ARDUINO_SELECTIVE_Insights=n
CONFIG_ARDUINO_SELECTIVE_Zigbee=n
```

For **M4 only**, add Matter bits:

```kconfig
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_FILENAME="partitions.csv"
CONFIG_PARTITION_TABLE_OFFSET=0xC000
CONFIG_BT_ENABLED=y
CONFIG_BT_NIMBLE_ENABLED=y
CONFIG_BT_BLUEDROID_ENABLED=n
CONFIG_LWIP_IPV6_AUTOCONFIG=y
CONFIG_LWIP_HOOK_IP6_ROUTE_DEFAULT=y
CONFIG_LWIP_HOOK_ND6_GET_GW_DEFAULT=y
CONFIG_LWIP_IPV6_NUM_ADDRESSES=6
CONFIG_MBEDTLS_HKDF_C=y
CONFIG_ESP_MATTER_MAX_DYNAMIC_ENDPOINT_COUNT=2
CONFIG_ENABLE_CHIP_SHELL=y
CONFIG_ENABLE_OTA_REQUESTOR=y
CONFIG_ESP_WIFI_SOFTAP_SUPPORT=n
# + the long CONFIG_SUPPORT_*_CLUSTER=n trim list to keep binary small
```

### Partition table for M4 (8 MB)

```csv
# Name,   Type, SubType, Offset,  Size, Flags
esp_secure_cert,  0x3F, ,0xd000,    0x2000, encrypted
nvs,      data, nvs,     0x10000,   0xC000,
nvs_keys, data, nvs_keys,,          0x1000, encrypted
otadata,  data, ota,     ,          0x2000
phy_init, data, phy,     ,          0x1000,
ota_0,    app,  ota_0,   0x20000,   0x200000,   # 2 MB
ota_1,    app,  ota_1,   0x220000,  0x200000,   # 2 MB
fctry,    data, nvs,     0x420000,  0x6000
```

M1–M3 can use a smaller default layout (no OTA, no secure cert, no factory partition).

### Code-level changes for quietness

1. **Pin motor FOC task to core 1** via `xTaskCreatePinnedToCore(... coreID=1)`.
2. **Call `encoder.enableInterrupts()` from *inside* the motor task body** — arduino-esp32's `attachInterrupt` registers the ISR on whichever core invoked it. Putting the call on core 1 means the MT6701 quadrature ISR runs on core 1 too, and `noInterrupts()` in `Encoder::update()` (which is per-core only) actually blocks the ISR.
3. **Raise motor task priority to 20** (above lwIP's 18 but below FreeRTOS idle-enforcement). The motor task yields 1 ms via `vTaskDelay(1)` every 32 iters so the IDLE watchdog on core 1 stays fed.
4. **Keep M2-knob tuning exactly**: `PHASE_RESISTANCE=5, CURRENT_LIMIT=0.5, LPF_VEL_TF=0.02, MOTION_DOWNSAMPLE=5, PID_velocity P=0.2 I=20, output_ramp=1000, SVPWM, center-aligned MCPWM`. Proven-quiet on the same mechanical hardware.
5. **Position loop** in the app task: linear + physics-limited trapezoidal profile, brake-on-reverse, M2-style idle-disable after `IDLE_DISABLE_MS` of rest (and **skip `motor.move()` while disabled** so `setPwm(0,0,0)` actually sticks — SimpleFOC's disable only grounds phases once).

---

## Staged migration (M1 → M4)

User asked for a staged bring-up so each milestone is individually testable. Each stage lives in its own folder under `s3/` as a **self-contained ESP-IDF project**; all share `s3/common/` for `pins.h`, `config.h`, `motor.{h,cpp}`, and `s3/components/SimpleFOC/` (vendored).

```
s3/
├── README.md                 # this file's quick-build reference
├── components/
│   └── SimpleFOC/            # v2.4.0, shared by all m-projects
├── common/                   # shared header/source component
│   ├── CMakeLists.txt
│   ├── pins.h                # XIAO S3 pin map
│   ├── config.h              # motor tuning constants
│   ├── motor.h               # public motor API + optional settle callback
│   └── motor.cpp             # FOC task, position loop, idle-disable
├── sdkconfig.defaults.common # shared defaults (USB, Arduino, dual-core)
├── m1-motor/                 # Stage 1: motor only, fixed target ramp
├── m2-knob/                  # Stage 2: + quadrature knob + stop button
├── m3-leds/                  # Stage 3: + WW/CW LEDC angle follower
└── m4-matter/                # Stage 4: + Matter ColorTemperatureLight
```

### Build / flash for any stage

```sh
# One-time setup (if not already done)
source ~/esp/esp-idf/export.sh
source ~/esp/esp-matter/export.sh   # M4 only

# Each stage is a normal IDF project
cd s3/m1-motor
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/cu.usbmodem101 flash monitor
```

For M4 only: apply the chip manifest patches (both `esp_insights` and `esp_rcp_update`) and PR#42320 if not already applied — see the production catalog.

Because the project path has spaces, each stage also syncs to `~/esp/brushlesslamp-s3-<stage>/` before building — same rsync pattern used on C6:

```sh
rsync -a --exclude=build --exclude=managed_components --exclude=dependencies.lock \
    --exclude=sdkconfig s3/m1-motor/ ~/esp/brushlesslamp-s3-m1/
cd ~/esp/brushlesslamp-s3-m1 && idf.py build
```

### Stage 1 — M1-motor

Goal: validate that the S3 toolchain compiles, FOC initializes on the S3 MCPWM, and the motor spins quietly under a programmatic target.

What runs:
- `motor_init_and_start()` (from `common/`) initializes SimpleFOC velocity mode with M2-proven tuning, spawns the FOC task pinned to core 1.
- `app_main` sets a target angle that ramps sinusoidally (e.g., ±10 rad with 15 s period) so you can listen for any audible artifacts.
- No knob, no LEDs, no Matter, no WiFi, no BLE.

Acceptance: motor smoothly ramps between endpoints, audibly identical to M2-knob sketch. Tail of log should show `maxLp < 100 µs` steady-state with no WiFi/CHIP load. Record `/tmp/s3_m1.log` for reference.

### Stage 2 — M2-knob

Adds `input.{h,cpp}`:
- MT6701 knob quadrature ISR (two-pin pair at GPIO3/GPIO4 on S3).
- Debounced button (GPIO5).
- Drain knob detents into `motor_nudge_target_angle(±KNOB_STEP_RAD)` at 10 ms poll.
- Button → `motor_set_target_angle(0)`.

Acceptance: knob detents move the lamp smoothly, same feel as C6. Button returns to origin.

### Stage 3 — M3-leds

Adds `leds.{h,cpp}`:
- LEDC at 25 kHz, 8-bit, two channels (GPIO6 = WW, GPIO43 = CW).
- 10 Hz follower task reading `motor_get_shaft_angle()`.
- `|shaft| > LED_ON_ANGLE_THRESH` → on at fixed `LED_DUTY`; else off.
- No color-temp source in M3 — just fixed WW/CW mix.

Acceptance: LEDs come on once the shaft is lifted past threshold, turn off when returned. No motor-audibility regression vs M2.

### Stage 4 — M4-matter

Adds `matter_app.{h,cpp}`:
- esp-matter `color_temperature_light` endpoint.
- Attribute callback → `motor_set_target_angle((level/254)·ANGLE_MAX)`.
- `matter_get_color_temp_mireds()` consumed by `leds.cpp` for WW/CW mix.
- Knob writes back to Matter via `motor_set_settle_callback(matter_push_level_from_angle)` — M1–M3 don't register a callback, so the same `motor.cpp` in `common/` is link-safe for all stages.
- OnOff=false → motor target 0 + idle-disable + LED off.

Acceptance: commission via Apple Home / Google Home. Slider moves lamp height; color temp slider shifts WW/CW mix. Knob turn reflects in the app within ~300 ms (deferred push on settle).

---

## Risks + rollback

- **USB-CDC might misbehave on first flash.** If the XIAO S3 enumerates differently than C6, `/dev/cu.usbmodem101` might change. Worst case: boot in DFU (hold BOOT, connect USB) and flash via esptool directly.
- **PR#42320 reapplication after any esp-matter reinstall** — documented in `PHASE13-PRODUCTION-CATALOG.md §3`.
- **Rollback to C6**: the existing `esp-idf/` directory at project root is untouched. `git checkout 40b9aa2 -- esp-idf/` brings back the last known-working C6 build.

---

## Deferred to Phase 16+

- Production-grade factory partition with per-device DAC via `esp-matter-mfg-tool` (catalog §5).
- CSA-signed VID/PID (replacing test 0xFFF1/0x8001).
- OTA rollout via chip-tool or Home app.
- Position control refinements (closed-loop torque compensation at heavy load).
