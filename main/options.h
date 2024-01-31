#include "sdkconfig.h"
//====FUNCTIONS====// 

// SPIFFS
#define SPIFFS_USE_FUNCTIONALITY
#define SPIFFS_CHECK_ON_START
// #define SPIFFS_LOGS

// #define SPIFFS_CLEAR_FILE_AFTER_READ_FROM
#define SPIFFS_CLEAR_FILES_ON_START
#define SPIFFS_READ_ALL_FILES
#define SPIFFS_PARSE_RESPONSE

#define SPIFFS_MAX_FILES 15
#define SPIFFS_NUMBER_OF_FILES SPIFFS_MAX_FILES
#define SPIFFS_MAX_FILE_NAME_LENGTH CONFIG_SPIFFS_OBJ_NAME_LEN - 8 // CONFIG_SPIFFS_OBJ_NAME_LEN - strlen("/spiffs/")
// #define SPIFFS_MAX_NUMBER_OF_SYMBOLS_IN_ONE_FILE 4000
#define SPIFFS_BASE_PATH "/spiffs"
#define SPIFFS_PARTITION_LABEL "data_storage"

#define SPIFFS_FILE_PATH SPIFFS_NORBI_FILE_PATH

typedef struct{
    char name[SPIFFS_MAX_FILE_NAME_LENGTH];
    int id;
} satellite; 

const static satellite satellites[SPIFFS_NUMBER_OF_FILES] = {
    {.name = "norbi.txt", .id = 46494},
    {.name = "2023-091t.txt", .id = 57183},
    {.name = "cstp-1.1.txt", .id = 57202},
    {.name = "cstp-1.2.txt", .id = 57186},
    {.name = "fees.txt", .id = 48082},
    {.name = "fossasat.txt", .id = 52776},
    {.name = "jilingaofen.txt", .id = 49338},
    {.name = "mdqubesat-2.txt", .id = 58665}, 
    {.name = "polytech_un.txt", .id = 57191},
    {.name = "rs52sb.txt", .id = 57200},
    {.name = "rs52sd.txt", .id = 57208},
    {.name = "rs52se.txt", .id = 57205},
    {.name = "rs52sg.txt", .id = 57170},
    {.name = "rs52sv.txt", .id = 57169},
    {.name = "tianqi-7.txt", .id = 54687}
};

// #define PRINT_PACKAGE
#define VORONEZH_UTC_CORRECTION
// #define ENABLE_TIME_CONVERTER_LOGS
#define HTTP_BUFFER_SIZE 4128
#define TIME_TO_PUSH_BUTTON_MS 60000
// WARNING! COULD BE STACK OVERFLOW, LOOK AT CONFIG_ESP_MAIN_TASK_STACK_SIZE
// in skkconfig / idf.py menuconfib  