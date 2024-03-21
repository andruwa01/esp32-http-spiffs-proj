#ifndef UDP_H_
#define UDP_H_

#include "options.h"
#include "spiffs_storage.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

void udp_task(void *pvParameters);
void initialize_udp_tasks(void);

#endif