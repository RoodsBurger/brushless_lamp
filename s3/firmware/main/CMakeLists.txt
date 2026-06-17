idf_component_register(
    SRCS
        "main.cpp"
        "input.cpp"
        "leds.cpp"
        "matter_app.cpp"
        "ota.cpp"
    INCLUDE_DIRS "."
    PRIV_INCLUDE_DIRS "$ENV{ESP_MATTER_PATH}/examples/common/utils"
    REQUIRES
        arduino-esp32
        common
        nvs_flash
        esp_driver_ledc
        chip
        esp_matter
        esp_matter_console
        esp_https_ota
        esp_http_client
        json
        mbedtls
        esp_app_format
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=comment
    -Wno-missing-field-initializers
    -Wno-volatile
    -DCHIP_HAVE_CONFIG_H
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    ARDUINO_USB_CDC_ON_BOOT=1
    ARDUINO_USB_MODE=1
)
