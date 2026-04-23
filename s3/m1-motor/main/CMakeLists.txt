idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        arduino-esp32
        brushlesslamp_core
        nvs_flash
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
