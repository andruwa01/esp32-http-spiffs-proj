#include "wifi.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// static const char *TAG = "wifi station";

// static EventGroupHandle_t s_wifi_event_group;
// static int wifi_retry_num = 0;

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    // if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
    //     esp_wifi_connect();
    // } else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED){
    //     if(wifi_retry_num < CONFIG_ESP_MAXIMUM_RETRY){
    //         esp_wifi_connect();
    //         wifi_retry_num++;
    //         ESP_LOGI(TAG, "retry connecting to the AP");
    //     } else {
    //         //TODO: read about this function in FreeRTOS 
    //         xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    //     }
    //     ESP_LOGI(TAG, "connect to the AP failed");
    // } else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
    //     ip_event_got_ip_t *event = (ip_event_got_ip_t*)event_data;
    //     ESP_LOGI(TAG, "Got ip: " IPSTR, IP2STR(&event->ip_info.ip));
    //     wifi_retry_num = 0;
    //     xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    // }

    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

void wifi_connection(){
    // 1 - Wi-Fi/LwIP Init Phase
    ESP_ERROR_CHECK(esp_netif_init());                    // TCP/IP initiation 					s1.1
    ESP_ERROR_CHECK(esp_event_loop_create_default());     // event loop 			                s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_initiation)); // 					                    s1.4
    // 2 - Wi-Fi Configuration Phase
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL));
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = "dron_ap",
            .password = "dron1542"}
        };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration));
    // 3 - Wi-Fi Start Phase
    ESP_ERROR_CHECK(esp_wifi_start());
    // 4- Wi-Fi Connect Phase
    ESP_ERROR_CHECK(esp_wifi_connect());
}