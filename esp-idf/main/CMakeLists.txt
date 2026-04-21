idf_component_register(
    SRCS
        "app_main.cpp"
        "app_driver.cpp"
        "leds.cpp"
        "motor.cpp"
        "input.cpp"
        "persistence.cpp"
    INCLUDE_DIRS "."
    PRIV_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}"
    REQUIRES
        chip
        esp_matter
        esp_matter_console
        nvs_flash
        driver
        esp_driver_gpio
        esp_driver_ledc
        esp_driver_pcnt
        esp_driver_gptimer
        espressif__esp_simplefoc
)

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
