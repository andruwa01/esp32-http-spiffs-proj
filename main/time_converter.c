#include "time_converter.h"

static const char* converter_tag = "time_converter";

// static time_t time_to_convert_default = 1703237825;
// static char default_format[] = "%d.%m.%Y %H:%M";


void time_converter(time_t *time_to_convert, char *format_to_convert, char *buffer_for_converted_string, int size_of_buffer){

        #ifdef ENABLE_TIME_CONVERTER_LOGS      
                ESP_LOGI(converter_tag, "Start to convert time with format: %s", format_to_convert);
        #endif

        struct tm time_buffer;
        localtime_r(time_to_convert, &time_buffer);

        #ifdef VORONEZH_UTC_CORRECTION
        if(time_buffer.tm_hour > 20 && time_buffer.tm_hour < 24){
                int previous_tm_hour = time_buffer.tm_hour;
                time_buffer.tm_hour = 24 - (previous_tm_hour + 1); 
        } else {
                time_buffer.tm_hour += 3; //voronezh utc + 3
        } 
        #endif

        //TODO write if for it and ESP_LOGE if something bad
        strftime(buffer_for_converted_string, size_of_buffer, format_to_convert, &time_buffer);

        #ifdef ENABLE_TIME_CONVERTER_LOGS
                ESP_LOGI(converter_tag, "Finish converting process");
        #endif
}