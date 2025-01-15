#include "bme280_sensor.h"
#include "bme280.h"
#include "platform_i2c.h"
#include "esp_log.h"

#define TAG "BME280_SENSOR"

static BME280 bme280_device; // Static instance of the BME280 structure

void bme280_sensor_init(void) {
    platform_i2c_init(); // Initialize I2C communication

    ESP_LOGI(TAG, "Initializing BME280...");
    bme280_init(&bme280_device, BME280_I2C_ADDR_0); // No need to assign return value
    ESP_LOGI(TAG, "BME280 successfully initialized");
}

esp_err_t bme280_get_temperature(float *temperature) {
    return bme280_read_temp(&bme280_device, temperature);
}

esp_err_t bme280_get_pressure(float *pressure) {
    return bme280_read_pressure(&bme280_device, pressure);
}

esp_err_t bme280_get_humidity(float *humidity) {
    return bme280_read_humidity(&bme280_device, humidity);
}
