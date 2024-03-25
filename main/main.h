// Header guard
#ifndef MAIN_H
#define MAIN_H 
//-------------------------------
#include "options.h"

// Standart C libraries
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <dirent.h>

// #include <cJSON.h>

// // Source headers (заголовки для работы с udp отдельным проектом, ещё не завершены)
#include "spiffs_storage.h"
#include "nvs.h"
#include "wifi.h"
#include "udp.h"

// #include "http_client.h"
#include "api_key.h" // key for request (to exclude from git)
#include "uart.h"
// #include "button.h"
#include "freertos_handler.h"
#include "satellite_data_parser.h"
#include "time_converter.h"
#include "command_handler.h"

// ESP-IDF headers
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/queue.h"
// #include "freertos/event_groups.h"
// #include "driver/gpio.h"
// #include "sdkconfig.h"
// #include "esp_spiffs.h"
// #include "esp_vfs.h"
// #include "esp_err.h"
// #include "esp_log.h"
// #include "string.h"
// #include "esp_netif.h"
// #include "esp_http_client.h"
// #include "esp_tls.h"
// #include <sys/unistd.h> 
// #include <sys/stat.h>
// #include <sys/param.h>
// #include "esp_http_client.h"
// #include "esp_wifi.h"
// #include "nvs_flash.h"
// #include "driver/uart.h"
// #include "esp_console.h"

// ESP-IDF component headers
// #include "iot_button.h"


//-------------------------------
#endif