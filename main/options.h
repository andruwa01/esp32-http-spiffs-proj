#include "sdkconfig.h"

#define GET_REQUEST_ALL_SAT

// #define LOAD_ONE_FILE_UART
#define LOAD_ALL_COMMAND_FILES

#define SPIFFS_USE_FUNCTIONALITY
#define SPIFFS_CHECK_ON_START
#define SPIFFS_LOGS
// #define SPIFFS_PRINT_ALL_FILES
// #define SPIFFS_CLEAR_FILES_ON_START
// #define SIPFFS_CLEAR_FILE_AFTER_READING
#define SPIFFS_CLEAR_FILES
#define SPIFFS_PARSE_RESPONSE
#define SPIFFS_MAX_FILES 15 // TODO configure this automatically 
#define SPIFFS_NUMBER_OF_FILES SPIFFS_MAX_FILES
#define SPIFFS_MAX_FILE_NAME_LENGTH CONFIG_SPIFFS_OBJ_NAME_LEN - 8 // CONFIG_SPIFFS_OBJ_NAME_LEN - strlen("/spiffs/")
// #define SPIFFS_MAX_NUMBER_OF_SYMBOLS_IN_ONE_FILE 4000
#define SPIFFS_BASE_PATH "/spiffs"
#define SPIFFS_PARTITION_LABEL "data_storage"
#define SPIFFS_FILE_PATH SPIFFS_NORBI_FILE_PATH
// #define UART_TEST
// #define SEND_DATA_FROM_SPIFFS_TO_UART
// #define PRINT_PACKAGE
#define VORONEZH_UTC_CORRECTION
// #define ENABLE_TIME_CONVERTER_LOGS
#define HTTP_TIMOUT_MS 15000
#define HTTP_BUFFER_SIZE 5512
#define TIME_TO_PUSH_BUTTON_MS 60000
// #define PRINT_DATA_FROM_UART
#define PASS_DATA_SIZE 5512
#define COMMAND_DATA_SIZE 1256
// WARNING! COULD BE STACK OVERFLOW, LOOK AT CONFIG_ESP_MAIN_TASK_STACK_SIZE
// in skkconfig / idf.py menuconfib  

// task delay for function and python. needs to be greater that timeout value
// of serial port opened in python script
#define TIME_DELAY_BEFORE_RESPONSE_SENDED_MS SPIFFS_MAX_FILES * 1000 + 6000
typedef struct{
    char name[SPIFFS_MAX_FILE_NAME_LENGTH];
    int id;
} satellite_data; 

extern satellite_data satellites[SPIFFS_NUMBER_OF_FILES];