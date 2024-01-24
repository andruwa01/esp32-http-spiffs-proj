#include "wifi.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi_sta";

static EventGroupHandle_t s_wifi_event_group;

static int wifi_retry_num = 0;

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
        esp_wifi_connect();
    } else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED){
        if(wifi_retry_num < CONFIG_ESP_MAXIMUM_RETRY){
            esp_wifi_connect();
            wifi_retry_num++;
            ESP_LOGW(TAG, "retry connecting to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGW(TAG, "connect to the AP fail");
        }
        // ESP_LOGW(TAG, "connect to the AP fail");
    } else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
        ip_event_got_ip_t *event = (ip_event_got_ip_t*)event_data;
        ESP_LOGI(TAG, "Got ip: " IPSTR, IP2STR(&event->ip_info.ip));
        wifi_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }

    // WORKING VARIANT
    // switch (event_id)
    // {
    // case WIFI_EVENT_STA_START:
    //     printf("WiFi connecting ... \n");
    //     break;
    // case WIFI_EVENT_STA_CONNECTED:
    //     printf("WiFi connected ... \n");
    //     break;
    // case WIFI_EVENT_STA_DISCONNECTED:
    //     printf("WiFi lost connection ... \n");
    //     break;
    // case IP_EVENT_STA_GOT_IP:
    //     printf("WiFi got IP ... \n\n");
    //     break;
    // default:
    //     break;
    // }
}

void initialize_wifi(){
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD}
        };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration));
    ESP_ERROR_CHECK(esp_wifi_start());

    // ESP_ERROR_CHECK(esp_wifi_connect());

    ESP_LOGI(TAG, "wifi_sta_initialization_finished");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    if(bits & WIFI_CONNECTED_BIT){
        ESP_LOGI(TAG, "connected to ap, SSID: %s, password: %s", CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
    } else if(bits & WIFI_FAIL_BIT){
        ESP_LOGI(TAG, "Failed to connect to SSID: %s, password: %s", CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}