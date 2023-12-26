#include "main.h"

// Convert time_t *time_to_convert -> char *buffer_for_converted_string
void time_converter(time_t *time_to_convert, char *format_to_convert, char *buffer_for_converted_string, int size_of_buffer);