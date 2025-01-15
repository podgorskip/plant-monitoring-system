#include "pump_control.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PUMP_GPIO GPIO_NUM_26
#define TAG "PUMP_CONTROL"

void pump_init(void) {
    gpio_reset_pin(PUMP_GPIO);
    gpio_set_direction(PUMP_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(PUMP_GPIO, 1); // Start z wyłączoną pompą
    ESP_LOGI(TAG, "Pump initialized on GPIO26");
}

void pump_on(void) {
    gpio_set_level(PUMP_GPIO, 0);
    ESP_LOGI(TAG, "Pump is ON");
}

void pump_off(void) {
    gpio_set_level(PUMP_GPIO, 1);
    ESP_LOGI(TAG, "Pump is OFF");
}
