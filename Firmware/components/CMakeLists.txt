idf_component_register(SRCS "checksum_8.c" "network_manager.c" "wifi.c" "http.c"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS "."
                       REQUIRES esp_wifi nvs_flash
                       REQUIRES esp_http_client
                       REQUIRES driver
                       REQUIRES main)