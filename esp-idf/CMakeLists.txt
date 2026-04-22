cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER "0.14.1-a")
set(PROJECT_VER_NUMBER 141)

# Phase 14a — motor-only validation. No esp-matter in the build graph; Matter comes back in 14c.
# arduino-esp32 is pulled in via main/idf_component.yml (Component Manager); SimpleFOC is
# vendored in components/SimpleFOC/ from the simplefoc/Arduino-FOC v2.4.0 tag.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(brushlesslamp)

# Do NOT override -std here — arduino-esp32 compiles as gnu++2b, and forcing gnu++17
# *after* it breaks aggregate-init emplace_back in HTTPClient.cpp (GCC14 + libstdc++14).
# SimpleFOC-specific warnings are relaxed inside components/SimpleFOC/CMakeLists.txt.
idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-format-nonliteral;-Wno-format-security" APPEND)
