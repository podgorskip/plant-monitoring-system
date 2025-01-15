#include <stdbool.h>

#ifndef WATER_SENSOR_H
#define WATER_SENSOR_H

void water_sensor_init(void);
int water_sensor_read(void); // Returns the raw ADC value
bool water_sensor_is_low(void); // Returns true if water is below threshold

#endif // WATER_SENSOR_H
