#ifndef MQTT_H
#define MQTT_H

#include <stddef.h>
#include "esp_err.h"

void start_mqtt_task(void *pvParameters); 
void mqtt_stop(esp_mqtt_client_handle_t);

extern bool mqtt_connected;
extern esp_mqtt_client_handle_t mqtt_client;

extern char saved_broker_url[128];
extern char saved_broker_username[64];
extern char saved_broker_password[64];
extern size_t broker_url_size;
extern size_t broker_username_size;
extern size_t broker_password_size;

#endif
