#include "water_sensor.h"
#include "driver/adc.h"
#include "esp_log.h"

#define WATER_SENSOR_ADC_CHANNEL ADC1_CHANNEL_6  // GPIO34
#define TAG "WATER_SENSOR"
#define WATER_SENSOR_THRESHOLD 0  // Threshold for low water detection (adjust based on testing)

void water_sensor_init(void) {
    adc1_config_width(ADC_WIDTH_BIT_12); // 12-bit resolution (0–4095)
    adc1_config_channel_atten(WATER_SENSOR_ADC_CHANNEL, ADC_ATTEN_DB_11); // 0–3.9V range
}

int water_sensor_read(void) {
    // int raw_value = adc1_get_raw(WATER_SENSOR_ADC_CHANNEL);
    // TODO: add dummy data for water sensor
    int raw_value = 200;
    return raw_value;
}

bool water_sensor_is_low(void) {
    int raw_value = water_sensor_read();
    return raw_value < WATER_SENSOR_THRESHOLD;
}
