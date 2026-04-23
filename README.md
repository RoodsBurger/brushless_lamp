# Brushless Lamp

> **Current build targets the Seeed XIAO ESP32-S3 — see [`s3/README.md`](s3/README.md)** for hardware, wiring, toolchain, build / flash, per-device Matter provisioning, workarounds, code architecture and troubleshooting. The sections below describe the earlier C6 build preserved at `esp-idf/` as a rollback target.

Kinetic smart lamp — BLDC variant of [CylinderLamp](../CylinderLamp). Same concept (lit core raised/lowered by a motor to control brightness), but the stepper + TMC2209 is replaced by a gimbal BLDC motor driven through a SimpleFOC Mini, with a Seeed XIAO ESP32-C6 running the FOC commutation itself.

## Current milestones — M1 + M2 + M3a + M3b + M4

Closed-loop **angular-position control** on the motor via an MT6701 ABZ encoder. A rotary encoder knob with push-button drives the user interface:
- **Position mode (default)**: knob turns the motor — the lit core rises/falls mechanically. Commanded position at 0 electrically turns the LEDs **off**.
- **Brightness mode**: knob adjusts LED brightness 0–100 % on a logarithmic curve.
- **Single click**: toggle between position and brightness mode.
- **Double click**: cycle motor speed preset `{10, 15, 20, 25}` rad/s.
- **Long-hold 5 s**: lamp LEDs pulse 5× as factory-reset warning.
- **Long-hold 9 s**: factory reset (clears Preferences, reboots).

`commanded`, `speed_idx`, `brightness`, and `cct` persist in flash. Boot applies a sensor_offset so the motor doesn't jump on startup.

### Hardware

- **MCU:** Seeed Studio XIAO ESP32-C6 (RISC-V, single HP core, WiFi 6, BLE 5, 802.15.4 for later Matter-over-Thread)
- **Driver:** SimpleFOC Mini (DRV8313; 3-phase, `IN1`/`IN2`/`IN3`/`EN` + `nSLEEP`/`nFAULT` + `U`/`V`/`W` phases)
- **Motor:** 4015 BLDC gimbal, 12–36 V, 11 pole pairs, hollow-shaft external rotor
- **Motor encoder:** MT6701 (ABZ mode, 1024 CPR)
- **User input:** PEC11-style rotary encoder with integrated push-button (20 detents/rev)
- **LED strip:** dual-channel CCT (common anode + warm-white cathode + cool-white cathode), driven at 24 V
- **LED switching:** 2× AO3400A N-channel logic-level MOSFETs (SOT-23), one per channel
- **Power:** Mean Well LRS-75-24 (24 V 3.2 A)
- **Buck:** Mini360 24 V → 5 V for the MCU

### Wiring

PWM + nSLEEP on D7–D10 (driver cluster), motor encoder A/B on D0/D1, rotary knob + push-button on D2–D4, LED MOSFET gates on D5/D6. All 11 broken-out GPIOs are used. EN is **hard-tied to 3V3 on the SimpleFOC Mini itself** — its pull-up is too weak to survive a floating MCU pin, and a lamp doesn't need software tri-state (`motor.disable()` still drives all three phases to 50 % duty, zero phase-to-phase differential, zero torque). nFAULT and Z are intentionally unconnected: stall detection covers faults, and Z only helps for single-turn absolute positioning which this project doesn't need.

| XIAO pin | Chip GPIO | Target | Notes |
|---|---|---|---|
| D10 | GPIO 18 | SimpleFOC Mini IN1 | PWM phase A (MCPWM) |
| D9  | GPIO 20 | SimpleFOC Mini IN2 | PWM phase B (MCPWM) |
| D8  | GPIO 19 | SimpleFOC Mini IN3 | PWM phase C (MCPWM) |
| D7  | GPIO 17 | SimpleFOC Mini nSLEEP | pulse LOW ≥ 30 µs to clear latched OCP/TSD |
| D6  | GPIO 16 | LED CW MOSFET gate | LEDC PWM 25 kHz, 10-bit |
| D5  | GPIO 23 | LED WW MOSFET gate | LEDC PWM 25 kHz, 10-bit |
| D0  | GPIO 0  | MT6701 A | quadrature, internal pull-up |
| D1  | GPIO 1  | MT6701 B | quadrature, internal pull-up |
| D2  | GPIO 2  | Rotary encoder A | PCNT ch0, hardware glitch-filtered |
| D3  | GPIO 21 | Rotary encoder B | PCNT ch1, hardware glitch-filtered |
| D4  | GPIO 22 | Rotary push button | INPUT_PULLUP, polled |
| 3V3 | — | MT6701 V+ · SimpleFOC Mini 3V3 · **EN jumper** · rotary encoder V+ | sourced from the XIAO's onboard LDO |
| GND | — | GND | common with Mean Well V−, Mini360 OUT−, and both LED MOSFET sources |
| 5V  | — | Mini360 OUT+ | XIAO power; onboard LDO then drives the 3V3 rail |
| — | — | Mean Well 24 V → Mini VCC / VM · LED strip VCC | motor rail + LED rail (shared 24 V) |

**Install a jumper from 3V3 to the EN header on the SimpleFOC Mini.** This is what keeps the DRV8313 enabled; without it the driver floats disabled.

#### Peripheral wiring (complete)

**Mean Well LRS-75-24 (24 V 3.2 A, mains-powered):**
- `V+` → SimpleFOC Mini `VM`, Mini360 `IN+`
- `V−` → SimpleFOC Mini `GND`, Mini360 `IN−`, XIAO `GND` (common)
- `L` / `N` / `⏚` → mains (respect mains safety: fused, enclosed, earth the chassis)

**Mini360 buck (24 → 5 V, trim set before connecting the XIAO):**
- `IN+` → Mean Well `V+`
- `IN−` → Mean Well `V−`
- `OUT+` → XIAO `5V`
- `OUT−` → XIAO `GND`

**SimpleFOC Mini (DRV8313 breakout):**
- `VM` → Mean Well `V+`
- `GND` → common ground
- `3V3` → XIAO `3V3` (logic rail + pull-ups)
- `IN1` → XIAO `D10`
- `IN2` → XIAO `D9`
- `IN3` → XIAO `D8`
- `EN` → **jumper to `3V3`** (on the breakout itself, not to the MCU)
- `nSLEEP` → XIAO `D7`
- `nFAULT` → not connected
- `U` / `V` / `W` (screw terminals) → motor phase wires. Initial phase order is a guess — if `initFOC` fails or the motor stutters, swap any two.

**MT6701 breakout (ABZ mode — verify the mode jumper matches this; the board ships in various configurations):**
- `V+` → XIAO `3V3`
- `GND` → common ground
- `A` pad (often labelled `DO` on the breakout's dual-purpose silk) → XIAO `D0`
- `B` pad (often labelled `CLK`) → XIAO `D1`
- `Z` pad (often labelled `CS`) → not connected

**Rotary encoder (PEC11-style, 5 pins — 3 on one side for the encoder, 2 on the other for the integrated push-button):**
- Encoder pin `A` → XIAO `D2`
- Encoder pin `B` → XIAO `D3`
- Encoder common (middle of the three) → `GND`
- Button pin 1 → XIAO `D4`
- Button pin 2 → `GND`
- (No external pull-ups needed — PCNT has a hardware glitch filter for A/B, and the button uses the MCU's `INPUT_PULLUP`.)

**LED MOSFET pair (AO3400A ×2, SOT-23, low-side switching, gate driven direct from 3V3 logic):**
- WW MOSFET: gate → XIAO `D5`, drain → LED strip `W` (warm-white cathode), source → `GND`
- CW MOSFET: gate → XIAO `D6`, drain → LED strip `C` (cool-white cathode), source → `GND`
- No gate resistor, pull-down, or flyback diode — same bare circuit as CylinderLamp. AO3400A is logic-level (Vgs(th) ≈ 0.9 V) and 3.3 V on the gate fully saturates it.
- Keep both MOSFET ground wires short and on the common GND rail; the 24 V LED return current flows through these sources.

**LED strip (dual-channel CCT, shared anode, 24 V):**
- `VCC` → Mean Well `V+` (24 V rail — tap the same distribution as the SimpleFOC Mini's `VM`)
- `W` (warm-white cathode) → WW MOSFET drain
- `C` (cool-white cathode) → CW MOSFET drain

### Build

```bash
arduino-cli compile --fqbn "esp32:esp32:XIAO_ESP32C6:CDCOnBoot=cdc,PartitionScheme=huge_app" arduino/BrushlessLamp/BrushlessLamp.ino
arduino-cli upload  --fqbn "esp32:esp32:XIAO_ESP32C6:CDCOnBoot=cdc,PartitionScheme=huge_app,EraseFlash=all" --port /dev/cu.usbmodem101 arduino/BrushlessLamp/BrushlessLamp.ino
```

`EraseFlash=all` is only required on the **first** flash after leftover partition state from another project (e.g. CylinderLamp). Subsequent uploads can drop it.

Requires arduino-esp32 core ≥ 3.0 (tested on 3.3.7) and the [SimpleFOC](https://github.com/simplefoc/Arduino-FOC) library (Library Manager → "Simple FOC", v2.4.x). On the C6, SimpleFOC selects the **MCPWM** backend automatically — you should see `SimpleFOC: compiling for ESP32 MCPWM driver` in the build output.

### Serial monitor

```bash
arduino-cli monitor --port /dev/cu.usbmodem101 --config baudrate=115200 --config dtr=off --config rts=off
```

**Don't omit `dtr=off` and `rts=off`** — on the XIAO ESP32-C6's USB-Serial/JTAG (same quirk as the C3 SuperMini), the default `dtr=on,rts=on` holds the chip in reset, so the sketch never runs while the monitor is attached. Symptom: connects cleanly, no banner ever appears.

### User gestures

The knob has two modes; single click toggles between them. The **measured shaft angle** acts as a position-proportional lamp gate: below 0.5 rad the LEDs are fully off, above 3 rad they run at the configured `brightness`, and in between they ramp linearly. Motion speed = fade speed — since the motor's trapezoidal profile makes shaft_angle move smoothly, the lamp fades in and out at whatever pace the motor is travelling.

| Gesture | Position mode (default) | Brightness mode |
|---|---|---|
| Turn knob CW | `commanded` += `RAD_PER_CLICK`, motor seeks | `brightness` +1 % |
| Turn knob CCW | `commanded` −= `RAD_PER_CLICK`, clamped at 0; the LEDs dim as the motor passes back through the 3 → 0.5 rad fade zone and go off below 0.5 rad | `brightness` −1 %, clamped at 0 |
| Single click | Switch to brightness mode | Switch to position mode |
| Double click | Advance motor speed preset (cycles `{10, 15, 20, 25}` rad/s) | Same |
| Hold ≥ 5 s | LEDs pulse 5× as factory-reset warning (~2.4 s) | Same |
| Hold ≥ 9 s | Factory reset — clears `Preferences`, reboots | Same |

`commanded`, `speed_idx`, `brightness`, and `cct` all persist via ESP32 `Preferences`. Knob mode itself is not persisted — every boot starts in position mode. On boot the firmware applies a `sensor_offset` so `shaft_angle` reads as the persisted `commanded`, the motor stays put, and LEDs illuminate at the saved brightness/CCT if `commanded > 0.1 rad`.

### Matter (M4)

The lamp is a **Matter ColorTemperatureLight** — one endpoint exposing OnOff, LevelControl, and ColorControl clusters. Controllers (Google Home, Apple Home, Alexa, Home Assistant) see it as a smart light with on/off, a brightness slider, and a color-temperature slider.

**Mapping to the physical lamp:**
- **On/Off** → `commanded` snaps to `POS_MIN` when off; restores the last Matter-level position when on.
- **Brightness (1..254)** → motor **position** (`commanded = brightness × POS_MAX / 254`). The LED gate then fades the lamp in as the motor passes through its 0.5 → 3 rad zone, same as the knob-driven path.
- **Color temperature (100–500 mireds)** → LED CCT mix (100 mireds = cool = `cct=100` = all CW; 500 mireds = warm = `cct=0` = all WW).

**LED brightness is deliberately knob-only** and not exposed to Matter. The Matter brightness slider is the motor slider; the lamp's own dimmer stays local on the knob.

**Commissioning:**
- On first boot the serial log prints `=== Matter: uncommissioned ===` followed by a QR-code URL and an 11-digit manual pairing code. Open Google Home → Add device → Matter-enabled device → scan the QR or type the manual code.
- **The motor + FOC ISR stays cold until the device is commissioned.** This keeps the 500 Hz SVPWM ISR from competing with the SPAKE2+ exchange on the single-core C6, which was the dominant cause of mid-commissioning panics. Once `Matter.isDeviceCommissioned()` flips true, the motor brings itself up automatically ~3 s later (after the BLE/WiFi handover settles). Already-paired devices skip the wait and init the motor immediately at boot.
- **Bench-test escape**: short-click the knob while uncommissioned to bring the motor up before pairing — useful for verifying the mechanical setup without a controller.
- BLE is **kept resident** post-commissioning. Earlier builds released it 3 s after pairing (saved ~60 KB heap), but the arduino-esp32 Matter library's WiFi-reconnect path on C6 dereferences the released memory and panics. Net cost: ~60 KB permanent heap, which the C6 can afford.
- The long-hold factory reset (9 s button press) both wipes `Preferences` and calls `Matter.decommission()` — the device re-appears as uncommissioned on next boot.

**Transport:** Matter-over-WiFi (which the arduino-esp32 Matter library picks for the XIAO C6 build by default). The C6 has the 802.15.4 radio for Matter-over-Thread too, but the arduino-esp32 Matter stack currently ships WiFi-configured for C6 — migrating to Thread is a follow-up (requires a custom build config; defer until needed).

**Known limitations:**
- **All flashed boards share the same QR code** — the arduino-esp32 Matter library doesn't expose a runtime API for per-device discriminator/passcode, so without patching the library or moving to full ESP-IDF every board presents identical commissioning credentials. Fine for a single-lamp household; would need addressing before any fleet deployment.
- **Consequence of the shared credentials**: if you already have a CylinderLamp (or another device built with the same arduino-esp32 Matter library defaults) commissioned in your Matter fabric, **decommission it before adding a new BrushlessLamp** — the commissioner will time out at the network step because it thinks the incoming device is a duplicate of the existing node. Symptom: commissioning gets as far as the BLE handshake, then fails with `Failsafe timer expired` and a `Cluster 0x0031 not found` line in the device's serial log.
- Devices appear as "Unverified" in the Home app because we don't carry a CSA-signed DAC. Harmless for personal use.

**FOC runs on a hardware timer ISR now** (500 Hz `hw_timer_t`). `motor.loopFOC()` no longer runs in `loop()` — the timer keeps SVPWM locked to the rotor regardless of Matter / Thread / BLE scheduler activity. `motor.move()` still runs in `loop()` and is tolerant of occasional ~4 ms scheduler preemption spikes (visible as `maxLp` jumps in the status line). 500 Hz still gives ~11 SVPWM updates per electrical cycle at the 25 rad/s top preset (11 pp × 25 rad/s ≈ 44 Hz electrical), and halving the ISR rate vs the prior 2 kHz / 1 kHz attempts leaves more headroom for the CHIP/WiFi tasks on the single-core C6.

**Outbound state syncs back to Matter at 10 Hz** (down from the original 200 ms / 5 Hz limit) and is wrapped in `chip::DeviceLayer::PlatformMgr().LockChipStack()` so we don't race the CHIP event-loop task's own attribute writes during inbound commands. A `bool btInUse(void) { return true; }` weak override at the top of the sketch keeps the arduino-esp32 core from auto-tearing the BT controller during the BLE → WiFi handover.

### Serial commands

- `G<float>` — goto absolute position in rad (clamped to `[POS_MIN, POS_MAX]`).
- `T<float>` — set `motor.velocity_limit` in rad/s (overrides the current speed preset until next double-click or reboot).
- `C<float>` — current limit, A (voltage-mode clamp; no shunt on DRV8313).
- `P<float>` — `motor.PID_velocity.P` gain.
- `I<float>` — `motor.PID_velocity.I` gain.
- `F<float>` — `motor.LPF_velocity.Tf` for velocity filtering, s.
- `R<float>` — phase resistance, Ω.
- `K<float>` — KV rating, rpm/V.
- `M<int>`   — motion downsample (PID runs every N `loopFOC` calls).
- `A<float>` — trapezoidal accel cap, rad/s² (how quickly `target` can change velocity during a seek).
- `B<int>`   — LED brightness 0..100 (applied through a gamma LUT, persisted).
- `W<int>`   — CCT mix 0..100 (0 = all warm-white, 100 = all cool-white; persisted, not touched by the knob — intended for Matter later).
- `X`        — reset driver (pulses nSLEEP to clear latched fault).

Periodic status line (1 Hz) reports `cmd` (user-commanded position), `tgt` (trapezoidal-profile position fed to the motor), `ang` (measured shaft angle), `vel` (measured velocity), `tvel` (profile velocity, internal), `vlim` (current velocity_limit preset), `spd` (preset index), `bri` (LED brightness 0–100), `cct` (LED CCT mix 0–100), `mode` (`P` = position, `B` = brightness), `en` (driver enabled), and `maxLp` (worst loop iteration in µs — steady state should be a few hundred µs).

### First-run checklist

1. Flash firmware with motor and LED strip disconnected. Confirm banner: `=== BrushlessLamp: closed-loop FOC position ===` followed by `loaded: commanded=X.XX rad …` and (on a fresh first flash) `uncommissioned — deferring motor/FOC init until pairing or short-click`. **No `driver.init()=1` / `motor.initFOC()=1` lines yet** — those appear only after commissioning succeeds or after a short-click.
2. Wire driver + motor + MT6701 encoder + rotary encoder + push button + LED MOSFETs + LED strip per the tables above with Mean Well **off**. Verify common ground (Mean Well V−, Mini360 OUT−, XIAO GND, both MOSFET sources).
3. Power on. The motor stays cold and silent until you commission via Google Home **or** short-click the knob to bench-test. Once either happens, `driver.init()=1`, `motor.init()=1`, `motor.initFOC()=1` print, the rotor aligns (visible shaft nudge), then **stays put** — the `sensor_offset` anchor makes the firmware treat the current physical position as the persisted `commanded`. LEDs start at the saved `brightness`/`cct` **only if** `commanded > 0.1 rad`; on first boot `commanded=0` so the lamp is dark. After 300 ms with no motion, driver auto-disables (`en:0`).
4. Turn the knob one detent CW — status line's `cmd` and `tgt` should increase by `RAD_PER_CLICK`, motor seeks, and as `ang` (the measured shaft angle) crosses 0.5 rad the LEDs start fading in, reaching the saved brightness around 3 rad. Keep turning CCW so `ang` drops back below 0.5 rad to turn the lamp fully off.
5. Single-click — `mode` flips from `P` to `B`. Knob now adjusts `bri` 1 % per detent. Single-click again to return to position mode.
6. Double-click — `spd` advances and `vlim` shows the new limit. Works in either mode.
7. Hold the button for 5 s — LEDs fade-pulse 5× (~2.4 s). Release before 9 s — no reset. Hold past 9 s — chip reboots and `Preferences` clear back to defaults (`commanded=0`, `spd=1`, `bri=50`, `cct=50`).
8. Bench commands: `G10` seeks to 10 rad (LEDs turn on). `B20` dims to ~20 %. `W80` shifts CCT cool. `T3` slows subsequent seeks to 3 rad/s. `A5` softens the accel ramp. `X` clears a latched DRV8313 fault after OCP. `maxLp` should stay under ~500 µs throughout.

## Roadmap

- **M1 ✓** Motor spins under serial control (MCPWM, 3-phase voltage torque).
- **M2 ✓** MT6701 encoder, closed-loop velocity, accel-limited velocity ramp, DRV8313 fault/stall recovery.
- **M3a ✓** Rotary encoder + push button user input. Position mode, PCNT-decoded knob, click/double-click/long-hold button state machine, speed-preset cycling, `Preferences` persistence of target + speed.
- **M3b ✓** CCT LED strip — WW/CW MOSFETs on D5/D6 driven via LEDC, log-curve brightness, position-gated lamp on/off, single-click brightness-mode toggle, long-hold warning fade-pulses on the lamp LEDs. CCT mix is set via serial (`W`) now and via Matter later.
- **M4 ✓** Matter integration over WiFi. ColorTemperatureLight endpoint — Matter brightness slider drives motor position, Matter color temperature drives LED CCT mix. LED brightness stays knob-only. FOC commutation moved to a 2 kHz hw-timer ISR so the FreeRTOS scheduler is free to run Matter/WiFi/BLE tasks without starving the motor.
- **M5 (deferred):** per-device unique QR codes (needs library patch or ESP-IDF migration), Matter-over-Thread transport, OTA updates over Matter.

## Known issues / references

- **Periodic motor "click" every ~2 s on single-core ESP32 variants (S2/C3/C6).** arduino-esp32 runs `loop()` inside a FreeRTOS task that gets preempted by an unidentified background task for ~4 ms. At motor speeds where the electrical period approaches the stall duration (e.g. 50 rad/s × 11 pp ≈ 11 ms electrical), SVPWM is held frozen at the wrong rotor angle for ~37 % of an electrical cycle, producing an audible torque step on resume. Worked around by never returning from `loop()`. The proper fix is a hardware timer ISR — see Roadmap M4.
  - [simplefoc/Arduino-FOC#292 — "Motor skips every 2 sec on ESP32, even in open loop"](https://github.com/simplefoc/Arduino-FOC/issues/292)
  - [SimpleFOC community — Periodic jerk in velocity control mode](https://community.simplefoc.com/t/periodic-jerk-in-velocity-control-mode/7641)
  - [SimpleFOC community — ESP32-C6 strange spikes in loop execution time](https://community.simplefoc.com/t/esp32-c6-strange-spikes-in-loop-execution-time/8003)
  - [SimpleFOC community — Clicking noise with ESP32 and Lib Version 2.3.4](https://community.simplefoc.com/t/clicking-noise-with-esp32-and-lib-version-2-3-4/5939)
  - [SimpleFOC docs — Hard real-time FOC loop using timers](https://docs.simplefoc.com/real_time_loop)
- **No FPU on C3/C6.** Every `%.2f` in a `printf` goes through soft-float and costs ~300 µs per arg. A 9-float status line was responsible for a ~4 ms hot-path stall before the loop was reworked; the sketch uses fixed-point integer formatting (`FX_HI` / `FX_LO`) for anything printed inside the FOC loop.
- **DRV8313 latched faults are cleared by `nSLEEP`, not by `EN`.** SimpleFOC Mini's `EN` header only tri-states the H-bridge outputs. To recover from OCP/TSD, pulse `nSLEEP` low for ≥ 30 µs (the sketch uses 50 µs) and wait ≥ 1 ms before re-enabling. `PIN_NSP` (D7 / GPIO 17) handles this in `resetDriver()`.
- **XIAO ESP32-C6 USB-CDC reset on monitor attach.** `arduino-cli monitor` defaults to `dtr=on,rts=on`, which holds the chip in reset and prevents the sketch from running. Always pass `--config dtr=off --config rts=off`.
- **arduino-esp32 Matter on C6 crashes on WiFi reconnect.** The library's WiFi-reconnect error path dereferences a null struct (`Load access fault at MTVAL 0x000000a8` inside `chip[DL]` after `esp_wifi_connect() failed: ESP_ERR_WIFI_CONN`). Upstream bug, not something we can fix locally.
  - [connectedhomeip#26052](https://github.com/project-chip/connectedhomeip/issues/26052) — same `0x0500300F Failed to get configured network` error.
  - [esp-matter#672](https://github.com/espressif/esp-matter/issues/672) — "Commissioning unreliable on ESP32-C6 (via WiFi)".
  - [esp-matter#1682](https://github.com/espressif/esp-matter/issues/1682) — Factory Reset operation failure when WiFi unavailable at boot.
  - **Mitigations already in-firmware:** the FOC ISR + motor init are now deferred until ~3 s after `Matter.isDeviceCommissioned()` flips true, so the SVPWM timer doesn't compete with SPAKE2+ during the BLE→WiFi handover. A `bool btInUse(void) { return true; }` weak symbol prevents the arduino-esp32 core from auto-tearing the BT controller mid-pairing. BLE is never released post-commissioning (release path triggers a use-after-free on C6). Outbound state writes hold `chip::DeviceLayer::PlatformMgr().LockChipStack()` so we don't race the CHIP event-loop task. A consecutive-panic-reboot counter in NVS triggers a full NVS wipe after three crashes in a row (you'll see the QR-code print again on serial and the device accepts re-commissioning); the "settled" preference distinguishes one-off glitches (cost = 1) from repeat crashes during init (cost = 2). The FreeRTOS task watchdog (10 s) reboots cleanly if the main loop hangs. Outbound sync is gated on `Matter.isWiFiConnected()` so we don't spam the library while its radio is flailing.
  - **User-visible symptom of auto-recovery**: LEDs flicker off, motor re-aligns, the device reappears in Google Home with a fresh QR code. Re-commission once and it carries on.
