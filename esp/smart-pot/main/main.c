#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "driver/gpio.h"
#include "esp_http_client.h"
#include "mqtt_client.h"
#include "driver/gpio.h"
#include "globals.h"
#include "mqtt.h"
#include "nvs_storage.h"
#include "esp_http_server.h"
#include "esp_sntp.h"
#include "esp_mac.h"
#include "/Users/patryk.podgorski/esp/esp-idf/components/bt/host/bluedroid/api/include/api/esp_gatt_common_api.h"
#include "/Users/patryk.podgorski/esp/esp-idf/components/bt/include/esp_bt.h"
#include "/Users/patryk.podgorski/esp/esp-idf/components/bt/host/bluedroid/api/include/api/esp_gap_ble_api.h"
#include "/Users/patryk.podgorski/esp/esp-idf/components/bt/host/bluedroid/api/include/api/esp_gatts_api.h"
#include "/Users/patryk.podgorski/esp/esp-idf/components/bt/host/bluedroid/api/include/api/esp_bt_main.h"

// GPIO for LED
#define LED_GPIO_PIN GPIO_NUM_2
#define RESET_BUTTON_GPIO GPIO_NUM_0 

// Event group for Wi-Fi
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

// Prototypes
void wifi_init_sta(void);
void initialize_sntp(void);

esp_err_t save_wifi_ssid(const char *ssid);
esp_err_t save_wifi_password(const char *password);
esp_err_t save_broker_url(const char *url);
esp_err_t save_broker_username(const char *username);
esp_err_t save_broker_password(const char *password);

// BLE config
#define GATTS_SERVICE_UUID   0xABCD
#define GATTS_CHAR_UUID      0x1234
#define GATTS_NUM_HANDLE 15
#define CHAR_VAL_LEN_MAX     128
#define UPDATE_INTERVAL_MS   15000 

esp_bt_uuid_t SSID_CHAR_UUID = { .len = ESP_UUID_LEN_16, .uuid.uuid16 = 0xFF01 };   
esp_bt_uuid_t PASSWORD_CHAR_UUID = { .len = ESP_UUID_LEN_16, .uuid.uuid16 = 0xFF02 }; 
esp_bt_uuid_t BROKER_URL_CHAR_UUID = { .len = ESP_UUID_LEN_16, .uuid.uuid16 = 0xFF03 }; 
esp_bt_uuid_t BROKER_USERNAME_CHAR_UUID = { .len = ESP_UUID_LEN_16, .uuid.uuid16 = 0xFF04 }; 
esp_bt_uuid_t BROKER_PASSWORD_CHAR_UUID = { .len = ESP_UUID_LEN_16, .uuid.uuid16 = 0xFF05 }; 
esp_bt_uuid_t READ_CHAR_UUID = { .len = ESP_UUID_LEN_16, .uuid.uuid16 = 0xFF06 }; 

static uint16_t conn_id = 0;
static esp_gatt_if_t gatts_if = 0;
static uint16_t service_handle = 0;

static uint8_t ssid_value[CHAR_VAL_LEN_MAX] = {0};
static uint8_t password_value[CHAR_VAL_LEN_MAX] = {0};
static uint8_t broker_url[CHAR_VAL_LEN_MAX] = {0};
static uint8_t broker_username[CHAR_VAL_LEN_MAX] = {0};
static uint8_t broker_password[CHAR_VAL_LEN_MAX] = {0};
char read_value[CHAR_VAL_LEN_MAX];

uint16_t ssid_handle;
uint16_t password_handle;
uint16_t broker_url_handle;
uint16_t broker_username_handle;
uint16_t broker_password_handle;

static esp_bd_addr_t remote_bda = {0};
char user_mac[18] = "XX:XX:XX:XX:XX:XX";
char device_mac[18] = "XX:XX:XX:XX:XX:XX";

esp_attr_control_t ssid_control = {
    .auto_rsp = ESP_GATT_AUTO_RSP,
};

esp_attr_control_t password_control = {
    .auto_rsp = ESP_GATT_AUTO_RSP,
};

esp_attr_control_t broker_url_control = {
    .auto_rsp = ESP_GATT_AUTO_RSP,
};

esp_attr_control_t broker_password_control = {
    .auto_rsp = ESP_GATT_AUTO_RSP,
};

esp_attr_control_t broker_username_control = {
    .auto_rsp = ESP_GATT_AUTO_RSP,
};

esp_attr_control_t read_control = {
    .auto_rsp = ESP_GATT_AUTO_RSP,
};

esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// Task handles
static TaskHandle_t s_blink_task_handle = NULL;

// Wi-Fi connection status
volatile bool is_wifi_connected = false;
volatile bool has_wifi_connected = false;

volatile bool configuration_mode = false;

esp_err_t ret;

wifi_config_t wifi_config = {
    .sta = {
        .ssid = "default_SSID",       
        .password = "default_pass",  
    },
};

// Tasks
static const char *BLE_GATT = "BLE_GATT";
static const char *BLE_GAP = "BLE_GAP";
static const char *BLE = "BLE";
static const char *SNTP = "SNTP";
static const char *WIFI = "WIFI";
static const char *BROKER = "BROKER";

// GPIO initialization
static void led_init(void)
{
    gpio_reset_pin(LED_GPIO_PIN);
    gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO_PIN, 0);
}

// Blink LED task
static void blink_task(void *pvParameter)
{
    while (1) {
        gpio_set_level(LED_GPIO_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LED_GPIO_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));

        if (s_blink_task_handle == NULL) {
            break; 
        }
    }

    vTaskDelete(NULL);
}

void mac_to_string(const esp_bd_addr_t mac, char *mac_str, size_t size) {
    snprintf(mac_str, size, "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void initialise_mac_address(void) {
    uint8_t mac[6];
    esp_err_t ret = esp_efuse_mac_get_default(mac);

    if (ret == ESP_OK) {
        snprintf(device_mac, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        ESP_LOGI("MAC_ADDRESS", "Device MAC address: %s", device_mac);
    } else {
        ESP_LOGE("MAC_ADDRESS", "Failed to get MAC address: %s", esp_err_to_name(ret));
    }
}

void ble_gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatt_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
       case ESP_GATTS_WRITE_EVT:
            ESP_LOGI(BLE_GATT, "Received write event, handle: %d", param->write.handle);
            
            if (param->write.handle == ssid_handle) {
                ESP_LOGI(BLE_GATT, "SSID received: %s", param->write.value);
                strncpy((char *)wifi_config.sta.ssid, (char *)param->write.value, sizeof(wifi_config.sta.ssid) - 1);
                ESP_LOGI(BLE_GATT, "Updated SSID: %s", wifi_config.sta.ssid);
                save_wifi_ssid((const char *)wifi_config.sta.ssid);
            } else if (param->write.handle == password_handle) {
                ESP_LOGI(BLE_GATT, "Password received: %s", param->write.value);
                strncpy((char *)wifi_config.sta.password, (char *)param->write.value, sizeof(wifi_config.sta.password) - 1);
                ESP_LOGI(BLE_GATT, "Updated Password: %s", wifi_config.sta.password);
                save_wifi_password((const char *)wifi_config.sta.password);
            } else if (param->write.handle == broker_url_handle) {
                ESP_LOGI(BLE_GATT, "Broker url received: %s", param->write.value);
                save_broker_url((const char *)param->write.value);
            } else if (param->write.handle == broker_username_handle) {
                ESP_LOGI(BLE_GATT, "Broker username received: %s", param->write.value);
                save_broker_username((const char *)param->write.value);
            } else if (param->write.handle == broker_password_handle) {
                ESP_LOGI(BLE_GATT, "Broker password received: %s", param->write.value);
                save_broker_password((const char *)param->write.value);
            } else {
                ESP_LOGW(BLE_GATT, "Unhandled handle in write event: %d", param->write.handle);
            }

            break;

        case ESP_GATTS_REG_EVT:
            ESP_LOGI(BLE_GATT, "Registering GATT service with app_id: %d", param->reg.app_id);
            esp_err_t ret = esp_ble_gatts_create_service(gatt_if, &(esp_gatt_srvc_id_t) {
                    .is_primary = true,
                    .id = {.inst_id = 0, .uuid = {.len = ESP_UUID_LEN_16, .uuid = {.uuid16 = GATTS_SERVICE_UUID}}},
                }, GATTS_NUM_HANDLE);

            if (ret != ESP_OK) {
                ESP_LOGE(BLE_GATT, "Failed to create GATT service: %s", esp_err_to_name(ret));
                return;
            }

            ESP_LOGI(BLE_GATT, "GATT service created successfully.");
            break;

        case ESP_GATTS_CONNECT_EVT:
            conn_id = param->connect.conn_id;
            memcpy(remote_bda, param->connect.remote_bda, sizeof(esp_bd_addr_t)); 
            mac_to_string(remote_bda, user_mac, sizeof(user_mac));

            ret = save_user_mac(user_mac);          
            if (ret != ESP_OK) {
                ESP_LOGE(BLE_GATT, "Failed to save user mac: %s", esp_err_to_name(ret));
            } else {
                snprintf(read_value, sizeof(read_value), "http://localhost:8080/users/%s/devices/%s", user_mac, device_mac);

                esp_attr_value_t read_attr_value = {
                    .attr_max_len = CHAR_VAL_LEN_MAX,       
                    .attr_len = strlen(read_value),       
                    .attr_value = (uint8_t *)read_value,   
                };

                ret = esp_ble_gatts_add_char(
                    service_handle,
                    &READ_CHAR_UUID,                         
                    ESP_GATT_PERM_READ,                      
                    ESP_GATT_CHAR_PROP_BIT_READ,             
                    &read_attr_value,                        
                    &read_control                                     
                );

                if (ret != ESP_OK) {
                    ESP_LOGE(BLE_GATT, "Failed to add redirect characteristic, error code = %d", ret);
                }
            }

            gatts_if = gatt_if;
            ESP_LOGI(BLE_GATT, "Device connected, conn_id = %d", conn_id);
            ESP_LOGI(BLE_GATT, "Connected device MAC address: %s", user_mac);
            break;


        case ESP_GATTS_DISCONNECT_EVT:
            ESP_LOGI(BLE_GATT, "Client disconnected, restarting advertising.");
            ret =   esp_ble_gap_start_advertising(&adv_params);

            if (ret != ESP_OK) {
                ESP_LOGE(BLE_GATT, "Failed to start advertising after disconnect: %s", esp_err_to_name(ret));
            } else {
                ESP_LOGI(BLE_GATT, "Advertising restarted successfully after disconnect.");
            }
            break;

        case ESP_GATTS_CREATE_EVT:
            service_handle = param->create.service_handle;
            esp_ble_gatts_start_service(service_handle);

            esp_attr_value_t ssid_attr_value = {
                .attr_max_len = CHAR_VAL_LEN_MAX,
                .attr_len = sizeof(ssid_value),
                .attr_value = ssid_value,
            };

            ret = esp_ble_gatts_add_char(
                service_handle,
                &SSID_CHAR_UUID,
                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE,
                &ssid_attr_value, 
                &ssid_control     
            );

            if (ret != ESP_OK) {
                ESP_LOGE(BLE_GATT, "Failed to add SSID characteristic, error code = %d", ret);
            }

            esp_attr_value_t password_attr_value = {
                .attr_max_len = CHAR_VAL_LEN_MAX,
                .attr_len = sizeof(password_value),
                .attr_value = password_value,
            };

            ret = esp_ble_gatts_add_char(
                service_handle,
                &PASSWORD_CHAR_UUID,
                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE,
                &password_attr_value, 
                &password_control     
            );

            if (ret != ESP_OK) {
                ESP_LOGE(BLE_GATT, "Failed to add password characteristic, error code = %d", ret);
            }

            esp_attr_value_t broker_url_attr_value = {
                .attr_max_len = CHAR_VAL_LEN_MAX,
                .attr_len = sizeof(broker_url),
                .attr_value = broker_url,
            };

            ret = esp_ble_gatts_add_char(
                service_handle,
                &BROKER_URL_CHAR_UUID,
                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE,
                &broker_url_attr_value, 
                &broker_url_control     
            );

            if (ret != ESP_OK) {
                ESP_LOGE(BLE_GATT, "Failed to add broker url characteristic, error code = %d", ret);
            }

            esp_attr_value_t broker_username_attr_value = {
                .attr_max_len = CHAR_VAL_LEN_MAX,
                .attr_len = sizeof(broker_username),
                .attr_value = broker_username,
            };

            ret = esp_ble_gatts_add_char(
                service_handle,
                &BROKER_USERNAME_CHAR_UUID,
                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE,
                &broker_username_attr_value, 
                &broker_username_control     
            );

            if (ret != ESP_OK) {
                ESP_LOGE(BLE_GATT, "Failed to add broker username characteristic, error code = %d", ret);
            }

            esp_attr_value_t broker_password_attr_value = {
                .attr_max_len = CHAR_VAL_LEN_MAX,
                .attr_len = sizeof(broker_password),
                .attr_value = broker_password,
            };

            ret = esp_ble_gatts_add_char(
                service_handle,
                &BROKER_PASSWORD_CHAR_UUID,
                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE,
                &broker_password_attr_value, 
                &broker_password_control     
            );

            if (ret != ESP_OK) {
                ESP_LOGE(BLE_GATT, "Failed to add broker password characteristic, error code = %d", ret);
            }

            break;
        case ESP_GATTS_START_EVT:
            if (param->start.status == ESP_GATT_OK) {
                ESP_LOGI(BLE_GATT, "Service started successfully.");

                ret = esp_ble_gap_start_advertising(&adv_params);
                if (ret != ESP_OK) {
                    ESP_LOGE(BLE_GATT, "Failed to start advertising: %s", esp_err_to_name(ret));
                } else {
                    ESP_LOGI(BLE_GATT, "Advertising started successfully.");
                }
            } else {
                ESP_LOGE(BLE_GATT, "Failed to start service. Error: %d", param->start.status);
            }
            break;

        case ESP_GATTS_ADD_CHAR_EVT:
            if (param->add_char.status == ESP_GATT_OK) {
                ESP_LOGI(BLE_GATT, "Characteristic added. Handle: %d", param->add_char.attr_handle);
                if (param->add_char.char_uuid.uuid.uuid16 == SSID_CHAR_UUID.uuid.uuid16) {
                    ssid_handle = param->add_char.attr_handle; 
                    ESP_LOGI(BLE_GATT, "SSID handle set to: %d", ssid_handle);
                } else if (param->add_char.char_uuid.uuid.uuid16 == PASSWORD_CHAR_UUID.uuid.uuid16) {
                    password_handle = param->add_char.attr_handle;
                    ESP_LOGI(BLE_GATT, "Password handle set to: %d", password_handle);
                } else if (param->add_char.char_uuid.uuid.uuid16 == BROKER_URL_CHAR_UUID.uuid.uuid16) {
                    broker_url_handle = param->add_char.attr_handle;
                    ESP_LOGI(BLE_GATT, "Broker url handle set to: %d", broker_url_handle);
                } else if (param->add_char.char_uuid.uuid.uuid16 == BROKER_USERNAME_CHAR_UUID.uuid.uuid16) {
                    broker_username_handle = param->add_char.attr_handle;
                    ESP_LOGI(BLE_GATT, "Broker username handle set to: %d", broker_username_handle);
                } else if (param->add_char.char_uuid.uuid.uuid16 == BROKER_PASSWORD_CHAR_UUID.uuid.uuid16) {
                    broker_password_handle = param->add_char.attr_handle;
                    ESP_LOGI(BLE_GATT, "Broker password handle set to: %d", broker_password_handle);
                }
            } else {
                ESP_LOGE(BLE_GATT, "Failed to add characteristic. Error: %d", param->add_char.status);
            }
            break;
        
        default:
            ESP_LOGW("BLE", "Unhandled GATT event: %d", event);
            break;
    }
}

static void ble_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
            ESP_LOGI(BLE_GAP, "BLE scan started.");
            break;
        case ESP_GAP_BLE_SCAN_RESULT_EVT:
            ESP_LOGI(BLE_GAP, "Scan result: device found.");
            break;
        case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
            ESP_LOGI(BLE_GAP, "Advertising data set successfully.");
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            ESP_LOGI(BLE_GAP, "Advertising started.");
            break;
        case ESP_GAP_BLE_AUTH_CMPL_EVT:
            ESP_LOGI(BLE_GAP, "Authentication completed.");
            break;

        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(BLE_GAP, "Advertising stopped successfully.");
            } else {
                ESP_LOGE(BLE_GAP, "Failed to stop advertising: %d", param->adv_stop_cmpl.status);
            }
            break;
        
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            esp_ble_gap_start_advertising(&adv_params);
            break;

        case ESP_GAP_BLE_SCAN_TIMEOUT_EVT:
            ESP_LOGW(BLE_GAP, "Advertising timeout, restarting advertisement");
            esp_ble_gap_start_advertising(&adv_params);
            break;

        default:
            ESP_LOGW(BLE_GAP, "Unhandled GAP event: %d", event);
            break;
    }
}

void ble_init(void) {
    esp_err_t ret;

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_LOGI(BLE, "Initializing Bluetooth...");
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(BLE, "Bluetooth controller initialization failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(BLE, "Bluetooth controller initialized.");

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(BLE, "Bluetooth controller enabling failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(BLE, "Bluetooth controller enabled.");


    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        ESP_LOGE(BLE, "Bluedroid initialization failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(BLE, "Bluedroid enabling failed: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(BLE, "Bluetooth initialized successfully!");

    ret = esp_ble_gap_register_callback(ble_gap_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(BLE, "Failed to register GAP event handler: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_ble_gatts_register_callback(ble_gatts_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(BLE, "Failed to register GATT callback: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_ble_gatts_app_register(0);    
    if (ret != ESP_OK) {
        ESP_LOGE(BLE, "Failed to register GATT app: %s", esp_err_to_name(ret));
         return;
    }
}

static void configuration_mode_task(void *pvParameter) {
    gpio_set_direction(RESET_BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RESET_BUTTON_GPIO, GPIO_PULLUP_ONLY);

    while (1) {
        if (gpio_get_level(RESET_BUTTON_GPIO) == 0) {
            configuration_mode = !configuration_mode;

            if (configuration_mode) {
                ESP_LOGI("CONF_MODE", "Configuration mode enabled.");
                ble_init();
            } else {
                ESP_LOGI("CONF_MODE", "Configuration mode disabled.");
                esp_restart();
            }

            vTaskDelay(pdMS_TO_TICKS(100)); 
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        is_wifi_connected = false;
        xTaskCreate(blink_task, "blink_task", 2048, NULL, 10, NULL);
        esp_wifi_connect();
        ESP_LOGI(WIFI, "Retrying connection...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(WIFI, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        is_wifi_connected = true;
        has_wifi_connected = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);

        if (s_blink_task_handle != NULL) {
            vTaskDelete(s_blink_task_handle); 
            s_blink_task_handle = NULL;    
        }
    }
}

static void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(SNTP, "Time synchronization event received.");
}

void initialize_sntp(void) {
    ESP_LOGI(SNTP, "Initializing SNTP...");

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");  
    sntp_set_time_sync_notification_cb(time_sync_notification_cb); 

    sntp_init();

    ESP_LOGI(SNTP, "Waiting for system time to be set...");
    for (int i = 0; i < 10; i++) { 
        if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
            ESP_LOGI(SNTP, "System time set successfully.");
            break;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    if (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) {
        ESP_LOGW(SNTP, "Failed to set system time after 10 seconds.");
    }
}

void wifi_init_sta(void) {
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();
    led_init();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(WIFI, "Wi-Fi initialization complete.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(WIFI, "Connected to AP SSID: %s", wifi_config.sta.ssid);
        is_wifi_connected = true;
        initialize_sntp();
    } else {
        ESP_LOGE(WIFI, "Failed to connect to AP");
    }
}

static void check_existing_wifi_and_broker() {
    ESP_LOGI(WIFI, "Checking saved Wi-Fi and broker credentials...");

    char saved_ssid[32] = {0};
    char saved_password[64] = {0};
    size_t ssid_size = sizeof(saved_ssid);
    size_t password_size = sizeof(saved_password);

    bool wifi_credentials_found = false;
    bool broker_credentials_found = false;
    bool mac_found = false;

    if (get_wifi_ssid(saved_ssid, ssid_size) == ESP_OK &&
        get_wifi_password(saved_password, password_size) == ESP_OK) {
        ESP_LOGI(WIFI, "Found saved Wi-Fi credentials: SSID: %s, Password: %s", saved_ssid, saved_password);
        strncpy((char *)wifi_config.sta.ssid, saved_ssid, sizeof(wifi_config.sta.ssid) - 1);
        strncpy((char *)wifi_config.sta.password, saved_password, sizeof(wifi_config.sta.password) - 1);
        is_ssid_set = true;
        is_password_set = true;
        wifi_credentials_found = true;
    } else {
        ESP_LOGW(WIFI, "No saved Wi-Fi credentials found.");
    }

    if (get_broker_url(saved_broker_url, broker_url_size) == ESP_OK &&
        get_broker_username(saved_broker_username, broker_username_size) == ESP_OK &&
        get_broker_password(saved_broker_password, broker_password_size) == ESP_OK) {
        ESP_LOGI(BROKER, "Found saved broker credentials: URL: %s, Username: %s, Password: %s",
        saved_broker_url, saved_broker_username, saved_broker_password);
        broker_credentials_found = true;
    } else {
        ESP_LOGW(BROKER, "No saved broker credentials found.");
    }

    if (get_user_mac(user_mac) != ESP_OK) {
        ESP_LOGW(BROKER, "No user mac.");
    } else {
        mac_found = true;
    }

    if (!mac_found) {
        ESP_LOGW(BLE, "Please enter configuration mode to set user MAC.");
    }

    if (!wifi_credentials_found) {
        ESP_LOGW(WIFI, "Please set Wi-Fi credentials in configuration mode.");
    } else {
        wifi_init_sta();
    }
    
    if (!broker_credentials_found) {
        ESP_LOGW(BROKER, "Please set broker credentials in configuration mode.");
    }
}

void app_main(void)
{
    esp_log_level_set("mqtt_client", ESP_LOG_NONE);
    esp_log_level_set("transport_base", ESP_LOG_NONE);
    esp_log_level_set("esp-tls", ESP_LOG_NONE);

    initialise_mac_address();
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    xTaskCreate(configuration_mode_task, "configuration_mode_task", 2048, NULL, 10, NULL);
    xTaskCreate(blink_task, "blink_task", 2048, NULL, 10, NULL);

    check_existing_wifi_and_broker();

    xTaskCreate(start_mqtt_task, "mqtt_task", 4096, NULL, 5, NULL);

    ESP_LOGI(WIFI, "ESP_WIFI_MODE_STA");
}
