## SimpleFOC v2.4.0 (simplefoc/Arduino-FOC) as an ESP-IDF component. Same globbed
## sources + platform-guard strategy that worked on C6; the esp32_mcpwm_mcu.cpp path
## activates on S3 identically (SOC_MCPWM_SUPPORTED is defined, 2 MCPWM groups).
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

## Relax SimpleFOC's known warnings in one place so every m-stage picks them up.
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
