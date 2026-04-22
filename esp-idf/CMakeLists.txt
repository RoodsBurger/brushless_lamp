cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER "0.14.3")
set(PROJECT_VER_NUMBER 143)

# Phase 14c wires esp-matter back into the build alongside arduino-esp32 + SimpleFOC.
# Must source ~/esp/esp-matter/export.sh before running idf.py so ESP_MATTER_PATH is set.
if(NOT DEFINED ENV{ESP_MATTER_PATH})
    message(FATAL_ERROR "ESP_MATTER_PATH unset — source ~/esp/esp-matter/export.sh first")
endif()

if(NOT DEFINED ENV{ESP_MATTER_DEVICE_PATH})
    if("${IDF_TARGET}" STREQUAL "esp32c6" OR "${IDF_TARGET}" STREQUAL "")
        set(ENV{ESP_MATTER_DEVICE_PATH} $ENV{ESP_MATTER_PATH}/device_hal/device/esp32c6_devkit_c)
    else()
        message(FATAL_ERROR "Phase 14c targets esp32c6; got ${IDF_TARGET}")
    endif()
endif()

set(ESP_MATTER_PATH $ENV{ESP_MATTER_PATH})
set(MATTER_SDK_PATH ${ESP_MATTER_PATH}/connectedhomeip/connectedhomeip)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include(${ESP_MATTER_PATH}/examples/common/cmake_common/components_include.cmake)
include($ENV{ESP_MATTER_DEVICE_PATH}/esp_matter_device.cmake)

set(EXTRA_COMPONENT_DIRS
    "${ESP_MATTER_PATH}/examples/common"
    "${MATTER_SDK_PATH}/config/esp32/components"
    "${ESP_MATTER_PATH}/components"
    "${ESP_MATTER_PATH}/device_hal/device"
    ${extra_components_dirs_append})

project(brushlesslamp)

# Do not set global -std= — arduino-esp32 3.3.8's HTTPClient.cpp relies on gnu++2b's
# aggregate-init emplace_back which breaks under gnu++17. chip/connectedhomeip and
# esp-matter compile cleanly at gnu++2b (IDF's default for C++ since 5.4).
idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
idf_build_set_property(CXX_COMPILE_OPTIONS "-Os;-Wno-overloaded-virtual" APPEND)
idf_build_set_property(COMPILE_OPTIONS
    "-Wno-format-nonliteral;-Wno-format-security;-DCHIP_HAVE_CONFIG_H"
    APPEND)
