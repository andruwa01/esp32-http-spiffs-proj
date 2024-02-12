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
    if(SPIFFS_NUMBER_OF_FILES > SPIFFS_MAX_FILES){
        ESP_LOGE(tag_main, "ERROR: SPIFFS_NUMBER_OF_FILES is more than SPIFFS_MAX_FILES: CHANGE IT IN options.h");
    }

    initialize_spiffs();
    initialize_nvs_flash();
    initialize_wifi();
    uart_configure();

    get_command_from_uart();

    // button_handler();

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
}