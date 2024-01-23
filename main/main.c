#include <main.h>


const static char* TAGmain = "main_app";

void xTaskButton(){
    ESP_LOGI(TAGmain, "xTaskButton called");
    button_handler();
    for(;;){
        vTaskSuspend(NULL);
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    uart_configure();
    
    TaskHandle_t TaskButtonHandle = NULL;
    if (xTaskCreate(xTaskButton, "button", 3000, NULL, 2, &TaskButtonHandle) != pdPASS){
        ESP_LOGE(TAGmain, "failed to create task");
    }

    ESP_LOGW(TAGmain, "You can push the button!");
    // vTaskDelay(pdMS_TO_TICKS(20000));

    vTaskDelete(TaskButtonHandle);

    // button_handler();

    // uart_configure();

    // #ifdef USE_SPIFFS
	//     spiffs_handler();
    // #endif

    // esp_err_t ret = nvs_flash_init();
    // if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }>

    // get_rest_function();
    // #ifdef USE_SPIFFS
    //     read_file_from_spiffs("/spiffs/norbi.txt");
    // #else
    //     ESP_LOGW(TAGmain, "You don't use spiffs!");
    // #endif
}
