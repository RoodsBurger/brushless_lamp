## Shared motor core used by every M-stage. Each m-project's top-level CMakeLists adds
## ../common to EXTRA_COMPONENT_DIRS, then main/CMakeLists lists `common` (the component
## name, taken from this directory's name) in REQUIRES — regardless of which stage's
## main.cpp is building.

idf_component_register(
    SRCS "motor.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        arduino-esp32
        SimpleFOC
        driver
        esp_driver_mcpwm
        esp_driver_ledc
        nvs_flash
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=comment
    -Wno-missing-field-initializers
    -Wno-volatile
)

## Route Arduino Serial to HWCDC via USB_SERIAL_JTAG. arduino-esp32 3.3.x does not
## surface these in Kconfig (`CONFIG_ARDUINO_USB_*` symbols are silently rejected);
## the bare preprocessor macros are what the runtime reads.
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    ARDUINO_USB_CDC_ON_BOOT=1
    ARDUINO_USB_MODE=1
)
