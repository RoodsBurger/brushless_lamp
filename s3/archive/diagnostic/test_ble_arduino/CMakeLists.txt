cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER "0.0.1-test_ble_arduino")

## test_ble + arduino-as-component (initArduino, ARDUINO_USB_CDC_ON_BOOT=1).
## Same NimBLE + WiFi as test_ble, but now with arduino-esp32's USB CDC and
## init layer pulled in. Bisects whether the m4-matter wedge is in
## arduino-as-component or in esp-matter / CHIP.
##
## - test_ble (no arduino, no Matter)            → works on external power
## - test_ble_arduino (arduino, no Matter)       → THIS BUILD
## - m4-matter   (arduino, Matter, motor, knob)  → wedges on external power

set(SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults.esp32s3")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(brushlesslamp_test_ble_arduino)
