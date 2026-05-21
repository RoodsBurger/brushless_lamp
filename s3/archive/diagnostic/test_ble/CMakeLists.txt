cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER "0.0.1-test_ble")

## Bare-bones BLE-only test — no Matter, no esp-matter, no arduino-as-component,
## no SimpleFOC, no common code. Just NimBLE init + advertise + a heartbeat task,
## plus the same NVS-persisted boot_dbg the m4-matter build uses.
##
## Goal: isolate whether the external-power wedge in m4-matter is in our app
## stack (Matter / arduino-esp32 / SimpleFOC) or in IDF/NimBLE/silicon. If THIS
## build wedges on external power at the same relative point in BLE init, the
## issue is below the framework. If it works fine, the issue is above.

set(SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults.esp32s3")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(brushlesslamp_test_ble)
