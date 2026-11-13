idf_component_register(SRCS "app_time.c" "wifi_profiles.c" INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "private"
    PRIV_REQUIRES esp_wifi esp_event esp_netif esp_timer nvs_flash esp_mosaico_app_recovery log)
