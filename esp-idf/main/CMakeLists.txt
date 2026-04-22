idf_component_register(
    SRCS
        "main.cpp"
        "motor.cpp"
        "input.cpp"
        "leds.cpp"
        "matter_app.cpp"
    INCLUDE_DIRS "."
    PRIV_INCLUDE_DIRS "$ENV{ESP_MATTER_PATH}/examples/common/utils"
    REQUIRES
        arduino-esp32
        SimpleFOC
        driver
        nvs_flash
        esp_driver_mcpwm
        esp_driver_ledc
        chip
        esp_matter
        esp_matter_console
        app_reset
)

## main/motor.cpp includes <SimpleFOC.h>, so SimpleFOC's header-level warnings
## (nested /* in MagneticSensorSPI.h) surface here too — silence them once.
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=comment
    -Wno-missing-field-initializers
    -Wno-volatile
    -DCHIP_HAVE_CONFIG_H
)

## arduino-esp32 3.3.8 on C6 does not expose ARDUINO_USB_CDC_ON_BOOT in Kconfig, so
## Serial defaults to UART0 on GPIO 17 = PIN_NSP. Force HWCDC routing via macro for
## any Arduino library that uses Serial directly.
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    ARDUINO_USB_CDC_ON_BOOT=1
    ARDUINO_USB_MODE=1
)
