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
    if(SPIFFS_NUMBER_OF_FILES > SPIFFS_MAX_FILES){
        ESP_LOGE(tag_main, "ERROR: SPIFFS_NUMBER_OF_FILES is more than SPIFFS_MAX_FILES: CHANGE IT IN options.h");
    }

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
    // }>

    // vTaskDelete(TaskButtonHandle);

    // uart_configure();

    //====================BUTTON==============================//
    //====================FIRST_VARIANT=======================//

    // #ifdef SPIFFS_USE_FUNCTIONALITY
	//     initialize_spiffs();
    // #endif

    // esp_err_t ret = nvs_flash_init();

    // if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }

    //====================FIRST_VARIANT=======================//

    // initialize_get_request();
    // #ifdef SPIFFS_USE_FUNCTIONALITY
    //     char spiffs_file_path[strlen(SPIFFS_BASE_PATH) + strlen("/") + SPIFFS_MAX_FILE_NAME_LENGTH];
    //     read_file_from_spiffs_file_and_format(spiffs_file_path, "data_storage")
    // #elif
    //     ESP_LOGW(tag_main, "You don't use spiffs!");
    // #endif

    #ifdef SPIFFS_USE_FUNCTIONALITY
        // for(int file_number = 0; file_number < SPIFFS_NUMBER_OF_FILES; file_number++){
        for(int file_number = 1; file_number < SPIFFS_NUMBER_OF_FILES; file_number++){ 
            char spiffs_file_path[strlen(SPIFFS_BASE_PATH) + strlen("/") + SPIFFS_MAX_FILE_NAME_LENGTH];
            sprintf(spiffs_file_path, "%s/%s", SPIFFS_BASE_PATH, spiffs_file_names[file_number]);
            read_file_from_spiffs_file_and_format(spiffs_file_path, "data_storage");
            // printf("\n"); - give huge mistake
        }
    #else
        ESP_LOGW(tag_main, "You don't use spiffs!");
    #endif
}