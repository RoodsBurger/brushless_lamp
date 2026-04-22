# BrushlessLamp — Phase 13 Production Catalog

**Purpose**: this file captures everything production-grade we built up across Phases 1–13 in `esp-idf/` BEFORE wiping the directory in Phase 14. When Phase 14 reaches the production-readiness milestone (Phase 14d), use this document to recreate the workflow without rediscovering it from scratch.

**Status**: snapshot taken on 2026-04-22. Source files under `esp-idf/` will be deleted. The `arduino/` sketch (Arduino-platform M4) and the M2-knob reference at `/tmp/m2_knob/` are unaffected.

---

## 1. Toolchain pinning (one-time setup)

```sh
# ESP-IDF v5.4.4 — latest patch in the v5.4 line; esp-matter pins 5.4.x for ESP32-C6
git clone -b v5.4.4 --recursive https://github.com/espressif/esp-idf.git ~/esp/esp-idf
~/esp/esp-idf/install.sh esp32c6
echo 'export IDF_CCACHE_ENABLE=1' >> ~/.zshrc

# esp-matter release/v1.5
git clone -b release/v1.5 https://github.com/espressif/esp-matter.git ~/esp/esp-matter
cd ~/esp/esp-matter
git submodule update --init --recursive --depth 1 connectedhomeip/connectedhomeip
./install.sh

# Apply PR#42320 patch (fixes the C6 WiFi-reconnect crash) — see Section 3 below
cd ~/esp/esp-matter/connectedhomeip/connectedhomeip
git apply /path/to/BrushlessLamp/PHASE13-PRODUCTION-CATALOG.md  # NO — extract patch from Section 3 first
```

For Phase 14, **arduino-esp32 is added on top** via the IDF Component Manager:

```sh
cd <project>
idf.py add-dependency "espressif/arduino-esp32^3.2.0"   # 3.3.x bumps to IDF v5.5; pin 3.2.x for v5.4.x
```

## 2. GN-no-spaces workaround

connectedhomeip's GN build tool **does not tolerate spaces in the project path**. Our project lives at `/Users/rodolfo/Documents/Personal Projects/BrushlessLamp/esp-idf` which has a space. Build from a no-space symlink instead:

```sh
ln -sf "/Users/rodolfo/Documents/Personal Projects/BrushlessLamp/esp-idf" ~/esp/brushlesslamp-idf
cd ~/esp/brushlesslamp-idf
idf.py set-target esp32c6
idf.py build
idf.py -p /dev/cu.usbmodem101 flash monitor
```

`idf.py monitor` attaches cleanly to the XIAO C6 USB-CDC without the `dtr=on,rts=on` reset quirk that bit `arduino-cli monitor`. Exit with `Ctrl+]`.

First build: ~20–30 min (1500+ components compile). Subsequent builds: ~30 s with `IDF_CCACHE_ENABLE=1`.

## 3. PR#42320 cherry-pick (REQUIRED for production reliability on ESP32-C6)

**Why**: stock arduino-esp32 3.3.7/3.3.8 + esp-matter release/v1.5 both pin connectedhomeip at a pre-fix commit. The bug ([connectedhomeip#42146](https://github.com/project-chip/connectedhomeip/issues/42146)) is a duplicate `SetWiFiStationMode(Disabled)` race that leaves a null pointer dangling for `chip[DL]` to deref → Load access fault `MTVAL=0x000000a8` after a failed `esp_wifi_connect()`. Triggers on every WiFi-reconnect attempt that fails, so devices boot-loop after first connection failure.

**Fix**: PR#42320 was merged upstream on 2025-12-12. Apply this cherry-pick to esp-matter's bundled connectedhomeip submodule.

**Where to apply**:
```sh
cd ~/esp/esp-matter/connectedhomeip/connectedhomeip
git apply /path/to/this-patch.diff
```

**Re-apply** after any `git submodule update` of connectedhomeip or any esp-matter reinstall.

**Patch content** (paste into a `.patch` file, then `git apply`):

```diff
From 23c34c757028347a36b894ce9e578f8df543f8c3 Mon Sep 17 00:00:00 2001
From: BrushlessLamp <brushlesslamp@local>
Date: Mon, 20 Apr 2026 23:48:21 -0700
Subject: [PATCH] Cherry-pick PR#42320: fix C6 chip[DL] WiFi-reconnect crash

Applies the ESP32 platform changes from project-chip/connectedhomeip#42320
to fix the duplicate SetWiFiStationMode(Disabled) race that causes a
Load access fault (MTVAL=0x000000a8) on ESP32-C6 after a failed
esp_wifi_connect().

Files: 4 (CHIPDevicePlatformConfig.h, ConnectivityManagerImpl_WiFi.cpp,
ESP32Utils.cpp, NetworkCommissioningDriver.cpp)
---
 src/platform/ESP32/CHIPDevicePlatformConfig.h | 21 +++++++++
 .../ESP32/ConnectivityManagerImpl_WiFi.cpp    | 10 ++---
 src/platform/ESP32/ESP32Utils.cpp             | 13 +++++-
 .../ESP32/NetworkCommissioningDriver.cpp      | 43 +++++++++++--------
 4 files changed, 63 insertions(+), 24 deletions(-)

diff --git a/src/platform/ESP32/CHIPDevicePlatformConfig.h b/src/platform/ESP32/CHIPDevicePlatformConfig.h
index 03103832..ae8acec4 100644
--- a/src/platform/ESP32/CHIPDevicePlatformConfig.h
+++ b/src/platform/ESP32/CHIPDevicePlatformConfig.h
@@ -106,8 +106,17 @@
 #define CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX
 #define CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN
 #define CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX
+#ifdef CONFIG_CHIPOBLE_SINGLE_CONNECTION
 #define CHIP_DEVICE_CONFIG_CHIPOBLE_SINGLE_CONNECTION CONFIG_CHIPOBLE_SINGLE_CONNECTION
+#else
+#define CHIP_DEVICE_CONFIG_CHIPOBLE_SINGLE_CONNECTION 0
+#endif
+
+#ifdef CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART
 #define CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART
+#else
+#define CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART 0
+#endif
 
 #ifdef CONFIG_ENABLE_TEST_SETUP_PARAMS
 #define CHIP_DEVICE_CONFIG_ENABLE_TEST_SETUP_PARAMS 1
@@ -136,10 +145,22 @@
 #define CHIP_DEVICE_CONFIG_CHIP_CONFIG_NAMESPACE_PARTITION CONFIG_CHIP_CONFIG_NAMESPACE_PARTITION_LABEL
 #define CHIP_DEVICE_CONFIG_CHIP_COUNTERS_NAMESPACE_PARTITION CONFIG_CHIP_COUNTERS_NAMESPACE_PARTITION_LABEL
 #define CHIP_DEVICE_CONFIG_CHIP_KVS_NAMESPACE_PARTITION CONFIG_CHIP_KVS_NAMESPACE_PARTITION_LABEL
+#ifdef CONFIG_ENABLE_ESP32_DEVICE_INSTANCE_INFO_PROVIDER
 #define CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER CONFIG_ENABLE_ESP32_DEVICE_INSTANCE_INFO_PROVIDER
+#else
+#define CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER 0
+#endif
 #define CHIP_DEVICE_CONFIG_DISCOVERY_TIMEOUT_SECS CONFIG_CHIP_DISCOVERY_TIMEOUT_SECS
+#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
 #define CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE CONFIG_ENABLE_ESP32_BLE_CONTROLLER
+#else
+#define CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE 0
+#endif
+#ifdef CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART
 #define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART
+#else
+#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 0
+#endif
 
 #ifdef CONFIG_ENABLE_BLE_EXT_ANNOUNCEMENT
 #define CHIP_DEVICE_CONFIG_EXT_ADVERTISING CONFIG_ENABLE_BLE_EXT_ANNOUNCEMENT
diff --git a/src/platform/ESP32/ConnectivityManagerImpl_WiFi.cpp b/src/platform/ESP32/ConnectivityManagerImpl_WiFi.cpp
index cef762e8..a2184a9d 100644
--- a/src/platform/ESP32/ConnectivityManagerImpl_WiFi.cpp
+++ b/src/platform/ESP32/ConnectivityManagerImpl_WiFi.cpp
@@ -82,7 +82,7 @@ CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(WiFiStationMode val)
         err = Internal::ESP32Utils::EnableStationMode();
         SuccessOrExit(err);
 
-        DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
+        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
     }
 
     if (mWiFiStationMode != val)
@@ -112,7 +112,7 @@ void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
             ChipLogError(DeviceLayer, "ClearWiFiStationProvision failed: %" CHIP_ERROR_FORMAT, error.Format());
             return;
         }
-        DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
+        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
     }
 }
 
@@ -408,7 +408,7 @@ void ConnectivityManagerImpl::OnWiFiPlatformEvent(const ChipDeviceEvent * event)
                 break;
             case WIFI_EVENT_STA_DISCONNECTED:
                 ChipLogProgress(DeviceLayer, "WIFI_EVENT_STA_DISCONNECTED");
-                NetworkCommissioning::ESPWiFiDriver::GetInstance().SetLastDisconnectReason(event);
+                TEMPORARY_RETURN_IGNORED NetworkCommissioning::ESPWiFiDriver::GetInstance().SetLastDisconnectReason(event);
                 if (mWiFiStationState == kWiFiStationState_Connecting)
                 {
                     ChangeWiFiStationState(kWiFiStationState_Connecting_Failed);
@@ -455,13 +455,13 @@ void ConnectivityManagerImpl::_OnWiFiScanDone()
 {
     // Schedule a call to DriveStationState method in case a station connect attempt was
     // deferred because the scan was in progress.
-    DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
+    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
 }
 
 void ConnectivityManagerImpl::_OnWiFiStationProvisionChange()
 {
     // Schedule a call to the DriveStationState method to adjust the station state as needed.
-    DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
+    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
 }
 
 void ConnectivityManagerImpl::DriveStationState()
diff --git a/src/platform/ESP32/ESP32Utils.cpp b/src/platform/ESP32/ESP32Utils.cpp
index bf6fd631..70a20b98 100644
--- a/src/platform/ESP32/ESP32Utils.cpp
+++ b/src/platform/ESP32/ESP32Utils.cpp
@@ -248,9 +248,20 @@ CHIP_ERROR ESP32Utils::ClearWiFiStationProvision(void)
 {
     wifi_config_t stationConfig;
 
+    esp_err_t err = esp_wifi_disconnect();
+    if (err != ESP_OK)
+    {
+        ChipLogProgress(DeviceLayer, "esp_wifi_disconnect() failed: %s", esp_err_to_name(err));
+        // Does not return error here as we just call esp_wifi_disconnect() to ensure that the Wi-Fi is not connecting.
+    }
     // Clear the ESP WiFi station configuration.
     memset(&stationConfig, 0, sizeof(stationConfig));
-    esp_wifi_set_config(WIFI_IF_STA, &stationConfig);
+    err = esp_wifi_set_config(WIFI_IF_STA, &stationConfig);
+    if (err != ESP_OK)
+    {
+        ChipLogError(DeviceLayer, "esp_wifi_set_config() failed: %s", esp_err_to_name(err));
+        return MapError(err);
+    }
 
     return CHIP_NO_ERROR;
 }
diff --git a/src/platform/ESP32/NetworkCommissioningDriver.cpp b/src/platform/ESP32/NetworkCommissioningDriver.cpp
index 113f426a..f12923f2 100644
--- a/src/platform/ESP32/NetworkCommissioningDriver.cpp
+++ b/src/platform/ESP32/NetworkCommissioningDriver.cpp
@@ -117,7 +117,7 @@ CHIP_ERROR ESPWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChange
     // If the network configuration backup exists, it means that the device has been rebooted with
     // the fail-safe armed. Since ESP-WiFi persists all wifi credentials changes, the backup must
     // be restored on the boot. If there's no backup, the below function is a no-op.
-    RevertConfiguration();
+    TEMPORARY_RETURN_IGNORED RevertConfiguration();
 
     return CHIP_NO_ERROR;
 }
@@ -129,8 +129,8 @@ void ESPWiFiDriver::Shutdown()
 
 CHIP_ERROR ESPWiFiDriver::CommitConfiguration()
 {
-    PersistedStorage::KeyValueStoreMgr().Delete(kWiFiSSIDKeyName);
-    PersistedStorage::KeyValueStoreMgr().Delete(kWiFiCredentialsKeyName);
+    TEMPORARY_RETURN_IGNORED PersistedStorage::KeyValueStoreMgr().Delete(kWiFiSSIDKeyName);
+    TEMPORARY_RETURN_IGNORED PersistedStorage::KeyValueStoreMgr().Delete(kWiFiCredentialsKeyName);
 
     return CHIP_NO_ERROR;
 }
@@ -170,8 +170,8 @@ CHIP_ERROR ESPWiFiDriver::RevertConfiguration()
 exit:
 
     // Remove the backup.
-    PersistedStorage::KeyValueStoreMgr().Delete(kWiFiSSIDKeyName);
-    PersistedStorage::KeyValueStoreMgr().Delete(kWiFiCredentialsKeyName);
+    TEMPORARY_RETURN_IGNORED PersistedStorage::KeyValueStoreMgr().Delete(kWiFiSSIDKeyName);
+    TEMPORARY_RETURN_IGNORED PersistedStorage::KeyValueStoreMgr().Delete(kWiFiCredentialsKeyName);
 
     return error;
 }
@@ -224,22 +224,14 @@ Status ESPWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableC
 
 CHIP_ERROR ESPWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
 {
-    // If device is already connected to WiFi, then disconnect the WiFi,
-    // clear the WiFi configurations and add the newly provided WiFi configurations.
+    // Clear the WiFi configurations and add the newly provided WiFi configurations.
     if (chip::DeviceLayer::Internal::ESP32Utils::IsStationProvisioned())
     {
-        ChipLogProgress(DeviceLayer, "Disconnecting WiFi station interface");
-        esp_err_t err = esp_wifi_disconnect();
-        if (err != ESP_OK)
-        {
-            ChipLogError(DeviceLayer, "esp_wifi_disconnect() failed: %s", esp_err_to_name(err));
-            return chip::DeviceLayer::Internal::ESP32Utils::MapError(err);
-        }
         CHIP_ERROR error = chip::DeviceLayer::Internal::ESP32Utils::ClearWiFiStationProvision();
         if (error != CHIP_NO_ERROR)
         {
             ChipLogError(DeviceLayer, "ClearWiFiStationProvision failed: %" CHIP_ERROR_FORMAT, error.Format());
-            return chip::DeviceLayer::Internal::ESP32Utils::MapError(err);
+            return error;
         }
     }
 
@@ -260,7 +252,6 @@ CHIP_ERROR ESPWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen,
         return chip::DeviceLayer::Internal::ESP32Utils::MapError(err);
     }
 
-    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));
     return ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled);
 }
 
@@ -290,7 +281,23 @@ void ESPWiFiDriver::OnConnectWiFiNetworkFailed()
 {
     if (mpConnectCallback)
     {
-        mpConnectCallback->OnResult(Status::kNetworkNotFound, CharSpan(), 0);
+        Status status = Status::kOtherConnectionFailure;
+        switch (mLastDisconnectedReason)
+        {
+        case WIFI_REASON_AUTH_FAIL:
+        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
+        case WIFI_REASON_HANDSHAKE_TIMEOUT:
+        case WIFI_REASON_MIC_FAILURE:
+        case WIFI_REASON_CONNECTION_FAIL:
+            status = Status::kAuthFailure;
+            break;
+        case WIFI_REASON_NO_AP_FOUND:
+            status = Status::kNetworkNotFound;
+            break;
+        default:
+            break;
+        }
+        mpConnectCallback->OnResult(status, CharSpan(), 0);
         mpConnectCallback = nullptr;
     }
 }
@@ -360,7 +367,7 @@ CHIP_ERROR ESPWiFiDriver::StartScanWiFiNetworks(ByteSpan ssid)
     }
     else
     {
-        err = esp_wifi_scan_start(NULL, false);
+        err = esp_wifi_scan_start(nullptr, false);
     }
     if (err != ESP_OK)
     {
-- 
2.48.1
```

## 4. partitions.csv (production layout)

```csv
# Name,   Type, SubType, Offset,  Size, Flags
# Copied verbatim from ~/esp/esp-matter/examples/light/partitions.csv
esp_secure_cert,  0x3F, ,0xd000,    0x2000, encrypted
nvs,      data, nvs,     0x10000,   0xC000,
nvs_keys, data, nvs_keys,,          0x1000, encrypted
otadata,  data, ota,     ,          0x2000
phy_init, data, phy,     ,          0x1000,
ota_0,    app,  ota_0,   0x20000,   0x1E0000,
ota_1,    app,  ota_1,   0x200000,  0x1E0000,
fctry,    data, nvs,     0x3E0000,  0x6000
```

**Roles:**
- `esp_secure_cert` (0xd000, encrypted, 8 KB) — DAC + PAI cert chain (`CONFIG_SEC_CERT_DAC_PROVIDER=y`).
- `nvs` (0x10000, 48 KB) — primary NVS for runtime app state.
- `nvs_keys` (encrypted, 4 KB) — NVS encryption keys.
- `otadata` (8 KB) — OTA boot-slot pointer.
- `phy_init` (4 KB) — RF calibration data.
- `ota_0` / `ota_1` (each 1.875 MB at 0x20000 / 0x200000) — A/B OTA app slots.
- `fctry` (0x3E0000, 24 KB) — per-device factory partition holding unique discriminator/passcode/serial — generated by `esp-matter-mfg-tool`, flashed once per board.

## 5. Per-device commissioning credentials (esp-matter-mfg-tool workflow)

For a sellable device each unit needs unique Matter commissioning credentials (discriminator, passcode, DAC). With stock sdkconfig, every board flashed from the same image shares the same test discriminator/passcode, which means only one can live in a given Matter fabric at a time. **For Phase 14d**, restore this `production/` workflow:

### One-time tooling install
Already installed when you ran `~/esp/esp-matter/install.sh` — the tool lives at `~/esp/esp-matter/tools/mfg_tool/` and is available in `$PATH` as `esp-matter-mfg-tool` once esp-matter's `export.sh` is sourced.

### Per-device generation

```sh
source ~/esp/esp-idf/export.sh
source ~/esp/esp-matter/export.sh
mkdir -p production && cd production

# Development / small-batch — uses test VID 0xFFF1 (unusable for CSA certification,
# fine for personal/hobby + early pilots)
esp-matter-mfg-tool \
    -v 0xFFF1 \
    -p 0x8001 \
    --product-name BrushlessLamp \
    --vendor-name "Rodolfo Home" \
    --hw-ver 1 \
    --hw-ver-str "rev1" \
    --dac-in-secure-cert \
    -cd "${ESP_MATTER_PATH}/connectedhomeip/connectedhomeip/credentials/development/commissioner/cd/Chip-Test-CD-FFF1-8001.der" \
    -pai "${ESP_MATTER_PATH}/connectedhomeip/connectedhomeip/credentials/development/attestation/Chip-Test-PAI-FFF1-8000-Cert.der" \
    -k  "${ESP_MATTER_PATH}/connectedhomeip/connectedhomeip/credentials/development/attestation/Chip-Test-PAI-FFF1-8000-Key.pem"
```

Output: a per-device directory `out/<UUID>/` with `fctry.bin` (the factory partition image) + `onb_codes.csv` (the human-readable QR + manual pairing code for that specific unit).

### Flashing the factory partition

**One time per board**, right before flashing the application firmware:

```sh
DEV_DIR=$(ls -dt out/* | head -1)   # most recently generated
esptool.py -p /dev/cu.usbmodem101 write_flash 0x3E0000 "$DEV_DIR/fctry.bin"
```

Then flash the app firmware as usual (`idf.py -p /dev/cu.usbmodem101 flash`). The app picks up the factory partition automatically when built with `CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER=y` + `CONFIG_FACTORY_PARTITION_DAC_PROVIDER=y` (or `CONFIG_SEC_CERT_DAC_PROVIDER=y` for the secure-cert flow).

### .gitignore the output

Factory images contain per-device private keys — NEVER commit `out/` to git.

### Real production (certified)

The test VID `0xFFF1` is a CSA reserved test range — certified ecosystems (Apple Home, Google Home, Amazon) will either reject it or mark the device "Unverified". A real product requires:

1. **CSA Participant membership** — $3-7k/year depending on company size. https://csa-iot.org/become-member/
2. **Production VID assignment** from CSA.
3. **Real DAC/PAI chain** — rooted in CSA's PAA (Product Attestation Authority).
4. **Matter certification testing** — $3-15k per product; required to use the official Matter logo.

None of this is strictly required to sell; you can ship with test creds and accept the "Unverified" badge. But Matter certification + real VID = better trust and wider ecosystem support.

## 6. Hardware pin map (XIAO ESP32-C6, current revision)

| Function           | XIAO label | GPIO | Notes                                          |
|--------------------|------------|------|------------------------------------------------|
| DRV8313 IN1 (PWM A)| D10        | 18   | MCPWM phase A                                  |
| DRV8313 IN2 (PWM B)| D9         | 20   | MCPWM phase B                                  |
| DRV8313 IN3 (PWM C)| D8         | 19   | MCPWM phase C                                  |
| DRV8313 nSLEEP     | D7         | 17   | Pulse LOW ≥30 µs to clear latched OCP/TSD      |
| MT6701 encoder A   | D0         | 0    | ABZ quadrature, 1024 CPR                       |
| MT6701 encoder B   | D1         | 1    |                                                |
| Rotary knob A      | D2         | 2    | PCNT-decoded                                   |
| Rotary knob B      | D3         | 21   | (was previously DRV8313 nFAULT — now repurposed) |
| Knob push-button   | D4         | 22   | Active-low, internal pull-up                   |
| LED warm-white gate| D5         | 23   | AO3400A through LEDC                           |
| LED cool-white gate| D6         | 16   | AO3400A through LEDC                           |

DRV8313 nFAULT pin is **NOT monitored** in this hardware revision (D3 was reassigned to the rotary knob). nSLEEP pulse handles all fault clearing.

## 7. Lessons learned across Phases 1–13

### Motor / SimpleFOC

- **Use Arduino-bundled SimpleFOC v2.4.0 directly via arduino-as-component**. Do NOT use Espressif's `esp_simplefoc` 1.2.3 + `arduino-foc` 2.3.0~3 fork — `BLDCMotor.cpp::setPhaseVoltage` and the Sensor base-class API both diverge from upstream, producing audibility and FOC-alignment regressions that can't be patched piecemeal.
- **Center-aligned MCPWM (UP_DOWN count + symmetric LOW(UP)/HIGH(DOWN) compare actions) is required for low coil whine.** Arduino-bundled SimpleFOC's ESP32 driver does this by default. esp_simplefoc's edge-aligned MCPWM was the primary Phase 11 noise source.
- **Velocity mode (no angle PID cascade) is quieter than angle mode.** M2 (velocity) is silent; M4 (angle) is louder. Implication: high-level position control should sit ABOVE velocity control as a separate layer (knob/Matter slider → position adapter → velocity setpoint → SimpleFOC velocity PID).
- **Always run fresh `motor.initFOC()` each boot.** Persisting `zero_electric_angle`/`sensor_direction` caused wrong-direction runaway when calibration went stale (observed in Phases 5–6).
- **`motor.disable()` ends with `setPwm(0,0,0)`** — on hardwired-EN DRV8313 this grounds all phases (back-EMF brake). Usually fine; if idle hum returns, the workaround is `setPwm(SUPPLY_VOLTAGE/2, ...)` (midpoint coast).

### Motor task scheduling under Matter

- Motor-task starvation under Matter CPU pressure was a recurring Phase 12 bug. Fix: priority 5 (above Matter CHIP=2, below TCP/IP=18) + periodic `vTaskDelay(1)` every 256 iters. Without the vTaskDelay, the IDLE-task watchdog panics and the chip boot-loops.
- Tight-loop `loopFOC()` at `priority 17` with no yield ALSO panics for the same IDLE-WDT reason. Don't go above priority 5.
- The previous attempt to put `loopFOC()` in a 1–2 kHz GPTimer ISR worked but capped the velocity-PID rate at FOC_ISR_HZ / motion_downsample = ~400 Hz, audibly coarser than M2's ~1.6 kHz from its `while(true)` tight loop. Tight-loop-as-task is preferred.

### M2 silent profile (the working tunings)

```c
constexpr float PHASE_RESISTANCE     = 5.0f;     // matches the gimbal's actual phase R
constexpr float CURRENT_LIMIT        = 0.5f;     // capped current → 2.5V peak coil voltage → silent + strong on M2
constexpr float VOLTAGE_SENSOR_ALIGN = 3.0f;
constexpr float LPF_VEL_TF           = 0.02f;
constexpr float PID_VEL_P            = 0.2f;
constexpr float PID_VEL_I            = 20.0f;
constexpr float PID_VEL_D            = 0.0f;
constexpr float PID_VEL_OUTPUT_RAMP  = 1000.0f;
constexpr int   MOTION_DOWNSAMPLE    = 5;
constexpr float STALL_VEL_THRESHOLD  = 0.2f;
constexpr uint32_t STALL_TIMEOUT_MS  = 500;
constexpr uint32_t STALL_WARMUP_MS   = 800;
constexpr uint32_t IDLE_DISABLE_MS   = 300;
constexpr float    KNOB_STEP_RAD_PER_SEC = 3.0f;  // each detent bumps target velocity by this
constexpr float    TARGET_ACCEL          = 10.0f; // M2's TARGET_ACCEL_DEFAULT for the velocity ramp
constexpr float    VELOCITY              = 30.0f; // velocity_limit + cruise speed
```

### Matter integration

- `esp_matter::start()` automatically prints the Matter onboarding QR + manual pairing code on the serial console. No extra code needed.
- Once commissioned, the device comes back online automatically on every power-cycle (Matter persists fabric in `nvs`). With PR#42320 applied, WiFi-reconnect failures don't crash.
- **Long-press (9 s) factory reset**: call `esp_matter::factory_reset()`. Decommissions, wipes Matter NVS, reboots — re-advertises BLE for re-commissioning.
- **Color-temperature-light endpoint** (`color_temperature_light::create`) gives On/Off + LevelControl + ColorControl-CT clusters in one shot — exactly what a lamp needs.

### Build infrastructure

- Project root must use `EXTRA_COMPONENT_DIRS` with `$ENV{ESP_MATTER_PATH}/examples/common`, `$ENV{ESP_MATTER_PATH}/components`, and `$ENV{ESP_MATTER_PATH}/connectedhomeip/connectedhomeip/config/esp32/components` to pick up the esp-matter component tree. See `~/esp/esp-matter/examples/light/CMakeLists.txt` for the canonical pattern.
- `ESP_MATTER_DEVICE_PATH` defaults to `~/esp/esp-matter/device_hal/device/esp32c6_devkit_c` for our C6 board. Set in CMakeLists.txt before `include(esp_matter_device.cmake)`.
- The XIAO C6 USB-CDC interface enumerates as `/dev/cu.usbmodem101` (or `5201` on a second cable). `idf.py monitor` works without DTR/RTS quirks.

### Reference logs + sketches

- **M2-knob silent reference**: `/tmp/m2_knob/BrushlessLamp/BrushlessLamp.ino`. Reference log: `/tmp/m2knob_reference.log` shows `tgt:30 ramp:30 vel:29.67 ang:-518 en:1 maxLp:155us` — motor reaches commanded velocity, tight loop runs at ~6.5 kHz, silent + strong torque per user.
- **M4 (Arduino + Matter) reference**: `arduino/BrushlessLamp/BrushlessLamp.ino` in this repo. Louder than M2 but acceptable per user.

## 8. Phase 14 starting point

Phase 14 plan lives at `~/.claude/plans/we-have-this-project-streamed-pike.md`. It calls for a clean-slate IDF project that uses **arduino-as-IDF-component** to bring in the Arduino runtime + Arduino-bundled SimpleFOC v2.4.0 verbatim, on top of the same esp-matter v1.5 + ESP-IDF v5.4.4 stack documented above.

When Phase 14 reaches the production-readiness milestone (14d), use Sections 3, 4, and 5 of this catalog to recreate:

- the PR#42320 patch (Section 3),
- the partitions.csv layout (Section 4),
- the per-device factory cred workflow (Section 5).
