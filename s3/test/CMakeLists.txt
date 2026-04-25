cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER "0.0.1-test")

## Bare-bones IDF blink — no Arduino, no Matter, no SimpleFOC, no common code.
## Used to prove the board boots from external 5 V independent of any of our
## application stacks. If THIS doesn't boot from the user's industrial
## 24→5 V converter + cap + Schottky, the failure is hardware.

set(SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults.esp32s3")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(brushlesslamp_test)
