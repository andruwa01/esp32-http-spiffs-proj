#include "sdkconfig.h"

#define GET_REQUEST_ALL_SAT

#define SPIFFS_USE_FUNCTIONALITY
#define SPIFFS_CHECK_ON_START
// #define SPIFFS_LOGS

// #define SPIFFS_READ_ALL_FILES
#define SPIFFS_CLEAR_FILES_ON_START
// #define SIPFFS_CLEAR_FILE_AFTER_READING
#define SPIFFS_PARSE_RESPONSE

#define SPIFFS_MAX_FILES 15
#define SPIFFS_NUMBER_OF_FILES SPIFFS_MAX_FILES
#define SPIFFS_MAX_FILE_NAME_LENGTH CONFIG_SPIFFS_OBJ_NAME_LEN - 8 // CONFIG_SPIFFS_OBJ_NAME_LEN - strlen("/spiffs/")
// #define SPIFFS_MAX_NUMBER_OF_SYMBOLS_IN_ONE_FILE 4000
#define SPIFFS_BASE_PATH "/spiffs"
#define SPIFFS_PARTITION_LABEL "data_storage"

#define SPIFFS_FILE_PATH SPIFFS_NORBI_FILE_PATH

#define SEND_DATA_FROM_SPIFFS_TO_UART

// #define PRINT_PACKAGE
#define VORONEZH_UTC_CORRECTION
// #define ENABLE_TIME_CONVERTER_LOGS
#define HTTP_TIMOUT_MS 10000
#define HTTP_BUFFER_SIZE 4128
#define TIME_TO_PUSH_BUTTON_MS 60000
// WARNING! COULD BE STACK OVERFLOW, LOOK AT CONFIG_ESP_MAIN_TASK_STACK_SIZE
// in skkconfig / idf.py menuconfib  

typedef struct{
    char name[SPIFFS_MAX_FILE_NAME_LENGTH];
    int id;
} satellite; 

extern satellite satellites[SPIFFS_NUMBER_OF_FILES];