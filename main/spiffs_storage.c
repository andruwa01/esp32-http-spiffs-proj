#include "main.h"

static char* spiffs_tag = "spiffs_storage";

void add_line_to_spiffs(char *path_to_spiffs_file, char *text_to_write){
    #ifdef ENABLE_SPIFFS_LOGS
        ESP_LOGI(spiffs_tag, "Writing to file . . .");
    #endif

    FILE *fpw = fopen(path_to_spiffs_file, "a");
    if(fpw == NULL){
        ESP_LOGE(spiffs_tag, "Failed to open file for writing");
        return;
    } else {
        fprintf(fpw, "\n%s", text_to_write);
        fclose(fpw);

        #ifdef ENABLE_SPIFFS_LOGS
            ESP_LOGI(spiffs_tag, "Text successfully writed");
        #endif
    }
}

void read_file_from_spiffs_file_and_format(char *path_to_spiffs_file, char *partition_label_to_format){
    ESP_LOGI(spiffs_tag, "Reading file");

    FILE* fpr = fopen(path_to_spiffs_file, "r");
    if(fpr == NULL){
        ESP_LOGE(spiffs_tag, "Failed to open file for reading");
        return;
    } else {
        // TODO finish it
        char buffer[NUMBER_OF_SYMBOLS_IN_SPIFFS_FILE_TO_READ_FROM];
        fread(buffer, sizeof(char), NUMBER_OF_SYMBOLS_IN_SPIFFS_FILE_TO_READ_FROM, fpr);  
        ESP_LOGI(spiffs_tag, "File %s successfully readed, contents:\n%s", path_to_spiffs_file, buffer);

        fclose(fpr);

        size_t total = 0, used = 0;
        esp_spiffs_info(partition_label_to_format, &total, &used);
        ESP_LOGW(spiffs_tag, "total: %i, used: %i", total, used);

        #ifdef CLEAR_SPIFFS_FILE_IF_IT_EXISTED_AFTER_READ_FROM_IT
            fclose(fopen(path_to_spiffs_file, "w"));
            ESP_LOGW(spiffs_tag, "File %s was cleared", path_to_spiffs_file);
        #endif

        // size_t total, used = 0;
        // esp_spiffs_info(partition_label_to_format, &total, &used);
        // ESP_LOGW(spiffs_tag, "Spiffs usage:\ntotal: %i, used: %i", total, used);
    }
}

void initialize_spiffs(){
    ESP_LOGI(spiffs_tag, "Initializing spiffs");

    // Configurate structure for esp_vfs_spiffs_register
    esp_vfs_spiffs_conf_t conf_spiffs = {
        .base_path = "/spiffs",
        .partition_label = PARTITION_LABEL,
        .max_files = 2,
        .format_if_mount_failed = true
    };

    // Mount spiffs to vfs 
    esp_err_t spiffs_status = esp_vfs_spiffs_register(&conf_spiffs);

    #ifdef CHECK_SPIFFS
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
    #ifdef SPIFFS_CHECK_ON_START
        ESP_LOGI(spiffs_tag, "Performing spiffs check");
        spiffs_status = esp_spiffs_check(conf_spiffs.partition_label);
        if (spiffs_status != ESP_OK) {
            ESP_LOGE(spiffs_tag, "SPIFFS_check() failed (%s)", esp_err_to_name(spiffs_status));
            return;
        } else {
            ESP_LOGI(spiffs_tag, "SPIFFS_check() successful");
        }
    #endif

    size_t total = 0, used = 0;
    spiffs_status = esp_spiffs_info(PARTITION_LABEL, &total, &used);

    // ESP_LOGW(spiffs_tag, "patittion label: %s", conf_spiffs.partition_label);

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