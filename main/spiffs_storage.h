#include "main.h"

// Initializing spiffs with corresponding logs
void initialize_spiffs();

// Add file to spiffs filesystem
void add_line_to_spiffs(char *path_to_spiffs_file, char *text_to_write);

// Read file contents from spiffs filesystem
void read_file_from_spiffs_file_and_format(char *path_to_spiffs_file, char *partition_label_to_format);