#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "nvs_storage.h"
#include "globals.h"
#include "parameter.h"

#define NVS_TEMPERATURE_NAMESPACE "temp_buf"
#define NVS_HUMIDITY_NAMESPACE "humidity_buf"
#define MAX_MESSAGES 10
#define NVS "NVS"

bool is_ssid_set = false;
bool is_password_set = false;
bool is_broker_url_set = false;
bool is_broker_username_set = false;
bool is_broker_password_set = false;

esp_err_t save_message_to_nvs(const char *message, enum Parameter param) {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    const char *namespace_name = (param == TEMPERATURE) ? 
        NVS_TEMPERATURE_NAMESPACE : NVS_HUMIDITY_NAMESPACE;

    err = nvs_open(namespace_name, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(namespace_name, "Nie udało się otworzyć bufora: %s", namespace_name);
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
                ESP_LOGI(namespace_name, "Zapisano z id: %s", key);
                nvs_commit(nvs_handle);
            }
            nvs_close(nvs_handle);
            return err;
        }
    }

    ESP_LOGW(namespace_name, "Bufor pełny - brak mozliwości zapisywania dalszych wiadomości.");
    nvs_close(nvs_handle);
    return ESP_ERR_NO_MEM;
}

void resent_messages(const char *namespace_name, const char *topic, esp_mqtt_client_handle_t client) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(namespace_name, NVS_READWRITE, &nvs_handle);

    if (err != ESP_OK) {
        ESP_LOGE(namespace_name, "Nie udało się otworzyć bufora: %s", namespace_name);
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
                        ESP_LOGI(MQTT, "Pomyślnie ponowiono wiadomość");
                        nvs_erase_key(nvs_handle, key);
                        nvs_commit(nvs_handle); 
                    } else {
                        ESP_LOGE(MQTT, "Nie udało się ponowić wiadomości.");
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
    ESP_LOGI("MQTT", "Wysyłanie danych z bufora...");
    resent_messages(NVS_TEMPERATURE_NAMESPACE, TEMPERATURE_TOPIC, client);
    resent_messages(NVS_HUMIDITY_NAMESPACE, HUMIDITY_TOPIC, client);
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
    nvs_handle_t my_handle;
    esp_err_t ret = nvs_open("broker_config", NVS_READWRITE, &my_handle);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = nvs_get_str(my_handle, "url", url, &url_size);
    nvs_close(my_handle);
    return ret;
}

esp_err_t get_broker_username(char *username, size_t username_size) {
    nvs_handle_t my_handle;
    esp_err_t ret = nvs_open("broker_config", NVS_READWRITE, &my_handle);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = nvs_get_str(my_handle, "username", username, &username_size);
    nvs_close(my_handle);
    return ret;
}

esp_err_t get_broker_password(char *password, size_t password_size) {
    nvs_handle_t my_handle;
    esp_err_t ret = nvs_open("broker_config", NVS_READWRITE, &my_handle);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = nvs_get_str(my_handle, "password", password, &password_size);
    nvs_close(my_handle);
    return ret;
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