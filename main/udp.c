#include "udp.h"

const static char *tag_udp = "udp";

void udp_task(void *pvParameters){
    // TickType_t xLastWakeTime;
    int sockfd;
    struct sockaddr_in pc_wifi_addr, pc_wifi_addr_send, board_wifi_addr;

    ESP_LOGI(tag_udp, "Create socket . . .\n");
    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0){
        ESP_LOGE(tag_udp, "Socket not created\n");
        vTaskDelete(NULL);
    }

    memset(&pc_wifi_addr, 0, sizeof(pc_wifi_addr));
    memset(&pc_wifi_addr_send, 0, sizeof(pc_wifi_addr_send));
    memset(&board_wifi_addr, 0, sizeof(board_wifi_addr));
    socklen_t pc_wifi_addr_len = sizeof(pc_wifi_addr_len);

    // board info
    board_wifi_addr.sin_family = AF_INET; // ipv4
    board_wifi_addr.sin_addr.s_addr = INADDR_ANY;
    board_wifi_addr.sin_port = htons(CONFIG_BOARD_PORT);

    // link socket with board
    if(bind(sockfd, (const struct sockaddr*) &board_wifi_addr, sizeof(struct sockaddr_in)) < 0){
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