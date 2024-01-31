#include "sdkconfig.h"
//====FUNCTIONS====// 

// SPIFFS
#define SPIFFS_USE_FUNCTIONALITY
#define SPIFFS_CHECK_ON_START
#define SPIFFS_PARTITION_LABEL "data_storage"
// #define SPIFFS_CLEAR_FILE_AFTER_READ_FROM
#define SPIFFS_CLEAR_FILES_ON_START
#define SPIFFS_MAX_FILES 15
#define SPIFFS_MAX_FILE_NAME_LENGTH CONFIG_SPIFFS_OBJ_NAME_LEN - 8 // CONFIG_SPIFFS_OBJ_NAME_LEN - strlen("/spiffs/")
#define SPIFFS_MAX_NUMBER_OF_SYMBOLS_IN_ONE_FILE 4000
#define SPIFFS_READ_ALL_FILES

#define SPIFFS_NUMBER_OF_FILES sizeof(spiffs_file_names) / sizeof(spiffs_file_names[0])

#define SPIFFS_BASE_PATH "/spiffs"
#define SPIFFS_FEES_FILE_PATH "/spiffs/fees.txt"
#define SPIFFS_NORBI_FILE_PATH "/spiffs/norbi.txt"

#define SPIFFS_FILE_PATH SPIFFS_NORBI_FILE_PATH

static char spiffs_file_names[SPIFFS_MAX_FILES][SPIFFS_MAX_FILE_NAME_LENGTH] = {
    "norbi.txt",
    "2023-091t.txt",
    "cstp-1.1.txt",
    "cstp-1.2.txt",
    "fees.txt",
    "fossasat.txt",
    "jilingaofen.txt",
    "mdqubesat-2.txt",
    "polytech_un.txt",
    "rs52sb.txt",
    "rs52sd.txt",
    "rs52se.txt",
    "rs52sg.txt",
    "rs52sv.txt",
    "tianqi-7.txt"
};

// struct satellites_filename_id{
//     int satellite_id;
//     char satellite_file_name[SPIFFS_MAX_FILE_NAME_LENGTH];
// };

// Modify it with structs
const static int satellites_id[SPIFFS_MAX_FILES] = {
    46494, // NORBI
    57183, // 2023-**
    57202, // cstp 1.1
    57186, // cstp 1.2
    48082, // fees
    52776, // fossat
    49338, // jilin
    58665, // mdqubesat
    57191, // polytech_universe
    57200, // rs52sb
    57208, // rs52sd
    57205, // rs52se
    57170, // rs52sg
    57169, // rs52sv
    54687, // tianqui-7
};

// #define SPIFFS_LOGS

// #define PRINT_PACKAGE
#define VORONEZH_UTC_CORRECTION
// #define ENABLE_TIME_CONVERTER_LOGS

//===== PARAMS =====//

#define SPIFFS_PARSE_RESPONSE
#define HTTP_BUFFER_SIZE 4128
#define TIME_TO_PUSH_BUTTON_MS 60000

// WARNING! COULD BE STACK OVERFLOW, LOOK AT CONFIG_ESP_MAIN_TASK_STACK_SIZE
// in skkconfig / idf.py menuconfib  