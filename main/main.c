#include "main.h"

const static char* tag_main = "main_app";

void app_main(void)
{
    if(SPIFFS_NUMBER_OF_FILES > SPIFFS_MAX_FILES){
        ESP_LOGE(tag_main, "ERROR: SPIFFS_NUMBER_OF_FILES is more than SPIFFS_MAX_FILES: CHANGE IT IN options.h");
    }

    initialize_nvs_flash();
    initialize_wifi();

    xTaskCreate(udp_task, "udp_task", 4096, NULL, 5, NULL);

    // wifi_ap_record_t wifi_info;

}