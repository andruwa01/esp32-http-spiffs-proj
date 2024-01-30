#include "button.h"
#include "uart.h"

const static char *button_tag = "gpio button";

void button_callback(){    
    ESP_LOGI(button_tag, "Message from callback function");
    // uart_send_message();
    // initialize_get_request();
    initialize_get_requests_for_all_satellites();
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
} 
    
    