#include <main.h>
#include "spiffsStorage.h"
#include "wifi.h"
#include "httpClient.h"

void app_main(void)
{
    // Initialize spiffs
    // spiffsHandler();

    // Initialize wifi sta
    wifi_sta_initialize();

    vTaskDelay(pdMS_TO_TICKS(2000));
    // Initizlize http client 
    // xTaskCreate(http_rest_with_url, "http_rest_task", 8192, NULL, 5, NULL);
    initialize_task();
}