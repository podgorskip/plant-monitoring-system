#ifndef BME280_SENSOR_H
#define BME280_SENSOR_H

#include <stdio.h>
#include "esp_err.h"

// Initialize the BME280 sensor
void bme280_sensor_init(void);

// Get temperature in °C
esp_err_t bme280_get_temperature(float *temperature);

// Get pressure in hPa
esp_err_t bme280_get_pressure(float *pressure);

// Get humidity in %
esp_err_t bme280_get_humidity(float *humidity);

#endif // BME280_SENSOR_H
