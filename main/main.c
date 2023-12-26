#include <main.h>
#include "spiffsStorage.h"
#include "wifi.h"
#include "http_client.h"

const static char* TAGmain = "main_app";

void app_main(void)
{
	spiffsHandler();

    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    wifi_connection();

    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAGmain, "wifi was initiated");

    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAGmain, "starting client . . .");

    get_rest_function();

    read_file_from_spiffs("/spiffs/data.txt");
}