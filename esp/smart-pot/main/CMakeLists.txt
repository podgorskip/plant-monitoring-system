idf_component_register(SRCS "main.c" "mqtt.c" "nvs_storage.c"
                      INCLUDE_DIRS "."
                      REQUIRES bt nvs_flash esp_wifi esp_event driver mqtt esp_http_client json esp_http_server)
