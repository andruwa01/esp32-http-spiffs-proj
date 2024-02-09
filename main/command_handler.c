#include "command_handler.h"

const static char* command_handler_tag = "command_handler";
const static char next_action_value[16] = "NEXT_ACTION\n";

static void response_next_action(){
    int data_length_bytes = 0;
    data_length_bytes = uart_write_bytes(UART_NUM_0, &next_action_value, strlen(next_action_value));
    ESP_LOGW(command_handler_tag, "%i bytes were sended", data_length_bytes);

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

    bool listen_uart = true;
    while(listen_uart){
        ESP_LOGW(command_handler_tag, "Waiting new command. . .");

        char command_buffer[NUMBER_OF_CHARS_IN_BUFFER_FOR_UART_DATA] = "";
        printf("current command: %s\n", command_buffer);

        int data_length_bytes = 0;
        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_0, (size_t*)&data_length_bytes));
        printf("bytes in Rx buffer: %i\n", data_length_bytes);

        if(data_length_bytes > sizeof(command_buffer)){
            ESP_LOGE(command_handler_tag, "ERROR! data from uart doesn't fit into uart buffer");
        };

        data_length_bytes = uart_read_bytes(UART_NUM_0, command_buffer, data_length_bytes, 10);
        if(data_length_bytes){
            command_buffer[data_length_bytes] = '\0';
            ESP_LOGW(command_handler_tag, "%i bytes was readed, contents: %s", data_length_bytes, (char*)command_buffer);
        }

        if(strcmp(command_buffer, "get all") == 0)
        {
            #if defined(GET_REQUEST_ALL_SAT)
            initialize_get_requests_for_all_satellites();

            command_buffer[0] = '\0';
            printf("current command (after clearence): %s\n", command_buffer);

            #endif

            command_buffer[0] = '\0';
            response_next_action();

        }
        else if(strcmp(command_buffer, "update buffer") == 0)
        {
            uart_flush_input(UART_NUM_0);

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


            command_buffer[0] = '\0';
            response_next_action();

        }
        else
        {
            ESP_LOGW(command_handler_tag, "No command was passed, new cycle");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

