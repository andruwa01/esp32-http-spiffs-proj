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

// Move it function to particular file or not think about it
void print_satellite_data(int satellite_index){
        if(esp_spiffs_mounted(SPIFFS_PARTITION_LABEL)){
            char spiffs_file_path[strlen(SPIFFS_BASE_PATH) + strlen("/") + strlen(satellites[satellite_index].name)];
            sprintf(spiffs_file_path, "%s/%s", SPIFFS_BASE_PATH, satellites[satellite_index].name);

            char data_from_spiffs_file[HTTP_BUFFER_SIZE];
            read_data_from_spiffs_file_to_buffer(spiffs_file_path, data_from_spiffs_file, HTTP_BUFFER_SIZE);
            printf("%s\n", data_from_spiffs_file);

            #if defined(SIPFFS_CLEAR_FILES_AFTER_READING)
            clear_data_from_spiffs_file(spiffs_file_path);
            #else
            ESP_LOGW(tag_main, "You don't use spiffs!");
            #endif

        } else {
            ESP_LOGE(tag_main, "ERROR: SPIFFS_UNMOUNTED ! RETURN FROM APP_MAIN");
            return;
        }
    }

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

    initialize_get_request(satellites[0].id, satellites[0].name);
    print_satellite_data(0);

    button_handler();

    #if defined(GET_REQUEST_ALL_SAT)
    initialize_get_requests_for_all_satellites();
    #endif

    #if defined(SPIFFS_READ_ALL_FILES)
        for(int satellite_index = 0; satellite_index < SPIFFS_NUMBER_OF_FILES; satellite_index++){ 
            print_satellite_data(satellite_index);
        }
        size_t total, used = 0;
        esp_spiffs_info(SPIFFS_PARTITION_LABEL, &total, &used);
        ESP_LOGW(tag_main, "total: %i, used: %i", total, used);
    #else
        ESP_LOGW(tag_main, "You don't use spiffs!");
    #endif

    // #if defined(SEND_DATA_FROM_SPIFFS_TO_UART)
    // char spiffs_file_path[strlen(SPIFFS_BASE_PATH) + strlen("/") + strlen(satellites[0].name)];
    // sprintf(spiffs_file_path, "%s/%s", SPIFFS_BASE_PATH, satellites[0].name);

    // char satellite_data[HTTP_BUFFER_SIZE];
    // read_data_from_spiffs_file_to_buffer(spiffs_file_path, satellite_data, HTTP_BUFFER_SIZE);

    // // write_data_from_spiffs_file_to_uart(); // push all data to uart -> TODO
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