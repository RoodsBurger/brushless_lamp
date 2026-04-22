idf_component_register(
    SRCS
        "main.cpp"
        "motor.cpp"
        "input.cpp"
        "leds.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        arduino-esp32
        SimpleFOC
        driver
        nvs_flash
        esp_driver_mcpwm
        esp_driver_ledc
)

## main/motor.cpp includes <SimpleFOC.h>, so SimpleFOC's header-level warnings
## (nested /* in MagneticSensorSPI.h) surface here too — silence them once.
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=comment
    -Wno-missing-field-initializers
    -Wno-volatile
)

## arduino-esp32 3.3.8 on C6 does not expose ARDUINO_USB_CDC_ON_BOOT in Kconfig, so
## Serial defaults to UART0 (GPIO 17 = PIN_NSP on our board). Define it as a compile
## macro so Arduino-platform code that uses Serial gets HWCDC (USB_SERIAL_JTAG) instead.
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    ARDUINO_USB_CDC_ON_BOOT=1
    ARDUINO_USB_MODE=1
)
