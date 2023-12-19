#include <main.h>
#include "spiffsStorage.h"
#include "wifi.h"
#include "httpClient.h"

static char *TAG = "main";

// extern const uint8_t certificate_pem_start[] asm("_binary_clientCert_pem_start");
// extern const uint8_t certificate_pem_end[]   asm("_binary_clientCert_pem_end");

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
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
    esp_netif_init();                    // TCP/IP initiation 					s1.1
    esp_event_loop_create_default();     // event loop 			                s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					                    s1.4
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
}

// Client
esp_err_t client_event_get_handler(
    esp_http_client_event_handle_t evt) {
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        // ESP_LOGI(TAG ,"Client HTTP_EVENT_ON_DATA, data value: %.*s\n", evt->data_len, (char*)evt->data);
        printf("Client HTTP_EVENT_ON_DATA, data value: %.*s\n", evt->data_len, (char*)evt->data);
        break;

    default:
        break;
    }
    return ESP_OK;
}

static void get_rest_function(){
    esp_http_client_config_t config_get = {
        // use .host + .path or only .url
        // .url = "http://worldtimeapi.org/api/timezone/America/Argentina/Salta", //work
        // .url = "http://httpbin.org/get", // work
        .url = "https://api.n2yo.com/rest/v1/satellite/tle/25544&apiKey=VKC8LB-XBX436-NS9KSA-56EJ",
        // .cert_pem = (const char*) certificate_pem_start,
        .method = HTTP_METHOD_GET,
        .event_handler = client_event_get_handler
    };

    esp_http_client_handle_t client = esp_http_client_init(&config_get);
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}


void app_main(void)
{
    nvs_flash_init();
    wifi_connection();

    vTaskDelay(pdMS_TO_TICKS(2000));
    printf("Wifi was initiated . . . \n\n");

    vTaskDelay(pdMS_TO_TICKS(2000));
    printf("Starting client . . . \n\n");
    get_rest_function();
}