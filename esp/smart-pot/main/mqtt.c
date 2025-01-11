#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "globals.h"
#include "mqtt.h"
#include "nvs_storage.h"
#include "parameter.h"
#include "cJSON.h"
#include "time.h"

#define MAX_RETRY_COUNT 5
#define MQTT_RECONNECT_DELAY pdMS_TO_TICKS(5000)

#define WATER_REQUEST_TOPIC "%s/%s/soil_humidity/request"

volatile int air_humidity_delay = 30;
volatile int soil_humidity_delay = 30;
volatile int temperature_delay = 30;
volatile int insolation_delay = 30;

const char *MQTT = "MQTT";
static const char *MQTT_TEMPERATURE_TAG = "MQTT (temperature)";
static const char *MQTT_HUMIDITY_TAG = "MQTT (air humidity)";
static const char *MQTT_SOIL_HUMIDITY_TAG = "MQTT (soil humidity)";
static const char *MQTT_INSOLATION_TAG = "MQTT (insolation)";

static int mqtt_retry_count = 0;

// todo - update data from sensors
volatile float temperature = 10.5;
volatile float humidity = 30.5;
volatile float soil_humidity = 30.1;
volatile float insolation = 10.7;

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

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(MQTT, "MQTT_EVENT_CONNECTED");
        mqtt_connected = true;
        mqtt_retry_count = 0;

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
                if (watering_time_sec > 0) {
                    ESP_LOGI(MQTT, "Watering for %d seconds.", watering_time_sec);
                    // todo - implement logic for plant watering
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
                } else {
                    ESP_LOGW(MQTT_INSOLATION_TAG, "Invalid frequency value received: %s", message);
                }
                return;
            }
        }
        break;

    default:
        break;
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

        vTaskDelay(pdMS_TO_TICKS(temperature_delay));  
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

        vTaskDelay(pdMS_TO_TICKS(air_humidity_delay));  
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

        vTaskDelay(pdMS_TO_TICKS(soil_humidity_delay));  
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

        vTaskDelay(pdMS_TO_TICKS(insolation_delay));  
    }
}

static void mqtt_app_start(void)
{
    get_broker_url(saved_broker_url, broker_url_size);
    get_broker_username(saved_broker_username, broker_username_size);
    get_broker_password(saved_broker_password, broker_password_size);

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

    xTaskCreate(publish_temperature, "publish_temperature", 4096, (void*)mqtt_client, 5, NULL);
    xTaskCreate(publish_humidity, "publish_humidity", 4096, (void*)mqtt_client, 5, NULL);
    xTaskCreate(publish_soil_humidity, "publish_soil_humidity", 4096, (void*)mqtt_client, 5, NULL);
    xTaskCreate(publish_insolation, "publish_insolation", 4096, (void*)mqtt_client, 5, NULL);
}

void start_mqtt_task(void *pvParameters)
{
    while (!is_wifi_connected) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    ESP_LOGI("wifi", "WiFi connection established, connecting to MQTT...");
    
    mqtt_app_start(); 
    vTaskDelete(NULL); 
}
