#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "globals.h"
#include "mqtt.h"
#include "nvs_storage.h"
#include "parameter.h"
#include "/Users/patryk.podgorski/esp/esp-idf/components/cJSON/cJSON.h"
#include "time.h"

#define MAX_RETRY_COUNT 5
#define MQTT_RECONNECT_DELAY pdMS_TO_TICKS(5000)

const char *MQTT = "MQTT";
static const char *MQTT_TEMPERATURE_TAG = "MQTT (temperature)";
static const char *MQTT_HUMIDITY_TAG = "MQTT (humidity)";
static int mqtt_retry_count = 0;

static float temperature = 10.5;
static float humidity = 30.5;

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

        temperature = temperature + 0.2;

        vTaskDelay(pdMS_TO_TICKS(20000));  
    }
}

static void publish_humidity(void *pvParameters)
{
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)pvParameters;
    char *payload = NULL;
    char topic[100];
    snprintf(topic, sizeof(topic), HUMIDITY_TOPIC, user_mac, device_mac);  

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
                ESP_LOGI("MQTT", "Received topic: %s", topic);
            }

            cJSON_Delete(json); 
            free(payload);
        } else {
            ESP_LOGW(MQTT_HUMIDITY_TAG, "No MQTT connection. Starting buffering.");
            save_message_to_nvs(payload, HUMIDITY);
        }

        humidity = humidity + 1;

        vTaskDelay(pdMS_TO_TICKS(20000));  
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

    xTaskCreate(publish_temperature, "publish_temperature", 4096, (void*)mqtt_client, 5, NULL);
    xTaskCreate(publish_humidity, "publish_humidity", 4096, (void*)mqtt_client, 5, NULL);
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
