idf_component_register(SRCS "bme280_sensor.c" "sensor_manager.c" "pump_control.c" "moisture_sensor.c" "water_sensor.c" "light_sensor.c" "main.c" "mqtt.c" "nvs_storage.c"
                      INCLUDE_DIRS "."
                      REQUIRES bt nvs_flash esp_wifi esp_event driver mqtt esp_http_client json esp_http_server bme280)
