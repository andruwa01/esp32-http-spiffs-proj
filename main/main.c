#include <main.h>
#include "spiffsStorage.h"

// TAG for ESP_LOGI
static const char *MainTAG = "main function";

// LED state for blinking in the end of the cycle
int ledState = 0;

void app_main(void)
{
    // Configure LED with CONFIG_LED_GPIO
    gpio_reset_pin(CONFIG_LED_GPIO); 
    gpio_set_direction(CONFIG_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(CONFIG_LED_GPIO, 0);

    // Initialize spiffs
    spiffsHandler();

    while (true)
    {
        ESP_LOGI(MainTAG, "Program works . . .");

        // Switch led in the end of each cycle
        gpio_set_level(CONFIG_LED_GPIO, (ledState++ & 1));

        // Then pause 1000ms = 1s
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}