#ifndef WIFI_H_
#define WIFI_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_err.h"

void initialize_wifi(void);

#endif