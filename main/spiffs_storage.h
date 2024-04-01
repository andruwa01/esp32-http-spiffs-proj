#ifndef SPIFFS_H
#define SPIFFS_H

#include "esp_err.h"
#include "esp_spiffs.h"

// Initializing spiffs with corresponding logs
void initialize_spiffs(void);

// Add line to spiffs filesystem
void add_line_to_spiffs(char* path_to_spiffs_file, char* text_to_write);

// Add file to spiffs filesystem
void add_file_to_spiffs(char *path_to_spiffs_file, char *buffer_to_write);

// Read file contents from spiffs filesystem
void read_data_from_spiffs_file_and_format_partition(const char* path_to_spiffs_file, char* partition_label_to_format);

// void* read_data_from_spiffs_file_and_format_partition(char* path_to_spiffs_file);
// Clear file with corresponding path
void clear_data_from_spiffs_file(char* path_to_spiffs_file);

// Returns data from spiffs file
esp_err_t read_data_from_spiffs_file_to_buffer(const char* path_to_spiffs_file, char* buffer_to_save_data, int buffer_size);

#endif