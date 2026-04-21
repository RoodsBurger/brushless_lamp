# BrushlessLamp — native ESP-IDF + esp-matter build

Production-grade Matter-over-WiFi ColorTemperatureLight for the Seeed XIAO ESP32-C6. Replaces the Arduino sketch in `../arduino/` with a native IDF project that includes an upstream bug fix ([connectedhomeip#42320](https://github.com/project-chip/connectedhomeip/pull/42320)) not yet packaged in any stock arduino-esp32 release.

## Why native IDF

The stock arduino-esp32 3.3.7/3.3.8 Matter library triggers a `chip[DL]` WiFi-reconnect crash on ESP32-C6 (Load access fault, `MTVAL=0x000000a8`, after `esp_wifi_connect() failed: ESP_ERR_WIFI_CONN`). The root cause ([connectedhomeip#42146](https://github.com/project-chip/connectedhomeip/issues/42146)) is a duplicate `SetWiFiStationMode(Disabled)` race that leaves a null pointer dangling for `chip[DL]` to deref. PR#42320 fixed it upstream on 2025-12-12, but espressif/esp-matter still pins connectedhomeip at a pre-fix commit — so building from source with a cherry-pick is the only way to get the fix today.

## One-time setup

```sh
# ESP-IDF v5.4.4 (latest patch in the v5.4 line; esp-matter officially supports v5.4.x for C6)
git clone -b v5.4.4 --recursive https://github.com/espressif/esp-idf.git ~/esp/esp-idf
~/esp/esp-idf/install.sh esp32c6
echo 'export IDF_CCACHE_ENABLE=1' >> ~/.zshrc

# esp-matter release/v1.5
git clone -b release/v1.5 https://github.com/espressif/esp-matter.git ~/esp/esp-matter
cd ~/esp/esp-matter
git submodule update --init --recursive --depth 1 connectedhomeip/connectedhomeip
./install.sh

# Apply PR#42320 patch (fixes the C6 WiFi-reconnect crash)
cd ~/esp/esp-matter/connectedhomeip/connectedhomeip
git apply /path/to/BrushlessLamp/esp-idf/connectedhomeip-pr42320.patch
# (or re-cherry-pick from PR#42320 directly — see plan file for verbose recipe)
```

## Build + flash

**The GN build tool that connectedhomeip uses does not tolerate spaces in the project path.** This repo lives at `.../Personal Projects/BrushlessLamp/esp-idf` which has a space. Build from a no-space symlink instead:

```sh
ln -sf "/Users/rodolfo/Documents/Personal Projects/BrushlessLamp/esp-idf" ~/esp/brushlesslamp-idf
```

Then every session:

```sh
source ~/esp/esp-idf/export.sh
source ~/esp/esp-matter/export.sh
cd ~/esp/brushlesslamp-idf

idf.py set-target esp32c6       # first time only
idf.py build
idf.py -p /dev/cu.usbmodem101 flash monitor
```

First build is ~20-30 min (1500+ components). Subsequent builds are ~30 s with `IDF_CCACHE_ENABLE=1`. Exit `idf.py monitor` with `Ctrl+]`.

`idf.py monitor` attaches cleanly to the XIAO C6 without the `dtr=on,rts=on` reset quirk that bit the `arduino-cli monitor`.

## Commissioning

On first boot the serial log prints the Matter onboarding info. Open Google Home → Add device → Matter-enabled → scan/enter the QR or manual pairing code.

Between-reboot behavior: once commissioned, the device comes back online automatically on each power-cycle. The PR#42320 fix removes the `chip[DL]` WiFi-reconnect crash that plagued the Arduino build.

## Factory reset

Long-press the button for 9 s. This calls `esp_matter::factory_reset()` (which decommissions, wipes Matter NVS, and reboots) and is a true full-wipe — next boot will re-advertise BLE for commissioning.

## Project layout

```
esp-idf/
├── CMakeLists.txt                  ← root project (mirrors esp-matter examples/light)
├── sdkconfig.defaults              ← cross-target defaults (flash size, Matter, BLE, etc.)
├── sdkconfig.defaults.esp32c6      ← C6 target tag
├── partitions.csv                  ← 4 MB layout w/ OTA + factory partition
├── connectedhomeip-pr42320.patch   ← the upstream fix; re-apply if esp-matter is reinstalled
└── main/
    ├── CMakeLists.txt              ← lists all *.cpp + REQUIRES components
    ├── idf_component.yml           ← managed_components: esp_simplefoc + iot_button
    ├── pins.h                      ← GPIO assignments
    ├── config.h                    ← tunables (POS_MAX, speed presets, etc.)
    ├── app_main.cpp                ← entry, Matter wiring, event handler
    ├── app_driver.{h,cpp}          ← bridges Matter attribute callbacks → hardware
    ├── motor.{h,cpp}               ← SimpleFOC + 2 kHz GPTimer ISR + trapezoidal ramp
    ├── leds.{h,cpp}                ← LEDC PWM for WW/CW MOSFETs + pulse animation
    ├── input.{h,cpp}               ← PCNT knob decoder + button state machine
    └── persistence.{h,cpp}         ← NVS for speed_idx + accel
```

## Dependencies

- `espressif/esp_simplefoc` ^1.2.3 — pure-IDF SimpleFOC with MCPWM backend on C6
- `espressif/button` ^3.x — required by esp-matter's `app_reset` (we include `examples/common` via EXTRA_COMPONENT_DIRS)

## Bench-test without commissioning

Short-click the knob → motor + FOC ISR come up (bypasses the "wait for commissioning" gate). Turn the knob, the lamp moves. Verify the motor + LED paths without needing Google Home.
