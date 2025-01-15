#include "light_sensor.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"

#define TAG "LIGHT_SENSOR"
#define LIGHT_SENSOR_DIGITAL_PIN GPIO_NUM_27  // Example digital pin
#define LIGHT_SENSOR_ANALOG_CHANNEL ADC1_CHANNEL_4  // GPIO32

void light_sensor_init(void) {
    // Initialize digital input
    gpio_reset_pin(LIGHT_SENSOR_DIGITAL_PIN);
    gpio_set_direction(LIGHT_SENSOR_DIGITAL_PIN, GPIO_MODE_INPUT);

    // Initialize analog input
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(LIGHT_SENSOR_ANALOG_CHANNEL, ADC_ATTEN_DB_11);
    ESP_LOGI(TAG, "Light sensor initialized");
}

bool light_sensor_get_digital_state(void) {
    // return gpio_get_level(LIGHT_SENSOR_DIGITAL_PIN);
    return 1;
}

const char* light_sensor_get_digital_condition(void) {
    return light_sensor_get_digital_state() ? "Bright" : "Dark";
}

int light_sensor_read_analog(void) {
    int raw_value = adc1_get_raw(LIGHT_SENSOR_ANALOG_CHANNEL);
    return raw_value;
}

float light_sensor_get_brightness_percentage(void) {
    int raw_value = light_sensor_read_analog();
    // float brightness_percentage = (1.0f - ((float)raw_value / 4095.0f)) * 100.0f;
    // TODO: add dummy data for brightness percentage
    float brightness_percentage = 80.0f;
    return brightness_percentage;
}
