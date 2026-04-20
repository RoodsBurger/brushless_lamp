# Brushless Lamp

Kinetic smart lamp — BLDC variant of [CylinderLamp](../CylinderLamp). Same concept (lit core raised/lowered by a motor to control brightness), but the stepper + TMC2209 is replaced by a gimbal BLDC motor driven through a SimpleFOC Mini, with a Seeed XIAO ESP32-C6 running the FOC commutation itself.

## Current milestones — M1 (motor) + M2 (closed-loop) + M3a (rotary input)

Firmware runs closed-loop **angular-position control** on the motor via an MT6701 ABZ encoder. A rotary encoder knob with push-button drives the user-facing control surface (position via knob, speed preset via double-click, factory reset via long-hold). Last commanded position and speed preset are persisted in flash. LEDs + brightness control land in M3b.

### Hardware

- **MCU:** Seeed Studio XIAO ESP32-C6 (RISC-V, single HP core, WiFi 6, BLE 5, 802.15.4 for later Matter-over-Thread)
- **Driver:** SimpleFOC Mini (DRV8313; 3-phase, `IN1`/`IN2`/`IN3`/`EN` + `nSLEEP`/`nFAULT` + `U`/`V`/`W` phases)
- **Motor:** 4015 BLDC gimbal, 12–36 V, 11 pole pairs, hollow-shaft external rotor
- **Motor encoder:** MT6701 (ABZ mode, 1024 CPR)
- **User input:** PEC11-style rotary encoder with integrated push-button (20 detents/rev)
- **Power:** Mean Well LRS-75-24 (24 V 3.2 A)
- **Buck:** Mini360 24 V → 5 V for the MCU

### Wiring

PWM + nSLEEP on D7–D10 (driver cluster), motor encoder A/B on D0/D1, rotary knob + push-button on D2–D4. EN is **hard-tied to 3V3 on the SimpleFOC Mini itself** — its pull-up is too weak to survive a floating MCU pin, and a lamp doesn't need software tri-state (`motor.disable()` still drives all three phases to 50 % duty, zero phase-to-phase differential, zero torque). nFAULT and Z are intentionally unconnected: stall detection covers faults, and Z only helps for single-turn absolute positioning which this project doesn't need. D5/D6 stay free for M3b's WW/CW LED PWMs.

| XIAO pin | Chip GPIO | Target | Notes |
|---|---|---|---|
| D10 | GPIO 18 | SimpleFOC Mini IN1 | PWM phase A (MCPWM) |
| D9  | GPIO 20 | SimpleFOC Mini IN2 | PWM phase B (MCPWM) |
| D8  | GPIO 19 | SimpleFOC Mini IN3 | PWM phase C (MCPWM) |
| D7  | GPIO 17 | SimpleFOC Mini nSLEEP | pulse LOW ≥ 30 µs to clear latched OCP/TSD |
| D0  | GPIO 0  | MT6701 A | quadrature, internal pull-up |
| D1  | GPIO 1  | MT6701 B | quadrature, internal pull-up |
| D2  | GPIO 2  | Rotary encoder A | PCNT ch0, hardware glitch-filtered |
| D3  | GPIO 21 | Rotary encoder B | PCNT ch1, hardware glitch-filtered |
| D4  | GPIO 22 | Rotary push button | INPUT_PULLUP, polled |
| (onboard) | GPIO 15 | XIAO user LED | temporary factory-reset warning blink (moves to lamp LEDs in M3b) |
| 3V3 | — | MT6701 V+ · SimpleFOC Mini 3V3 · **EN jumper** · rotary encoder V+ | sourced from the XIAO's onboard LDO |
| GND | — | GND | common with Mean Well V− and Mini360 OUT− |
| 5V  | — | Mini360 OUT+ | XIAO power; onboard LDO then drives the 3V3 rail |
| — | — | Mean Well 24 V → Mini VCC / VM | motor rail |

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

| Gesture | Effect |
|---|---|
| Turn knob CW | `target` position increases by `RAD_PER_CLICK` per detent, motor seeks to it |
| Turn knob CCW | `target` decreases, clamped at `POS_MIN` |
| Single click | Stub — prints a reminder that brightness mode lands in M3b |
| Double click | Advances speed preset (cycles `{10, 15, 20, 25}` rad/s), applies to `motor.velocity_limit`, persists |
| Hold ≥ 5 s | Onboard LED blinks at 5 Hz as factory-reset warning |
| Hold ≥ 9 s | Factory reset — clears `Preferences`, fast-blinks 5×, reboots |

Last `commanded` position and speed-preset index survive reboot via ESP32 `Preferences`. On boot the firmware applies a `sensor_offset` so `shaft_angle` reads as the persisted `commanded` — the motor **does not** seek after `initFOC`; it stays wherever it physically is and treats that as the saved position. The next knob turn or `G` command is what actually moves the motor.

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
- `X`        — reset driver (pulses nSLEEP to clear latched fault).

Periodic status line (1 Hz) reports `cmd` (user-commanded position), `tgt` (trapezoidal-profile position fed to the motor), `ang` (measured shaft angle), `vel` (measured velocity), `tvel` (profile velocity, internal), `vlim` (current velocity_limit preset), `spd` (preset index), `en` (driver enabled), and `maxLp` (worst loop iteration in µs over the last second — steady state should be a few hundred µs).

### First-run checklist

1. Flash firmware with motor disconnected. Confirm banner: `=== BrushlessLamp: closed-loop FOC position ===` followed by `driver.init()=1`, `motor.init()=1`, `motor.initFOC()=1`, then a `loaded: target=X.XX speed_idx=Y (Z rad/s)` line from `Preferences`.
2. Wire driver + motor + MT6701 encoder + rotary encoder + push button per the table above with Mean Well **off**. Verify common ground.
3. Power on. Motor aligns during `initFOC()` (visible shaft nudge), then **stays put** — the `sensor_offset` anchor makes the firmware treat the current physical position as the persisted `commanded`. After 300 ms with no motion, driver auto-disables (`en:0`). Any whine during `initFOC` → lower `C` with `C0.3`.
4. Turn the knob one detent CW — status line's `cmd` and `tgt` should increase by `RAD_PER_CLICK` and the motor should seek to it. CCW decreases. At the soft limits (`POS_MIN` / `POS_MAX` — currently `0` / `1000` rad as a testing placeholder) the target stops accumulating.
5. Double-click the push button — `spd` field advances and `vlim` shows the new limit. The next knob turn seeks at that speed.
6. Hold the button for 5 s — XIAO onboard LED starts blinking. Release before 9 s — blinking stops, no reset. Hold past 9 s — fast confirmation blink, chip reboots, `Preferences` clear back to defaults (`target=0`, `spd=1`).
7. Bench commands: `G10` seeks to 10 rad. `T3` slows subsequent seeks to 3 rad/s. `A5` softens the accel ramp. `X` clears a latched DRV8313 fault after OCP. `maxLp` should stay under ~500 µs throughout, including the idle→motion wake (non-blocking `resetDriver` state machine).

## Roadmap

- **M1 ✓** Motor spins under serial control (MCPWM, 3-phase voltage torque).
- **M2 ✓** MT6701 encoder, closed-loop velocity, accel-limited velocity ramp, DRV8313 fault/stall recovery.
- **M3a ✓** Rotary encoder + push button user input. Position mode, PCNT-decoded knob, click/double-click/long-hold button state machine, speed-preset cycling, `Preferences` persistence of target + speed.
- **M3b:** CCT LED strip — WW/CW MOSFET PWMs on D5/D6, single-click brightness-mode toggle, long-hold warning moves from onboard LED to the lamp LEDs.
- **M4:** Matter integration (port from CylinderLamp). The C6 supports **802.15.4**, so Matter-over-Thread is on the table — likely more robust than Matter-over-WiFi given the motor switching noise and tighter latency budget.
  - **Refactor required first:** the current sketch spins forever inside `loop()` (`while (true) { ... }`) to dodge the arduino-esp32 single-core scheduler quirk that preempts `loop()` for ~4 ms every ~2 s — see "Known issues" below. That works fine with no networking, but Matter/WiFi/Thread/BLE all run on FreeRTOS tasks at priorities 5–23 and need the scheduler to actually schedule them. Before pulling Matter in, move `motor.loopFOC()` onto a hardware timer (`hw_timer_t` + `IRAM_ATTR` ISR at 5–10 kHz) so commutation is deterministic regardless of what else runs, and let `loop()` return normally.

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
