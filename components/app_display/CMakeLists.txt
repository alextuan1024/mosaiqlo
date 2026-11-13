idf_component_register(SRCS "app_display.c" "app_brightness.c" "app_rotation.c" "orientation.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "private"
    REQUIRES esp-gsp
    PRIV_REQUIRES esp-mosaico-bsp esp_lcd esp_display_present esp_timer nvs_flash log)
