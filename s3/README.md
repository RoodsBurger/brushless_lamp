# BrushlessLamp — XIAO ESP32-S3 + Matter

A kinetic smart lamp. A BLDC gimbal motor raises / lowers a lit core — the
higher the core, the brighter the CCT LED strip that rides with it. A single
rotary-encoder knob with push-button drives everything locally; Matter over
WiFi + BLE exposes the lamp to Apple Home / Google Home / SmartThings. The
ESP32-S3 runs the full SimpleFOC v2.4.0 FOC loop itself (no external stepper
driver) on a dedicated core while WiFi, BLE and CHIP share the other core.

The firmware ships as four independently-buildable stages sharing the same
FOC core; only the surface (knob, LEDs, Matter) changes.

```
s3/
├── components/SimpleFOC/        vendored Arduino-FOC v2.4.0
├── common/                      shared motor.{h,cpp} / config.h / pins.h
├── sdkconfig.defaults.common    stacked by every stage
├── m1-motor/                    Stage 1 — audibility sweep
├── m2-knob/                     Stage 2 — + knob velocity-nudge
├── m3-leds/                     Stage 3 — + WW/CW angle-driven LEDs
└── m4-matter/                   Stage 4 — + Matter ColorTemperatureLight
```

M4 is the production stage; M1–M3 are debug scaffolds retained for
regression testing (bring up the motor quietly → add knob → add LEDs → add
Matter). The rest of this doc targets M4.

---

## 1. Product overview

- **Physical action**: a 4015 gimbal motor rotates a hollow-shaft core
  vertically through 100 motor-revolutions of travel. At rest-position 0 the
  core is fully retracted, LEDs dark. At `ANGLE_MAX` (`100 × 2π rad`) the
  core is fully raised and the LEDs are at `max_duty × gamma(2.2)`.
- **LEDs follow the motor, not the slider**: brightness is a continuous
  function of `shaft_angle`, not of Matter Level. Moving the Matter slider
  or the knob changes the *target* angle; the LEDs fade with the physical
  travel. This matches the "lamp is lit because it's physically raised"
  mental model.
- **Idle-disabled at rest**: when the motor reaches its target and settles,
  the driver cuts PWM entirely (250 ms idle debounce) — no holding torque,
  no audible hum.
- **Stall-aware**: block the shaft with a finger and the motor gives up
  after 1.2 s, accepts the physical position as the new target, and pushes
  the corresponding level back to Matter so the Home app slider snaps to
  reality.
- **Per-device Matter identity**: each unit is provisioned with a unique
  VID / PID / discriminator / Spake2+ verifier via `esp-matter-mfg-tool` so
  two lamps can coexist on the same fabric.

---

## 2. Hardware

### BOM

| Qty | Part                                       | Notes |
|----:|--------------------------------------------|-------|
| 1   | Seeed XIAO ESP32-S3 (ESP32-S3R8 variant)   | dual-core Xtensa, 8 MB flash + 8 MB Octal PSRAM (PSRAM stays off in firmware) |
| 1   | SimpleFOC Mini (DRV8313 breakout)          | 3-phase driver, `IN1/2/3`, `EN`, `nSLEEP`, `nFAULT` |
| 1   | 4015 BLDC gimbal motor                     | 11 pole pairs, hollow-shaft outer rotor, 12–36 V |
| 1   | MT6701 magnetic encoder (ABZ mode)         | 1024 CPR quadrature |
| 1   | PEC11-style rotary encoder w/ push-button  | 20 detents / revolution |
| 1   | dual-channel CCT LED strip (WW + CW)       | common-anode on 24 V, separate cathodes |
| 2   | AO3400A N-channel MOSFET (SOT-23)          | one per LED channel |
| 1   | Mean Well LRS-75-24                        | 24 V 3.2 A mains supply |
| 1   | Mini360 buck 24 V → 5 V                    | feeds XIAO `5V` |
| 1   | SS14 Schottky diode (or equivalent, ≥ 1 A) | series between Mini360 OUT+ and XIAO `5V` — blocks USB-host back-feed during dev/flash |
| 1   | 470 µF–1000 µF low-ESR electrolytic, ≥ 6.3 V | bulk decoupling on XIAO `5V`/GND, leads as short as possible (< 5 mm to the pads) |

### Wiring

> **Production target shown.** The external-supply path now boots reliably as
> of 2026-04-28 (CHIP_SHELL fix in § 7.1). The 470 µF + SS14 stays in the BOM
> as transient defense against Mini360 droop.

```
              ┌──────────────────┐
   230 VAC ─┤ Mean Well LRS-75-24├── V+ (24 V) ─┬─ SimpleFOC Mini VM
              │                   │               ├─ LED strip V+ (common anode)
              └───────────────────┘               └─ Mini360 IN+
                                                        │
                                    Mini360 OUT 5 V ────┘
                                         │
                                       SS14 ─►  (anode→Mini, cathode→XIAO 5V)
                                         │
                                         ├──[470 µF]── GND   (cap directly across
                                         │                    XIAO 5V/GND pads,
                                         │                    < 5 mm leads)
                                         │
                                         └── XIAO S3 5V  (onboard LDO → 3V3)
                                                   │
                                                   ├── MT6701 V+
                                                   ├── SimpleFOC Mini 3V3 / EN jumper
                                                   └── rotary encoder V+

SimpleFOC Mini ── U / V / W ─── 3-phase motor
                │
                └── nSLEEP → XIAO D7 (GPIO 44; UART0 RX strap — harmless at boot)
                └── IN1/IN2/IN3 → XIAO D10 / D9 / D8 (MCPWM-capable, phase C/B/A)

MT6701 A/B ──── XIAO D0 / D1 (GPIO 1 / 2)

rotary encoder A/B/SW ─── XIAO D2 / D3 / D4 (GPIO 3 / 4 / 5)
                                          ^ GPIO 3 is the JTAG-select strap;
                                            internal pull-up at boot is fine.

LED WW gate ─── XIAO D5 (GPIO 6)  ── AO3400 ── WW cathode (strip)
LED CW gate ─── XIAO D6 (GPIO 43) ── AO3400 ── CW cathode (strip)
                                          ^ GPIO 43 is UART0 TX — prints a
                                            short ROM banner at power-up,
                                            visible as a flicker on CW.
```

`EN` on the SimpleFOC Mini must be jumpered to `3V3` on the breakout itself.
Its internal pull-up is too weak to survive a floating MCU pin and the lamp
doesn't need software tri-state (idle-disable drives the phases to balanced
50 % duty → zero phase-to-phase differential → zero torque, which is all the
software "disable" means anyway).

### Pin map

| D-label | GPIO | Peripheral          | Notes |
|--------:|-----:|---------------------|-------|
| D0      | 1    | MT6701 A            | encoder quadrature, internal pull-up |
| D1      | 2    | MT6701 B            | encoder quadrature |
| D2      | 3    | Knob rot A          | JTAG-select strap — safe at boot |
| D3      | 4    | Knob rot B          | |
| D4      | 5    | Knob push-button    | INPUT_PULLUP, polled |
| D5      | 6    | LED WW MOSFET gate  | LEDC PWM 25 kHz / 8-bit |
| D6      | 43   | LED CW MOSFET gate  | UART0 TX strap — brief ROM flicker at boot |
| D7      | 44   | DRV8313 nSLEEP       | UART0 RX strap — external pull-up holds HIGH |
| D8      | 7    | DRV8313 IN3 (phase C)| MCPWM |
| D9      | 8    | DRV8313 IN2 (phase B)| MCPWM |
| D10     | 9    | DRV8313 IN1 (phase A)| MCPWM |
| 3V3     | —    | MT6701 · SimpleFOC 3V3 / EN jumper · encoder V+ | |
| 5V      | —    | Mini360 OUT+         | XIAO onboard LDO to 3V3 |
| GND     | —    | common               | shared with Mean Well V−, Mini360 OUT−, LED MOSFET sources |

### Pin map: XIAO vs Teyleten SuperMini

Same chip (ESP32-S3) — only the physical pin location each peripheral wires to
differs. `s3/common/pins.h` selects the right GPIO numbers based on the
`BRUSHLESSLAMP_BOARD_TEYLETEN` build define.

| Signal       | XIAO GPIO | Teyleten GPIO | Notes |
|--------------|----------:|--------------:|-------|
| `PIN_ENC_A`  | 1  | 1  | unchanged |
| `PIN_ENC_B`  | 2  | 2  | unchanged |
| `PIN_ROT_A`  | 3  | 3  | JTAG strap, internal pull-up at boot is fine |
| `PIN_ROT_B`  | 4  | 4  | unchanged |
| `PIN_BTN`    | 5  | 5  | unchanged |
| `PIN_LED_WW` | 6  | 6  | unchanged |
| `PIN_LED_CW` | 43 | 7  | off the UART0 TX strap on Teyleten — no boot flicker |
| `PIN_DRV_EN` | — (jumpered to 3V3) | 8 | software-controlled on Teyleten |
| `PIN_NSP`    | 44 | 10 | off the UART0 RX strap on Teyleten |
| `PIN_PWM_C`  | 7  | 11 | DRV8313 IN3 |
| `PIN_PWM_B`  | 8  | 12 | DRV8313 IN2 |
| `PIN_PWM_A`  | 9  | 13 | DRV8313 IN1 |
| (spare)      | —  | 9  | unused on Teyleten |

---

## 3. Control surface

All gestures come from the single rotary-encoder knob with integrated push-
button.

| Gesture                                        | Action |
|------------------------------------------------|--------|
| Turn the knob                                  | **Motor mode (default)**: each detent nudges the motor target by `KNOB_STEP_RAD` (one full motor rotation). LEDs fade with travel. |
| Single click (< 400 ms, no follow-up)          | Toggle into **brightness mode**: the knob nudges LED `max_duty` instead (gamma-2.2 perceptual curve, floored at `LED_MAX_DUTY_MIN`). Another click returns to motor mode. |
| Double click (two clicks < 400 ms apart)       | Cycle through `MOTION_VELOCITY_PRESETS` (8 / 15 / 25 / 40 rad/s). LEDs flash the new preset's index + 1 times as feedback. Persisted to NVS. |
| Hold ≥ 5 s                                     | 5 warning blinks. Release to cancel. |
| Hold ≥ 9 s                                     | `esp_matter::factory_reset()`: wipes the Matter fabric + CHIP-KVS + our `foc_cal` / `leds` / `input` NVS namespaces, then reboots. |
| Matter OnOff toggle (Home app)                 | Off → motor target 0, LEDs fade with travel. On → motor target = `level / 254 × ANGLE_MAX`. |
| Matter Level slider                            | Sets motor target. Knob motion pushes the current level back into the Home app once the motor settles. |
| Matter ColorTemperature slider                 | Retargets the LED warm/cool mix (153 mireds = cool, 500 = warm). Applied on the next fader tick. |

---

## 4. Toolchain setup

One-time, per development machine.

```sh
source ~/esp/esp-idf/export.sh          # ESP-IDF v5.4.4
source ~/esp/esp-matter/export.sh       # esp-matter release/v1.5
```

`~/esp/esp-matter/connectedhomeip/connectedhomeip/config/esp32/components/chip/idf_component.yml` needs two version downgrades the first time you clone esp-matter:

```sh
sed -i '' 's|version: "\^1.2.4"|version: "1.2.2"|' \
  ~/esp/esp-matter/connectedhomeip/connectedhomeip/config/esp32/components/chip/idf_component.yml
sed -i '' 's|version: "~1.3.0"|version: "~1.2.0"|' \
  ~/esp/esp-matter/connectedhomeip/connectedhomeip/config/esp32/components/chip/idf_component.yml
```

Also confirm `connectedhomeip` PR #42320 is cherry-picked on the submodule
— it's the WiFi-reconnect fix that survives commissioning drop-outs.

---

## 5. Build + flash

`m4-matter/` is **the production build**. The project path (`Personal
Projects/BrushlessLamp`) contains a space which libsodium's unquoted `-include`
chokes on, so every build runs in a no-space shadow directory under
`~/esp/brushlesslamp-s3/`.

> Substitute `<PORT>` with your XIAO's USB-CDC device (typically
> `/dev/cu.usbmodem1101` on macOS; check with `ls /dev/cu.usbmodem*`).

```sh
# One-time rsync + every edit after
rsync -a --delete \
      --exclude=build --exclude=managed_components --exclude=dependencies.lock \
      --exclude=sdkconfig --exclude=sdkconfig.old --exclude=mfg_out \
      "/Users/rodolfo/Documents/Personal Projects/BrushlessLamp/s3/" \
      ~/esp/brushlesslamp-s3/

cd ~/esp/brushlesslamp-s3/m4-matter
idf.py set-target esp32s3
idf.py build
idf.py -p <PORT> flash monitor
```

M1 / M2 / M3 are the staged debug scaffolds — same recipe, swap the stage
directory. They don't need `esp-matter` (no Matter, no BLE), so sourcing
`~/esp/esp-matter/export.sh` is optional.

### Experimental: Teyleten ESP32-S3 SuperMini build

`s3/supermini_m4/` is a sibling of `m4-matter/` for the Teyleten board.
Same firmware behavior — only the GPIO numbers wired to peripherals differ
(see § 2 Hardware "Pin map: XIAO vs Teyleten"). The supermini build sets
`-DBRUSHLESSLAMP_BOARD_TEYLETEN=1` at project scope, which selects the
alternate branch in `s3/common/pins.h`. Drops OTA + uses 4 MB flash layout
to fit Teyleten's smaller flash (`partitions.csv`, `sdkconfig.defaults`).

```sh
cd ~/esp/brushlesslamp-s3/supermini_m4
idf.py set-target esp32s3
idf.py build
idf.py -p <PORT> flash monitor
```

### Stage acceptance

| Stage | What runs | Acceptance |
|-------|-----------|------------|
| M1    | motor + audibility sweep loop | Motor walks ±10 / ±20 / ±30 rad/s silently. Report `maxLp ≤ 300 µs` at steady state. |
| M2    | + knob velocity-nudge + button | Knob detents adjust target velocity ±10 rad/s (cap ±50). Button zeros. Same audibility as M1. |
| M3    | + LEDs | Knob moves motor; LEDs fade smoothly on/off with travel. Double-click cycles speed presets with 1–4 blinks of feedback. Brightness-mode click works. No motor regression. |
| M4    | + Matter | Commission via Apple/Google/Samsung Home (see §6). Slider moves lamp, color-temp slider shifts mix, knob-driven moves push back to the app, 5 s pulse + 9 s factory-reset work. |

---

## 6. Per-device Matter identity

`CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER=y` is set in M4's sdkconfig.
Without a flashed `fctry` + `esp_secure_cert` partition, the factory-data
provider returns `CHIP_ERROR_INVALID_ARGUMENT` for every lookup and
commissioning simply doesn't work. The two partitions are generated by
`esp-matter-mfg-tool` (installed with esp-matter; `pip show
esp-matter-mfg-tool` to confirm).

### Generate credentials for N devices

```sh
mkdir -p ~/esp/brushlesslamp-s3/m4-matter/mfg_out
cd       ~/esp/brushlesslamp-s3/m4-matter/mfg_out

esp-matter-mfg-tool \
  -n 1 \
  --target esp32s3 \
  -v 0xFFF2 -p 0x8001 \
  --vendor-name "Rods" --product-name "BrushlessLamp" \
  --hw-ver 1 --hw-ver-str "m4" \
  --pai \
  --dac-in-secure-cert \
  -c  $ESP_MATTER_PATH/connectedhomeip/connectedhomeip/credentials/test/attestation/Chip-Test-PAI-FFF2-8001-Cert.pem \
  -k  $ESP_MATTER_PATH/connectedhomeip/connectedhomeip/credentials/test/attestation/Chip-Test-PAI-FFF2-8001-Key.pem \
  -cd $ESP_MATTER_PATH/connectedhomeip/connectedhomeip/credentials/test/certification-declaration/Chip-Test-CD-FFF2-8001.der
```

VID `0xFFF2` / PID `0x8001` are the CHIP test values with PAI + CD bundled
in esp-matter's credentials tree (so no CSA signing needed for a hobby
build). For a production run, swap for your CSA-assigned VID + PAA/PAI.

The tool writes a per-device directory:

```
mfg_out/out/fff2_8001/<uuid>/
├── <uuid>_esp_secure_cert.bin      → flash at 0xd000
├── <uuid>-partition.bin            → flash at 0x420000 (fctry)
├── <uuid>-onb_codes.csv            ← grab the QR / manual code here
└── <uuid>-qrcode.png               ← scan this into the Home app
```

The `-onb_codes.csv` line gives the commissioning data:

```
qrcode,manualcode,discriminator,passcode
MT:634J0...XXXXXXXXXX,XXXX-XXX-XXXX,NNNN,NNNNNNNN
```

Write these down before you flash — the QR CHIP prints in the serial log
is a fallback (the factory partition only stores the Spake2+ verifier, not
the raw passcode).

### Flash firmware + per-device partitions together

```sh
B1=~/esp/brushlesslamp-s3/m4-matter/mfg_out/out/fff2_8001/<uuid>

idf.py -p <PORT> erase-flash        # wipe any prior state

python -m esptool --chip esp32s3 -p <PORT> -b 460800 \
  --before default_reset --after hard_reset write_flash \
  --flash_mode dio --flash_size 8MB --flash_freq 80m \
  0x0        build/bootloader/bootloader.bin        \
  0xc000     build/partition_table/partition-table.bin \
  0xd000     $B1/*_esp_secure_cert.bin              \
  0x1d000    build/ota_data_initial.bin             \
  0x20000    build/brushlesslamp_m4_matter.bin      \
  0x420000   $B1/*-partition.bin
```

Offsets match `partitions.csv`:

```
esp_secure_cert  0x0d000  0x02000
nvs              0x10000  0x0c000
nvs_keys         0x1c000  0x01000
otadata          0x1d000  0x02000
phy_init         0x1f000  0x01000
ota_0            0x20000  0x200000
ota_1            0x220000 0x200000
fctry            0x420000 0x06000
```

### Commission

1. Plug the lamp in. Watch the serial log for a line like
   `[matter] factory data: VID=0xFFF2 PID=0x8001 discriminator=NNNN` —
   that's confirmation the per-device partitions loaded. If you see
   `Using default EXAMPLE passcode 20202021`, that's expected (CHIP can't
   regenerate the QR from the verifier alone) — the actual device accepts
   the real passcode from `-onb_codes.csv`.
2. Apple Home: `+` → Add Accessory → "More options…" → enter the
   **manual code** from the CSV, or scan the PNG.
   Google Home: `+` → Set up device → Works with Google → Matter → scan.
3. Give the device your WiFi (2.4 GHz only — the S3's radio is single-band).
4. The Home app lands on a ColorTemperatureLight. OnOff, Level,
   ColorTemperature all work. Decommission from the Home app → the device
   reopens a 300 s pairing window automatically.

---

## 7. Workarounds

| Problem | Symptom | Where it's fixed | Upstream |
|---------|---------|------------------|----------|
| `initArduino()` calls `esp_bt_controller_mem_release()` which frees the BT controller's own memory into the heap. | `Guru Meditation Error: LoadProhibited` inside `r_llm_env_init` during BLE bring-up. | Strong override of weak `btInUse()` in `s3/m4-matter/main/matter_app.cpp`. `__attribute__((used))` keeps it under LTO. | Same fix ESP Rainmaker uses on S3. |
| `esp_secure_cert_mgr` 2.9.x removes `esp_secure_cert_get_priv_key()` that CHIP's `ESP32SecureCertDACProvider.cpp` still calls. | Compile error: "'esp_secure_cert_get_priv_key' was not declared in this scope". | Exact pin `"2.5.0"` in `s3/m4-matter/main/idf_component.yml`. | [connectedhomeip#31382](https://github.com/project-chip/connectedhomeip/issues/31382), [esp-matter#798](https://github.com/espressif/esp-matter/issues/798). |
| Both `ESP_SECURE_CERT_DS_PERIPHERAL` and `ESP_SECURE_CERT_SUPPORT_LEGACY_FORMATS` gate the TLV API that CHIP's DAC provider needs. | Build picks one API visible and the other not, compile error on whichever is referenced. | Both flags explicitly off in `sdkconfig.defaults.esp32s3`. | — |
| Default `CHIP_FACTORY_NAMESPACE_PARTITION_LABEL` is `"nvs"`, which is CHIP's KVS partition — not where our mfg_tool data lives. | `GetSetupDiscriminator() failed: 201` at boot, device keeps falling back to test defaults. | `CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION_LABEL="fctry"` in the M4 sdkconfig. | — |
| MT6701 runs in incremental quadrature; its counter starts at 0 on every boot regardless of physical rotor position. | Caching `zero_electric_angle` across boots produces the wrong Uq direction → motor has no torque. | Motor only caches `sensor_direction` in NVS; `zero_electric_angle` is re-measured by the brief 700 ms align pulse on every boot. | — |
| Project path contains a space → libsodium's unquoted `-include` fails. | Random compile errors under `managed_components/espressif__libsodium/...`. | All builds run out of `~/esp/brushlesslamp-s3/` (rsynced from the project tree). | — |
| BT controller `malloc` is pinned to `INTERNAL|DMA` caps. | Enabling PSRAM doesn't help (controller can't allocate from it) and can destabilise BLE init on the S3R8. | `CONFIG_SPIRAM is not set` in the M4 sdkconfig. | — |
| Matter factory NVS only stores the Spake2+ verifier, not the raw passcode. | CHIP's `PrintOnboardingCodes` in the serial log regenerates a fallback QR — not the one that actually commissions the device. | Use the QR PNG or the `manualcode` column from `mfg_out/out/.../onb_codes.csv`. | expected Matter behavior. |
| `app_reset` + device_hal transitively require `button` and `led_driver` components. | Build failure: "component `button` / `led_driver` could not be found". | `EXTRA_COMPONENT_DIRS` in top-level `CMakeLists.txt` adds both `device_hal/button_driver` and `device_hal/led_driver`, even though our code doesn't use them directly. | — |
| ESP-IDF default CPU frequency is 160 MHz; software ECDSA (PASE_Pake2, CSR keypair) at that clock takes ~800 ms / ~530 ms and blows past the Matter commissioner's per-step timeout (`CHIP_ERROR_TIMEOUT` 32). | Commissioning aborts after CSRRequest with `Long dispatch time: 530 ms` in the CHIP log and no fabric added. | `CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240=y` + `CONFIG_ESP32S3_DEFAULT_CPU_FREQ_240=y` in `sdkconfig.defaults.esp32s3` — runs at the S3's rated 240 MHz and cuts crypto latency by ~1/3. | — |
| Default BLE ATT MTU 256 fragments Matter's AttestationResponse / CSRResponse / AddNOC (400–700 B) into 6+ L2CAP PDUs, each costing a 30–50 ms connection interval. | Slow commissioners (Nest Hub) bail on the CSRRequest step before we finish responding. | `CONFIG_BT_NIMBLE_ATT_PREFERRED_MTU=517` + `CONFIG_BT_NIMBLE_LL_CFG_FEAT_LE_DATA_LENGTH_EXTENSION=y` in `sdkconfig.defaults`. | — |
| `esp_matter::factory_reset()` only clears CHIP's `chip-config`, `chip-counters`, KVS namespaces — our `foc_cal` / `leds` / `input` NVS stays. | After a factory reset, motor calibration uses a stale `sensor_direction` cache; rotor alignment misbehaves on certain starting positions. | `matter_wipe_local_nvs()` in `matter_app.cpp`, invoked before `esp_matter::factory_reset()` (button path) and on the `kFabricRemoved` event (remote decommission). | — |
| `CONFIG_ENABLE_CHIP_SHELL=y` (esp-matter default) starts a `console` task running `chip::Shell::Engine::Root().RunMainLoop()` → `linenoise()` → `usb_serial_jtag_read()`. With no USB host attached the read returns `-1` immediately on every call without yielding; the task pegs core 0 at high priority, IDLE never runs, the Task WDT panics at 5 s. | Lamp boots, motor / knob / LEDs interactive for ~5 s, then chip resets in a loop. ISR heartbeat keeps blinking until the panic, so the device looks "wedged." Reset reason `0x06` (`ESP_RST_TASK_WDT`) once `CONFIG_ESP_TASK_WDT_PANIC=y` is also set. | `# CONFIG_ENABLE_CHIP_SHELL is not set` + `CONFIG_ESP_TASK_WDT_PANIC=y` in `s3/m4-matter/sdkconfig.defaults*`. Diagnostic story in § 7.1. | — (CHIP shell is dev-only; safe to disable on USB-Serial-JTAG-only boards.) |

---

## 7.1 Resolved: external 5 V boot now works (2026-04-28)

> **Resolution:** `# CONFIG_ENABLE_CHIP_SHELL is not set` in `s3/m4-matter/sdkconfig.defaults`,
> plus `CONFIG_ESP_TASK_WDT_PANIC=y` in `s3/m4-matter/sdkconfig.defaults.esp32s3`.
> The chip wasn't wedged in an EN-pin partial-reset state — it was crash-rebooting
> from a starved core-0 IDLE task on every host-less boot.

### Symptom

Boot banner prints, motor / knob / LEDs come up for ~5 s thanks to the
deferred `matter_app_init()`, then the chip resets in a loop or appears to
freeze (depends on whether `CONFIG_ESP_TASK_WDT_PANIC` is set). Reset reason
is `0x06` (`ESP_RST_TASK_WDT`) once panic mode is on.

### Root cause

`CONFIG_ENABLE_CHIP_SHELL=y` (esp-matter Kconfig default) spawns a
`ChipShellTask` running `linenoise()` → `usb_serial_jtag_read()`. With no
USB host attached the read returns `-1` immediately without yielding; the
task pegs core 0 at high priority, IDLE never runs, the Task WDT panics at
5 s.

### Fix

In `s3/m4-matter/sdkconfig.defaults`:
- `# CONFIG_ENABLE_CHIP_SHELL is not set`
- `CONFIG_ESP_TASK_WDT_PANIC=y` (production safety; clean reboot if any task
  ever starves IDLE again).

### Status of the cap+diode hardware

The 470 µF + SS14 Schottky combo stays in the BOM as transient defense
against Mini360 droop, but it is **no longer the gating fix**. A 3.0 V
voltage supervisor IC (MCP809-class) on the RST pad is still explicitly not
pursued — unnecessary now.

### Reference

- [`project_chip_shell_external_power.md`](../../.claude/projects/-Users-rodolfo-Documents-Personal-Projects-BrushlessLamp/memory/project_chip_shell_external_power.md) — the actual root cause + fix.
- [`project_xiao_s3_external_power.md`](../../.claude/projects/-Users-rodolfo-Documents-Personal-Projects-BrushlessLamp/memory/project_xiao_s3_external_power.md) — preserved as historical context; failure-mode attribution superseded.
- [Seeed forum 283702](https://forum.seeedstudio.com/t/external-power-to-the-5v-pin-does-not-work-for-xiao-esp32-s3-and-xiao-esp32-c3/283702) — the EN-pin partial-reset hypothesis. Real on some boards / supplies; was the wrong root cause for ours.

---

## 8. Code architecture

### Dual-core task layout

```
Core 1 (CORE_MOTOR, dedicated)            Core 0 (CORE_OTHERS)
────────────────────────────             ────────────────────────────
motor_foc (prio 20, 200 µs)              main (IDF default)
  - busy-wait @ 5 kHz                    sys_evt / WiFi task (IDF)
  - loopFOC + move                       CHIP task (esp-matter)
  - stall detection                      input_task   (prio 3, 10 ms)
  - idle-disable                         leds_fade    (prio 2, 10 ms)
  - settle callback → Matter             leds_pulse   (prio 2, spawned)
  - encoder ISR (same core, attached       - BT controller, NimBLE host
    from inside this task)                 - lwIP, mDNS
                                           - LEDC writes (ledcWrite)
```

`motor_foc` deletes its core's IDLE watchdog subscription — on a dedicated
core the idle task would never run. The busy-wait is self-yielding via
`micros()` polling; core 0 is never blocked.

### Module map

| File | Role |
|------|------|
| `common/config.h`           | Every tunable: motor electrical, PID, position loop, LED fade curve, NVS key strings are in the stage-specific files. |
| `common/pins.h`             | XIAO S3 D-label → GPIO map with strap annotations. |
| `common/motor.h/.cpp`       | FOC task, position loop, manual-velocity mode, stall detection, NVS-cached `sensor_direction`. Used verbatim by every stage. |
| `m4-matter/main/input.cpp`  | Knob quadrature decoder (ISR), button state machine (click/double-click/long-press ladder), NVS-persisted speed preset. |
| `m4-matter/main/leds.cpp`   | 10 ms fader task, continuous angle → duty curve with gamma 2.2, pulse feedback task (symmetric N-blink around the current state), NVS-persisted `max_duty`. |
| `m4-matter/main/matter_app.cpp` | Matter node / endpoint creation, `attribute_update_cb` inbound dispatch, `matter_push_level_from_angle` outbound (with OnOff=false at level 0 → the Home app can actually show "Off"), `btInUse` weak-override. |
| `m4-matter/main/main.cpp`   | `app_main` — NVS init, arduino init, wire the settle callback, start everything in an order that keeps BLE init away from the busy-wait motor task. |

### Control flow

```
Home app          device                       motor           LEDs
   │                │                            │               │
   │ OnOff / Level  │                            │               │
   │   / ColorTemp  │                            │               │
   ├───────────────►│ attribute_update_cb        │               │
   │                │   echo? skip.              │               │
   │                │   else cache + apply_state │               │
   │                │       leds_set_colortemp───┼──────────────►│ fader picks up
   │                │       motor_set_target_angle               │
   │                │                            │───►FOC loop   │
   │                │                            │   slew        │
   │                │                            │   shaft ang ──┼─►peak_for_angle
   │                │                            │               │  → gamma → duty
   │                │                            │   idle_disable │
   │                │ settle_cb                  │◄──             │
   │                │◄───────────────────────────│                │
   │                │ matter_push_level_from_angle                │
   │   level + OnOff│                            │                │
   │◄───────────────┤                            │                │
   │                │                            │                │
knob turn           │                            │                │
   │───►input_task  │                            │                │
       │            │                            │                │
       ├── motor_nudge_target_angle─────────────►│                │
       └── motor_request_matter_sync_on_settle   │                │
                    │          (settle fires → push as above)     │
```

### NVS layout

| Namespace  | Key       | Written by                       | Purpose |
|------------|-----------|----------------------------------|---------|
| `foc_cal`  | `dir`     | `motor_foc_task`                 | cached `sensor_direction` (±1). Present after first successful `initFOC()`; skips the direction sweep on subsequent boots. |
| `leds`     | `maxduty` | `leds_fader_task` (debounced)    | user-selected brightness, persists across reboots. Floored at `LED_MAX_DUTY_MIN`. |
| `input`    | `spd_idx` | `input_task` double-click handler | selected speed preset index (0..3). |
| `chip-*`   | —         | CHIP stack                       | Matter fabric, counters, NOC. `esp_matter::factory_reset` wipes everything in this NVS partition. |

`esp_secure_cert` (separate partition) and `fctry` (separate NVS partition)
hold the per-device identity and are NOT touched by factory-reset.

---

## 9. Troubleshooting

**"GetSetupDiscriminator failed: 201" at boot** — the `fctry` partition at
`0x420000` either wasn't flashed or contains data from a different
VID/PID. Re-flash the `-partition.bin` from `mfg_out/`.

**`LoadProhibited` in `r_llm_env_init` during BLE init** — the `btInUse()`
override got dropped at link time. Verify `__attribute__((used))` is still
present on it in `matter_app.cpp` and that your build isn't forcing LTO in
a way that ignores it.

**"Using default EXAMPLE passcode 20202021" on every boot** — expected
behavior. CHIP can't regenerate the raw passcode from the Spake2+ verifier.
Scan the QR PNG from `mfg_out/out/<vid>_<pid>/<uuid>/` or enter the
`manualcode` column from `-onb_codes.csv`.

**Motor won't move, `[motor] cmd:N.NN` climbs but `vel:0`** — either the
shaft is physically blocked (stall detection should fire within 1.2 s and
snap target to shaft) or the calibration is stale. Factory-reset (9 s
button hold) wipes `foc_cal`; a fresh direction sweep runs on next boot.

**Commissioning hangs at "Configuring device"** — the Matter controller
needs 2.4 GHz WiFi reachable to the S3. Apple Home additionally needs a
Matter controller hub on the same network (HomePod mini 2022+, Apple TV
4K 2022+, or an iPad kept at home). Google Home needs a Nest Hub 2nd gen /
Nest WiFi Pro / Chromecast-with-Google-TV 4K as hub.

**Commissioning aborts with `CHIP_ERROR_TIMEOUT` (32) after CSRRequest** —
the Matter commissioner gave up waiting for the CSR response. Verify the
CPU is at 240 MHz (`cpu freq: 240000000 Hz` in the boot log) and that
`CONFIG_BT_NIMBLE_ATT_PREFERRED_MTU=517` + DLE are active in sdkconfig —
both are the gates on this step finishing inside the controller's budget.

**Commissioning works with `idf.py monitor` attached but fails without it** —
classic symptom of CPU running at 160 MHz (the IDF default). USB-CDC host
polling would happen to keep tasks scheduled fast enough to squeak inside
the commissioner's timeout; closing the monitor removes that luck. Bump
to 240 MHz per the workarounds table, which makes the outcome consistent.

**Motor is briefly louder on the boot after a factory reset** — `initFOC`
re-runs the full direction sweep when `foc_cal` is empty. The chip caches
`sensor_direction` to NVS on first calibration; subsequent boots skip the
sweep and stay quiet.

**Chip works on USB but reset-loops on external 5 V** — your sdkconfig has
`CONFIG_ENABLE_CHIP_SHELL=y`. Default is now `=n`; verify with
`grep CHIP_SHELL build/sdkconfig`. Background in § 7.1.

**`CMake Error: Failed to resolve component 'button' / 'led_driver'`** —
the esp-matter device_hal components need both in `EXTRA_COMPONENT_DIRS`
even if our code doesn't call them. Check `s3/m4-matter/CMakeLists.txt`.

---

## 10. Rollback

The `../esp-idf/` tree is the last known-good C6 build (Phase 14c commit
`40b9aa2`). `git checkout 40b9aa2 -- esp-idf/` reverts to it; the S3 tree
is left untouched.
