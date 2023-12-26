#include "spiffsStorage.h"

//===================================
// Options for #ifdef...#endif 

#define CHECK_SPIFFS
#define SPIFFS_CHECK_ON_START

//===================================

static char* spiffs_tag = "spiffs";

void add_to_spiffs(char *path_to_spiffs_file, char *text_to_write){
    ESP_LOGI(spiffs_tag, "Writing to file . . .");
    // FILE* fpw = fopen("/spiffs/hello.txt", "w");
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

void spiffsHandler(){
    // Delay, so we can wee info about initialization and partition
    vTaskDelay(pdMS_TO_TICKS(2500));

    ESP_LOGI(spiffs_tag, "Initializing spiffs. . .");  

    // Configurate structure for esp_vfs_spiffs_register
    esp_vfs_spiffs_conf_t confStructure = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 2,
        .format_if_mount_failed = true
    };

    // Mount spiffs to vfs 
    esp_err_t spiffsStatus = esp_vfs_spiffs_register(&confStructure);

    // Checking spiffs mount status
    #ifdef CHECK_SPIFFS
    if (spiffsStatus != ESP_OK){
        if (spiffsStatus == ESP_FAIL){
            ESP_LOGE(spiffs_tag, "Failed to mount filesystem");
        } else if (spiffsStatus == ESP_ERR_NOT_FOUND){
            ESP_LOGE(spiffs_tag, "Failed to find spiffs partition");
        } else {
            ESP_LOGE(spiffs_tag, "Failed to initialize SPIFFS (%s)", esp_err_to_name(spiffsStatus));
        }          
    } 
    #endif

    // Checking spiffs connection status (integrity of partition label)
    #ifdef SPIFFS_CHECK_ON_START
    ESP_LOGI(spiffs_tag, "Performing SPIFFS_check(). . .");
    spiffsStatus = esp_spiffs_check(confStructure.partition_label);
    if (spiffsStatus != ESP_OK) {
        ESP_LOGE(spiffs_tag, "SPIFFS_check() failed (%s)", esp_err_to_name(spiffsStatus));
        return;
    } else {
        ESP_LOGI(spiffs_tag, "SPIFFS_check() successful");
    }
    #endif
}