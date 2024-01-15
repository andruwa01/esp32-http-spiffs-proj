#include "main.h"

// Initializing spiffs with corresponding logs
void spiffs_handler();

// Add file to spiffs filesystem
void add_to_spiffs(char *path_to_spiffs_file, char *text_to_write);

// Read file contents from spiffs filesystem
void read_file_from_spiffs(char *path_to_spiffs_file);