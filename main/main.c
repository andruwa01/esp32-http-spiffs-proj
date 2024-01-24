#include <main.h>

const static char* tag_main = "main_app";

// void xTaskButton(){
//     ESP_LOGI(tag_main, "xTaskButton called");
//     button_handler();
//     for(;;){
//         vTaskSuspend(NULL);
//     }
//     vTaskDelete(NULL);
// }

void app_main(void)
{
    //====================NEW_VARIANT==========================//
    initialize_spiffs();
    initialize_nvs_flash();

    initialize_wifi();
    uart_configure();
    initialize_freertos_tasks();

    //====================NEW_VARIANT==========================//

    //====================BUTTON==============================//

    // uart_configure();
    
    // TaskHandle_t TaskButtonHandle = NULL;
    // if (xTaskCreate(xTaskButton, "button", 3000, NULL, 2, &TaskButtonHandle) != pdPASS){
    //     ESP_LOGE(tag_main, "failed to create task");
    // }

    // ESP_LOGW(tag_main, "You can push the button!");
    // vTaskDelay(pdMS_TO_TICKS(20000));

    // vTaskDelete(TaskButtonHandle);

    // uart_configure();

    //====================BUTTON==============================//
    //====================FIRST_VARIANT=======================//

    // #ifdef USE_SPIFFS
	//     initialize_spiffs();
    // #endif

    // esp_err_t ret = nvs_flash_init();

    // if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }

    // initialize_get_request();

    // #ifdef USE_SPIFFS
    //     read_file_from_spiffs("/spiffs/norbi.txt");
    // #else
    //     ESP_LOGW(tag_main, "You don't use spiffs!");
    // #endif

    //====================FIRST_VARIANT=======================//
}
