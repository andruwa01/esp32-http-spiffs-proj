#include "spiffsStorage.h"

//===================================
// Options for #ifdef...#endif 

#define CHECK_SPIFFS
#define SPIFFS_CHECK_ON_START

//===================================

static char* spiffs_tag = "spiffs";

void add_to_spiffs(char *path_to_spiffs_file, char *text_to_write){
    ESP_LOGI(spiffs_tag, "Writing to file . . .");
    FILE *fpw = fopen(path_to_spiffs_file, "a");
    if(fpw == NULL){
        ESP_LOGE(spiffs_tag, "Failed to open file for writing");
        return;
    } else {
        fprintf(fpw, "\n%s", text_to_write);
        fclose(fpw);

        ESP_LOGI(spiffs_tag, "Text successfully writed");
    }
}

void read_file_from_spiffs(char *path_to_spiffs_file){
    ESP_LOGI(spiffs_tag, "Reading file");

    FILE* fpr = fopen(path_to_spiffs_file, "r");
    if(fpr == NULL){
        ESP_LOGE(spiffs_tag, "Failed to open file for reading");
        return;
    } else {
        // TODO finish it
        char buffer[1024];
        fread(buffer, sizeof(char), 1024, fpr);  
        ESP_LOGI(spiffs_tag, "File %s successfully readed, contents:\n%s", path_to_spiffs_file, buffer);
    }
}

void spiffs_handler(){
    // Delay, so we can wee info about initialization and partition
    vTaskDelay(pdMS_TO_TICKS(2500));

    ESP_LOGI(spiffs_tag, "Initializing spiffs. . .");  

    // Configurate structure for esp_vfs_spiffs_register
    esp_vfs_spiffs_conf_t conf_spiffs = {
        .base_path = "/spiffs",
        .partition_label = NULL,
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

    // Checking spiffs connection status (integrity of partition label)
    #ifdef SPIFFS_CHECK_ON_START
    ESP_LOGI(spiffs_tag, "Performing SPIFFS_check(). . .");
    spiffs_status = esp_spiffs_check(conf_spiffs.partition_label);
    if (spiffs_status != ESP_OK) {
        ESP_LOGE(spiffs_tag, "SPIFFS_check() failed (%s)", esp_err_to_name(spiffs_status));
        return;
    } else {
        ESP_LOGI(spiffs_tag, "SPIFFS_check() successful");
    }
    #endif

    size_t total = 0, used = 0;
    spiffs_status = esp_spiffs_info(NULL, &total, &used);
    if(spiffs_status != ESP_OK){
        ESP_LOGE(spiffs_tag, "Failed to get SPIFFS partition info, error code: %s", esp_err_to_name(spiffs_status));
        esp_spiffs_format(conf_spiffs.partition_label);
        return;
    } else {
        ESP_LOGI(spiffs_tag, "Partition size: %d, used %d", total, used);
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