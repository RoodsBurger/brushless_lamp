# BrushlessLamp — XIAO ESP32-S3 port

Staged bring-up for the S3 migration. Four independently-buildable IDF projects share a common motor library + vendored SimpleFOC so every stage runs the same FOC core, only the surface (knob, LEDs, Matter) changes.

See `../PHASE15-S3-MIGRATION.md` for the full design notes — which C6 patches carry over, what changes for S3, pin map, partition table, risks.

## Layout

```
s3/
├── components/SimpleFOC/            # vendored Arduino-FOC v2.4.0 (shared)
├── common/                           # shared pins.h / config.h / motor.{h,cpp}
│   ├── CMakeLists.txt                # component name: brushlesslamp_core
│   ├── pins.h                        # XIAO S3 D-label → GPIO map
│   ├── config.h                      # M2-proven motor tuning + S3-specific affinity consts
│   ├── motor.h / motor.cpp           # FOC task pinned to core 1
├── sdkconfig.defaults.common         # stacked via SDKCONFIG_DEFAULTS in each stage
├── m1-motor/                         # Stage 1 — motor only (sweep target to validate audibility)
├── m2-knob/                          # Stage 2 — motor + knob + stop button
├── m3-leds/                          # Stage 3 — + WW/CW LEDC angle follower
└── m4-matter/                        # Stage 4 — + Matter ColorTemperatureLight
```

## One-time setup

```sh
source ~/esp/esp-idf/export.sh          # ESP-IDF v5.4.4
source ~/esp/esp-matter/export.sh       # M4 only — sets ESP_MATTER_PATH
```

First time on this machine, apply the two chip manifest patches (same as C6 — see `PHASE13-PRODUCTION-CATALOG.md §3.1`) before building M4:

```sh
sed -i '' 's|version: "\^1.2.4"|version: "1.2.2"|' \
  ~/esp/esp-matter/connectedhomeip/connectedhomeip/config/esp32/components/chip/idf_component.yml
sed -i '' 's|version: "~1.3.0"|version: "~1.2.0"|' \
  ~/esp/esp-matter/connectedhomeip/connectedhomeip/config/esp32/components/chip/idf_component.yml
```

And confirm PR#42320 is cherry-picked on the connectedhomeip submodule — required for WiFi-reconnect stability on both C6 and S3.

## Build & flash (any stage)

The project path contains a space (`Personal Projects`), which breaks libsodium's unquoted `-include`. Work in a no-space shadow directory:

```sh
# M1 example — swap "m1-motor" for m2-knob / m3-leds / m4-matter as needed
rsync -a --exclude=build --exclude=managed_components --exclude=dependencies.lock \
      --exclude=sdkconfig \
      "/Users/rodolfo/Documents/Personal Projects/BrushlessLamp/s3/" \
      ~/esp/brushlesslamp-s3/
cd ~/esp/brushlesslamp-s3/m1-motor

idf.py set-target esp32s3
idf.py build
idf.py -p /dev/cu.usbmodem101 flash monitor
```

Edits happen in the source tree (`~/Documents/Personal Projects/BrushlessLamp/s3/...`); re-`rsync` before each build.

## Acceptance checklist

| Stage | What's running | Acceptance |
|---|---|---|
| M1 | motor + sweep loop | Motor ramps 0 → ANGLE_MAX/2 → 0 silently. `maxLp` < 100 µs steady-state (dedicated core 1 + FPU). Record as `/tmp/s3_m1.log`. |
| M2 | + knob + button | Each detent moves the lamp by 1 rad, same audibility as M1. Button returns to origin cleanly. |
| M3 | + LEDs | LEDs come on above `LED_ON_ANGLE_THRESH`, off below. No motor regression vs M2. |
| M4 | + Matter | Commission via Apple/Google Home. Slider moves lamp; color-temp slider shifts WW/CW mix. Knob-driven moves push back to the app within ~300 ms. 5 s button hold = factory reset. |

## Rollback to C6

The existing `../esp-idf/` directory (Phase 14c snapshot at commit `40b9aa2`) is untouched. `git checkout 40b9aa2 -- esp-idf/` reverts to the last known-good C6 build.
