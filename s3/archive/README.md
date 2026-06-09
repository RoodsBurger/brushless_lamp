# Archive

Historical and diagnostic targets kept for reference. **None of these ship.**
The shipping firmware lives in `../firmware/`.

## `milestones/`

Per-stage incremental builds from the M1→M4 bring-up. Each directory is a
self-contained ESP-IDF project that builds and flashes on its own; they share
the SimpleFOC component in `../components/SimpleFOC/` and the motor module in
`../common/` (which evolved across the milestones, so older stages may not
build cleanly against the current `common/` headers).

| Dir | What it added | Status |
|---|---|---|
| `m1-motor/` | SimpleFOC velocity-mode driving the gimbal motor; first working FOC + encoder loop on the S3. | Verified, baseline for M2. |
| `m2-knob/` | Rotary knob + button local control on top of M1; introduced the velocity-nudge UX. | Verified, baseline for M3. |
| `m3-leds/` | LEDC dual-channel WW/CW + gamma + fader task on top of M2. | Verified, baseline for M4. |
| `supermini_m4/` | XIAO ESP32-S3 SuperMini pin variant of M4 (one board's GPIO map differs). | Build-tested, not in active use. |

## `diagnostic/`

Standalone test sketches used during bring-up to isolate problems. Each is a
minimal Arduino IDE / ESP-IDF project — not part of the production build.

| Dir | What it tests |
|---|---|
| `test/` | Encoder + driver smoke test, no FOC loop. |
| `test_ble/` | Bare NimBLE advertising — used to bisect the BBPLL consumer / BLE bring-up issue. |
| `test_ble_arduino/` | NimBLE on top of arduino-as-component — same bisection, different harness. |
| `test_ble_arduino_async/` | Async-NimBLE variant of the same. |
