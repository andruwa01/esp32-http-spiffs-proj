#include "main.h"

static char* spiffs_tag = "spiffs_storage";
FILE *fp_to_end_of_default_content;

void add_line_to_spiffs(char *path_to_spiffs_file, char *text_to_write){
    #ifdef SPIFFS_LOGS
        ESP_LOGI(spiffs_tag, "Writing to file . . .");
    #endif

    FILE *fpw = fopen(path_to_spiffs_file, "a");
    if(fpw == NULL){
        ESP_LOGE(spiffs_tag, "Failed to open file for writing");
        return;
    } else {
        fprintf(fpw, "\n%s", text_to_write);
        fclose(fpw);
        #ifdef SPIFFS_LOGS
            ESP_LOGI(spiffs_tag, "Text successfully writed to file %s", path_to_spiffs_file);
        #endif
    }
}

void read_file_from_spiffs_file_and_format(char *path_to_spiffs_file, char *partition_label_to_format){
    ESP_LOGI(spiffs_tag, "Reading file");

    if(!esp_spiffs_mounted(SPIFFS_PARTITION_LABEL)){
        ESP_LOGE(spiffs_tag, "ERROR! SPIFFS unmounted -> impossible to work!");
        return;
    }

    FILE* fpr = fopen(path_to_spiffs_file, "r");
    if(fpr == NULL){
        ESP_LOGE(spiffs_tag, "Failed to open file for reading");
        return;
    } else {
        fseek(fpr, 0, SEEK_END);
        int size_of_file = ftell(fpr);
        ESP_LOGW(spiffs_tag, "size of %s is %i", path_to_spiffs_file, size_of_file);

        char buffer[size_of_file];
        fseek(fpr, 0, SEEK_SET);
        fread(buffer, sizeof(char), size_of_file, fpr);  
        // Add null-terminated symbol so printf(or esp_logx) could correctly read string until
        // this symbol
        buffer[size_of_file] = '\0'; 
        ESP_LOGI(spiffs_tag, "File %s successfully readed, contents:\n%s", path_to_spiffs_file, buffer);

        fclose(fpr);

        #ifdef SPIFFS_CLEAR_FILE_AFTER_READ_FROM
            fclose(fopen(path_to_spiffs_file, "w"));
            ESP_LOGW(spiffs_tag, "File %s was cleared", path_to_spiffs_file);
        #endif

        size_t total = 0, used = 0;
        esp_spiffs_info(partition_label_to_format, &total, &used);
        ESP_LOGW(spiffs_tag, "total: %i, used: %i", total, used);
    }
}

void initialize_spiffs(){
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

    #ifdef SPIFFS_CHECK_ON_START
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
            ESP_LOGE(spiffs_tag, "spiffs check failed (%s)", esp_err_to_name(spiffs_status));
            return;
        } else {
            ESP_LOGI(spiffs_tag, "spiffs check successful");
        }
    #endif

    #if defined(SPIFFS_CLEAR_FILES_ON_START)
    for(int file_number = 0; file_number < SPIFFS_NUMBER_OF_FILES; file_number++){
        char spiffs_file_path[strlen(SPIFFS_BASE_PATH) + strlen("/") + SPIFFS_MAX_FILE_NAME_LENGTH];
        sprintf(spiffs_file_path, "%s/%s", SPIFFS_BASE_PATH, spiffs_file_names[file_number]);
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
    spiffs_status = esp_spiffs_info(SPIFFS_PARTITION_LABEL, &total, &used);
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