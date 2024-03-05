#include "command_handler.h"

const static char* command_handler_tag = "command_handler";
const static char next_action_value[16] = "NEXT_ACTION\n";
const static char* next_file_command = "NEXT_FILE";
const static char* text_file_extension = ".txt"; 
const static char* name_postfix_command = "_command";
const static char* name_postfix_pass = "_passes";
const static size_t command_size = 128;


// static void wait_response_from_pc(void){
//     char response_correct_value[] = "RESPONSE FROM PC";
//     char response_from_uart[256]; 

//     int data_length_chars = 0;
//     ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_0, (size_t*)&data_length_chars));
//     printf("chars in Rx buffer: %i\n", data_length_chars);

//     while(true){
//         ESP_LOGW(command_handler_tag, "Watiting response from pc . . .");
//         data_length_chars = uart_read_bytes(UART_NUM_0, response_from_uart, 256, 100);

//         if(strcmp(response_from_uart, response_correct_value) == 0){
//             if(data_length_chars){
//                 ESP_LOGW(command_handler_tag, "%i chars was readed, contents: %s", data_length_chars, (char*)response_from_uart);
//             }
//             break;
//         }

//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
//     ESP_LOGW(command_handler_tag, "Got response from pc!");
//     uart_flush(UART_NUM_0);
// }

static void wait_until_python_process(int ms_to_wait){
    printf("wait %i seconds until python performs. . .\n", ms_to_wait / 1000);
    vTaskDelay(pdMS_TO_TICKS(ms_to_wait));
}

static void get_data_from_uart(char* buffer_for_contents, size_t size_of_buffer){
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

static void create_spiffs_txt_file_path_by_params(char* file_name, const char* name_postfix, char* buffer_to_save_path){
        if (file_name == NULL || buffer_to_save_path == NULL){
            ESP_LOGE(command_handler_tag, "ERROR! NULL pointer was detected! Check function arguments");
            return;
        } else if (name_postfix == NULL){
            name_postfix = "";
        }
        // create file path by satellite name
        // char user_input_folder[] = "passes_user_input";
        char spiffs_satellites_user_input_path[
            strlen(SPIFFS_BASE_PATH) + 
            strlen("/") + 
            strlen(file_name) +
            strlen(name_postfix) +
            strlen(text_file_extension)
            ];

        sprintf(spiffs_satellites_user_input_path, "%s/%s%s%s", SPIFFS_BASE_PATH, file_name, name_postfix, text_file_extension);

        stpcpy(buffer_to_save_path, spiffs_satellites_user_input_path);

        // test print
        // printf("command file path: %s\n", buffer_to_save_path);
}

static void clear_spiffs_file_by_params(char* file_name, const char* name_postfix){
        char file_path_buffer[SPIFFS_MAX_FILE_NAME_LENGTH];
        create_spiffs_txt_file_path_by_params(file_name, name_postfix, file_path_buffer);
        clear_data_from_spiffs_file(file_path_buffer);
}

static void response_next_action(void){
    wait_until_python_process(1000);

    uart_flush(UART_NUM_0);

    int data_length_chars = 0;
    data_length_chars = uart_write_bytes(UART_NUM_0, &next_action_value, strlen(next_action_value));
    ESP_LOGW(command_handler_tag, "%i bytes were sended", data_length_chars);
    
    uart_flush(UART_NUM_0);
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

        // if(strcmp(command_buffer, "get all") == 0)
        // {
        //     #if defined(GET_REQUEST_ALL_SAT)
        //     initialize_get_requests_for_all_satellites();

        //     printf("current command (after clearence): %s\n", command_buffer);
        //     #endif

        //     response_next_action();
        // }
        if(strcmp(command_buffer, "give spiffs data to pc") == 0)
        {
            // we got command
            response_next_action(); 

            // give some time to python script for writing options file to uart (esle we got runtime exception)
            wait_until_python_process(1000);

            #if defined(SEND_DATA_FROM_SPIFFS_TO_UART)
            for(int file_index = 0; file_index < SPIFFS_NUMBER_OF_FILES; file_index++){
                char spiffs_file_path[strlen(SPIFFS_BASE_PATH) + strlen("/") + strlen(satellites[file_index].name)];
                sprintf(spiffs_file_path, "%s/%s", SPIFFS_BASE_PATH, satellites[file_index].name);

                char satellite_data[HTTP_BUFFER_SIZE];
                read_data_from_spiffs_file_to_buffer(spiffs_file_path, satellite_data, HTTP_BUFFER_SIZE);
                strcat(satellite_data, "END_OF_THE_FILE\n");

                int sended_bytes = uart_write_bytes(UART_NUM_0, (const char*)&satellite_data, strlen(satellite_data));
                ESP_LOGW(command_handler_tag, "%i bytes were sended", sended_bytes);

            }
            #endif

            // int size_of_buffer = 256;
            // char temp_data_buffer[size_of_buffer];
            // get_data_from_uart(temp_data_buffer, size_of_buffer);

            // we read file
            // response_next_action();
            
            char options_file_name[] = "input_options"; 
            char spiffs_satellites_user_input_path[SPIFFS_MAX_FILE_NAME_LENGTH];

            while(true){
                // test delay wait time for python script
                wait_until_python_process(1000);

                int size_of_buffer = 256;
                char temp_data_buffer[size_of_buffer];
                get_data_from_uart(temp_data_buffer, size_of_buffer);
                
                // TODO delete
                // got data from python
                // response_next_action();

                if(strcmp(temp_data_buffer, "END FILES TRANSMISSION") == 0){
                    printf("%s\n", "END FILES TRANSMISSION RAISED!");

                    // so stop iterate over command files and send info about this in python 
                    response_next_action();
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
                    // char* first_line_in_file = strtok(temp_data_buffer, "\n");
                    strtok(temp_data_buffer, "\n");
                    // first data line, to interate over it
                    char* data_line = strtok(NULL, "\n");
                    // Parse all lines and add them to spiffs
                    while(data_line){
                        if(strcmp(data_line, "END_FILE") == 0){
                            // test
                            // read data from python finished
                            response_next_action();

                            // so we need to break loop of current file reading (to not add this line to spiffs) 
                            break;
                        }
                        // printf("%s\n", data_line);
                        add_line_to_spiffs(spiffs_satellites_user_input_path, data_line);
                        // test print
                        // printf("%s\n", data_line);
                        data_line = strtok(NULL, "\n");
                    }

                    // TODO think about to save this data to spiffs to then reuse it
                    // clear this file after working with it
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
                create_spiffs_txt_file_path_by_params(sat_id, name_postfix_pass, spiffs_path);
                char pass_buffer[PASS_DATA_SIZE];
                read_data_from_spiffs_file_to_buffer(spiffs_path, pass_buffer, PASS_DATA_SIZE);
                strcat(pass_buffer, "END_OF_THE_FILE\n");

                //test
                create_spiffs_txt_file_path_by_params(sat_id, name_postfix_command, spiffs_path);
                char command_buffer[COMMAND_DATA_SIZE];
                read_data_from_spiffs_file_to_buffer(spiffs_path, command_buffer, COMMAND_DATA_SIZE);
                strcat(command_buffer, "END_OF_THE_FILE\n");

                // send sat data to uart to python script get it
                int pass_bytes = uart_write_bytes(UART_NUM_0, (const char*)&pass_buffer, strlen(pass_buffer));
                ESP_LOGW(command_handler_tag, "%i bytes were sended", pass_bytes);

                // test
                int command_bytes = uart_write_bytes(UART_NUM_0, (const char*)&command_buffer, strlen(command_buffer));
                ESP_LOGW(command_handler_tag, "%i bytes were sended", command_bytes);

                // test print
                // printf("sat_name: %s\n", sat_name);
                data_line = strtok_r(NULL, "\n", &data_line_saveptr);
            }
            // TODO delete
            // finish writing file
            // response_next_action();

            // clear command file on finish
            clear_data_from_spiffs_file(spiffs_satellites_user_input_path);

            wait_until_python_process(TIME_DELAY_BEFORE_RESPONSE_SENDED_MS);
            // response_next_action();

            response_next_action();
        }


        else if(strcmp(command_buffer, "clean all") == 0)


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

            response_next_action();
        }


        else if(strcmp(command_buffer, "clean spiffs") == 0)


        {
            #if defined(SPIFFS_CLEAR_FILES)

            response_next_action();

            // todo сделать функцию, которая будет ожидать ответа от python о том, что она готова продолжить,
            // тогда можно будет применить её и к этой команде и к тому случаю ,когда у нас выгружаются большие объёмы данных с 
            // python. В данном случае delay является костылём - каким он и является в случае любого большого объёма данных и костыль этот работает до тех пор, пока 
            // данные с питона отправляются достаточно быстро, в противном случае эти данные не попадут в код строчек ниже и пройдут дальше, что будет являться ошибкой

            // give 5 sec write list to buffer
            vTaskDelay(pdMS_TO_TICKS(5000));

            // get data from uart to buffer 
            size_t buffer_size = 256;
            char temp_data_buffer[buffer_size];
            get_data_from_uart(temp_data_buffer, buffer_size);

            // parse list of satellites names (start interating over it)
            char* first_line_in_file = strtok(temp_data_buffer, "\n");

            // test print
            printf("first line in file: %s\n", first_line_in_file);

            // check if broken package / first line is empty
            if(first_line_in_file == NULL){
                ESP_LOGE(command_handler_tag, "ERROR! First line is empty! File is empty!");
                return;
            }

            // clear first line in spiffs file (first filename)
            clear_spiffs_file_by_params(first_line_in_file, name_postfix_pass);
            clear_spiffs_file_by_params(first_line_in_file, name_postfix_command);

            // next line in file (filename)
            char* data_line = strtok(NULL, "\n");

            // clear_satellite_files_by_name();
            while(data_line){
                clear_spiffs_file_by_params(data_line, name_postfix_pass);
                clear_spiffs_file_by_params(data_line, name_postfix_command);
                // clear next file name line 
                data_line = strtok(NULL, "\n");
            }
            #endif

            response_next_action();
        }


        else if(strcmp(command_buffer, "push command files") == 0)


        {
            printf("%s command is realized\n", command_buffer);

            // send signal to python that we got command
            response_next_action();

            #if defined(LOAD_ALL_COMMAND_FILES)

            // iterate over command files
            while(true){

                // give 1 sec to avoid bug when while(true) works too fast and couses problems
                vTaskDelay(pdMS_TO_TICKS(1000));

                int data_length_chars = 0;
                ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_0, (size_t*)&data_length_chars));
                printf("chars in Rx buffer: %i\n", data_length_chars);

                // todo safely remove this variable
                int data_length = data_length_chars;

                char temp_data_buffer[data_length_chars];

                // add null terminated symbol so we could correctly read temp_data_buffer
                temp_data_buffer[data_length_chars] = '\0';

                data_length_chars = uart_read_bytes(UART_NUM_0, temp_data_buffer, data_length_chars, 100);

                if(strcmp(temp_data_buffer, "END FILES TRANSMISSION") == 0){
                    printf("%s\n", "END FILES TRANSMISSION RAISED!");

                    // so stop iterate over command files and send info about this in python 
                    response_next_action();
                    break;
                }

                // test print
                // printf("\n%s\n", temp_data_buffer);

                // clone the old buffer because when we use strtok - it changes input string
                char temp_data_buffer_for_getting_name[strlen(temp_data_buffer)];
                strcpy(temp_data_buffer_for_getting_name, temp_data_buffer);

                // start parse data buffer (consists of data_line(s))
                char* first_line_in_file = strtok(temp_data_buffer_for_getting_name, "\n");

                if(strcmp(first_line_in_file, "START_FILE") == 0){
                    char* data_line_with_name = strtok(NULL, "\n");
                    // get satellite name
                    char* satellite_name_string = strtok(data_line_with_name, "=");
                    char* satellite_name = strtok(NULL, satellite_name_string);
                    printf("satellite name: %s\n", satellite_name);
                    // create file path by satellite name
                    char spiffs_satellites_user_input_path[SPIFFS_MAX_FILE_NAME_LENGTH];
                    create_spiffs_txt_file_path_by_params(satellite_name, (char*)name_postfix_command, spiffs_satellites_user_input_path);
                    // first data line is start file line so we need to skip it
                    strtok(temp_data_buffer, "\n");
                    // first data line, to interate over it
                    char* data_line = strtok(NULL, "\n");
                    // Parse all lines and add them to spiffs
                    while(data_line){
                        if(strcmp(data_line, "END_FILE") == 0){
                            // so we need to break loop of current file reading (to not add this line to spiffs) 
                            break;
                        }
                        add_line_to_spiffs(spiffs_satellites_user_input_path, data_line);
                        // test print
                        // printf("%s\n", data_line);
                        data_line = strtok(NULL, "\n");
                    }
                    char data_buffer[data_length];
                    read_data_from_spiffs_file_to_buffer(spiffs_satellites_user_input_path, data_buffer, data_length);
                    printf("content from spiffs file with user input data:\n%s", data_buffer);
                    // clean data about file from uart to free space for other file
                    uart_flush(UART_NUM_0);
                }
                // signal to python that we could read another file
                response_next_action();
            }
            // send signal to python that we could read another command 
            response_next_action();
            #endif
        }


        else if(strcmp(command_buffer, "get spiffs info") == 0)


        {
            printf("%s command is realized\n", command_buffer);

            ESP_ERROR_CHECK(esp_spiffs_check(SPIFFS_PARTITION_LABEL));

            // wait until python starts to read data
            vTaskDelay(pdMS_TO_TICKS(1000));

            size_t spiffs_total, spiffs_used = 0;
            ESP_ERROR_CHECK(esp_spiffs_info(SPIFFS_PARTITION_LABEL, &spiffs_total, &spiffs_used));
            char spiffs_data[128];
            sprintf(spiffs_data, "bytes: spiffs_total=%i spiffs_used=%i\n", spiffs_total, spiffs_used);
            ESP_LOGW(command_handler_tag, "\n%s", spiffs_data);

            // transmit this data to print in in python script 
            // протестировать возможности функций send with break и на стороне python cancel_read
            uart_write_bytes(UART_NUM_0, (void*)spiffs_data, strlen(spiffs_data));

            // wait until python wait response function activated
            // vTaskDelay(pdMS_TO_TICKS(1000));
            wait_until_python_process(1000);

            // send info that we are finished sending first part of statistics about spiffs 
            response_next_action();

            // push information about all files in spiffs

            // initialize empty string
            char spiffs_files_info[2 * SPIFFS_MAX_FILES * (SPIFFS_MAX_FILE_NAME_LENGTH + strlen(": ") + 4 * sizeof(char))];
            spiffs_files_info[0] = '\0';

            DIR *dptr;
            struct dirent *dir;
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

                    // name_of_file.txt: 10
                    char file_stats_line[strlen(dir->d_name) + strlen(": ") + 4 * sizeof(char)];
                    sprintf(file_stats_line, "%s: %i\n", dir->d_name, size_of_file);
                    strcat(spiffs_files_info, file_stats_line);
                }
                closedir(dptr);
            }

            uart_write_bytes(UART_NUM_0, spiffs_files_info, strlen(spiffs_files_info));
        }


        else if(strcmp(command_buffer, "load spiffs data to pc") == 0)


        {
            printf("%s command is realized\n", command_buffer);

            // send signal to python that we got command
            response_next_action();

            // get stats about spiffs
            size_t total = 0, used = 0;
            esp_spiffs_info(SPIFFS_PARTITION_LABEL, &total, &used);
            size_t free_space = total - used;

            char free_space_buffer[strlen("free=") + 4 * sizeof(char) + sizeof("\n")];
            // free_space_buffer[0] = '\0';

            sprintf(free_space_buffer, "free=%i\n", free_space);

            // give some time so uart in python will start reading
            wait_until_python_process(5000);

            // send free space information to python script
            uart_write_bytes(UART_NUM_0, free_space_buffer, strlen(free_space_buffer));
            response_next_action();

            // wait until python gets data and handle it
            wait_until_python_process(15000);

            // response to python that we finished writing info about free space
            response_next_action();

            // iterate over command files
            while(true){
                // give 1 sec to avoid bug when while(true) works too fast and couses problems
                wait_until_python_process(1000);

                char pass_buffer[PASS_DATA_SIZE];
                get_data_from_uart(pass_buffer, PASS_DATA_SIZE);

                // test print
                ESP_LOGI(command_handler_tag, "\n%s", pass_buffer);

                // remember this value for print data from spiffs in the end for the test
                size_t data_length = strlen(pass_buffer);

                if(strcmp(pass_buffer, "END FILES TRANSMISSION") == 0){
                    printf("%s\n", "END FILES TRANSMISSION RAISED!");
                    // so stop iterate over command files and send info about this in python 
                    response_next_action();
                    break;
                }

                // clone the old buffer because when we use strtok - it changes input string
                char temp_data_buffer_for_getting_name[strlen(pass_buffer)];
                strcpy(temp_data_buffer_for_getting_name, pass_buffer);

                const char line_delimiter[] = "\n";
                const char req_data_delimiter[] = ": "; 
                const char param_data_delimiter[] = "=";

                char* line_saveptr = NULL;
                char* element_saveptr = NULL;

                // start parse data buffer (consists of data_line(s))
                char* first_line_in_file = strtok_r(temp_data_buffer_for_getting_name, line_delimiter, &line_saveptr);
                if(strcmp(first_line_in_file, "START_FILE") == 0){
                    char* data_line_with_id = strtok_r(NULL, line_delimiter, &line_saveptr);

                    // // test
                    char old_line_with_id[strlen(data_line_with_id)];
                    strcpy(old_line_with_id, data_line_with_id);

                    // test
                    char current_delimiter[strlen(req_data_delimiter) + 1];
                    strcpy(current_delimiter, req_data_delimiter);

                    // test print
                    // printf("%s\n", current_delimiter);

                    // test
                    char name_postfix[strlen(name_postfix_command) + 1];
                    char* test_ptr = strtok_r(data_line_with_id, current_delimiter, &element_saveptr);

                    // test print
                    ESP_LOGW(command_handler_tag, "test ptr: %s\n", test_ptr);

                    // if it is no pass
                    if(strcmp(test_ptr, old_line_with_id) == 0){
                        // change delimiter to param
                        strcpy(current_delimiter, param_data_delimiter);
                        // skip element left from =
                        char* left_element = strtok_r(test_ptr, current_delimiter, &element_saveptr);
                        // ERROR !
                        // test print
                        printf("left element: %s\n", left_element);
                        // change file postfix to param postfix
                        strcpy(name_postfix, name_postfix_command);

                        printf("current postfix: %s\n", name_postfix);
                    } else {
                        // change file postfix to pass postfix
                        strcpy(name_postfix, name_postfix_pass);

                        printf("current postfix: %s\n", name_postfix);
                    }
                    printf("current delimiter: %s\n", current_delimiter);
                    char* satellite_id = strtok_r(NULL, current_delimiter, &element_saveptr); 

                    // skip "sat_id" part (left from delimiter)
                    // char* element_ptr = strtok_r(data_line_with_id, req_data_delimiter, &element_saveptr);

                    // char* satellite_id = strtok_r(NULL, req_data_delimiter, &element_saveptr);
                    char spiffs_passes_file_path[SPIFFS_MAX_FILE_NAME_LENGTH];
                    // create_spiffs_txt_file_path_by_params(satellite_id, name_postfix_pass, spiffs_passes_file_path);
                    // test
                    create_spiffs_txt_file_path_by_params(satellite_id, name_postfix, spiffs_passes_file_path);
                    // first data line is start file line so we need to skip it
                    char* first_line_in_file = strtok(pass_buffer, "\n");
                    // first data line, to interate over it
                    char* data_line = strtok(NULL, "\n");
                    // Parse all lines and add them to spiffs
                    while(data_line){
                        if(strcmp(data_line, "END_FILE") == 0){
                            // so we need to break loop of current file reading (to not add this line to spiffs) 
                            break;
                        }
                        add_line_to_spiffs(spiffs_passes_file_path, data_line);
                        // test print
                        // printf("%s\n", data_line);
                        data_line = strtok(NULL, "\n");
                    }
                    // test print content of just created spiffs file
                    char data_buffer[data_length];
                    read_data_from_spiffs_file_to_buffer(spiffs_passes_file_path, data_buffer, data_length);
                    printf("content from spiffs file with user input data:\n%s", data_buffer);

                    // test
                    // clear_data_from_spiffs_file(spiffs_passes_file_path);

                    // clean data about file from uart to free space for other file
                    uart_flush(UART_NUM_0);
                }

                // signal to python that we could read another file
                response_next_action();
            }

            // we don't need here response_next_action(); because last response will be in while cycle 
        }
        else
        {
            ESP_LOGW(command_handler_tag, "\nNo command was passed, new cycle");
        }

        // wait 1 sec to avoid while loop looping too fast
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}