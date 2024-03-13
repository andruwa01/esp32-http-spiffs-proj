#include "main.h"
#include "esp_err.h"

// Initializing spiffs with corresponding logs
void init_spiffs(void);

// Add file to spiffs filesystem
void add_line_to_spiffs(char* path_to_spiffs_file, char* text_to_write);

// Read file contents from spiffs filesystem
void read_data_from_spiffs_file_and_format_partition(char* path_to_spiffs_file, char* partition_label_to_format);

// void* read_data_from_spiffs_file_and_format_partition(char* path_to_spiffs_file);
// Clear file with corresponding path
void clear_data_from_spiffs_file(char* path_to_spiffs_file);

// Returns data from spiffs file
esp_err_t read_data_from_spiffs_file_to_buffer(char* path_to_spiffs_file, char* buffer_to_save_data, int buffer_size);