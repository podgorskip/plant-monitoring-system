#ifndef GLOBALS_H
#define GLOBALS_H

extern volatile bool is_wifi_connected; 
extern const char *MQTT;
extern const char *WI_FI;
extern char user_mac[18];
extern char device_mac[18];

#define TEMPERATURE_TOPIC "%s/%s/temperature"
#define HUMIDITY_TOPIC "%s/%s/humidity"

#endif