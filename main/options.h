#ifndef OPTIONS_H
#define OPTIONS_H

#include "sdkconfig.h"

//=======================
// define method of working with program
// #define HANDLE_UART
#define HANDLE_UDP

// + change set sdkconfigdefaults file in high level cmakelists.txt
//=======================

// #define GET_REQUEST_ALL_SAT
// #define LOAD_ONE_FILE_UART
// #define LOAD_ALL_COMMAND_FILES

// #define SPIFFS_USE_FUNCTIONALITY
// #define SPIFFS_CHECK_ON_START
// #define SPIFFS_LOGS
// #define SPIFFS_PRINT_ALL_FILES
// #define SPIFFS_CLEAR_FILES_ON_START
// #define SIPFFS_CLEAR_FILE_AFTER_READING
// #define SPIFFS_CLEAR_FILES
// #define SPIFFS_PARSE_RESPONSE
#define SPIFFS_MAX_FILES 15 // TODO configure this automatically 
#define SPIFFS_NUMBER_OF_FILES SPIFFS_MAX_FILES
#define SPIFFS_FILE_NAME_LENGTH_MAX CONFIG_SPIFFS_OBJ_NAME_LEN - strlen("/spiffs/") // CONFIG_SPIFFS_OBJ_NAME_LEN - strlen("/spiffs/")
// #define SPIFFS_MAX_NUMBER_OF_SYMBOLS_IN_ONE_FILE 4000
#define SPIFFS_BASE_PATH "/spiffs"
#define SPIFFS_PARTITION_LABEL "data_storage"
// #define SPIFFS_FILE_PATH SPIFFS_NORBI_FILE_PATH
// #define UART_TEST
// #define SEND_DATA_FROM_SPIFFS_TO_UART
// #define PRINT_PACKAGE
// #define VORONEZH_UTC_CORRECTION
// #define ENABLE_TIME_CONVERTER_LOGS
// #define HTTP_TIMOUT_MS 15000
// #define HTTP_BUFFER_SIZE 5512
// #define TIME_TO_PUSH_BUTTON_MS 60000
// #define PRINT_DATA_FROM_UART

#define SIZE_RESPONSE_DATA_MAX 6536
#define SIZE_COMMAND_DATA_MAX 1256
#define SIZE_OPTIONS_FILE_MAX 512
#define SIZE_DATA_CHUNK_UDP_MAX 512 

// WARNING! COULD BE STACK OVERFLOW, LOOK AT CONFIG_ESP_MAIN_TASK_STACK_SIZE
// in skkconfig / idf.py menuconfib  

#endif