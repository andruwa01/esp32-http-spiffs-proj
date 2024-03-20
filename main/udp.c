#include "udp.h"

const static char *tag_udp = "udp";
const static char *tag_udp_test = "udp test"; // Логи с этой меткой можно впоследствии убрать (они чисто вспомогательные)

const static char *event_udp_get_command = "get command from pc";
const static char *event_udp_finish_action = "finish working with action by command";

const static char *response_send_udp = "response0"; 
const static char *response_receive_udp = "response1";

const static char *command_template               = "commandx";
const static char *command_send_spiffs_data_to_pc = "command3";

#define BIT_NEXT_COMMAND BIT0
#define BIT_TEST_COMMAND BIT2
#define BIT_SEND_SPIFFS_DATA_TO_PC_COMMAND BIT3

int sockfd;
struct sockaddr_in board_wifi_addr_receive, pc_wifi_addr_receive, pc_wifi_addr_send;
socklen_t pc_wifi_addr_len = sizeof(pc_wifi_addr_receive);

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

static void send_response_to_pc(const char *sending_event_info){
    // wait(1000);
    // uart_flush(UART_NUM_0);
    ESP_LOGW(tag_udp_test, "udp sent event: %s", sending_event_info);
    size_t sent_bytes = 0;
    // sent_bytes = uart_write_bytes(UART_NUM_0, next_action_value, strlen(next_action_value));
    sent_bytes = sendto(sockfd, response_send_udp, strlen(response_send_udp), 0, (struct sockaddr*) &pc_wifi_addr_send, sizeof(pc_wifi_addr_send));
    ESP_LOGW(tag_udp, "message \"%s\" sent to port %i, size %i bytes", response_send_udp, CONFIG_PC_PORT, strlen(response_send_udp));
    ESP_LOGW(tag_udp_test, "%i bytes sent", sent_bytes);
    // uart_flush(UART_NUM_0);
    // wait(1000);
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

void udp_command_task(void *xCommandGroup){
    // Ожидать команду (пока не получит командный буфер длиной 128 байт).
    // Принять команду, остановить получение новых команд.
    // В зависимости от принятой команды - отправить её в обработчик команд (через очередь) ИЛИ выставлятся
    // определённое события в группе событий, отвечающих за команды. 
    // В обработчике команд в зависимости от события - будет отрабатываться отпределённое действие, каждое в своём task. 
    // Возобновить ожидания команд как только получит сигнал о том, что задача отработала своё.

    // EventGroupHandle_t xCommandGroup = (EventGroupHandle_t)xCommandGroup;
    
    for(;;){
        // Начато ожидание новой команды
        ESP_LOGW(tag_udp, "Waiting command from port %i . . .", CONFIG_BOARD_PORT);
        // Ждём команды в стиле commandx , где x - номер комманды
        char command_buffer[strlen(command_template)];
        size_t command_length = recvfrom(sockfd, &command_buffer, sizeof(command_buffer), 0, (struct sockaddr*) &pc_wifi_addr_receive, &pc_wifi_addr_len); 
        if(command_length == 8){
            // Получение новых команд остановилось
            // Устанавливаем в группе событий событие, соответствующее нужной нам задаче (в зависимости от полученной команды)
            command_buffer[command_length] = '\0';
            ESP_LOGW(tag_udp ,"Current command: %s\n", command_buffer);
            if(strcmp(command_buffer, command_template) == 0)
            {
                xEventGroupSetBits((EventGroupHandle_t)xCommandGroup, BIT_TEST_COMMAND);
            }
            else if(strcmp(command_buffer, command_send_spiffs_data_to_pc) == 0)
            {
                xEventGroupSetBits((EventGroupHandle_t)xCommandGroup, BIT_SEND_SPIFFS_DATA_TO_PC_COMMAND);
            }

            // else if
            // Обрабатываем следующую группу событий

            // Ждём сигнал о том, что можно прееходить к ожиданию следующей команды (реализовать через группу событий).
            xEventGroupWaitBits(
                (EventGroupHandle_t)xCommandGroup,
                BIT_NEXT_COMMAND,
                pdTRUE,
                pdFALSE,
                portMAX_DELAY
            );
        }
    }
}

void command_handler_task(void *xCommandGroup){
    for(;;){
        // Ждём события из группы событий
        EventBits_t xCommandBits;
        xCommandBits = xEventGroupWaitBits(
            (EventGroupHandle_t)xCommandGroup,
            BIT_TEST_COMMAND | BIT_SEND_SPIFFS_DATA_TO_PC_COMMAND,  
            pdTRUE,
            pdFALSE,
            portMAX_DELAY        
        );

        if(xCommandBits && BIT_TEST_COMMAND)
        
        {
            // Ответить питону, что получили команду
            send_response_to_pc(event_udp_get_command);

            wait_response_from_pc("test wating 1");
            // Действия над командой
            ESP_LOGW(tag_udp, "action to test command!");
            vTaskDelay(pdMS_TO_TICKS(5000));

            wait_response_from_pc("test waiting 2");
            // Сообщаем обработчику команд, что можно считывать следующую команду
            xEventGroupSetBits(
                (EventGroupHandle_t)xCommandGroup,
                BIT_NEXT_COMMAND
            );
            // Ответить питону, что сделали всё, что надо было
            send_response_to_pc(event_udp_finish_action);
        }

        else if(xCommandBits && BIT_SEND_SPIFFS_DATA_TO_PC_COMMAND)
        
        {
            send_response_to_pc(event_udp_get_command);
            
            ESP_LOGW(tag_udp_test, "action on command");
            vTaskDelay(pdMS_TO_TICKS(5000));

            xEventGroupSetBits(
                (EventGroupHandle_t)xCommandGroup,
                BIT_NEXT_COMMAND
            );

            send_response_to_pc(event_udp_finish_action);
        }

        else
        
        {
            ESP_LOGE(tag_udp, "No command passed!");
        }
    }
}

void initialize_udp_tasks(void){
    initialize_sockets();

    EventGroupHandle_t xCommandGroup = xEventGroupCreate();

    TaskHandle_t xCommandHandle = NULL;
    TaskHandle_t xCommandHandlerHandle = NULL;
    BaseType_t xReturned;

    xReturned = xTaskCreate(
        udp_command_task, 
        "command_task", 
        2048, 
        (void*)xCommandGroup, 
        5, 
        &xCommandHandle
    );

    if(xReturned != pdPASS){
        ESP_LOGE(tag_udp, "Error! Task not created");
        vTaskDelete(xCommandHandle);
    }

    xReturned = xTaskCreate(
        command_handler_task, 
        "command_handler_task", 
        4096, 
        (void*)xCommandGroup, 
        5, 
        &xCommandHandlerHandle
    );

    if(xReturned != pdPASS){
        ESP_LOGE(tag_udp, "Error! Task not created");
        vTaskDelete(xCommandHandlerHandle);
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