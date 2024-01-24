#include "freertos_handler.h"

#define TIME_TO_PUSH_BUTTON_MS 20000

const static char *tag_freertos = "freertos_manager";

void xTaskButton(){
    ESP_LOGI(tag_freertos, "xTaskButton called");
    button_handler();
    for(;;){
        vTaskSuspend(NULL);
    }
    vTaskDelete(NULL);
}

void initialize_freertos_tasks(){
    TaskHandle_t TaskButtonHandle = NULL;
    if (xTaskCreate(xTaskButton, "button", 3000, NULL, 2, &TaskButtonHandle) != pdPASS){
        ESP_LOGE(tag_freertos, "failed to create task");
    }

    ESP_LOGW(tag_freertos, "You can push the button and have only %i seconds to do it, then app_main return", TIME_TO_PUSH_BUTTON_MS / 1000);
    vTaskDelay(pdMS_TO_TICKS(TIME_TO_PUSH_BUTTON_MS));

    ESP_LOGW(tag_freertos, "%i seconds finished", TIME_TO_PUSH_BUTTON_MS);
    vTaskDelete(TaskButtonHandle);
}