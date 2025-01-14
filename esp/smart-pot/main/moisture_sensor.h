#ifndef MOISTURE_SENSOR_H
#define MOISTURE_SENSOR_H

void moisture_sensor_init(void);
int moisture_sensor_read(void);
int moisture_sensor_get_percentage(int raw_value);
const char* moisture_sensor_get_condition(int raw_value);

#endif // MOISTURE_SENSOR_H
