#include "http_client.h"
#include "time_converter.h"
#include "satellite_data_parser.h"

const static char* TAGhttp = "HTTP";

// GET request data NORBI
static int id = 46494;
static float observer_lat = 51.671667;
static float observer_lng = 39.210556;
static float observer_alt = 99;
static int days = 10;
static int min_elevation = 40;
static char *api_key = "VKC8LB-XBX436-NS9KSA-56EJ";
//===============================================//

static int event_counter = 0;

// Client
esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt) {
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        event_counter++;
        // event_counter == 1 missed because we don't need this information
        if(event_counter > 1){
            printf("Current data_len of data: %i\n", (int)evt->data_len);
            printf("Client HTTP_EVENT_ON_DATA, data value:\n%.*s\n",evt->data_len, (char*)evt->data);
            printf("\n");

            #ifdef PARSE_DATA
                json_parser((char*)evt->data);
            #endif
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

void get_rest_function(){
    char url_buffer[256];

    sprintf(url_buffer, "http://api.n2yo.com/rest/v1/satellite/radiopasses/%i/%f/%f/%f/%i/%i/&apiKey=%s", 
        id,
        observer_lat,
        observer_lng,
        observer_alt, 
        days,
        min_elevation,
        api_key
    );

    esp_http_client_config_t config_get = {
        // use .host + .path or only .url

        // .url = "http://worldtimeapi.org/api/timezone/America/Argentina/Salta", //work
        // .url = "http://httpbin.org/get",                                       //work

        // WARNING: disabled security options in menuconfig to work with this url 
        // .url = "http://api.n2yo.com/rest/v1/satellite/radiopasses/25544/41.702/-76.014/0/10/40/&apiKey=VKC8LB-XBX436-NS9KSA-56EJ", // old data from api
        .url = url_buffer,
        // .cert_pem = (const unsigned char*) certificate_pem_start,
        // .client_cert_pem = root_ca,
        // .cert_pem = root_ca_n2yo,
        .buffer_size = 4096,
        .method = HTTP_METHOD_GET,
        .event_handler = client_event_get_handler,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .skip_cert_common_name_check = true
    };

    esp_http_client_handle_t client = esp_http_client_init(&config_get);

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}
