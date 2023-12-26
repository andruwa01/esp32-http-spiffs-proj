#include "http_client.h"
#include "time_converter.h"
#include "satellite_data_parser.h"

const static char* TAGhttp = "HTTP";

int event_counter = 0;

// Client
esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt) {
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        event_counter++;
        // event_counter == 1 missed because we don't need this information
        if(event_counter > 1){
            printf("Client HTTP_EVENT_ON_DATA, data value: %.*s\n",evt->data_len, (char*)evt->data);
            printf("\n");
            json_parser((char*)evt->data);
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

void get_rest_function(){
    esp_http_client_config_t config_get = {
        // use .host + .path or only .url

        // .url = "http://worldtimeapi.org/api/timezone/America/Argentina/Salta", //work
        // .url = "http://httpbin.org/get",                                       //work

        // WARNING: disabled security options in menuconfig to work with this url 
        .url = "http://api.n2yo.com/rest/v1/satellite/radiopasses/25544/41.702/-76.014/0/10/40/*",
        // .cert_pem = (const unsigned char*) certificate_pem_start,
        // .client_cert_pem = root_ca,
        // .cert_pem = root_ca_n2yo,
        .buffer_size = 4096,
        .method = HTTP_METHOD_GET,
        .event_handler = client_event_get_handler
    };

    esp_http_client_handle_t client = esp_http_client_init(&config_get);
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}
