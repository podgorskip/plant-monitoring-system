#include "sensor_manager.h"
#include "moisture_sensor.h"
#include "light_sensor.h"
#include "water_sensor.h"
#include "bme280_sensor.h"
#include "esp_log.h"

#define TAG "SENSOR_MANAGER"

void sensor_manager_init(void) {
    ESP_LOGI(TAG, "Initializing sensors...");
    
    // Initialize each sensor
    moisture_sensor_init();
    light_sensor_init();
    water_sensor_init();
    bme280_sensor_init();

    ESP_LOGI(TAG, "All sensors initialized successfully");
}

void sensor_manager_read_all(void) {
    // Variables for sensor readings
    float temperature = 0, pressure = 0, humidity = 0;

    // Read Moisture Sensor
    int moisture_raw = moisture_sensor_read();
    int moisture_percentage = moisture_sensor_get_percentage(moisture_raw);
    const char* moisture_condition = moisture_sensor_get_condition(moisture_raw);

    // Read Light Sensor (Analog + Digital)
    int light_analog = light_sensor_read_analog();
    const char* light_condition = light_sensor_get_digital_condition();
    // Read Water Level Sensor
    int water_level = water_sensor_read();

    // Read BME280 Sensor (Temperature, Pressure, Humidity)
    if (bme280_get_temperature(&temperature) == ESP_OK &&
        bme280_get_pressure(&pressure) == ESP_OK &&
        bme280_get_humidity(&humidity) == ESP_OK) {
    } else {
        ESP_LOGE(TAG, "Failed to read data from BME280");
    }
}

// Individual Sensor Read Functions

int sensor_manager_read_light_analog(void) {
    return light_sensor_read_analog();
}

int sensor_manager_read_moisture(void) {
    return moisture_sensor_get_percentage(moisture_sensor_read());
}

int sensor_manager_read_water_level(void) {
    return water_sensor_read();
}

float sensor_manager_read_temperature(void) {
    float temperature = 0;
    if (bme280_get_temperature(&temperature) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read temperature from BME280");
    }
    return temperature;
}

float sensor_manager_read_humidity(void) {
    float humidity = 0;
    if (bme280_get_humidity(&humidity) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read humidity from BME280");
    }
    return humidity;
}

float sensor_manager_read_pressure(void) {
    float pressure = 0;
    if (bme280_get_pressure(&pressure) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read pressure from BME280");
    }
    return pressure;
}
