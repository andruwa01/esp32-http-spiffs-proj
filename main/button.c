#include "main.h"

const static char *button_tag = "gpio_button";

void button_callback(){    
    ESP_LOGI(button_tag, "Message from callback function");

    #if defined(UART_TEST)
    #if defined(SEND_DATA_FROM_SPIFFS_TO_UART)
        for(int file_index = 0; file_index < SPIFFS_NUMBER_OF_FILES; file_index++){
            char spiffs_file_path[strlen(SPIFFS_BASE_PATH) + strlen("/") + strlen(satellites[file_index].name)];
            sprintf(spiffs_file_path, "%s/%s", SPIFFS_BASE_PATH, satellites[file_index].name);

            char satellite_data[HTTP_BUFFER_SIZE];
            read_data_from_spiffs_file_to_buffer(spiffs_file_path, satellite_data, HTTP_BUFFER_SIZE);
            strcat(satellite_data, "END_OF_THE_FILE\n");

            printf("%s\n", (const char*)satellite_data);

            int sended_bytes = uart_write_bytes(UART_NUM_0, (const char*)satellite_data, strlen(satellite_data));
            ESP_LOGW(button_tag, "%i bytes were sended", sended_bytes);
        }
    #endif
    #endif

    // uart_send_message();
    // initialize_get_request();
    // initialize_get_requests_for_all_satellites();
    // initialize_get_request(49338, "jilin-01_gaofen_2f.txt");
    // initialize_get_request(57191, "polytech_universe-3.txt");
}

void button_handler(){
    button_config_t btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = 45,
            .active_level = 1 
        },
    };
    
    button_handle_t gpio_btn = iot_button_create(&btn_cfg);

    if(gpio_btn == NULL){
        ESP_LOGE(button_tag, "Button create failed");
    }

    button_event_config_t cfg = {
        .event = BUTTON_PRESS_DOWN,
    };

    iot_button_register_event_cb(gpio_btn, cfg, button_callback, NULL);

    ESP_LOGW(button_tag, "Button was initiated!");
} 
    
    