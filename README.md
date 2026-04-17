# Brushless Lamp

Kinetic smart lamp — BLDC variant of [CylinderLamp](../CylinderLamp). Same concept (lit core raised/lowered by a motor to control brightness), but the stepper + TMC2209 is replaced by a gimbal BLDC motor driven through a SimpleFOC Mini, with the ESP32-C3 running the FOC commutation itself.

## Current Milestone — M1: Motor Spin Only

Minimal firmware that energises the motor and lets you drive it from USB serial. No encoder feedback, no LEDs, no Matter.

### Hardware

- **MCU:** ESP32-C3 SuperMini
- **Driver:** SimpleFOC Mini (3-phase FOC, `IN1`/`IN2`/`IN3`/`EN` + `U`/`V`/`W` phases)
- **Motor:** 4015 BLDC gimbal, 12–36V, hollow-shaft external rotor (AS5600/MT6701 encoder not wired yet)
- **Power:** Mean Well LRS-75-24 (24V 3.2A)
- **Buck:** Mini360 24V→5V for the ESP32

### Wiring

All driver signals are on one side of the board (pins 5–10).

| ESP32-C3 GPIO | SimpleFOC Mini | Notes |
|---|---|---|
| 5 | IN1 | PWM phase A |
| 6 | IN2 | PWM phase B |
| 7 | IN3 | PWM phase C |
| 10 | EN | Drive HIGH to enable |
| GND | GND | Common ground with Mean Well V− and Mini360 OUT− |
| — (24V rail) | VCC / VM | Motor power |
| 5V (via Mini360) | — | ESP32 power |

Motor phase wires go to the driver's `U`/`V`/`W` screw terminals — order doesn't have to be correct on first try; if the motor stutters, swap any two.

Driver pins `nSLEEP` / `nRESET` / `nFAULT` are **left unconnected** for M1 — the SimpleFOC Mini's onboard pull-ups hold the driver out of sleep and reset. If the motor won't energise at all, tie `nSLEEP` and `nRESET` to **3.3V** as a first diagnostic step.

### Build

```bash
arduino-cli compile --fqbn "esp32:esp32:esp32c3:CDCOnBoot=cdc,PartitionScheme=huge_app" arduino/BrushlessLamp/BrushlessLamp.ino
arduino-cli upload  --fqbn "esp32:esp32:esp32c3:CDCOnBoot=cdc,PartitionScheme=huge_app,EraseFlash=all" --port /dev/cu.usbmodem101 arduino/BrushlessLamp/BrushlessLamp.ino
```

`EraseFlash=all` is only required on the **first** flash after leftover partition state from another project (e.g. CylinderLamp). Subsequent uploads can drop it.

Requires the [pioarduino ESP32 core](https://github.com/pioarduino/platform-espressif32) and the [SimpleFOC](https://github.com/simplefoc/Arduino-FOC) library (Library Manager → "Simple FOC", v2.4.x).

### Serial monitor

```bash
arduino-cli monitor --port /dev/cu.usbmodem101 --config baudrate=115200 --config dtr=off --config rts=off
```

**Don't omit `dtr=off` and `rts=off`** — on the ESP32-C3 SuperMini's USB-Serial/JTAG, the default `dtr=on,rts=on` holds the chip in reset with `BOOT` low, so the sketch never runs while the monitor is attached. Symptom: connects cleanly, no banner ever appears.

### Serial commands (115200 baud)

- `T<float>` — set target velocity in rad/s. `T2` spins forward, `T0` stops, `T-2` reverses.
- `L<float>` — set motor voltage limit in volts (safety headroom).

### First-run checklist

1. Flash firmware with motor disconnected. Confirm serial banner: `=== BrushlessLamp M1: motor only ===`.
2. Wire driver + motor per the table above with Mean Well **off**. Verify common ground.
3. Power on. Motor should be energised but stationary (`target_velocity = 0`). Any whine / heat → send `L1`.
4. Send `T2`. Shaft should rotate smoothly. If it stutters, swap two phase wires at the driver.
5. Push limits gradually: `T5`, then `L4` if torque is insufficient.

## Roadmap

- **M1 (this):** Motor spins under serial control.
- **M2:** AS5600 / MT6701 encoder, closed-loop position, pole-pair auto-calibration.
- **M3:** CCT LED strip (WW/CW MOSFETs) + rotary encoder user input.
- **M4:** Matter / Google Home integration (port from CylinderLamp).
  - **Refactor required first:** the current sketch spins forever inside `loop()` (`while (true) { ... }`) to dodge the arduino-esp32 single-core scheduler quirk that preempts `loop()` for ~4 ms every 2 s — see "Known issues" below. That works fine with no networking, but Matter/WiFi/BLE all run on FreeRTOS tasks at priorities 5–23 and need the scheduler to actually schedule them. Before pulling Matter in, move `motor.loopFOC()` onto a hardware timer (`hw_timer_t` + `IRAM_ATTR` ISR at 5–10 kHz) so commutation is deterministic regardless of what else runs, and let `loop()` return normally to feed the rest of the system.

## Known issues / references

- **Periodic motor "click" every ~2 s on ESP32-C3 single-core, all SimpleFOC versions.** The arduino-esp32 framework runs `loop()` inside a FreeRTOS task that gets preempted by an unidentified background task for ~4 ms roughly every 2 s. At motor speeds where the electrical period approaches the stall duration (e.g. 50 rad/s × 11 pole pairs ≈ 11 ms electrical), SVPWM is held frozen at the wrong rotor angle for ~37 % of an electrical cycle, producing an audible torque step on resume. M1 works around this by never returning from `loop()`. The proper fix is to run `loopFOC()` from a hardware timer ISR (see Roadmap M4).
  - [simplefoc/Arduino-FOC#292 — "Motor skips every 2 sec on ESP32, even in open loop"](https://github.com/simplefoc/Arduino-FOC/issues/292)
  - [SimpleFOC community — Periodic jerk in velocity control mode](https://community.simplefoc.com/t/periodic-jerk-in-velocity-control-mode/7641)
  - [SimpleFOC community — ESP32-C6 strange spikes in loop execution time](https://community.simplefoc.com/t/esp32-c6-strange-spikes-in-loop-execution-time/8003)
  - [SimpleFOC community — Clicking noise with ESP32 and Lib Version 2.3.4](https://community.simplefoc.com/t/clicking-noise-with-esp32-and-lib-version-2-3-4/5939)
  - [SimpleFOC docs — Hard real-time FOC loop using timers](https://docs.simplefoc.com/real_time_loop)
- **ESP32-C3 has no FPU.** Every `%.2f` in a `printf` runs through soft-float and costs ~300 µs per argument. A 9-float status line was responsible for an additional ~4 ms hot-path stall before the loop was reworked; the current sketch uses fixed-point integer formatting (`FX_HI` / `FX_LO`) for anything printed inside the FOC loop.
- **DRV8313 latched faults are cleared by `nSLEEP`, not by `EN`.** SimpleFOC Mini's `EN` header only tri-states the H-bridge outputs. To recover from OCP/TSD, pulse `nSLEEP` low for ≥30 µs (the sketch uses 50 µs) and wait ≥1 ms before re-enabling. `PIN_NSP` (GPIO 21) handles this in `resetDriver()`.
- **ESP32-C3 SuperMini USB-CDC reset on monitor attach.** `arduino-cli monitor` defaults to `dtr=on,rts=on`, which holds `BOOT` low and prevents the sketch from running. Always pass `--config dtr=off --config rts=off`.
