#include "udp.h"

// Убрать это в spiffs_storage.c или spiffs_storage.h 
// =======================================================
const static char *text_file_extension = ".txt";
const static char *name_options_file = "input_options"; 
const static char *name_postfix_command = "_command";
const static char *name_postfix_response = "_response";
// =======================================================

const static char *tag_udp      = "udp";
const static char *tag_udp_test = "udp test"; // Логи с этой меткой можно впоследствии убрать (они чисто вспомогательные)

const static char *event_board_get_command = "get command from pc";
const static char *event_udp_finish_action = "finish working with action by command";

const static char *response_send_udp    = "response0"; 
const static char *response_receive_udp = "response1";

#define BIT_NEXT_COMMAND BIT0
// #define BIT_STOP_WAITING_FILES BIT1

const static char *command_template               = "commandx";
const static char *command_send_spiffs_data_to_pc = "command0";
const static char *command_send_pc_data_to_spiffs = "command1";
// const static char *command_stop_waiting_files     = "command2";

const static char *new_line_delimiter   = "\n";
const static char *req_data_delimiter   = ": "; 
const static char *param_data_delimiter = "=";

// UDP sockets structs
int sockfd;
struct sockaddr_in board_wifi_addr_receive, pc_wifi_addr_receive, pc_wifi_addr_send;
socklen_t pc_wifi_addr_len = sizeof(pc_wifi_addr_receive);

// Переписать эту функцию в spiffs_storage.c
static void create_spiffs_txt_file_path_by_params(const char *file_name, const char *name_postfix, char *buffer_to_save_path){
        if (file_name == NULL || buffer_to_save_path == NULL){
            ESP_LOGE(tag_udp, "ERROR! NULL pointer was detected! Check function arguments");
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
        strcpy(buffer_to_save_path, spiffs_satellites_user_input_path);

        // TODO return ESP_OK если файл с таким путём существует, иначе return ESP_FAIL. Тогда если это получится можно убрать
        // esp_err_t return type у функции которая считывает файл из spiffs по path
}

static void initialize_sockets(void){
    ESP_LOGI(tag_udp, "Create socket . . .\n");
    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0){
        ESP_LOGE(tag_udp, "Socket not created\n");
        vTaskDelete(NULL);
    }

    memset(&board_wifi_addr_receive, 0, sizeof(board_wifi_addr_receive));
    memset(&pc_wifi_addr_receive,    0, sizeof(pc_wifi_addr_receive));
    memset(&pc_wifi_addr_send,       0, sizeof(pc_wifi_addr_send));

    // board info
    board_wifi_addr_receive.sin_family = AF_INET; // ipv4
    board_wifi_addr_receive.sin_addr.s_addr = INADDR_ANY;
    board_wifi_addr_receive.sin_port = htons(CONFIG_BOARD_PORT);

    // pc info
    pc_wifi_addr_send.sin_family = AF_INET;// ipv4
    pc_wifi_addr_send.sin_addr.s_addr = inet_addr(CONFIG_PC_IP_WIFI);
    pc_wifi_addr_send.sin_port = htons(CONFIG_PC_PORT);

    // link socket with board
    if(bind(sockfd, (const struct sockaddr*) &board_wifi_addr_receive, sizeof(struct sockaddr_in)) < 0){
        ESP_LOGE(tag_udp, "socket not binded\n");
        vTaskDelete(NULL);
    }
    ESP_LOGI(tag_udp, "socket was binded\n");
}

static void send_response_to_pc(const char *sending_event_info){
    ESP_LOGW(tag_udp_test, "send event: %s", sending_event_info);
    size_t sent_bytes = 0;
    // sent_bytes = uart_write_bytes(UART_NUM_0, next_action_value, strlen(next_action_value));
    sent_bytes = sendto(sockfd, response_send_udp, strlen(response_send_udp), 0, (struct sockaddr*) &pc_wifi_addr_send, sizeof(pc_wifi_addr_send));
    if(sent_bytes != strlen(response_send_udp)){
        ESP_LOGE(tag_udp, "send bytes not equal to size of response, something wrong");
        return;
    }

    ESP_LOGW(tag_udp_test, "message \"%s\" sent to port %i, size %i bytes", response_send_udp, CONFIG_PC_PORT, sent_bytes);
}

static void wait_response_from_pc(const char *waiting_event_info){
    // ESP_LOGW(tag_udp, "receive event: %s", waiting_event_info);
    // char data_buffer[strlen(response_receive_udp)];
    // size_t data_length = recvfrom(sockfd, data_buffer, sizeof(data_buffer), 0, (struct sockaddr*) &pc_wifi_addr_receive, &pc_wifi_addr_len);
    // data_buffer[data_length] = '\0';
    // ESP_LOGW(tag_udp, "get data: %s\n", data_buffer);

    char data_buffer[strlen(response_receive_udp) + 1];
    while(strcmp(data_buffer, response_receive_udp) != 0){
        ESP_LOGW(tag_udp, "udp receive event: %s", waiting_event_info);
        int data_length = recvfrom(sockfd, data_buffer, sizeof(data_buffer), 0, (struct sockaddr*) &pc_wifi_addr_receive, &pc_wifi_addr_len);
        data_buffer[data_length] = '\0';
        if(data_length == -1){
            ESP_LOGE(tag_udp, "ERROR! this is not response from pc");
        }
        ESP_LOGW(tag_udp, "get data: %s", data_buffer);

        // Delete if comment long time
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void get_sub_str_by_index(int start_index, int end_index, char src_string[], char substr_buf[]){
    char *start_ptr  = &src_string[start_index];
    char *end_ptr    = &src_string[end_index];
    char *substr_ptr = (char *)calloc(1, end_ptr - start_ptr + 1);
    memcpy(substr_ptr, start_ptr, end_ptr - start_ptr);
    strcpy(substr_buf, substr_ptr);
    free(substr_ptr);
}

static void send_file_over_udp(const char *spiffs_file_path){
    char file_data_string[SIZE_RESPONSE_DATA_MAX];
    if(read_data_from_spiffs_file_to_buffer(spiffs_file_path, file_data_string, sizeof(file_data_string)) == ESP_OK){

        send_response_to_pc("ready to send file");

        size_t sent_bytes_start_file = sendto(sockfd, "START_FILE", strlen("START_FILE"), 0, (struct sockaddr *)&pc_wifi_addr_send, sizeof(pc_wifi_addr_send));
        ESP_LOGW(tag_udp_test, "START_FILE sent, size %i bytes", sent_bytes_start_file);

        wait_response_from_pc("pc start getting file chunks");

        // remember length of data in file
        size_t file_data_length = strlen(file_data_string);
        size_t sent_package_size = 0;
        size_t start_chunk_index = 0;
        while(file_data_length > 0){
            u_int end_chunk_index = start_chunk_index + 512;
            if(end_chunk_index > strlen(file_data_string)){
                end_chunk_index = strlen(file_data_string);
            }
            char data_chunk_string[SIZE_DATA_CHUNK_UDP_MAX]; 
            get_sub_str_by_index(
                start_chunk_index,
                end_chunk_index, 
                file_data_string,
                data_chunk_string
            ); 
            start_chunk_index = end_chunk_index;
            size_t sent_bytes_by_chunk = sendto(sockfd, data_chunk_string, strlen(data_chunk_string), 0, (struct sockaddr *) &pc_wifi_addr_send, sizeof(pc_wifi_addr_send));
            file_data_length -= sent_bytes_by_chunk;
            sent_package_size += sent_bytes_by_chunk;

            wait_response_from_pc("wait info about reading new chunk");
        }

        // Delete if comment
        // vTaskDelay(pdMS_TO_TICKS(5000));

        size_t sent_bytes_end_file = sendto(sockfd, "END_FILE", strlen("END_FILE"), 0, (struct sockaddr *)&pc_wifi_addr_send, sizeof(pc_wifi_addr_send));
        ESP_LOGW(tag_udp_test, "END_FILE sent, size %i bytes", sent_bytes_end_file);

        ESP_LOGI(tag_udp, "message sent, size: %i bytes", sent_package_size);
    } else {
        ESP_LOGW(tag_udp, "file %s was not sent, probably it is empty", spiffs_file_path);
    }
}

static int receive_file_over_udp(char *empty_data_buffer, size_t buffer_size){
    if(empty_data_buffer[0] != '\0'){
        ESP_LOGE(tag_udp, "ERROR! This data buffer will be updated, so you need to make it full empty with first char null-terminated");
        return -1;
    }

    wait_response_from_pc("wait signal from pc that it ready to send file");

    char start_file_buffer[strlen("START_FILE") + 1]; 
    while(strcmp(start_file_buffer, "START_FILE") != 0){
        int start_file_bytes = recvfrom(sockfd, start_file_buffer, sizeof(start_file_buffer) - 1, 0, (struct sockaddr*) &pc_wifi_addr_receive, &pc_wifi_addr_len);
        start_file_buffer[start_file_bytes] = '\0';
        if(start_file_bytes == -1){
            ESP_LOGE(tag_udp, "this is not first line of udp file");
        }
    }

    ESP_LOGW(tag_udp_test, "get START_FILE");
    send_response_to_pc("board get file");
    
    char data_chunk[1024];
    size_t used_bytes = 0;
    while(true){
        ESP_LOGW(tag_udp, "ready to get new file chunk");
        size_t received_bytes = recvfrom(sockfd, data_chunk, sizeof(data_chunk) - 1, 0, (struct sockaddr*) &pc_wifi_addr_receive, &pc_wifi_addr_len);   
        data_chunk[received_bytes] = '\0';
        if(strcmp(data_chunk, "END_FILE") == 0){
            ESP_LOGW(tag_udp, "data chunk finished");
            break;
        }
        // printf("received chunk:\n====\n%s\n====\n", data_chunk);
        strcat(empty_data_buffer, data_chunk);

        used_bytes += received_bytes;
        if(buffer_size < used_bytes){
            ESP_LOGE(tag_udp, "ERROR! %i bytes package does not fit into buffer of %i bytes", used_bytes, buffer_size);
            return -1;
        }
        send_response_to_pc("ready to get new chunk");
    }

    ESP_LOGI(tag_udp_test, "finish file handle");
    return used_bytes;
}

// void task_udp_wait_command(void *xCommandGroup){
void task_udp_wait_command(){

    // Отправляем сигнал task_udp_wait_command, чтобы она начала считывать первую команду
    // xEventGroupSetBits(
    //     (EventGroupHandle_t)xCommandGroup,
    //     BIT_NEXT_COMMAND
    // );
    
    for(;;){
        ESP_LOGW(tag_udp_test, "ожидаем события на считывания следующей команды");
        // Ждём сигнал о том, что можно прееходить к ожиданию следующей команды (реализовать через группу событий).
        // xEventGroupWaitBits(
        //     (EventGroupHandle_t)xCommandGroup,
        //     BIT_NEXT_COMMAND,
        //     pdTRUE,
        //     pdFALSE,
        //     portMAX_DELAY
        // );
        // Начато ожидание новой команды
        ESP_LOGI(tag_udp, "waiting command from port %i . . .", CONFIG_BOARD_PORT);
        // Ждём команды в стиле commandx , где x - номер комманды
        char command_buffer[strlen(command_template)];
        size_t command_length = recvfrom(sockfd, &command_buffer, 16, 0, (struct sockaddr*) &pc_wifi_addr_receive, &pc_wifi_addr_len); 
        command_buffer[command_length] = '\0';
        ESP_LOGW(tag_udp ,"current command: %s", command_buffer);

        if(strcmp(command_buffer, command_template) == 0)


        {
            // Ответить питону, что получили команду
            send_response_to_pc(event_board_get_command);
            wait_response_from_pc("test wating 1");
            // Действия над командой
            ESP_LOGW(tag_udp, "action to test command!");
            vTaskDelay(pdMS_TO_TICKS(5000));
            wait_response_from_pc("test waiting 2");
            // Ответить питону, что сделали всё, что надо было
            send_response_to_pc(event_udp_finish_action);
            ESP_LOGW(tag_udp_test, "сообщаем обработчику команд, что можно считывать следующую команду");
            // Сообщаем обработчику команд, что можно считывать следующую команду
            // xEventGroupSetBits(
            //     (EventGroupHandle_t)xCommandGroup,
            //     BIT_NEXT_COMMAND
            // );
        }


        else if(strcmp(command_buffer, command_send_spiffs_data_to_pc) == 0)


        {
            send_response_to_pc(event_board_get_command);

            char request_options[SIZE_OPTIONS_FILE_MAX];
            memset(request_options, '\0', sizeof(request_options));
            ESP_LOGW(tag_udp, "udp receive event: %s", "waiting request options file");
            int data_length = receive_file_over_udp(request_options, SIZE_OPTIONS_FILE_MAX);
            if(data_length == -1){
                ESP_LOGE(tag_udp, "mistake during file handling");
                break;
            }
            // size_t data_length = recvfrom(sockfd, request_options, size_max_request_options_file, 0, (struct sockaddr*) &pc_wifi_addr_receive, &pc_wifi_addr_len);
            // request_options[data_length] = '\0';
            ESP_LOGW(tag_udp, "get data package, content:\n====\n%s\n====", request_options);

            char spiffs_request_options_path[SPIFFS_FILE_NAME_LENGTH_MAX];
            create_spiffs_txt_file_path_by_params(name_options_file, (char*)name_postfix_command, spiffs_request_options_path);

            // clear spiffs file before writing to it (to override old file)
            clear_data_from_spiffs_file(spiffs_request_options_path);
            add_file_to_spiffs(spiffs_request_options_path, request_options);
            char request_options_spiffs_data[data_length];
            read_data_from_spiffs_file_to_buffer(spiffs_request_options_path, request_options_spiffs_data, sizeof(request_options_spiffs_data));
            ESP_LOGW(tag_udp_test, "options from spiffs:\n====\n%s\n====\n", request_options_spiffs_data);

            send_response_to_pc("board finished managing options file");

            // interate over file
            char *data_line_saveptr = NULL;
            char *sat_saveptr = NULL;
            char *data_line = strtok_r(request_options_spiffs_data, new_line_delimiter, &data_line_saveptr);
            while(data_line){
                // skip sat_name and get to sat_id
                strtok_r(data_line, param_data_delimiter, &sat_saveptr);
                char *sat_id = strtok_r(NULL, param_data_delimiter, &sat_saveptr);
                // create file spiffs path for sat_name
                char spiffs_response_file_path[SPIFFS_FILE_NAME_LENGTH_MAX];
                create_spiffs_txt_file_path_by_params(sat_id, name_postfix_response, spiffs_response_file_path);
                FILE *response_file_ptr = fopen(spiffs_response_file_path, "r");
                if(response_file_ptr != NULL){
                    fclose(response_file_ptr);
                    // send singal CONTINUE to reading files loop
                    sendto(sockfd, "CONTINUE", strlen("CONTINUE"), 0, (struct sockaddr *) &pc_wifi_addr_send, sizeof(pc_wifi_addr_send));
                    ESP_LOGI(tag_udp_test, "CONTINUE sent");
                    wait_response_from_pc("pc get continue signal");
                    send_file_over_udp(spiffs_response_file_path); 
                    wait_response_from_pc("board can send next file");
                } else {
                    ESP_LOGW(tag_udp, "file with path %s can't be opened for reading before sending (progably is does not exist)", spiffs_response_file_path);
                    fclose(response_file_ptr);
                    // TODO подумать об случае, когда последний файл из настроек запрашивается, но его нет в spiffs - тогда если не отослать тут
                    // сигнал, то получится, что wait_response_from_board в питоне будет бесконечно ждать + проверить это руками
                    // sendto(sockfd, "CONTINUE", strlen("CONTINUE"), 0, (struct sockaddr *) &pc_wifi_addr_send, sizeof(pc_wifi_addr_send));
                    // ESP_LOGW(tag_udp_test, "CONTINUE sent (but so such file with path %s)", spiffs_response_file_path);
                } 
                char spiffs_command_file_path[SPIFFS_FILE_NAME_LENGTH_MAX];
                create_spiffs_txt_file_path_by_params(sat_id, name_postfix_command, spiffs_command_file_path);
                FILE *command_file_ptr = fopen(spiffs_command_file_path, "r");
                if(command_file_ptr != NULL){
                    fclose(command_file_ptr);
                    // send signal CONTINUE to reading files loop
                    sendto(sockfd, "CONTINUE", strlen("CONTINUE"), 0, (struct sockaddr *) &pc_wifi_addr_send, sizeof(pc_wifi_addr_send));
                    ESP_LOGI(tag_udp_test, "CONTINUE sent");
                    wait_response_from_pc("pc get continue signal");
                    send_file_over_udp(spiffs_command_file_path);
                    wait_response_from_pc("pc can read next file");
                } else {
                    ESP_LOGW(tag_udp, "file with path %s can't be opened for reading before sending (progably is does not exists)", spiffs_command_file_path);
                    fclose(command_file_ptr);
                    // sendto(sockfd, "CONTINUE", strlen("CONTINUE"), 0, (struct sockaddr *) &pc_wifi_addr_send, sizeof(pc_wifi_addr_send));
                    // ESP_LOGW(tag_udp_test, "CONTINUE sent (but so such file with path %s)", spiffs_response_file_path);
                }

                data_line = strtok_r(NULL, new_line_delimiter, &data_line_saveptr);
            }

            // send singnal BREAK to reading files loop
            sendto(sockfd, "BREAK", strlen("BREAK"), 0, (struct sockaddr*) &pc_wifi_addr_send, sizeof(pc_wifi_addr_send));

            wait_response_from_pc("pc finished working with files");

            // xEventGroupSetBits(
            //     (EventGroupHandle_t)xCommandGroup,
            //     BIT_NEXT_COMMAND
            // );

            send_response_to_pc(event_udp_finish_action);
        }

        else if(strcmp(command_buffer, command_send_pc_data_to_spiffs) == 0)
        
        {
            send_response_to_pc(event_board_get_command);
            wait_response_from_pc("python starts reading files");

            // get stats about spiffs
            size_t total = 0, used = 0;
            esp_spiffs_info(SPIFFS_PARTITION_LABEL, &total, &used);
            size_t free_space = total - used;
            char free_space_buffer[strlen("free=") + 4 * sizeof(char) + sizeof("\n")];
            sprintf(free_space_buffer, "free=%i\n", free_space);
            // send free space information to python script
            size_t sent_bytes = sendto(sockfd, free_space_buffer, strlen(free_space_buffer), 0, (struct sockaddr*) &pc_wifi_addr_send, sizeof(pc_wifi_addr_send));
            ESP_LOGW(tag_udp_test, "sent info about free space, sent %i bytes", sent_bytes);
            wait_response_from_pc("finish reading data");
            // response to python that we ready to get files with data
            send_response_to_pc("ready to get files with data");

            while(true){

                char loop_signal[16];
                size_t message_length = recvfrom(sockfd, loop_signal, sizeof(loop_signal) - 1, 0, (struct sockaddr*) &pc_wifi_addr_receive, &pc_wifi_addr_len);
                loop_signal[message_length] = '\0'; 
                if(strcmp(loop_signal, "BREAK") == 0){
                    ESP_LOGW(tag_udp_test, "loop signal: %s", loop_signal);
                    break;
                } else if(strcmp(loop_signal, "CONTINUE") == 0){
                    ESP_LOGW(tag_udp_test, "loop signal: %s", loop_signal);
                } else {
                    ESP_LOGE(tag_udp, "ERROR! wrong signal to file handling loop");
                    break;
                }

                char file_buffer[SIZE_RESPONSE_DATA_MAX];
                memset(file_buffer, '\0', sizeof(file_buffer));
                
                int file_size = receive_file_over_udp(file_buffer, SIZE_RESPONSE_DATA_MAX);
                if(file_size == 0){
                    ESP_LOGE(tag_udp, "some error raised");
                    break;
                }

                printf("data from pc:\n====\n%s\n====\n", file_buffer);

                char temp_data_buffer_for_getting_name[SIZE_RESPONSE_DATA_MAX];
                strcpy(temp_data_buffer_for_getting_name, file_buffer);

                char *line_saveptr = NULL;
                char *element_saveptr = NULL;
                // start parse file buffer (consists of data_line(s))
                // choose right delmiiter (depends on command file or response file)
                // get data line with id (first iteration of parsing)
                char *data_line_with_id = strtok_r(temp_data_buffer_for_getting_name, new_line_delimiter, &line_saveptr);
                char old_line_with_id[strlen(data_line_with_id)];
                strcpy(old_line_with_id, data_line_with_id);
                char current_delimiter[strlen(req_data_delimiter) + 1];
                strcpy(current_delimiter, req_data_delimiter);
                char name_postfix[strlen(name_postfix_command) + strlen(name_postfix_response)];
                char *new_line_with_id = strtok_r(data_line_with_id, current_delimiter, &element_saveptr);
                ESP_LOGW(tag_udp_test, "test ptr: %s\n", new_line_with_id);
                // choose right delimiter
                // if 
                if(strcmp(new_line_with_id, old_line_with_id) == 0){
                    // change delimiter to param
                    strcpy(current_delimiter, param_data_delimiter);
                    // skip element left from =
                    strtok_r(new_line_with_id, current_delimiter, &element_saveptr);
                    // change file postfix to param postfix
                    strcpy(name_postfix, name_postfix_command);
                } else {
                    // change file postfix to response postfix
                    strcpy(name_postfix, name_postfix_response);
                }
                char *satellite_id = strtok_r(NULL, current_delimiter, &element_saveptr); 
                char spiffs_passes_file_path[SPIFFS_FILE_NAME_LENGTH_MAX];
                create_spiffs_txt_file_path_by_params(satellite_id, name_postfix, spiffs_passes_file_path);
                // clear spiffs file before writing to it (to override old file)
                clear_data_from_spiffs_file(spiffs_passes_file_path);
                add_file_to_spiffs(spiffs_passes_file_path, file_buffer);
                // Бывает ошибка, когда send_response_to_board почему-то не попадает на файл
                vTaskDelay(pdMS_TO_TICKS(1000));
                send_response_to_pc("board can read another file");
            }

            send_response_to_pc("finish working with files");

            // конец действий
            // xEventGroupSetBits(
            //     (EventGroupHandle_t)xCommandGroup,
            //     BIT_NEXT_COMMAND
            // );

            send_response_to_pc(event_udp_finish_action);
        }

        else {
            ESP_LOGE(tag_udp, "no such command presents in functionlaity\n");
        }
    }
}


void initialize_udp_tasks(void){
    initialize_sockets();

    // EventGroupHandle_t xCommandGroup = xEventGroupCreate();

    ESP_LOGW(tag_udp_test, "первая отправка сигнала на считывания первой команды");
    TaskHandle_t xCommandHandle = NULL;
    BaseType_t xReturned;

    xReturned = xTaskCreate(
        task_udp_wait_command, 
        "task_udp_wait_command", 
        4096 * 50, 
        // (void*)xCommandGroup, 
        NULL,
        5, 
        &xCommandHandle
    );

    if(xReturned != pdPASS){
        ESP_LOGE(tag_udp, "Error! Task not created");
        vTaskDelete(xCommandHandle);
    }
}

void udp_task(void *pvParameters){
    // TickType_t xLastWakeTime;
    int sockfd;
    struct sockaddr_in pc_wifi_addr, pc_wifi_addr_send, board_wifi_addr_receive;

    ESP_LOGI(tag_udp, "Create socket . . .\n");
    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0){
        ESP_LOGE(tag_udp, "Socket not created\n");
        vTaskDelete(NULL);
    }

    memset(&pc_wifi_addr, 0, sizeof(pc_wifi_addr));
    memset(&pc_wifi_addr_send, 0, sizeof(pc_wifi_addr_send));
    memset(&board_wifi_addr_receive, 0, sizeof(board_wifi_addr_receive));
    socklen_t pc_wifi_addr_len = sizeof(pc_wifi_addr_len);

    // board info
    board_wifi_addr_receive.sin_family = AF_INET; // ipv4
    board_wifi_addr_receive.sin_addr.s_addr = INADDR_ANY;
    board_wifi_addr_receive.sin_port = htons(CONFIG_BOARD_PORT);

    // link socket with board
    if(bind(sockfd, (const struct sockaddr*) &board_wifi_addr_receive, sizeof(struct sockaddr_in)) < 0){
        ESP_LOGE(tag_udp, "socket not binded\n");
        vTaskDelete(NULL);
    }
    ESP_LOGI(tag_udp, "socket was binded\n");

    // pc info
    pc_wifi_addr_send.sin_family = AF_INET;// ipv4
    pc_wifi_addr_send.sin_addr.s_addr = inet_addr(CONFIG_PC_IP_WIFI);
    pc_wifi_addr_send.sin_port = htons(CONFIG_PC_PORT);

    char buffer_raw_data[128];
    char message_to_send[] = "hello from esp32";

    while(true){
        ESP_LOGW(tag_udp, "Waiting data from port %i", CONFIG_BOARD_PORT);
        size_t message_length = recvfrom(sockfd, &buffer_raw_data, sizeof(buffer_raw_data) - 1, 0, (struct sockaddr*) &pc_wifi_addr, &pc_wifi_addr_len);
        buffer_raw_data[message_length] = '\0'; 
        ESP_LOGI(tag_udp, "data from pc: %s\n", buffer_raw_data);

        vTaskDelay(pdMS_TO_TICKS(5000));

        sendto(sockfd, &message_to_send, strlen(message_to_send), 0, (struct sockaddr*) &pc_wifi_addr_send, sizeof(pc_wifi_addr_send));
        ESP_LOGW(tag_udp, "message \"%s\" sent to port %i, size %i bytes", message_to_send, CONFIG_PC_PORT, strlen(message_to_send));

        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    shutdown(sockfd, 0);
    close(sockfd);
    vTaskDelete(NULL);
}