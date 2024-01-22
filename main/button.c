#include "button.h"
#include "uart.h"

const static char *button_tag = "gpio button";

static void button_callback(void *arg, void *usr_data){
    // ESP_LOGI(button_tag, "Message from callback function");
    uart_send_message();
}

void button_handler(){
    button_config_t btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .short_press_time = 100,
        .gpio_button_config = {
            .gpio_num = 45,
            .active_level = 0 
        },
    };
    
    button_handle_t gpio_btn = iot_button_create(&btn_cfg);

    if(gpio_btn == NULL){
        ESP_LOGE(button_tag, "Button create failed");
    }

    ESP_LOGI(button_tag ,"you can push the button . . .");

    button_event_config_t cfg = {
        .event = BUTTON_SINGLE_CLICK,
    };

    iot_button_register_event_cb(gpio_btn, cfg, button_callback, NULL);
} 
    
    