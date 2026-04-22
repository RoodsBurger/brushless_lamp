## SimpleFOC v2.4.0 (simplefoc/Arduino-FOC) as an ESP-IDF component.
## Motor / driver / sensor / platform sources are globbed; non-ESP32 platforms are no-ops
## behind their own `#if defined(ARDUINO_ARCH_*)` guards, so the link is size-correct.

file(GLOB_RECURSE SIMPLEFOC_SRCS "src/*.cpp")

idf_component_register(
    SRCS ${SIMPLEFOC_SRCS}
    INCLUDE_DIRS
        "src"
        "src/common"
        "src/common/base_classes"
    REQUIRES
        arduino-esp32
        driver
        esp_driver_mcpwm
        esp_driver_ledc
        esp_driver_gptimer
)

## SimpleFOC ships switch statements without defaults, nested /* in a header comment
## (MagneticSensorSPI.h:38), an uninitialized-member warning in MagneticSensorAnalog.cpp,
## plus Arduino-style volatile/enum-float usage. Relaxed here so arduino-esp32's own
## flags aren't touched.
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=switch
    -Wno-error=comment
    -Wno-error=uninitialized
    -Wno-volatile
    -Wno-deprecated-enum-float-conversion
    -Wno-missing-field-initializers
    -Wno-unused-function
    -Wno-unused-variable
)
