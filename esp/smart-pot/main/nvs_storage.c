#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "nvs_storage.h"
#include "globals.h"
#include "parameter.h"

#define NVS_TEMPERATURE_NAMESPACE "temp_buf"
#define NVS_HUMIDITY_NAMESPACE "air_humidity_buf"
#define NVS_SOIL_HUMIDITY_NAMESPACE "soil_humidity_buf"
#define NVS_INSOLATION_NAMESPACE "insolation_buff"
#define NVS_INSOLATION_DIGITAL_NAMESPACE "insolation_digital_buff"


#define MAX_MESSAGES 10
#define NVS "NVS"

bool is_ssid_set = false;
bool is_password_set = false;
bool is_broker_url_set = false;
bool is_broker_username_set = false;
bool is_broker_password_set = false;

const char* get_namespace_name(enum Parameter param) {
    switch (param) {
        case TEMPERATURE:
            return NVS_TEMPERATURE_NAMESPACE;
        case HUMIDITY:
            return NVS_HUMIDITY_NAMESPACE;
        case SOIL_HUMIDITY:
            return NVS_SOIL_HUMIDITY_NAMESPACE;
        case INSOLATION:
            return NVS_INSOLATION_NAMESPACE;
        case INSOLATION_DIGITAL:
            return NVS_INSOLATION_DIGITAL_NAMESPACE;
        default:
            ESP_LOGE("NVS", "Invalid parameter provided");
            return NULL; 
    }
}

esp_err_t save_message_to_nvs(const char *message, enum Parameter param) {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    const char *namespace_name = get_namespace_name(param);

    if (namespace_name != NULL) {
        ESP_LOGI(NVS, "Using namespace: %s for parameter: %d", namespace_name, param);
    } else {
        ESP_LOGE(NVS, "Failed to get namespace for parameter: %d", param);
    }

    err = nvs_open(namespace_name, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(namespace_name, "Could not open buffer: %s", namespace_name);
        return err;
    }

    for (int i = 0; i < MAX_MESSAGES; i++) {
        char key[16];
        snprintf(key, sizeof(key), "msg_%d", i);

        size_t required_size;
        err = nvs_get_str(nvs_handle, key, NULL, &required_size);

        if (err == ESP_ERR_NVS_NOT_FOUND) {
            err = nvs_set_str(nvs_handle, key, message);
            if (err == ESP_OK) {
                ESP_LOGI(namespace_name, "Saved, id=%s", key);
                nvs_commit(nvs_handle);
            }
            nvs_close(nvs_handle);
            return err;
        }
    }

    ESP_LOGW(namespace_name, "Buffer full - cannot save more messages.");
    nvs_close(nvs_handle);
    return ESP_ERR_NO_MEM;
}

void resent_messages(const char *namespace_name, const char *topic, esp_mqtt_client_handle_t client) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(namespace_name, NVS_READWRITE, &nvs_handle);

    if (err != ESP_OK) {
        ESP_LOGE(namespace_name, "Could not open the buffer: %s", namespace_name);
        return;
    }

    for (int i = 0; i < MAX_MESSAGES; i++) {
        char key[16];
        snprintf(key, sizeof(key), "msg_%d", i);

        size_t required_size;
        err = nvs_get_str(nvs_handle, key, NULL, &required_size);

        if (err == ESP_OK) {
            char *message = malloc(required_size);
            if (message) {
                err = nvs_get_str(nvs_handle, key, message, &required_size);
                if (err == ESP_OK) {
                    int msg_id = esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
                    if (msg_id != -1) {
                        ESP_LOGI(MQTT, "Successfully sent message");
                        nvs_erase_key(nvs_handle, key);
                        nvs_commit(nvs_handle); 
                    } else {
                        ESP_LOGE(MQTT, "Could not resent message.");
                    }
                }
                free(message); 
            }
        } else {
            break; 
        }
    }

    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
}

void resend_messages_from_nvs(esp_mqtt_client_handle_t client) {
    ESP_LOGI("MQTT", "Sending messagess from buffer..");
    resent_messages(NVS_TEMPERATURE_NAMESPACE, TEMPERATURE_TOPIC, client);
    resent_messages(NVS_HUMIDITY_NAMESPACE, AIR_HUMIDITY_TOPIC, client);
}

esp_err_t save_wifi_ssid(const char *ssid) {
    nvs_handle_t my_handle;
    ESP_ERROR_CHECK(nvs_open("wifi_config", NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_str(my_handle, "ssid", ssid));
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
    is_ssid_set = true;

    return ESP_OK;
}

esp_err_t save_wifi_password(const char *password) {
    nvs_handle_t my_handle;
    ESP_ERROR_CHECK(nvs_open("wifi_config", NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_str(my_handle, "password", password));
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
    is_password_set = true;

    return ESP_OK;
}

esp_err_t save_broker_url(const char *url) {
    nvs_handle_t my_handle;
    ESP_ERROR_CHECK(nvs_open("broker_config", NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_str(my_handle, "url", url));
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
    is_broker_url_set = true;

    return ESP_OK;
}

esp_err_t save_user_mac(const char *mac) {
    nvs_handle_t my_handle;
    ESP_ERROR_CHECK(nvs_open("user_config", NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_str(my_handle, "mac", mac));
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);

    return ESP_OK;
}

esp_err_t save_broker_username(const char *username) {
    nvs_handle_t my_handle;
    ESP_ERROR_CHECK(nvs_open("broker_config", NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_str(my_handle, "username", username));
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
    is_broker_username_set = true;

    return ESP_OK;
}

esp_err_t save_broker_password(const char *password) {
    nvs_handle_t my_handle;
    ESP_ERROR_CHECK(nvs_open("broker_config", NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_str(my_handle, "password", password));
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
    is_broker_password_set = true;

    return ESP_OK;
}

esp_err_t get_wifi_ssid(char *ssid, size_t ssid_size) {
    nvs_handle_t my_handle;
    esp_err_t ret = nvs_open("wifi_config", NVS_READWRITE, &my_handle);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = nvs_get_str(my_handle, "ssid", ssid, &ssid_size);
    nvs_close(my_handle);
    return ret;
}

esp_err_t get_wifi_password(char *password, size_t password_size) {
    nvs_handle_t my_handle;
    esp_err_t ret = nvs_open("wifi_config", NVS_READWRITE, &my_handle);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = nvs_get_str(my_handle, "password", password, &password_size);
    nvs_close(my_handle);
    return ret;
}

esp_err_t get_broker_url(char *url, size_t url_size) {
    const char *hardcoded_url = "mqtt://192.168.9.125:1883"; // Replace with your broker URL
    if (strlen(hardcoded_url) >= url_size) {
        return ESP_ERR_INVALID_SIZE; // Ensure buffer is large enough
    }
    strncpy(url, hardcoded_url, url_size - 1);
    url[url_size - 1] = '\0'; // Null-terminate the string
    return ESP_OK;
}

esp_err_t get_broker_username(char *username, size_t username_size) {
    const char *hardcoded_username = "patryk"; // Replace with your username
    if (strlen(hardcoded_username) >= username_size) {
        return ESP_ERR_INVALID_SIZE; // Ensure buffer is large enough
    }
    strncpy(username, hardcoded_username, username_size - 1);
    username[username_size - 1] = '\0'; // Null-terminate the string
    return ESP_OK;
}

esp_err_t get_broker_password(char *password, size_t password_size) {
    const char *hardcoded_password = "Haslo1"; // Replace with your password
    if (strlen(hardcoded_password) >= password_size) {
        return ESP_ERR_INVALID_SIZE; // Ensure buffer is large enough
    }
    strncpy(password, hardcoded_password, password_size - 1);
    password[password_size - 1] = '\0'; // Null-terminate the string
    return ESP_OK;
}
esp_err_t get_user_mac(char *mac) {
    nvs_handle_t my_handle;
    size_t mac_size = 18;
    esp_err_t ret = nvs_open("user_config", NVS_READWRITE, &my_handle);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = nvs_get_str(my_handle, "mac", mac, &mac_size);
    nvs_close(my_handle);
    return ret;
}