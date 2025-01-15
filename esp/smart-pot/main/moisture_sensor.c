#include "moisture_sensor.h"
#include "driver/adc.h"
#include "esp_log.h"

#define MOISTURE_SENSOR_ADC_CHANNEL ADC1_CHANNEL_7  // GPIO35
#define TAG "MOISTURE_SENSOR"

// Calibration values
#define DRY_AIR_ADC_VALUE 2460   // ADC value in air (0%)
#define DRY_SOIL_ADC_VALUE 2000  // ADC value in dry soil (~30%)
#define WET_SOIL_ADC_VALUE 1100  // ADC value in wet soil (~80%)
#define WATER_ADC_VALUE 930     // ADC value in fully submerged (100%)

void moisture_sensor_init(void) {
    adc1_config_width(ADC_WIDTH_BIT_12); // 12-bit resolution (0–4095)
    adc1_config_channel_atten(MOISTURE_SENSOR_ADC_CHANNEL, ADC_ATTEN_DB_11); // 0–3.9V range
}

// Read raw ADC value
int moisture_sensor_read(void) {
    int raw_value = adc1_get_raw(MOISTURE_SENSOR_ADC_CHANNEL);
    return raw_value;
}

// Convert raw ADC value to moisture percentage
int moisture_sensor_get_percentage(int raw_value) {
    // if (raw_value >= DRY_AIR_ADC_VALUE) return 0;    
    // if (raw_value <= WATER_ADC_VALUE) return 100;   

    // Map ADC range to percentage
    // int percentage = 100 - ((raw_value - WATER_ADC_VALUE) * 100) / (DRY_AIR_ADC_VALUE - WATER_ADC_VALUE);
    // TODO: add dumy data for moisture percentage
    int percentage = 35;
    return percentage;
}

// Determine soil condition
const char* moisture_sensor_get_condition(int raw_value) {
    if (raw_value >= DRY_AIR_ADC_VALUE) return "Air (Dry)";
    if (raw_value >= DRY_SOIL_ADC_VALUE) return "Dry Soil";
    if (raw_value >= WET_SOIL_ADC_VALUE) return "Wet Soil";
    return "Waterlogged";
}
