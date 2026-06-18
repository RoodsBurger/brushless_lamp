cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER "1.2.3")
set(PROJECT_VER_NUMBER 10203)

## Matter uses its own toolchain layer, so ESP_MATTER_PATH must be set before
## project(). Source ~/esp/esp-matter/export.sh before running idf.py build.
if(NOT DEFINED ENV{ESP_MATTER_PATH})
    message(FATAL_ERROR "ESP_MATTER_PATH unset — source ~/esp/esp-matter/export.sh first")
endif()

if(NOT DEFINED ENV{ESP_MATTER_DEVICE_PATH})
    if("${IDF_TARGET}" STREQUAL "esp32s3" OR "${IDF_TARGET}" STREQUAL "")
        set(ENV{ESP_MATTER_DEVICE_PATH} $ENV{ESP_MATTER_PATH}/device_hal/device/esp32s3_devkit_c)
    else()
        message(FATAL_ERROR "M4 targets esp32s3 on this board; got ${IDF_TARGET}")
    endif()
endif()

set(ESP_MATTER_PATH $ENV{ESP_MATTER_PATH})
set(MATTER_SDK_PATH ${ESP_MATTER_PATH}/connectedhomeip/connectedhomeip)

# button_driver + led_driver are required transitively by esp-matter's
# examples/common components (app_reset REQUIRES button; device_hal/device
# REQUIRES led_driver). We don't consume either ourselves (M4 has its own
# input.cpp / leds.cpp), but the component resolver still needs them visible.
set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../common"
    "${CMAKE_CURRENT_LIST_DIR}/../components"
    "${ESP_MATTER_PATH}/examples/common"
    "${MATTER_SDK_PATH}/config/esp32/components"
    "${ESP_MATTER_PATH}/components"
    "${ESP_MATTER_PATH}/device_hal/device"
    "${ESP_MATTER_PATH}/device_hal/button_driver"
    "${ESP_MATTER_PATH}/device_hal/led_driver")

set(SDKCONFIG_DEFAULTS
    "${CMAKE_CURRENT_LIST_DIR}/../sdkconfig.defaults.common"
    "${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults"
    "${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults.esp32s3")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include(${ESP_MATTER_PATH}/examples/common/cmake_common/components_include.cmake)
include($ENV{ESP_MATTER_DEVICE_PATH}/esp_matter_device.cmake)

project(brushlesslamp)

idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
## Keep -std off the CXX flags — arduino-esp32 compiles as gnu++2b and forcing
## gnu++17 later breaks aggregate-init emplace_back in HTTPClient.cpp under GCC 14.
idf_build_set_property(CXX_COMPILE_OPTIONS "-Os;-Wno-overloaded-virtual" APPEND)
idf_build_set_property(COMPILE_OPTIONS
    "-Wno-format-nonliteral;-Wno-format-security;-DCHIP_HAVE_CONFIG_H" APPEND)
