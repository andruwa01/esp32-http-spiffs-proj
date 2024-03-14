#include <main.h>

const static char* tag_main = "main_app";

// Move it function to particular file or not think about it
// static void print_satellite_data(int satellite_index){
//         if(esp_spiffs_mounted(SPIFFS_PARTITION_LABEL)){
//             char spiffs_file_path[strlen(SPIFFS_BASE_PATH) + strlen("/") + strlen(satellites[satellite_index].name)];
//             sprintf(spiffs_file_path, "%s/%s", SPIFFS_BASE_PATH, satellites[satellite_index].name);

//             char data_from_spiffs_file[HTTP_BUFFER_SIZE];
//             read_data_from_spiffs_file_to_buffer(spiffs_file_path, data_from_spiffs_file, HTTP_BUFFER_SIZE);

//             printf("%s\n", data_from_spiffs_file);

//             #if defined(SIPFFS_CLEAR_FILES_AFTER_READING)
//             clear_data_from_spiffs_file(spiffs_file_path);
//             #else
//             ESP_LOGW(tag_main, "You don't use spiffs!");
//             #endif

//         } else {
//             ESP_LOGE(tag_main, "ERROR: SPIFFS_UNMOUNTED ! RETURN FROM APP_MAIN");
//             return;
//         }
//     }

void app_main(void)
{
    if(SPIFFS_NUMBER_OF_FILES > SPIFFS_MAX_FILES){
        ESP_LOGE(tag_main, "ERROR: SPIFFS_NUMBER_OF_FILES is more than SPIFFS_MAX_FILES: CHANGE IT IN options.h");
    }

    // init_spiffs();
    // init_uart();
    // init_command_handler();

    initialize_nvs_flash();
    initialize_wifi();

    // wifi_ap_record_t wifi_info;

    // initialize_wifi();
    // button_handler();
}