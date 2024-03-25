#include "main.h"

const static char* tag_main = "main_app";

void app_main(void)
{
    if(SPIFFS_NUMBER_OF_FILES > SPIFFS_MAX_FILES){
        ESP_LOGE(tag_main, "ERROR: SPIFFS_NUMBER_OF_FILES is more than SPIFFS_MAX_FILES: CHANGE IT IN options.h");
    }

    #if defined(HANDLE_UART)
    initialize_uart();
    initialize_spiffs();
    initialize_command_handler();
    #endif

    #if defined(HANDLE_UDP)
    initialize_spiffs();
    initialize_nvs_flash();
    initialize_wifi();
    initialize_udp_tasks();
    #endif
}