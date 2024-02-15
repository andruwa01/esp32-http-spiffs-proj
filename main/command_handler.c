#include "command_handler.h"

const static char* command_handler_tag = "command_handler";
const static char next_action_value[16] = "NEXT_ACTION\n";

static void response_next_action(void){
    int data_length_chars = 0;
    data_length_chars = uart_write_bytes(UART_NUM_0, &next_action_value, strlen(next_action_value));
    ESP_LOGW(command_handler_tag, "%i bytes were sended", data_length_chars);

    uart_flush(UART_NUM_0);

    size_t tx_buffer_size = 0;
    if(uart_get_buffered_data_len(UART_NUM_0, (size_t*)&tx_buffer_size) > 0){
        ESP_LOGE(command_handler_tag, "ERROR! buffer not cleared");
        return;
    }
}

void get_command_from_uart(){
    if(!uart_is_driver_installed(UART_NUM_0)){
        ESP_LOGE(command_handler_tag, "ERROR! uart driver is not installed");
        return;
    }

    while(true){
        ESP_LOGW(command_handler_tag, "Waiting new command. . .");

        char command_buffer[NUMBER_OF_CHARS_IN_BUFFER_FOR_UART_DATA] = "";
        printf("current command: %s\n", command_buffer);

        int data_length_chars = 0;
        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_0, (size_t*)&data_length_chars));
        printf("chars in Rx buffer: %i\n", data_length_chars);

        if(data_length_chars > NUMBER_OF_CHARS_IN_BUFFER_FOR_UART_DATA){
            ESP_LOGE(command_handler_tag, "ERROR! data from uart doesn't fit into uart buffer");
        };

        data_length_chars = uart_read_bytes(UART_NUM_0, command_buffer, (size_t)data_length_chars, 10);
        if(data_length_chars){
            ESP_LOGW(command_handler_tag, "%i chars was readed, contents: %s", data_length_chars, (char*)command_buffer);
        }

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

            // task delay for function and python. needs to be greater that timeout value
            // of serial port opened in python script
            vTaskDelay(pdMS_TO_TICKS(TIME_DELAY_BEFORE_RESPONSE_SENDED_MS)); 

            response_next_action();
        }
        else if(strcmp(command_buffer, "clear spiffs") == 0)
        {
            #if defined(SPIFFS_CLEAR_FILES_BY_COMMAND)
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

            size_t total, used = 0;
            ESP_ERROR_CHECK(esp_spiffs_info(SPIFFS_PARTITION_LABEL, &total, &used));

            ESP_LOGW(command_handler_tag, "total (bytes): %i used (bytes): %i", total, used);
            #else
            ESP_LOGW(spiffs_tag, "You don't use spiffs!");
            #endif

            response_next_action();
        }
        else if(strcmp(command_buffer, "push command files") == 0)
        {
            printf("%s command is realized\n", command_buffer);

            vTaskDelay(pdMS_TO_TICKS(1000));

            #if defined(LOAD_ALL_COMMAND_FILES)


            // iterate over commands
            while(true){


                int data_length_chars = 0;
                ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_0, (size_t*)&data_length_chars));
                printf("chars in Rx buffer: %i\n", data_length_chars);


                int data_length = data_length_chars;

                char temp_data_buffer[data_length_chars];

                data_length_chars = uart_read_bytes(UART_NUM_0, temp_data_buffer, data_length_chars, 10);

                // add null terminated symbol so we could correctly read temp_data_buffer
                temp_data_buffer[data_length_chars] = '\0';

                if(strcmp(temp_data_buffer, "END FILES TRANSMISSION") == 0){
                    printf("%s\n", "END FILES TRANSMISSION RAISED!");
                    break;
                }

                // test print
                // printf("\n%s\n", temp_data_buffer);

                // clone the old buffer because when we use strtok - it changes input string
                char temp_data_buffer_for_getting_name[strlen(temp_data_buffer)];
                strcpy(temp_data_buffer_for_getting_name, temp_data_buffer);

                // start parse data buffer (consists of data_line(s))
                char* data_line_with_name = strtok(temp_data_buffer_for_getting_name, "\n");
                if(data_line_with_name == NULL){
                    ESP_LOGE(command_handler_tag, "line from imported file in empty");
                    return;
                } 

                // get satellite name
                char* satellite_name_line = strtok(data_line_with_name, "=");
                char* satellite_name = strtok(NULL, satellite_name_line);
                printf("satellite name: %s\n", satellite_name);

                // create file by satellite name
                char user_input_folder[] = "passes_user_input";
                char file_extension[] = "txt";
                char spiffs_satellites_user_input_path[
                    strlen(SPIFFS_BASE_PATH) + 
                    strlen("/") + 
                    strlen(satellite_name) + 
                    strlen("_commands.") + 
                    strlen(file_extension)]; 

                sprintf(spiffs_satellites_user_input_path, "%s/%s_commands.%s", SPIFFS_BASE_PATH, satellite_name, file_extension);

                // first data line, to interate over it
                char* data_line = strtok(temp_data_buffer, "\n");
                // Parse all lines and add them to spiffs
                while(data_line){
                    add_line_to_spiffs(spiffs_satellites_user_input_path, data_line);
                    // test print
                    // printf("%s\n", data_line);
                    data_line = strtok(NULL, "\n");
                }

                char data_buffer[data_length];
                read_data_from_spiffs_file_to_buffer(spiffs_satellites_user_input_path, data_buffer, data_length);
                printf("content from spiffs file with user input data:\n%s", data_buffer);

                // TODO
                clear_data_from_spiffs_file(spiffs_satellites_user_input_path);



                vTaskDelay(pdMS_TO_TICKS(1000));
            }

            // send signal to python that we could read another command 
            response_next_action();
            #endif

            #if defined(LOAD_ONE_FILE_UART)
            int data_length_chars = 0;
            ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_0, (size_t*)&data_length_chars));
            printf("chars in Rx buffer: %i\n", data_length_chars);

            int data_length = data_length_chars;

            char temp_data_buffer[data_length_chars];

            data_length_chars = uart_read_bytes(UART_NUM_0, temp_data_buffer, data_length_chars, 10);

            // add null terminated symbol so we could correctly read temp_data_buffer
            temp_data_buffer[data_length_chars] = '\0';

            // test print
            // printf("\n%s\n", temp_data_buffer);

            // clone the old buffer because when we use strtok - it changes input string
            char temp_data_buffer_for_getting_name[strlen(temp_data_buffer)];
            strcpy(temp_data_buffer_for_getting_name, temp_data_buffer);

            // start parse data buffer (consists of data_line(s))
            char* data_line_with_name = strtok(temp_data_buffer_for_getting_name, "\n");
            if(data_line_with_name == NULL){
                ESP_LOGE(command_handler_tag, "line from imported file in empty");
                return;
            } 

            // get satellite name
            char* satellite_name_line = strtok(data_line_with_name, "=");
            char* satellite_name = strtok(NULL, satellite_name_line);
            printf("satellite name: %s\n", satellite_name);

            // create file by satellite name
            char user_input_folder[] = "passes_user_input";
            char file_extension[] = "txt";
            char spiffs_satellites_user_input_path[
                strlen(SPIFFS_BASE_PATH) + 
                strlen("/") + 
                strlen(satellite_name) + 
                strlen("_commands.") + 
                strlen(file_extension)]; 

            sprintf(spiffs_satellites_user_input_path, "%s/%s_commands.%s", SPIFFS_BASE_PATH, satellite_name, file_extension);

            // first data line, to interate over it
            char* data_line = strtok(temp_data_buffer, "\n");
            // Parse all lines and add them to spiffs
            while(data_line){
                add_line_to_spiffs(spiffs_satellites_user_input_path, data_line);
                // test print
                // printf("%s\n", data_line);
                data_line = strtok(NULL, "\n");
            }

            char data_buffer[data_length];
            read_data_from_spiffs_file_to_buffer(spiffs_satellites_user_input_path, data_buffer, data_length);
            printf("content from spiffs file with user input data:\n%s", data_buffer);

            // TODO
            clear_data_from_spiffs_file(spiffs_satellites_user_input_path);

            response_next_action();

            #endif

            
        }
        else
        {
            ESP_LOGW(command_handler_tag, "\nNo command was passed, new cycle");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}