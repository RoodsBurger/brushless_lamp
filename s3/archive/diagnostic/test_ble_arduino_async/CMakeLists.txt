cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER "0.0.1-test_ble_arduino_async")

## test_ble_arduino but with BLE init deferred to a separate high-priority task
## on core 0, mimicking CHIP's PlatformMgr().StartEventLoopTask + ScheduleWork
## pattern. If this wedges on external power where test_ble_arduino does not,
## the trigger is the cross-task BLE-init pattern, not BLE init itself.

set(SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults.esp32s3")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(brushlesslamp_test_ble_arduino_async)
