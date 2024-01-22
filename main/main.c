#include <main.h>
#include "spiffs_storage.h"
#include "wifi.h"
#include "http_client.h"
#include "uart.h"
#include "button.h"

const static char* TAGmain = "main_app";

void xTaskButton(){
    for(;;){
        ESP_LOGI(TAGmain, "xTaskButton called");
        button_handler();
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    uart_configure();
    uart_write_bytes(UART_NUM_0, "\n\r", sizeof("\n\r"));

    TaskHandle_t TaskButtonHandle;
    if (xTaskCreate(xTaskButton, "button", 3000, NULL, 2, &TaskButtonHandle) != pdPASS){
        ESP_LOGE(TAGmain, "failed to create task");
    }

    vTaskDelay(pdMS_TO_TICKS(10000));

    ESP_LOGW(TAGmain, "Program finished");
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