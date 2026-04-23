idf_component_register(
    SRCS
        "main.cpp"
        "input.cpp"
        "leds.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        arduino-esp32
        common
        nvs_flash
        esp_driver_ledc
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=comment
    -Wno-missing-field-initializers
    -Wno-volatile
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    ARDUINO_USB_CDC_ON_BOOT=1
    ARDUINO_USB_MODE=1
)
