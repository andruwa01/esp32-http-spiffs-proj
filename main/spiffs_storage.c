#include "main.h"

static char* spiffs_tag = "spiffs_storage";
// FILE *fp_to_end_of_default_content;

void add_line_to_spiffs(char* path_to_spiffs_file, char* text_to_write){
    #ifdef SPIFFS_LOGS
        ESP_LOGI(spiffs_tag, "Writing to file . . .");
    #endif

    FILE *fpw = fopen(path_to_spiffs_file, "a");

    if(fpw == NULL){
        ESP_LOGE(spiffs_tag, "Failed to open file for writing");
        return;
    } else {
        fprintf(fpw, "%s\n", text_to_write);
        fclose(fpw);

        #ifdef SPIFFS_LOGS
            ESP_LOGI(spiffs_tag, "Line successfully writed to file %s", path_to_spiffs_file);
        #endif
    }
}

void clear_data_from_spiffs_file(char* path_to_spiffs_file){
    fclose(fopen(path_to_spiffs_file, "w"));
    ESP_LOGW(spiffs_tag, "File %s was cleared", path_to_spiffs_file);
}

void read_data_from_spiffs_file_to_buffer(char* path_to_spiffs_file, char* buffer_to_save_data, int buffer_size){
    ESP_LOGI(spiffs_tag, "reading file: %s", path_to_spiffs_file);

    FILE* fpr = fopen(path_to_spiffs_file, "r"); 

    if(fpr == NULL){
        ESP_LOGE(spiffs_tag, "Failed to open file %s for reading", path_to_spiffs_file);
        return;
    }

    fseek(fpr, 0, SEEK_END);
    int size_of_spiffs_data = ftell(fpr);
    fseek(fpr, 0, SEEK_SET);

    if(size_of_spiffs_data > buffer_size){
        ESP_LOGE(spiffs_tag, "ERROR! %i bytes don't fit in %i size buffer, return from function", size_of_spiffs_data, buffer_size);
        return;
    }

    fread(buffer_to_save_data, sizeof(char), size_of_spiffs_data, fpr);

    buffer_to_save_data[size_of_spiffs_data] = '\0'; // add null-terminated symbol for correct print, if needs

    fclose(fpr);

    ESP_LOGI(spiffs_tag, "Readed %i bytes from %s file:", size_of_spiffs_data, path_to_spiffs_file);
}

void initialize_spiffs(void){
    ESP_LOGI(spiffs_tag, "Initializing spiffs");

    // Configurate structure for esp_vfs_spiffs_register
    esp_vfs_spiffs_conf_t conf_spiffs = {
        .base_path = SPIFFS_BASE_PATH,
        .partition_label = SPIFFS_PARTITION_LABEL,
        .max_files = SPIFFS_MAX_FILES,
        .format_if_mount_failed = true
    };

    // Mount spiffs to vfs 
    esp_err_t spiffs_status = esp_vfs_spiffs_register(&conf_spiffs);

    #if defined(SPIFFS_CHECK_ON_START)
        if (spiffs_status != ESP_OK){
            if (spiffs_status == ESP_FAIL){
                ESP_LOGE(spiffs_tag, "Failed to mount filesystem");
            } else if (spiffs_status == ESP_ERR_NOT_FOUND){
                ESP_LOGE(spiffs_tag, "Failed to find spiffs partition");
            } else {
                ESP_LOGE(spiffs_tag, "Failed to initialize SPIFFS (%s)", esp_err_to_name(spiffs_status));
            }          
        } 
    #endif

    // Checking spiffs partition label
    #if defined(SPIFFS_CHECK_ON_START)
        ESP_LOGI(spiffs_tag, "Performing spiffs check");
        spiffs_status = esp_spiffs_check(conf_spiffs.partition_label);
        if (spiffs_status != ESP_OK) {
            ESP_LOGE(spiffs_tag, "spiffs check failed (%s)", esp_err_to_name(spiffs_status));
            return;
        } else {
            ESP_LOGI(spiffs_tag, "spiffs check successful");
        }
    #endif

    #if defined(SPIFFS_CLEAR_FILES_ON_START)
    for(int satellite_index = 0; satellite_index < SPIFFS_NUMBER_OF_FILES; satellite_index++){
        char spiffs_file_path[strlen(SPIFFS_BASE_PATH) + strlen("/") + strlen(satellites[satellite_index].name)];
        sprintf(spiffs_file_path, "%s/%s", SPIFFS_BASE_PATH, satellites[satellite_index].name);
        if(fclose(fopen(spiffs_file_path, "w")) != 0){
            ESP_LOGE(spiffs_tag, "Error: can't open and close (clear) file %s", spiffs_file_path);
            return;
        } else {
            ESP_LOGW(spiffs_tag, "File %s was cleared", spiffs_file_path);
        };
    }
    #else
    ESP_LOGW(spiffs_tag, "You don't use spiffs!");
    #endif

    size_t total = 0, used = 0;
    spiffs_status = esp_spiffs_info(conf_spiffs.partition_label, &total, &used);
    if(spiffs_status != ESP_OK){
        ESP_LOGE(spiffs_tag, "Failed to get SPIFFS partition info, error code: %s", esp_err_to_name(spiffs_status));
        esp_spiffs_format(conf_spiffs.partition_label);
        return;
    } else {
        ESP_LOGW(spiffs_tag, "Partition size: %d, used %d", total, used);
    }

    if(used > total){
        ESP_LOGW(spiffs_tag, "Number of used bytes can not be larger that total. Performing spiffs check. . .");
        spiffs_status = esp_spiffs_check(conf_spiffs.partition_label);
        if(spiffs_status != ESP_OK){
            ESP_LOGE(spiffs_tag, "Spiffs check failed, code of error: %s", esp_err_to_name(spiffs_status));
            return;
        } else {
            ESP_LOGI(spiffs_tag, "Spiffs check successful");
        }
    }
}