cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER "0.15.1-m1")

## Shared motor core + vendored SimpleFOC live up a level; this stage has no other
## components of its own.
set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../common"
    "${CMAKE_CURRENT_LIST_DIR}/../components")

## Stack common defaults + S3 target overrides.
set(SDKCONFIG_DEFAULTS
    "${CMAKE_CURRENT_LIST_DIR}/../sdkconfig.defaults.common"
    "${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults.esp32s3")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(brushlesslamp_m1_motor)

idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
idf_build_set_property(CXX_COMPILE_OPTIONS "-Os;-Wno-overloaded-virtual" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-format-nonliteral;-Wno-format-security" APPEND)
