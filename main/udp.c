#include "udp.h"

// Убрать это в spiffs_storage.c или spiffs_storage.h 
// =======================================================
const static char *text_file_extension = ".txt";
const static char *name_options_file = "input_options"; 
const static char *name_postfix_command = "_command";
const static char* name_postfix_response = "_response";
// =======================================================

const static char *tag_udp = "udp";
const static char *tag_udp_test = "udp test"; // Логи с этой меткой можно впоследствии убрать (они чисто вспомогательные)

const static char *event_udp_get_command = "get command from pc";
const static char *event_udp_finish_action = "finish working with action by command";

const static char *response_send_udp = "response0"; 
const static char *response_receive_udp = "response1";

#define BIT_NEXT_COMMAND BIT0

const static char *command_template               = "commandx";
const static char *command_send_spiffs_data_to_pc = "command0";


// UDP sockets structs
int sockfd;
struct sockaddr_in board_wifi_addr_receive, pc_wifi_addr_receive, pc_wifi_addr_send;
socklen_t pc_wifi_addr_len = sizeof(pc_wifi_addr_receive);

// const static size_t size_max_package = 5632;
const static size_t size_max_request_options_file = 1256;

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
        stpcpy(buffer_to_save_path, spiffs_satellites_user_input_path);
}

static void initialize_sockets(void){
    ESP_LOGI(tag_udp, "Create socket . . .\n");
    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0){
        ESP_LOGE(tag_udp, "Socket not created\n");
        vTaskDelete(NULL);
    }

    memset(&board_wifi_addr_receive, 0, sizeof(board_wifi_addr_receive));
    memset(&pc_wifi_addr_receive, 0, sizeof(pc_wifi_addr_receive));
    memset(&pc_wifi_addr_send, 0, sizeof(pc_wifi_addr_send));

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

// // Как вариант?
// void udp_send_data_task(void *pvParameters){
//     for(;;){
//         // Ждём события, что пора отправлять какие-то данные по udp
//         // Отправка данных по udp
//     }
// }

// // Как вариант?
// void udp_get_data_task(void *pvParameters){
//     for(;;){
//         // Ждём события, что пора получать какие-то данные по udp
//         // Получение данных по udp
//     }
// }

// Можно потом поправить
// static void read_data_from_port(char *data_buffer, size_t buffer_size){
//     size_t data_length = recvfrom(sockfd, data_buffer, buffer_size, 0, (struct sockaddr*) &pc_wifi_addr_receive, &pc_wifi_addr_len);
//     data_buffer[data_length] = '\0';
// }

// Можно потом добавить, чтобы была универсальная команда по получению данных (экономия кода)
// static void udp_get_data_package(char* package_buffer, size_t buffer_size, const char *receive_event_info){
//     // Проверка на буфер нужного размера (пока непонятно нужна ли)
//     // if(buffer_size < package_size){
//     //     ESP_LOGE(tag_udp, "")
//     // }

//     ESP_LOGW(tag_udp, "udp receive event: %s", receive_event_info);
//     size_t data_length = recvfrom(sockfd, package_buffer, buffer_size, 0, (struct sockaddr*) &pc_wifi_addr_receive, &pc_wifi_addr_len);
//     package_buffer[data_length] = '\0';
//     ESP_LOGW(tag_udp, "get data package, content:\n====\n%s\n====", package_buffer);
// }

static void send_response_to_pc(const char *sending_event_info){
    ESP_LOGW(tag_udp_test, "udp send event: %s", sending_event_info);
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
    ESP_LOGW(tag_udp, "udp receive event: %s", waiting_event_info);
    char data_buffer[strlen(response_receive_udp)];
    size_t data_length = recvfrom(sockfd, data_buffer, sizeof(data_buffer), 0, (struct sockaddr*) &pc_wifi_addr_receive, &pc_wifi_addr_len);
    data_buffer[data_length] = '\0';
    ESP_LOGW(tag_udp, "get data: %s\n", data_buffer);

    while(strcmp(data_buffer, response_receive_udp) != 0){
        ESP_LOGW(tag_udp, "udp receive event: %s", waiting_event_info);
        size_t data_length = recvfrom(sockfd, data_buffer, strlen(data_buffer), 0, (struct sockaddr*) &pc_wifi_addr_receive, &pc_wifi_addr_len);
        data_buffer[data_length] = '\0';
        ESP_LOGW(tag_udp, "get data: %s\n", data_buffer);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task_udp_wait_command(void *xCommandGroup){
    // Ожидать команду (пока не получит командный буфер длиной 128 байт).
    // Принять команду, остановить получение новых команд.
    // В зависимости от принятой команды - отправить её в обработчик команд (через очередь) ИЛИ выставлятся
    // определённое события в группе событий, отвечающих за команды. 
    // В обработчике команд в зависимости от события - будет отрабатываться отпределённое действие, каждое в своём task. 
    // Возобновить ожидания команд как только получит сигнал о том, что задача отработала своё.

    // EventGroupHandle_t xCommandGroup = (EventGroupHandle_t)xCommandGroup;

    // Отправляем сигнал task_udp_wait_command, чтобы она начала считывать первую команду
    xEventGroupSetBits(
        (EventGroupHandle_t)xCommandGroup,
        BIT_NEXT_COMMAND
    );
    
    for(;;){
        ESP_LOGW(tag_udp_test, "ожидаем события на считывания следующей команды");
        // Ждём сигнал о том, что можно прееходить к ожиданию следующей команды (реализовать через группу событий).
        xEventGroupWaitBits(
            (EventGroupHandle_t)xCommandGroup,
            BIT_NEXT_COMMAND,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY
        );
        // Начато ожидание новой команды
        ESP_LOGI(tag_udp, "waiting command from port %i . . .", CONFIG_BOARD_PORT);
        // Ждём команды в стиле commandx , где x - номер комманды
        char command_buffer[strlen(command_template)];
        size_t command_length = recvfrom(sockfd, &command_buffer, sizeof(command_buffer), 0, (struct sockaddr*) &pc_wifi_addr_receive, &pc_wifi_addr_len); 
        if(command_length == 8){
            // Получение новых команд остановилось
            // Устанавливаем в группе событий событие, соответствующее нужной нам задаче (в зависимости от полученной команды)
            command_buffer[command_length] = '\0';
            ESP_LOGW(tag_udp ,"current command: %s\n", command_buffer);


            if(strcmp(command_buffer, command_template) == 0)


            {
                // Ответить питону, что получили команду
                send_response_to_pc(event_udp_get_command);
                wait_response_from_pc("test wating 1");
                // Действия над командой
                ESP_LOGW(tag_udp, "action to test command!");
                vTaskDelay(pdMS_TO_TICKS(5000));
                wait_response_from_pc("test waiting 2");
                // Ответить питону, что сделали всё, что надо было
                send_response_to_pc(event_udp_finish_action);
                ESP_LOGW(tag_udp_test, "сообщаем обработчику команд, что можно считывать следующую команду");
                // Сообщаем обработчику команд, что можно считывать следующую команду
                xEventGroupSetBits(
                    (EventGroupHandle_t)xCommandGroup,
                    BIT_NEXT_COMMAND
                );
            }


            else if(strcmp(command_buffer, command_send_spiffs_data_to_pc) == 0)


            {
                send_response_to_pc(event_udp_get_command);

                char request_options[size_max_request_options_file];

                ESP_LOGW(tag_udp, "udp receive event: %s", "waiting request options file");
                size_t data_length = recvfrom(sockfd, request_options, size_max_request_options_file, 0, (struct sockaddr*) &pc_wifi_addr_receive, &pc_wifi_addr_len);
                request_options[data_length] = '\0';
                ESP_LOGW(tag_udp, "get data package, content:\n====\n%s\n====", request_options);

                char spiffs_request_options_path[SPIFFS_MAX_FILE_NAME_LENGTH];
                char* first_line_in_file = strtok(request_options, "\n");
                if(strcmp(first_line_in_file, "START_FILE") == 0){
                    create_spiffs_txt_file_path_by_params(name_options_file, (char*)name_postfix_command, spiffs_request_options_path);
                    char* data_line = strtok(NULL, "\n");
                    while(data_line){
                        if(strcmp(data_line, "END_FILE") == 0){
                            send_response_to_pc("send info that we finished read info about python");
                            break;
                        }
                        add_line_to_spiffs(spiffs_request_options_path, data_line);
                        data_line = strtok(NULL, "\n");
                    }
                } else {
                    ESP_LOGW(tag_udp, "this is not first line in file");
                }

                char request_options_spiffs_data[data_length];
                read_data_from_spiffs_file_to_buffer(spiffs_request_options_path, request_options_spiffs_data, sizeof(request_options_spiffs_data));
                ESP_LOGW(tag_udp_test, "options from spiffs:\n====\n%s\n====\n", request_options_spiffs_data);

                // // general buffer string for package of all files that would be send
                // char files_buffer[SPIFFS_MAX_FILES * RESPONSE_DATA_SIZE] = {'\0'};

                // // interate over file
                // char* data_line_saveptr = NULL;
                // char* sat_saveptr = NULL;
                // char* data_line = strtok_r(request_options_spiffs_data, "\n", &data_line_saveptr);
                // while(data_line){
                //     // skip sat_name and get to sat_id
                //     strtok_r(data_line, "=", &sat_saveptr);
                //     char* sat_id = strtok_r(NULL, "=", &sat_saveptr);
                //     // create file spiffs path for sat_name
                //     char spiffs_path[SPIFFS_MAX_FILE_NAME_LENGTH];
                //     create_spiffs_txt_file_path_by_params(sat_id, name_postfix_response, spiffs_path);
                //     char response_buffer[RESPONSE_DATA_SIZE];
                //     if(read_data_from_spiffs_file_to_buffer(spiffs_path, response_buffer, RESPONSE_DATA_SIZE) == ESP_OK){
                //         char response_buffer_to_send[RESPONSE_DATA_SIZE] = {'\0'};
                //         strcat(response_buffer_to_send, "START_FILE\n");
                //         strcat(response_buffer_to_send, response_buffer);
                //         strcat(response_buffer_to_send, "END_FILE\n");
                //         // send sat data to uart to python script get it
                //         // int pass_bytes = uart_write_bytes(UART_NUM_0, (const char*)&response_buffer, strlen(response_buffer));
                //         strcat(files_buffer, response_buffer_to_send);
                //         // ESP_LOGW(command_handler_tag, "%i bytes were sended", pass_bytes);
                //     }
                //     create_spiffs_txt_file_path_by_params(sat_id, name_postfix_command, spiffs_path);
                //     char command_buffer[COMMAND_DATA_SIZE];
                //     if(read_data_from_spiffs_file_to_buffer(spiffs_path, command_buffer, COMMAND_DATA_SIZE) == ESP_OK){
                //         char command_buffer_to_send[RESPONSE_DATA_SIZE] = {'\0'};
                //         strcat(command_buffer_to_send, "START_FILE\n");
                //         strcat(command_buffer_to_send, command_buffer);
                //         strcat(command_buffer_to_send, "END_FILE\n");
                //         // int command_bytes = uart_write_bytes(UART_NUM_0, (const char*)&command_buffer, strlen(command_buffer));
                //         // ESP_LOGW(command_handler_tag, "%i bytes were sended", command_bytes);
                //         strcat(files_buffer, command_buffer_to_send);
                //     }
                //     data_line = strtok_r(NULL, "\n", &data_line_saveptr);
                // }

                // // send all files over udp
                // size_t sent_bytes = sendto(sockfd, files_buffer, strlen(files_buffer), 0, (struct sockaddr*) &pc_wifi_addr_send, sizeof(pc_wifi_addr_send));
                // ESP_LOGW(tag_udp_test, "message sent to port %i, size %i bytes", CONFIG_PC_PORT, sent_bytes);

                // clear_data_from_spiffs_file(spiffs_request_options_path);
                // wait_response_from_pc("python finished handle data");

                xEventGroupSetBits(
                    (EventGroupHandle_t)xCommandGroup,
                    BIT_NEXT_COMMAND
                );

                send_response_to_pc(event_udp_finish_action);
            }

            // else if
            // Обрабатываем следующую группу событий

        } else {
            ESP_LOGE(tag_udp, "don't get command in right format");
        }
    }
}


void initialize_udp_tasks(void){
    initialize_sockets();

    EventGroupHandle_t xCommandGroup = xEventGroupCreate();

    ESP_LOGW(tag_udp_test, "первая отправка сигнала на считывания первой команды");
    TaskHandle_t xCommandHandle = NULL;
    BaseType_t xReturned;

    xReturned = xTaskCreate(
        task_udp_wait_command, 
        "task_udp_wait_command", 
        4096, 
        (void*)xCommandGroup, 
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
        size_t message_length = recvfrom(sockfd, &buffer_raw_data, sizeof(buffer_raw_data), 0, (struct sockaddr*) &pc_wifi_addr, &pc_wifi_addr_len);
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