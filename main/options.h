//====FUNCTIONS====// 

// SPIFFS
#define SPIFFS_USE_FUNCTIONALITY
#define SPIFFS_CHECK
#define SPIFFS_CHECK_ON_START
#define SPIFFS_PARTITION_LABEL "data_storage"
#define SPIFFS_CLEAR_FILE_AFTER_READ_FROM
#define SPIFFS_MAX_FILES 15
#define SPIFFS_MAX_FILE_NAME_LENGTH 25
#define SPIFFS_MAX_NUMBER_OF_SYMBOLS_IN_ONE_FILE 4000

#define SPIFFS_NUMBER_OF_FILES sizeof(spiffs_file_names) / sizeof(spiffs_file_names[0])

#define SPIFFS_BASE_PATH "/spiffs"
#define SPIFFS_FEES_FILE_PATH "/spiffs/fees.txt"
#define SPIFFS_NORBI_FILE_PATH "/spiffs/norbi.txt"

#define SPIFFS_FILE_PATH SPIFFS_NORBI_FILE_PATH

const static char spiffs_file_names[SPIFFS_MAX_FILES][SPIFFS_MAX_FILE_NAME_LENGTH] = {
    "norbi.txt",
    "2023-091t.txt",
    "cstp-1.1.txt",
    "cstp-1.2.txt",
    "fees.txt",
    "fossasat2e13.txt",
    "jilin-01_gaofen_2f.txt",
    "mdqubesat-2.txt",
    "polytech_universe-3.txt",
    "rs52sb.txt",
    "rs52sd.txt",
    "rs52se.txt",
    "rs52sg.txt",
    "rs52sv.txt",
    "tianqi-7.txt"
};


#define SPIFFS_LOGS

// #define PRINT_PACKAGE
#define VORONEZH_UTC_CORRECTION
// #define ENABLE_TIME_CONVERTER_LOGS

//===== PARAMS =====//

#define HTTP_BUFFER_SIZE 4096

// WARNING! COULD BE STACK OVERFLOW, LOOK AT CONFIG_ESP_MAIN_TASK_STACK_SIZE
// in skkconfig / idf.py menuconfib  