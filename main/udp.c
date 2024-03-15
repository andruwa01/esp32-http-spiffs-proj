#include "udp.h"

const static char *tag_udp = "udp";

void udp_task(void *pvParameters){
    TickType_t xLastWakeTime;
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    ESP_LOGI(tag_udp, "Create socket . . .\n");
    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0){
        ESP_LOGE(tag_udp, "Socket not created\n");
        vTaskDelete(NULL);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // client info
    cliaddr.sin_family = AF_INET; // ipv4
    cliaddr.sin_addr.s_addr = INADDR_ANY;
    cliaddr.sin_port = htons(CONFIG_CLIENT_PORT);

    // link socket with client
    if(bind(sockfd, (const struct sockaddr *)&cliaddr, sizeof(struct sockaddr_in)) < 0){
        ESP_LOGE(tag_udp, "socket not binded\n");
        vTaskDelete(NULL);
    }
    ESP_LOGI(tag_udp, "socket was binded\n");

    // server info
    servaddr.sin_family = AF_INET;// ipv4
    servaddr.sin_addr.s_addr = inet_addr(CONFIG_SERVER_IP);
    servaddr.sin_port = htons(CONFIG_SERVER_PORT);

    // xLastWakeTime = xTaskGetTickCount();

    for(short i = 0;;i++){
        sendto(sockfd, &i, 2, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));

        ESP_LOGW(tag_udp, "Package sent . . .");
        // vTaskDelayUntil(xLastWakeTime, 10 / configTICK_RATE_HZ );
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    shutdown(sockfd, 0);
    close(sockfd);
    vTaskDelete(NULL);
}