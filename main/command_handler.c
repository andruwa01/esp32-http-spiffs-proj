#include "command_handler.h"

const static char* command_handler_tag = "command_handler";
const static char* next_action_value = "NEXT_ACTION\n";
const static char* text_file_extension = ".txt"; 
const static char* name_postfix_command = "_command";
const static char* name_postfix_response = "_response";

const static char* message_get_command = "get command";
const static char* message_finish_command = "finish working with command";

const static size_t command_size = 128;

const static char* command_send_spiffs_data_to_pc  = "send spiffs data to pc"; 
const static char* command_send_spiffs_info_to_pc  = "get spiffs info";
const static char* command_clean_spiffs_by_id      = "clean spiffs";
const static char* command_clean_spiffs_all        = "clean all";
const static char* command_load_pc_data_to_spiffs  = "load pc data to spiffs";
const static char* command_test                    = "command test";

static void wait(int ms_to_wait){
    // printf("Waiting %i seconds to synchronize time. . .\n", ms_to_wait / 1000);
    vTaskDelay(pdMS_TO_TICKS(ms_to_wait));
}

static void get_data_from_uart(char* buffer_for_contents, size_t size_of_buffer){
    // wait(2500);
    // wait(1000);

    int data_length_chars = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_0, (size_t*)&data_length_chars));
    printf("data in buffer: chars in Rx buffer: %i\n", data_length_chars);

    if(size_of_buffer < data_length_chars){
        ESP_LOGE(command_handler_tag, "Error! buffer of %i bytes don't fit for the data from uart of %i bytes!", size_of_buffer, data_length_chars);
        return;
    }

    // add null terminated symbol so we could correctly read temp_data_buffer
    buffer_for_contents[data_length_chars] = '\0';

    data_length_chars = uart_read_bytes(UART_NUM_0, buffer_for_contents, data_length_chars, 100);

    #if defined(PRINT_DATA_FROM_UART)
    printf("\n%s\n", buffer_for_contents);
    #endif
}

static void wait_response_from_python(char* waiting_debug_info){
        // wait(1500);

        // clear command_buffer value from uart rx
        uart_flush_input(UART_NUM_0);

        char response_buffer[256];
        get_data_from_uart(response_buffer, 256);
        // printf("waiting context: %s\n", waiting_debug_info);
        ESP_LOGW(command_handler_tag, "wait event: %s", waiting_debug_info);

        while(strcmp(response_buffer, "NEXT_ACTION_BOARD") != 0){
            // printf("%s\n", "wait next action message . . .");
            // printf("wait event: %s\n", waiting_debug_info);
            ESP_LOGW(command_handler_tag, "wait event: %s", waiting_debug_info);
            get_data_from_uart(response_buffer, 256);
            printf("%s\n", response_buffer);

            wait(1000);
        }

        // uart_flush_input(UART_NUM_0);
        // wait(1500);
}

static void wait_until_python_process(int ms_to_wait){
    printf("wait %i seconds until python performs. . .\n", ms_to_wait / 1000);
    vTaskDelay(pdMS_TO_TICKS(ms_to_wait));
}

static void create_spiffs_txt_file_path_by_params(char* file_name, const char* name_postfix, char* buffer_to_save_path){
        if (file_name == NULL || buffer_to_save_path == NULL){
            ESP_LOGE(command_handler_tag, "ERROR! NULL pointer was detected! Check function arguments");
            return;
        } else if (name_postfix == NULL){
            name_postfix = "";
        }
        // create file path by satellite name
        char spiffs_satellites_user_input_path[
            strlen(SPIFFS_BASE_PATH) + 
            strlen("/") + 
            strlen(file_name) +
            strlen(name_postfix) +
            strlen(text_file_extension)
        ];

        sprintf(spiffs_satellites_user_input_path, "%s/%s%s%s", SPIFFS_BASE_PATH, file_name, name_postfix, text_file_extension);
        stpcpy(buffer_to_save_path, spiffs_satellites_user_input_path);
}

static void clear_spiffs_file_by_params(char* file_name, const char* name_postfix){
        if(!file_name || !name_postfix){
            ESP_LOGE(command_handler_tag, "file_name or name_postfix is null. Working impossible");
        }
        char file_path_buffer[SPIFFS_MAX_FILE_NAME_LENGTH];
        create_spiffs_txt_file_path_by_params(file_name, name_postfix, file_path_buffer);
        clear_data_from_spiffs_file(file_path_buffer);
}

static void send_response_to_pc(const char* sending_debug_info){
    wait(1000);
    uart_flush(UART_NUM_0);

    ESP_LOGW(command_handler_tag, "send event: %s", sending_debug_info);
    int data_length_chars = 0;
    data_length_chars = uart_write_bytes(UART_NUM_0, next_action_value, strlen(next_action_value));
    ESP_LOGW(command_handler_tag, "%i bytes were sended", data_length_chars);
    
    uart_flush(UART_NUM_0);
    wait(1000);
}

void init_command_handler(){
    if(!uart_is_driver_installed(UART_NUM_0)){
        ESP_LOGE(command_handler_tag, "ERROR! uart driver is not installed");
        return;
    }

    while(true){
        ESP_LOGW(command_handler_tag, "Waiting new command. . .");

        char command_buffer[command_size];
        get_data_from_uart(command_buffer, command_size);


        if(strcmp(command_buffer, command_send_spiffs_data_to_pc) == 0)


        {
            send_response_to_pc(message_get_command); 

            // give some time to python script for writing options file to uart (esle we got runtime exception)
            wait_until_python_process(1000);
            // TODO переписать код выше на wait/send
            char options_file_name[] = "input_options"; 
            char spiffs_satellites_user_input_path[SPIFFS_MAX_FILE_NAME_LENGTH];
            // read file with options
            while(true){
                // test delay wait time for python script
                wait_until_python_process(1000);

                int size_of_buffer = 256;
                char temp_data_buffer[size_of_buffer];
                get_data_from_uart(temp_data_buffer, size_of_buffer);
                
                if(strcmp(temp_data_buffer, "END FILES TRANSMISSION") == 0){
                    printf("%s\n", "END FILES TRANSMISSION RAISED!");

                    send_response_to_pc("stop iterating over file");
                    break;
                }
                // clone the old buffer because when we use strtok - it changes input string
                char temp_data_buffer_changed[strlen(temp_data_buffer)];
                strcpy(temp_data_buffer_changed, temp_data_buffer);
                // start parse data buffer (consists of data_line(s))
                char* first_line_in_file = strtok(temp_data_buffer_changed, "\n");
                if(strcmp(first_line_in_file, "START_FILE") == 0){
                    create_spiffs_txt_file_path_by_params(options_file_name, (char*)name_postfix_command, spiffs_satellites_user_input_path);
                    // first data line is start file line so we need to skip it
                    strtok(temp_data_buffer, "\n");
                    // first data line, to interate over it
                    char* data_line = strtok(NULL, "\n");
                    // Parse all lines and add them to spiffs
                    while(data_line){
                        if(strcmp(data_line, "END_FILE") == 0){
                            // test
                            // read data from python finished
                            send_response_to_pc("send info that we finished read info about python");
                            // so we need to break loop of current file reading (to not add this line to spiffs) 
                            break;
                        }
                        // printf("%s\n", data_line);
                        add_line_to_spiffs(spiffs_satellites_user_input_path, data_line);
                        data_line = strtok(NULL, "\n");
                    }
                }
            }
            //write files to buffer and send after that to python over uart
            char options_data_buffer[256];
            read_data_from_spiffs_file_to_buffer(spiffs_satellites_user_input_path, options_data_buffer, sizeof(options_data_buffer));
            printf("\n%s\n", options_data_buffer);
            // interate over file
            char* data_line_saveptr = NULL;
            char* sat_saveptr = NULL;
            char* data_line = strtok_r(options_data_buffer, "\n", &data_line_saveptr);
            while(data_line){
                // skip sat_name and get to sat_id
                strtok_r(data_line, "=", &sat_saveptr);
                char* sat_id = strtok_r(NULL, "=", &sat_saveptr);
                // create file spiffs path for sat_name
                char spiffs_path[SPIFFS_MAX_FILE_NAME_LENGTH];
                create_spiffs_txt_file_path_by_params(sat_id, name_postfix_response, spiffs_path);
                char response_buffer[RESPONSE_DATA_SIZE];
                if(read_data_from_spiffs_file_to_buffer(spiffs_path, response_buffer, RESPONSE_DATA_SIZE) == ESP_OK){
                    strcat(response_buffer, "END_OF_THE_FILE\n");
                    // send sat data to uart to python script get it
                    int pass_bytes = uart_write_bytes(UART_NUM_0, (const char*)&response_buffer, strlen(response_buffer));
                    ESP_LOGW(command_handler_tag, "%i bytes were sended", pass_bytes);
                }
                create_spiffs_txt_file_path_by_params(sat_id, name_postfix_command, spiffs_path);
                char command_buffer[COMMAND_DATA_SIZE];
                if(read_data_from_spiffs_file_to_buffer(spiffs_path, command_buffer, COMMAND_DATA_SIZE) == ESP_OK){
                    strcat(command_buffer, "END_OF_THE_FILE\n");
                    int command_bytes = uart_write_bytes(UART_NUM_0, (const char*)&command_buffer, strlen(command_buffer));
                    ESP_LOGW(command_handler_tag, "%i bytes were sended", command_bytes);
                }
                data_line = strtok_r(NULL, "\n", &data_line_saveptr);
            }
            clear_data_from_spiffs_file(spiffs_satellites_user_input_path);
            wait_response_from_python("wait signal from python that it finished handle data");
            send_response_to_pc(message_finish_command);
        }


        else if(strcmp(command_buffer, command_clean_spiffs_all) == 0)


        {
            DIR *dptr;
            struct dirent *dir;
            dptr = opendir(SPIFFS_BASE_PATH);
            if(dptr){
                while((dir = readdir(dptr)) != NULL){
                    char file_path[SPIFFS_MAX_FILE_NAME_LENGTH + strlen("/") + strlen(dir->d_name)];
                    sprintf(file_path, "%s/%s", SPIFFS_BASE_PATH, dir->d_name);
                    clear_data_from_spiffs_file(file_path);
                }
                closedir(dptr);
            }
            send_response_to_pc("finish working with command");
        }


        else if(strcmp(command_buffer, command_clean_spiffs_by_id) == 0)


        {
            #if defined(SPIFFS_CLEAR_FILES)
            send_response_to_pc(message_get_command);
            send_response_to_pc("start waiting list of satellites");
            wait_response_from_python("python wrote list to buffer");
            // get data from uart to buffer 
            size_t buffer_size = 256;
            char temp_data_buffer[buffer_size];
            get_data_from_uart(temp_data_buffer, buffer_size);
            // parse list of satellites names (start interating over it)
            char* first_line_in_file = strtok(temp_data_buffer, "\n");
            // check if broken package / first line is empty
            if(first_line_in_file == NULL){
                ESP_LOGE(command_handler_tag, "ERROR! First line is empty! File is empty!");
                return;
            }
            // clear first line in spiffs file (first filename)
            clear_spiffs_file_by_params(first_line_in_file, name_postfix_response);
            clear_spiffs_file_by_params(first_line_in_file, name_postfix_command);
            // next line in file (filename)
            char* data_line = strtok(NULL, "\n");
            // clear_satellite_files_by_name();
            while(data_line){
                clear_spiffs_file_by_params(data_line, name_postfix_response);
                clear_spiffs_file_by_params(data_line, name_postfix_command);
                // clear next file name line 
                data_line = strtok(NULL, "\n");
            }
            #endif
            send_response_to_pc(message_finish_command);
        }


        else if(strcmp(command_buffer, command_send_spiffs_info_to_pc) == 0)


        {
            printf("%s command is realized\n", command_buffer);

            send_response_to_pc(message_get_command);
            wait_response_from_python("python starts reading data");
            // check spiffs
            ESP_ERROR_CHECK(esp_spiffs_check(SPIFFS_PARTITION_LABEL));
            size_t spiffs_total, spiffs_used = 0;
            ESP_ERROR_CHECK(esp_spiffs_info(SPIFFS_PARTITION_LABEL, &spiffs_total, &spiffs_used));
            char spiffs_data[128];
            sprintf(spiffs_data, "bytes: spiffs_total=%i spiffs_used=%i\n", spiffs_total, spiffs_used);
            ESP_LOGW(command_handler_tag, "\n%s", spiffs_data);
            // transmit this data to print in in python script 
            uart_write_bytes(UART_NUM_0, (void*)spiffs_data, strlen(spiffs_data));
            send_response_to_pc("signal that we finished writing first part of info");
            // wait signal that we can write to python 
            wait_response_from_python("board can write files to python");
            // push information about all files in spiffs
            // initialize empty string
            char spiffs_files_info[2 * SPIFFS_MAX_FILES * (SPIFFS_MAX_FILE_NAME_LENGTH + strlen(": ") + 4 * sizeof(char))];
            spiffs_files_info[0] = '\0';

            DIR* dptr;
            struct dirent* dir;
            dptr = opendir(SPIFFS_BASE_PATH);
            if(dptr){
                while((dir = readdir(dptr)) != NULL){
                    char file_path[SPIFFS_MAX_FILE_NAME_LENGTH + strlen("/") + strlen(dir->d_name)];
                    sprintf(file_path, "%s/%s", SPIFFS_BASE_PATH, dir->d_name);
                    // get the size of one file
                    size_t size_of_file;
                    FILE* fp = fopen(file_path, "r");
                    fseek(fp, 0, SEEK_END);
                    size_of_file = ftell(fp);
                    fclose(fp);
                    char file_stats_line[strlen(dir->d_name) + strlen(": ") + 4 * sizeof(char)];
                    sprintf(file_stats_line, "%s: %i\n", dir->d_name, size_of_file);
                    strcat(spiffs_files_info, file_stats_line);
                }
                closedir(dptr);
            }
            uart_write_bytes(UART_NUM_0, spiffs_files_info, strlen(spiffs_files_info));
            wait_response_from_python("python finishes working with data");
            send_response_to_pc(message_finish_command);
        }


        else if(strcmp(command_buffer, command_load_pc_data_to_spiffs) == 0)


        {
            printf("%s command is realized\n", command_buffer);
            // send signal to python that we got command
            send_response_to_pc(message_get_command);
            // continue only when python starts reading files
            wait_response_from_python("wait when python starts reading files");
            // get stats about spiffs
            size_t total = 0, used = 0;
            esp_spiffs_info(SPIFFS_PARTITION_LABEL, &total, &used);
            size_t free_space = total - used;
            char free_space_buffer[strlen("free=") + 4 * sizeof(char) + sizeof("\n")];
            sprintf(free_space_buffer, "free=%i\n", free_space);
            // send free space information to python script
            uart_write_bytes(UART_NUM_0, free_space_buffer, strlen(free_space_buffer));
            // wait info from python that it finished reading data
            wait_response_from_python("finish reading data");
            // response to python that we ready to get files with data
            send_response_to_pc("ready to get files with data");
            // iterate over command files
            while(true){
                // give 1 sec to avoid bug when while(true) works too fast and couses problems
                wait(1000);
                char pass_buffer[RESPONSE_DATA_SIZE];
                get_data_from_uart(pass_buffer, RESPONSE_DATA_SIZE);
                // test print
                ESP_LOGI(command_handler_tag, "\n%s", pass_buffer);
                if(strcmp(pass_buffer, "END FILES TRANSMISSION") == 0){
                    printf("%s\n", "END FILES TRANSMISSION RAISED!");
                    // so stop iterate over command files and send info about this in python 
                    send_response_to_pc("end files transmission");
                    break;
                }
                // clone the old buffer because when we use strtok - it changes input string
                char temp_data_buffer_for_getting_name[strlen(pass_buffer)];
                strcpy(temp_data_buffer_for_getting_name, pass_buffer);

                // TODO добавить три линии ниже на самый верх файла через const static char*
                const char line_delimiter[] = "\n";
                const char req_data_delimiter[] = ": "; 
                const char param_data_delimiter[] = "=";

                char* line_saveptr = NULL;
                char* element_saveptr = NULL;
                // start parse data buffer (consists of data_line(s))
                char* first_line_in_file = strtok_r(temp_data_buffer_for_getting_name, line_delimiter, &line_saveptr);
                if(strcmp(first_line_in_file, "START_FILE") == 0){
                    char* data_line_with_id = strtok_r(NULL, line_delimiter, &line_saveptr);
                    char old_line_with_id[strlen(data_line_with_id)];
                    strcpy(old_line_with_id, data_line_with_id);
                    char current_delimiter[strlen(req_data_delimiter) + 1];
                    strcpy(current_delimiter, req_data_delimiter);
                    char name_postfix[strlen(name_postfix_command) + strlen(name_postfix_response)];
                    char* test_ptr = strtok_r(data_line_with_id, current_delimiter, &element_saveptr);
                    ESP_LOGW(command_handler_tag, "test ptr: %s\n", test_ptr);
                    // if it is no pass
                    if(strcmp(test_ptr, old_line_with_id) == 0){
                        // change delimiter to param
                        strcpy(current_delimiter, param_data_delimiter);
                        // skip element left from =
                        strtok_r(test_ptr, current_delimiter, &element_saveptr);
                        // change file postfix to param postfix
                        strcpy(name_postfix, name_postfix_command);
                    } else {
                        // change file postfix to pass postfix
                        strcpy(name_postfix, name_postfix_response);
                    }
                    char* satellite_id = strtok_r(NULL, current_delimiter, &element_saveptr); 
                    char spiffs_passes_file_path[SPIFFS_MAX_FILE_NAME_LENGTH];
                    create_spiffs_txt_file_path_by_params(satellite_id, name_postfix, spiffs_passes_file_path);
                    // clear spiffs file before writing to it (to override old file)
                    clear_data_from_spiffs_file(spiffs_passes_file_path);
                    // first data line is start file line so we need to skip it
                    strtok(pass_buffer, "\n");
                    // first data line, to interate over it
                    char* data_line = strtok(NULL, "\n");
                    // Parse all lines and add them to spiffs
                    while(data_line){
                        if(strcmp(data_line, "END_FILE") == 0){
                            // so we need to break loop of current file reading (to not add this line to spiffs) 
                            break;
                        }
                        add_line_to_spiffs(spiffs_passes_file_path, data_line);
                        data_line = strtok(NULL, "\n");
                    }
                    // clean data about file from uart to free space for other file
                    uart_flush(UART_NUM_0);
                }
                // signal to python that we could read another file
                send_response_to_pc("we can read another file");
            }
            send_response_to_pc(message_finish_command);
        }


        else if(strcmp(command_buffer, command_test) == 0)


        {
            printf("get command %s\n", command_buffer);
            send_response_to_pc(message_get_command);
            wait_response_from_python("wait data is sended");
            char file_buffer[128];
            get_data_from_uart(file_buffer, 128);
            printf("%s\n", file_buffer);
            // send info that we end working with this command
            send_response_to_pc(message_finish_command);
        }


        else


        {
            ESP_LOGW(command_handler_tag, "\nNo command, new cycle");
        }

        wait(1000);
    }
    esp_get_free_heap_size()
}