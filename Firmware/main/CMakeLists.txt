idf_component_register(SRCS "vSignalTask.c" "processar_bit.c" "crypto.c" "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES "esp_adc"
                       REQUIRES "driver"
                       REQUIRES "esp_timer"
                       REQUIRES "components"
                       REQUIRES "mbedtls"
                       EMBED_TXTFILES "certs/private_key.pem")
