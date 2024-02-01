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
    // uart_configure();
    // button_handler();

    initialize_get_requests_for_all_satellites();

    #if defined(SPIFFS_READ_ALL_FILES)
        for(int satellite_index = 0; satellite_index < SPIFFS_NUMBER_OF_FILES; satellite_index++){ 
            if(esp_spiffs_mounted(SPIFFS_PARTITION_LABEL)){
                char spiffs_file_path[strlen(SPIFFS_BASE_PATH) + strlen("/") + SPIFFS_MAX_FILE_NAME_LENGTH];
                sprintf(spiffs_file_path, "%s/%s", SPIFFS_BASE_PATH, satellites[satellite_index].name);

                // read_data_from_spiffs_file_and_format_partition(spiffs_file_path, SPIFFS_PARTITION_LABEL);
                // char data_from_spiffs_file[HTTP_BUFFER_SIZE]; 
                // strcpy(data_from_spiffs_file, read_data_from_spiffs_file(spiffs_file_path));

                ESP_LOGI(tag_main, "Data from file %s:%s", spiffs_file_path, read_data_from_spiffs_file(spiffs_file_path));

                #if defined(SIPFFS_CLEAR_FILES_AFTER_RADING)
                clear_data_from_spiffs_file(spiffs_file_path);
                #else
                ESP_LOGW(tag_main, "You don't use spiffs!");
                #endif

            } else {
                ESP_LOGE(tag_main, "ERROR: SPIFFS_UNMOUNTED ! RETURN FORM APP_MAIN");
                return;
            }
        }
    #else
        ESP_LOGW(tag_main, "You don't use spiffs!");
    #endif

    // #if defined(SEND_DATA_FROM_SPIFFS_TO_UART)
    // char spiffs_data[] = read_data_from_spiffs_file();
    // write_data_from_spiffs_file_to_uart();
    // #endif

    // read_data_from_spiffs_file_and_format_partition("/spiffs/norbi.txt", SPIFFS_PARTITION_LABEL);
    // read_data_from_spiffs_file_and_format_partition("/spiffs/rs52sv.txt", SPIFFS_PARTITION_LABEL);

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

}