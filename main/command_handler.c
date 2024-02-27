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

static void wait_until_next_action_in_python_ms(int ms_to_wait){
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
    wait_until_next_action_in_python_ms(1000);

    uart_flush(UART_NUM_0);

    int data_length_chars = 0;
    data_length_chars = uart_write_bytes(UART_NUM_0, &next_action_value, strlen(next_action_value));
    ESP_LOGW(command_handler_tag, "%i bytes were sended", data_length_chars);
    
    uart_flush(UART_NUM_0);
}

void get_command_from_uart(){
    if(!uart_is_driver_installed(UART_NUM_0)){
        ESP_LOGE(command_handler_tag, "ERROR! uart driver is not installed");
        return;
    }

    while(true){
        ESP_LOGW(command_handler_tag, "Waiting new command. . .");

        char command_buffer[command_size];
        get_data_from_uart(command_buffer, command_size);

        if(strcmp(command_buffer, "get all") == 0)
        {
            #if defined(GET_REQUEST_ALL_SAT)
            initialize_get_requests_for_all_satellites();

            printf("current command (after clearence): %s\n", command_buffer);
            #endif

            response_next_action();
        }
        else if(strcmp(command_buffer, "update buffer") == 0)
        {
            // we got command
            response_next_action();

            // give some time to python script for writing options file to uart (esle we got runtime exception)
            wait_until_next_action_in_python_ms(1000);

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
                wait_until_next_action_in_python_ms(1000);

                int size_of_buffer = 256;
                char temp_data_buffer[size_of_buffer];
                get_data_from_uart(temp_data_buffer, size_of_buffer);
                // got data 
                response_next_action();

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
                // printf("%s\n", pass_buffer);

                // send sat data to uart to python script get it
                int sended_bytes = uart_write_bytes(UART_NUM_0, (const char*)&pass_buffer, strlen(pass_buffer));
                ESP_LOGW(command_handler_tag, "%i bytes were sended", sended_bytes);

                // test print
                // printf("sat_name: %s\n", sat_name);
                data_line = strtok_r(NULL, "\n", &data_line_saveptr);
            }
            // finish interating over file
            response_next_action();

            clear_data_from_spiffs_file(spiffs_satellites_user_input_path);

            wait_until_next_action_in_python_ms(TIME_DELAY_BEFORE_RESPONSE_SENDED_MS);
            response_next_action();
        }
        else if(strcmp(command_buffer, "clear spiffs") == 0)
        {
            #if defined(SPIFFS_CLEAN_FULL_PASSES_FILES)
            for(int satellite_index = 0; satellite_index < SPIFFS_NUMBER_OF_FILES; satellite_index++){
                char spiffs_file_path[strlen(SPIFFS_BASE_PATH) + strlen("/") + strlen(satellites[satellite_index].name)];
                sprintf(spiffs_file_path, "%s/%s", SPIFFS_BASE_PATH, satellites[satellite_index].name);
                if(fclose(fopen(spiffs_file_path, "w")) != 0){
                    ESP_LOGE(command_handler_tag, "Error: can't open and close (clear) file %s", spiffs_file_path);
                    return;
                } else {
                    ESP_LOGW(command_handler_tag, "File %s was cleared", spiffs_file_path);
                };
            }

            #else
            ESP_LOGW(command_handler_tag, "You don't use spiffs!");
            #endif

            #if defined(SPIFFS_CLEAR_COMMAND_FILES)
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

            // next line in file (filename)
            char* data_line = strtok(NULL, "\n");

            // clear_satellite_files_by_name();
            while(data_line){
                clear_spiffs_file_by_params(data_line, name_postfix_pass);
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

            // wait until python starts to read data
            vTaskDelay(pdMS_TO_TICKS(1000));

            size_t total, used = 0;
            ESP_ERROR_CHECK(esp_spiffs_info(SPIFFS_PARTITION_LABEL, &total, &used));
            char spiffs_data[128];
            sprintf(spiffs_data, "total (bytes): %i used (bytes): %i\n", total, used);
            ESP_LOGW(command_handler_tag, "%s", spiffs_data);

            // transmit this data to print in in python script 
            // протестировать возможности функций send with break и на стороне python cancel_read
            size_t sended_bytes = uart_write_bytes(UART_NUM_0, (void*)spiffs_data, strlen(spiffs_data));

            // wait until python wait response function activated
            vTaskDelay(pdMS_TO_TICKS(1000));

            response_next_action();
        }


        else if(strcmp(command_buffer, "load passes to spiffs") == 0)


        {
            printf("%s command is realized\n", command_buffer);

            // send signal to python that we got command
            response_next_action();

            // iterate over command files
            while(true){
                // give 1 sec to avoid bug when while(true) works too fast and couses problems
                vTaskDelay(pdMS_TO_TICKS(1000));

                char pass_buffer[PASS_DATA_SIZE];
                get_data_from_uart(pass_buffer, PASS_DATA_SIZE);

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
                const char element_delimiter[] = ": "; 

                char* line_saveptr = NULL;
                char* element_saveptr = NULL;

                // start parse data buffer (consists of data_line(s))
                char* first_line_in_file = strtok_r(temp_data_buffer_for_getting_name, line_delimiter, &line_saveptr);

                if(strcmp(first_line_in_file, "START_FILE") == 0){
                    char* data_line_with_id = strtok_r(NULL, line_delimiter, &line_saveptr);
                    // get satellite name (WARNING! separator is ": ", which is not equal to separator in case of command file)
                    // skip "sat_id" part (left from delimiter)
                    strtok_r(data_line_with_id, element_delimiter, &element_saveptr);
                    char* satellite_id = strtok_r(NULL, element_delimiter, &element_saveptr);
                    char spiffs_passes_file_path[SPIFFS_MAX_FILE_NAME_LENGTH];
                    create_spiffs_txt_file_path_by_params(satellite_id, name_postfix_pass, spiffs_passes_file_path);
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