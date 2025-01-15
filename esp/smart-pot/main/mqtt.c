#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "globals.h"
#include "mqtt.h"
#include "nvs_storage.h"
#include "parameter.h"
#include "cJSON.h"
#include "time.h"
#include "sensor_manager.h"
#include "bme280_sensor.h"
#include "moisture_sensor.h"
#include "water_sensor.h"
#include "light_sensor.h"
#include "pump_control.h"
#include "esp_log.h"


#define MAX_RETRY_COUNT 5
#define MQTT_RECONNECT_DELAY pdMS_TO_TICKS(5000)

#define MAX_PUMP_RUNTIME_MS 1000     // Max time the pump can run in one cycle (2 seconds)
#define PUMP_COOLDOWN_MS 10000       // Time to wait before re-checking moisture (60 seconds)
#define TAG "MAIN"

#define WATER_REQUEST_TOPIC "%s/%s/soil_humidity/request"

volatile int air_humidity_delay = 1;
volatile int soil_humidity_delay = 1;
volatile int temperature_delay = 1;
volatile int insolation_delay = 1;
volatile int insolation_digital_delay = 1;

volatile bool pump_state = false; // Current pump state

TaskHandle_t temperature_task_handle = NULL;
TaskHandle_t humidity_task_handle = NULL;
TaskHandle_t soil_humidity_task_handle = NULL;
TaskHandle_t insolation_task_handle = NULL;
TaskHandle_t insolation_digital_task_handle = NULL;



const char *MQTT = "MQTT";
static const char *MQTT_TEMPERATURE_TAG = "MQTT (temperature)";
static const char *MQTT_HUMIDITY_TAG = "MQTT (air humidity)";
static const char *MQTT_SOIL_HUMIDITY_TAG = "MQTT (soil humidity)";
static const char *MQTT_INSOLATION_TAG = "MQTT (insolation)";
static const char *MQTT_INSOLATION_DIGITAL_TAG = "MQTT (insolation digital)";

static int mqtt_retry_count = 0;

// todo - update data from sensors
float temperature = 0.0;
float humidity = 0.0;
float soil_humidity = 0.0;
float insolation = 0.0;
int insolation_digital = 0;
float water_level = 0.0;

bool mqtt_connected = false;

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

char saved_broker_url[128] = {0};
char saved_broker_username[64] = {0};
char saved_broker_password[64] = {0};
size_t broker_url_size = sizeof(saved_broker_url);
size_t broker_username_size = sizeof(saved_broker_username);
size_t broker_password_size = sizeof(saved_broker_password);

esp_mqtt_client_handle_t mqtt_client = NULL;

esp_mqtt_client_handle_t get_mqtt_client() {
    return mqtt_client;
}

void update_sensor_data(void) {
    ESP_LOGI(TAG, "Updating sensor data...");

    // Read temperature from BME280
    if (bme280_get_temperature(&temperature) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to read temperature.");
    }

    // Read humidity from BME280
    if (bme280_get_humidity(&humidity) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to read humidity.");
    }

    // Read soil humidity from moisture sensor
    soil_humidity = moisture_sensor_get_percentage(moisture_sensor_read());

    // Read light intensity (analog value)
    insolation = light_sensor_get_brightness_percentage();

    insolation_digital = light_sensor_get_digital_state();

    // Read water level (analog value)
    water_level = water_sensor_read();


    // Log updated values
    ESP_LOGI(TAG, "Sensor Data → Temp: %.2f°C, Humidity: %.2f%%, Soil Moisture: %.2f%%, Insolation: %.2f, Water Level: %.2f, Insolation digital: %d",
             temperature, humidity, soil_humidity, insolation, water_level, insolation_digital);
}

void sensor_task(void *pvParameter) {
    while (1) {
        update_sensor_data();
        vTaskDelay(pdMS_TO_TICKS(5000));  // Update every 5 seconds
    }
}


void create_topic_with_frequency(char *output, size_t output_size, const char *base_topic, const char *user_mac, const char *device_mac, bool add_frequency) {
    snprintf(output, output_size, base_topic, user_mac, device_mac);
    if (add_frequency) {
        strncat(output, "/frequency", output_size - strlen(output) - 1);
    }
}

static void mqtt_reconnect_with_backoff(esp_mqtt_client_handle_t client) {
    int retry_delay = 1000; 
    for (int attempt = 1; attempt <= MAX_RETRY_COUNT; attempt++) {
        if (!mqtt_connected) {
            ESP_LOGI(MQTT, "%d attempt to connect to the MQTT broker...", attempt);
            esp_mqtt_client_reconnect(client);
            if (mqtt_connected) break;

            vTaskDelay(retry_delay / portTICK_PERIOD_MS);
            retry_delay = MIN(retry_delay * 2, 32000);
        } else {
            ESP_LOGI(MQTT, "Successfully connected");
            return;
        }
    }
    if (!mqtt_connected) {
        ESP_LOGE(MQTT, "Maximum number of MQTT connection attempts reached.");
    }
}


static void publish_temperature(void *pvParameters)
{
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)pvParameters;
    char *payload = NULL;
    char topic[100];
    sprintf(topic, TEMPERATURE_TOPIC, user_mac, device_mac);

    while (1) {
        if (mqtt_connected) {
            time_t now;
            time(&now);
            struct tm timeinfo;
            localtime_r(&now, &timeinfo);

            cJSON *json = cJSON_CreateObject();
            cJSON_AddNumberToObject(json, "value", temperature);
            cJSON_AddStringToObject(json, "unit", "C");
            cJSON_AddNumberToObject(json, "timestamp", (int)now);

            payload = cJSON_Print(json);

            ESP_LOGI("payload", "Payload: %s", payload);
            ESP_LOGI("topic", "Topic: %s", topic);


            int msg_id = esp_mqtt_client_publish(client, topic, payload, 0, 1, 0);
            if (msg_id == -1) {
                ESP_LOGE(MQTT_TEMPERATURE_TAG, "Failed to publish message.");
            } else {
                ESP_LOGI(MQTT_TEMPERATURE_TAG, "Published: %s", payload);
            }

            cJSON_Delete(json); 
            free(payload);

        } else {
            ESP_LOGW(MQTT_TEMPERATURE_TAG, "No MQTT connection. Starting buffering.");
            save_message_to_nvs(payload, TEMPERATURE);
        }

        vTaskDelay(pdMS_TO_TICKS(temperature_delay * 10 * 1000));  
    }
}

static void publish_humidity(void *pvParameters)
{
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)pvParameters;
    char *payload = NULL;
    char topic[100];
    snprintf(topic, sizeof(topic), AIR_HUMIDITY_TOPIC, user_mac, device_mac);  

    while (1) {
        if (mqtt_connected) {
            time_t now;
            time(&now);
            struct tm timeinfo;
            localtime_r(&now, &timeinfo);

            cJSON *json = cJSON_CreateObject();
            cJSON_AddNumberToObject(json, "value", humidity);
            cJSON_AddStringToObject(json, "unit", "%");
            cJSON_AddNumberToObject(json, "timestamp", (int)now);

            payload = cJSON_Print(json);

            int msg_id = esp_mqtt_client_publish(client, topic, payload, 0, 1, 0);
            if (msg_id == -1) {
                ESP_LOGE(MQTT_HUMIDITY_TAG, "Failed to publish message.");
            } else {
                ESP_LOGI(MQTT_HUMIDITY_TAG, "Published: %s", payload);
            }

            cJSON_Delete(json); 
            free(payload);
        } else {
            ESP_LOGW(MQTT_HUMIDITY_TAG, "No MQTT connection. Starting buffering.");
            save_message_to_nvs(payload, HUMIDITY);
        }

        vTaskDelay(pdMS_TO_TICKS(air_humidity_delay * 10 * 1000));  
    }
}

static void publish_soil_humidity(void *pvParameters)
{
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)pvParameters;
    char *payload = NULL;
    char topic[100];
    snprintf(topic, sizeof(topic), SOIL_HUMIDITY_TOPIC, user_mac, device_mac);  

    while (1) {
        if (mqtt_connected) {
            time_t now;
            time(&now);
            struct tm timeinfo;
            localtime_r(&now, &timeinfo);

            cJSON *json = cJSON_CreateObject();
            cJSON_AddNumberToObject(json, "value", soil_humidity);
            cJSON_AddStringToObject(json, "unit", "%");
            cJSON_AddNumberToObject(json, "timestamp", (int)now);

            payload = cJSON_Print(json);

            int msg_id = esp_mqtt_client_publish(client, topic, payload, 0, 1, 0);
            if (msg_id == -1) {
                ESP_LOGE(MQTT_SOIL_HUMIDITY_TAG, "Failed to publish message.");
            } else {
                ESP_LOGI(MQTT_SOIL_HUMIDITY_TAG, "Published: %s", payload);
            }

            cJSON_Delete(json); 
            free(payload);
        } else {
            ESP_LOGW(MQTT_SOIL_HUMIDITY_TAG, "No MQTT connection. Starting buffering.");
            save_message_to_nvs(payload, SOIL_HUMIDITY);
        }

        vTaskDelay(pdMS_TO_TICKS(soil_humidity_delay * 10 * 1000));  
    }
}

static void publish_insolation(void *pvParameters)
{
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)pvParameters;
    char *payload = NULL;
    char topic[100];
    snprintf(topic, sizeof(topic), INSOLATION_TOPIC, user_mac, device_mac);  

    while (1) {
        if (mqtt_connected) {
            time_t now;
            time(&now);
            struct tm timeinfo;
            localtime_r(&now, &timeinfo);

            cJSON *json = cJSON_CreateObject();
            cJSON_AddNumberToObject(json, "value", insolation);
            cJSON_AddStringToObject(json, "unit", "%");
            cJSON_AddNumberToObject(json, "timestamp", (int)now);

            payload = cJSON_Print(json);

            int msg_id = esp_mqtt_client_publish(client, topic, payload, 0, 1, 0);
            if (msg_id == -1) {
                ESP_LOGE(MQTT_INSOLATION_TAG, "Failed to publish message.");
            } else {
                ESP_LOGI(MQTT_INSOLATION_TAG, "Published: %s", payload);
            }

            cJSON_Delete(json); 
            free(payload);
        } else {
            ESP_LOGW(MQTT_INSOLATION_TAG, "No MQTT connection. Starting buffering.");
            save_message_to_nvs(payload, INSOLATION);
        }

        vTaskDelay(pdMS_TO_TICKS(insolation_delay * 10 * 1000));  
    }
}

static void publish_insolation_digital(void *pvParameters)
{
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)pvParameters;
    char *payload = NULL;
    char topic[100];
    snprintf(topic, sizeof(topic), INSOLATION_DIGITAL_TOPIC, user_mac, device_mac);  

    while (1) {
        if (mqtt_connected) {
            time_t now;
            time(&now);
            struct tm timeinfo;
            localtime_r(&now, &timeinfo);

            cJSON *json = cJSON_CreateObject();
            cJSON_AddNumberToObject(json, "value", insolation_digital);
            cJSON_AddStringToObject(json, "unit", "int");
            cJSON_AddNumberToObject(json, "timestamp", (int)now);

            payload = cJSON_Print(json);

            int msg_id = esp_mqtt_client_publish(client, topic, payload, 0, 1, 0);
            if (msg_id == -1) {
                ESP_LOGE(MQTT_INSOLATION_DIGITAL_TAG, "Failed to publish message.");
            } else {
                ESP_LOGI(MQTT_INSOLATION_DIGITAL_TAG, "Published: %s", payload);
            }

            cJSON_Delete(json); 
            free(payload);
        } else {
            ESP_LOGW(MQTT_INSOLATION_DIGITAL_TAG, "No MQTT connection. Starting buffering.");
            save_message_to_nvs(payload, INSOLATION_DIGITAL);
        }

        vTaskDelay(pdMS_TO_TICKS(insolation_digital_delay * 10 * 1000));  
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(MQTT, "MQTT_EVENT_CONNECTED");
        mqtt_connected = true;
        mqtt_retry_count = 0;

        char topic[256];

        snprintf(topic, sizeof(topic), WATER_REQUEST_TOPIC, user_mac, device_mac);
        esp_mqtt_client_subscribe(mqtt_client, topic, 1);
        ESP_LOGI(MQTT_SOIL_HUMIDITY_TAG, "Subscribed to water request topic: %s", topic);

        create_topic_with_frequency(topic, sizeof(topic), SOIL_HUMIDITY_TOPIC, user_mac, device_mac, true);
        esp_mqtt_client_subscribe(mqtt_client, topic, 1);
        ESP_LOGI(MQTT_SOIL_HUMIDITY_TAG, "Subscribed to soil moist frequency change topic: %s", topic);

        create_topic_with_frequency(topic, sizeof(topic), TEMPERATURE_TOPIC, user_mac, device_mac, true);
        esp_mqtt_client_subscribe(mqtt_client, topic, 1);
        ESP_LOGI(MQTT_TEMPERATURE_TAG, "Subscribed to temperature frequency change topic: %s", topic);

        create_topic_with_frequency(topic, sizeof(topic), AIR_HUMIDITY_TOPIC, user_mac, device_mac, true);
        esp_mqtt_client_subscribe(mqtt_client, topic, 1);
        ESP_LOGI(MQTT_HUMIDITY_TAG, "Subscribed to air humidity frequency change topic: %s", topic);

        create_topic_with_frequency(topic, sizeof(topic), INSOLATION_TOPIC, user_mac, device_mac, true);
        esp_mqtt_client_subscribe(mqtt_client, topic, 1);
        ESP_LOGI(MQTT_INSOLATION_TAG, "Subscribed to insolation frequency change topic: %s", topic);

        create_topic_with_frequency(topic, sizeof(topic), INSOLATION_DIGITAL_TOPIC, user_mac, device_mac, true);
        esp_mqtt_client_subscribe(mqtt_client, topic, 1);
        ESP_LOGI(MQTT_INSOLATION_DIGITAL_TAG, "Subscribed to insolation digital frequency change topic: %s", topic);

        resend_messages_from_nvs(client);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(MQTT, "MQTT_EVENT_DISCONNECTED");

        mqtt_connected = false;

        if (!is_wifi_connected) {
            ESP_LOGW(MQTT, "No Wi-Fi connection. MQTT reconnection delayed.");
            break;
        }

        ESP_LOGI(MQTT, "Retrying connection to the MQTT broker...");
        mqtt_reconnect_with_backoff(client);  
        break;

    case MQTT_EVENT_SUBSCRIBED:
        if (event->topic) {
            ESP_LOGI(MQTT, "Subscribed to topic %s", event->topic);
        } else {
            ESP_LOGW(MQTT, "Subscribed to topic: <no topic>");
        }
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(MQTT, "Message published successfully, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(MQTT, "event->topic_len: %d, event->data_len: %d", event->topic_len, event->data_len);

        if (event->topic_len > 0 && event->data_len > 0)
        {
            char topic[event->topic_len + 1];
            char message[event->data_len + 1];

            strncpy(topic, event->topic, event->topic_len);
            topic[event->topic_len] = '\0';

            strncpy(message, event->data, event->data_len);
            message[event->data_len] = '\0';

            ESP_LOGI(MQTT, "Received message on topic: %s -> %s", topic, message);

            char expected_topic[128];
            snprintf(expected_topic, sizeof(expected_topic), WATER_REQUEST_TOPIC, user_mac, device_mac);

            if (strcmp(topic, expected_topic) == 0) {
                ESP_LOGI(MQTT, "Handling water request: %s", message);
                int watering_time_sec = atoi(message); 
                if (watering_time_sec > 0 && water_level > 20) {
                    ESP_LOGI(MQTT, "Watering for %d seconds.", watering_time_sec);
                    pump_on();
                    pump_state = true;  
                    vTaskDelay(pdMS_TO_TICKS(1000*watering_time_sec));  
                    pump_off();
                    pump_state = false; 
                } else {
                    ESP_LOGW(MQTT, "Invalid watering time received: %s", message);
                }
                return;
            }

            char frequency_topic[128];
            create_topic_with_frequency(frequency_topic, sizeof(frequency_topic), SOIL_HUMIDITY_TOPIC, user_mac, device_mac, true);

            if (strcmp(topic, frequency_topic) == 0) {
                ESP_LOGI(MQTT_SOIL_HUMIDITY_TAG, "Handling soil moisture frequency change request: %s", message);
                int new_delay = atoi(message); 
                if (new_delay > 0) {
                    soil_humidity_delay = new_delay; 
                    ESP_LOGI(MQTT_SOIL_HUMIDITY_TAG, "Soil moisture frequency updated to %d minutes.", soil_humidity_delay);

                    // Stop and restart the soil humidity task
                    if (soil_humidity_task_handle != NULL) {
                        vTaskDelete(soil_humidity_task_handle);
                    }
                    xTaskCreate(publish_soil_humidity, "publish_soil_humidity", 4096, (void*)mqtt_client, 5, &soil_humidity_task_handle);

                } else {
                    ESP_LOGW(MQTT_SOIL_HUMIDITY_TAG, "Invalid frequency value received: %s", message);
                }
                return;
            }

            create_topic_with_frequency(frequency_topic, sizeof(frequency_topic), TEMPERATURE_TOPIC, user_mac, device_mac, true);

            if (strcmp(topic, frequency_topic) == 0) {
                ESP_LOGI(MQTT, "Handling temperature frequency change request: %s", message);
                int new_delay = atoi(message); 
                if (new_delay > 0) {
                    temperature_delay = new_delay; 
                    ESP_LOGI(MQTT_TEMPERATURE_TAG, "Temperature frequency updated to %d minutes.", temperature_delay);

                    if (temperature_task_handle != NULL) {
                        vTaskDelete(temperature_task_handle);
                    }
                    xTaskCreate(publish_temperature, "publish_temperature", 4096, (void*)mqtt_client, 5, &temperature_task_handle);

                } else {
                    ESP_LOGW(MQTT_TEMPERATURE_TAG, "Invalid frequency value received: %s", message);
                }
                return;
            }

            create_topic_with_frequency(frequency_topic, sizeof(frequency_topic), AIR_HUMIDITY_TOPIC, user_mac, device_mac, true);

            if (strcmp(topic, frequency_topic) == 0) {
                ESP_LOGI(MQTT_HUMIDITY_TAG, "Handling air humidity frequency change request: %s", message);
                int new_delay = atoi(message); 
                if (new_delay > 0) {
                    air_humidity_delay = new_delay; 
                    ESP_LOGI(MQTT_HUMIDITY_TAG, "Air humidity frequency updated to %d minutes.", air_humidity_delay);

                    if (humidity_task_handle != NULL) {
                        vTaskDelete(humidity_task_handle);
                    }
                    xTaskCreate(publish_humidity, "publish_humidity", 4096, (void*)mqtt_client, 5, &humidity_task_handle);

                } else {
                    ESP_LOGW(MQTT_HUMIDITY_TAG, "Invalid frequency value received: %s", message);
                }
                return;
            }

            create_topic_with_frequency(frequency_topic, sizeof(frequency_topic), INSOLATION_TOPIC, user_mac, device_mac, true);

            if (strcmp(topic, frequency_topic) == 0) {
                ESP_LOGI(MQTT_INSOLATION_TAG, "Handling insolation frequency change request: %s", message);
                int new_delay = atoi(message); 
                if (new_delay > 0) {
                    insolation_delay = new_delay; 
                    ESP_LOGI(MQTT_INSOLATION_TAG, "Insolation frequency updated to %d minutes.", insolation_delay);

                    if (insolation_task_handle != NULL) {
                        vTaskDelete(insolation_task_handle);
                    }
                    xTaskCreate(publish_insolation, "publish_insolation", 4096, (void*)mqtt_client, 5, &insolation_task_handle);

                } else {
                    ESP_LOGW(MQTT_INSOLATION_TAG, "Invalid frequency value received: %s", message);
                }
                return;
            }

            create_topic_with_frequency(frequency_topic, sizeof(frequency_topic), INSOLATION_DIGITAL_TOPIC, user_mac, device_mac, true);

            if (strcmp(topic, frequency_topic) == 0) {
                ESP_LOGI(MQTT_INSOLATION_DIGITAL_TAG, "Handling insolation digital frequency change request: %s", message);
                int new_delay = atoi(message); 
                if (new_delay > 0) {
                    insolation_digital_delay = new_delay; 
                    ESP_LOGI(MQTT_INSOLATION_DIGITAL_TAG, "Insolation digital frequency updated to %d minutes.", insolation_digital_delay);

                    if (insolation_digital_task_handle != NULL) {
                        vTaskDelete(insolation_digital_task_handle);
                    }
                    xTaskCreate(publish_insolation_digital, "publish_insolation_digital", 4096, (void*)mqtt_client, 5, &insolation_digital_task_handle);

                } else {
                    ESP_LOGW(MQTT_INSOLATION_DIGITAL_TAG, "Invalid frequency value received: %s", message);
                }
                return;
            }
        } 
        break;

    default:
        break;
    }
}

static void mqtt_app_start(void)
{
    get_broker_url(saved_broker_url, broker_url_size);
    get_broker_username(saved_broker_username, broker_username_size);
    get_broker_password(saved_broker_password, broker_password_size);

    ESP_LOGI(TAG, "Connecting to MQTT broker: %s", saved_broker_url);
    ESP_LOGI(TAG, "Username: %s", saved_broker_username);
    ESP_LOGI(TAG, "Password: %s", saved_broker_password);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = saved_broker_url
            }
        },
        .credentials = {
            .username = saved_broker_username,
            .authentication = {
                .password = saved_broker_password
            }
        },
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler,  NULL);
    esp_mqtt_client_start(mqtt_client);

    mqtt_connected = true;
    pump_init();
    xTaskCreate(sensor_task, "Sensor Task", 4096, NULL, 5, NULL);
    xTaskCreate(publish_temperature, "publish_temperature", 4096, (void*)mqtt_client, 5, NULL);
    xTaskCreate(publish_humidity, "publish_humidity", 4096, (void*)mqtt_client, 5, NULL);
    xTaskCreate(publish_soil_humidity, "publish_soil_humidity", 4096, (void*)mqtt_client, 5, NULL);
    xTaskCreate(publish_insolation, "publish_insolation", 4096, (void*)mqtt_client, 5, NULL);
    xTaskCreate(publish_insolation_digital, "publish_insolation_digital", 4096, (void*)mqtt_client, 5, NULL);
}

void start_mqtt_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Starting MQTT task...");
    while (!is_wifi_connected) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    ESP_LOGI("wifi", "WiFi connection established, connecting to MQTT...");
    
    mqtt_app_start(); 
    vTaskDelete(NULL); 
}
