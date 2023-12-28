// Header guard
#ifndef HEADER_H_MAIN
#define HEADER_H_MAIN
//-------------------------------
#include "options.h"

#include <stdio.h>
#include <cJSON.h>

// ESP-IDF headers
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_spiffs.h"
#include "esp_err.h"
#include "esp_log.h"
#include "string.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "esp_tls.h"
#include <sys/unistd.h> 
#include <sys/stat.h>
#include <sys/param.h>
#include "esp_http_client.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "time.h"
#include "stdlib.h"
//-------------------------------
#endif