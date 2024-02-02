#include "uart.h"

#define UART_BUFFER_SIZE 256
#define UART_BAUD_RATE 115200

#define UART0_TX_PIN 43
#define UART0_RX_PIN 44
#define UART0_RTS_PIN 15
#define UART0_CTS_PIN 16

const static char *uart_tag = "uart"; 

void uart_send_message(){
    char data_to_transmit[4000] = "message from the board over UART0\n"; 

    ESP_LOGW(uart_tag, "start transmitting data via uart. . .");
    int sended_bytes = uart_write_bytes(UART_NUM_0, data_to_transmit, strlen(data_to_transmit));
    ESP_LOGW(uart_tag, "%i bytes was sended", sended_bytes);
}

void uart_configure(){
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, UART_BUFFER_SIZE, UART_BUFFER_SIZE, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART0_TX_PIN, UART0_RX_PIN, UART0_RTS_PIN, UART0_CTS_PIN));
}