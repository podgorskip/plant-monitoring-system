#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

void sensor_manager_init(void);
void sensor_manager_read_all(void);
int sensor_manager_read_moisture(void);
int sensor_manager_read_water_level(void); 
float sensor_manager_read_temperature(void);  // New
float sensor_manager_read_humidity(void);    // New
float sensor_manager_read_pressure(void);    // New
int sensor_manager_read_light_analog(void);  // New

#endif // SENSOR_MANAGER_H
