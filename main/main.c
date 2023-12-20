#include <main.h>
#include "spiffsStorage.h"
#include "wifi.h"
#include "httpClient.h"

const static char* TAGhttp = "HTTP";

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
esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt) {
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("Client HTTP_EVENT_ON_DATA, data value: %.*s\n",evt->data_len, (char*)evt->data);

        ESP_LOGI(TAGhttp, "Parsing data from JSON string . . .");

        int status = 0;
        const cJSON *info_elements = NULL;
        const cJSON *info_element = NULL;

        cJSON *json_satellite_data = cJSON_Parse(evt->data);

        if(json_satellite_data == NULL){
            const char *error_ptr = cJSON_GetErrorPtr();
            if(error_ptr != NULL){
                ESP_LOGE(TAGhttp, "Error before %s", error_ptr);
            }
            status = 0;
            goto end;
        }

        info_elements = cJSON_GetObjectItemCaseSensitive(json_satellite_data, "info");
        cJSON_ArrayForEach(info_element,info_elements){
            cJSON *sat_id = cJSON_GetObjectItemCaseSensitive(info_element, "satid");
            cJSON *sat_name = cJSON_GetObjectItemCaseSensitive(info_element, "satname");
            cJSON *trans_count = cJSON_GetObjectItemCaseSensitive(info_element, "transactioncount");
            cJSON *passes_count = cJSON_GetObjectItemCaseSensitive(info_element, "passescount");
        }

        end:
            cJSON_Delete(json_satellite_data);
            ESP_LOGI(TAGhttp, "Status from parsing process: %i", status);
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
        // .url = "http://httpbin.org/get",                                       //work
        // .url = "http://api.n2yo.com/rest/v1/satellite/tle/25544&apiKey=VKC8LB-XBX436-NS9KSA-56EJ",
        .url = "https://api.n2yo.com/rest/v1/satellite/radiopasses/25544/41.702/-76.014/0/2/40/&apiKey=VKC8LB-XBX436-NS9KSA-56EJ",
        // .cert_pem = (const unsigned char*) certificate_pem_start,
        // .client_cert_pem = root_ca,
        // .cert_pem = root_ca_n2yo,
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