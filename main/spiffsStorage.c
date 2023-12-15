#include "spiffsStorage.h"

//===================================
// Options for #ifdef...#endif 

#define CHECK_SPIFFS
#define SPIFFS_CHECK_ON_START

//===================================

static char* TAGspiffs = "spiffs";

void writeToFile(){
    ESP_LOGI(TAGspiffs, "Writing to file");
    FILE* fpw = fopen("/spiffs/hello.txt", "w");
    if(fpw == NULL){
        ESP_LOGE(TAGspiffs, "Failed to open file for writing");
        return;
    } else {
        fprintf(fpw, "Entered text !\n");
        fclose(fpw);
        ESP_LOGI(TAGspiffs, "File successfully writed");
    }
}

void readFile(){
    ESP_LOGI(TAGspiffs, "Reading file (before editing)");
    FILE* fpr = fopen("/spiffs/hello.txt", "r");
    if(fpr == NULL){
        ESP_LOGE(TAGspiffs, "Failed to open file for reading");
        return;
    } else {
        ESP_LOGI(TAGspiffs, "File successfully readed");
    }
    
    char buf[64];
    memset(buf, 0, sizeof(buf));
    fread(buf, 1, sizeof(buf), fpr);
    fclose(fpr);

    ESP_LOGI(TAGspiffs, "Read from hello.txt: %s", buf);
}
void testFileInOut(){

    // Reading file before editing
    readFile();
    
    // Writing new information to file
    writeToFile();

    // Reading file after editing
    readFile();
}  

void spiffsHandler(){
    // Delay, so we can wee info about initialization and partition
    vTaskDelay(pdMS_TO_TICKS(2500));

    ESP_LOGI(TAGspiffs, "Initializing spiffs. . .");  

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
            ESP_LOGE(TAGspiffs, "Failed to mount filesystem");
        } else if (spiffsStatus == ESP_ERR_NOT_FOUND){
            ESP_LOGE(TAGspiffs, "Failed to find spiffs partition");
        } else {
            ESP_LOGE(TAGspiffs, "Failed to initialize SPIFFS (%s)", esp_err_to_name(spiffsStatus));
        }          
    } 
    #endif

    // Checking spiffs connection status (integrity of partition label)
    #ifdef SPIFFS_CHECK_ON_START
    ESP_LOGI(TAGspiffs, "Performing SPIFFS_check(). . .");
    spiffsStatus = esp_spiffs_check(confStructure.partition_label);
    if (spiffsStatus != ESP_OK) {
        ESP_LOGE(TAGspiffs, "SPIFFS_check() failed (%s)", esp_err_to_name(spiffsStatus));
        return;
    } else {
        ESP_LOGI(TAGspiffs, "SPIFFS_check() successful");
    }
    #endif

    testFileInOut();
}