#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <stdbool.h>  // Required for 'bool' type

void light_sensor_init(void);
bool light_sensor_get_digital_state(void);  // Ensure this matches the definition
const char* light_sensor_get_digital_condition(void);
int light_sensor_read_analog(void);
float light_sensor_get_brightness_percentage(void);

#endif // LIGHT_SENSOR_H
