# Brushless Lamp

Kinetic smart lamp — BLDC variant of [CylinderLamp](../CylinderLamp). Same concept (lit core raised/lowered by a motor to control brightness), but the stepper + TMC2209 is replaced by a gimbal BLDC motor driven through a SimpleFOC Mini, with a Seeed XIAO ESP32-C6 running the FOC commutation itself.

## Current milestones — M1 (motor) + M2 (closed-loop)

Firmware energises the motor and runs closed-loop velocity control via an MT6701 ABZ encoder. USB serial exposes a bench-tuning interface for the PID, current limit, and velocity ramp.

### Hardware

- **MCU:** Seeed Studio XIAO ESP32-C6 (RISC-V, single HP core, WiFi 6, BLE 5, 802.15.4 for later Matter-over-Thread)
- **Driver:** SimpleFOC Mini (DRV8313; 3-phase, `IN1`/`IN2`/`IN3`/`EN` + `nSLEEP`/`nFAULT` + `U`/`V`/`W` phases)
- **Motor:** 4015 BLDC gimbal, 12–36 V, 11 pole pairs, hollow-shaft external rotor
- **Encoder:** MT6701 (ABZ mode, 1024 CPR)
- **Power:** Mean Well LRS-75-24 (24 V 3.2 A)
- **Buck:** Mini360 24 V → 5 V for the MCU

### Wiring

PWM + nSLEEP occupy one side of the XIAO (D7–D10); encoder A/B on D0/D1. EN is **hard-tied to 3V3 on the SimpleFOC Mini itself** — its pull-up is too weak to survive a floating MCU pin, and a lamp doesn't need software tri-state (`motor.disable()` still drives all three phases to 50 % duty, zero phase-to-phase differential, zero torque). nFAULT and Z are intentionally unconnected: stall detection covers faults, and Z only helps for single-turn absolute positioning which this project doesn't need. D2/D3/D4/D5/D6 stay free for M3.

| XIAO pin | Chip GPIO | Target | Notes |
|---|---|---|---|
| D10 | GPIO 18 | SimpleFOC Mini IN1 | PWM phase A (MCPWM) |
| D9  | GPIO 20 | SimpleFOC Mini IN2 | PWM phase B (MCPWM) |
| D8  | GPIO 19 | SimpleFOC Mini IN3 | PWM phase C (MCPWM) |
| D7  | GPIO 17 | SimpleFOC Mini nSLEEP | pulse LOW ≥ 30 µs to clear latched OCP/TSD |
| D0  | GPIO 0  | MT6701 A | quadrature, internal pull-up |
| D1  | GPIO 1  | MT6701 B | quadrature, internal pull-up |
| 3V3 | — | MT6701 V+ · SimpleFOC Mini 3V3 · **EN jumper** | sourced from the XIAO's onboard LDO |
| GND | — | GND | common with Mean Well V− and Mini360 OUT− |
| 5V  | — | Mini360 OUT+ | XIAO power; onboard LDO then drives the 3V3 rail |
| — | — | Mean Well 24 V → Mini VCC / VM | motor rail |

**Install a jumper from 3V3 to the EN header on the SimpleFOC Mini.** This is what keeps the DRV8313 enabled; without it the driver floats disabled.

Motor phase wires go to the driver's `U`/`V`/`W` screw terminals. Initial phase order is a guess — if `initFOC` fails or the motor stutters, swap any two.

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

### Serial commands

- `T<float>` — target velocity, rad/s. `T2` forward, `T0` stop, `T-2` reverse.
- `C<float>` — current limit, A (enforced as a voltage-mode clamp; no shunt sensing on DRV8313).
- `P<float>` — velocity PID P gain.
- `I<float>` — velocity PID I gain.
- `F<float>` — LPF Tf for velocity filtering, s.
- `R<float>` — phase resistance, Ω.
- `K<float>` — KV rating, rpm/V.
- `M<int>`   — motion downsample (PID runs every N loopFOC calls).
- `A<float>` — velocity acceleration cap, rad/s².
- `X`        — reset driver (pulses nSLEEP to clear latched fault).

Periodic status line (1 Hz) reports `tgt`, `ramp`, `vel`, `ang`, `en`, and `maxLp` (worst loop iteration in µs over the last second — steady-state should be a few hundred µs).

### First-run checklist

1. Flash firmware with motor disconnected. Confirm banner: `=== BrushlessLamp: closed-loop FOC ===` followed by `driver.init()=1`, `motor.init()=1`, `motor.initFOC()=1`.
2. Wire driver + motor + encoder per the table above with Mean Well **off**. Verify common ground.
3. Power on. Motor aligns during `initFOC()` (visible shaft nudge), then auto-disables after 300 ms of no command. Steady-state idle status line should read `tgt:0.00 ... en:0`. Any whine during `initFOC` → lower `C` with `C0.3`.
4. Send `T5`. Shaft should rotate smoothly. If it stutters or `initFOC` failed, swap two phase wires at the driver.
5. Push target gradually: `T20`, `T50`. Watch `maxLp` — steady state should stay under ~500 µs. OCP trip is expected at high speed/torque and clears via `X`.

## Roadmap

- **M1 ✓** Motor spins under serial control (MCPWM, 3-phase voltage torque).
- **M2 ✓** MT6701 encoder, closed-loop velocity, accel-limited velocity ramp, DRV8313 fault/stall recovery.
- **M3:** CCT LED strip (WW/CW MOSFETs) + rotary encoder user input.
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
