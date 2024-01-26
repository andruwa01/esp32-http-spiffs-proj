#include "main.h"

const static char* tag_http_client = "http_client";

//==========GET request data NORBI===============//
static int id = 46494;
static float observer_lat = 51.671667;
static float observer_lng = 39.210556;
static float observer_alt = 99;
static int days = 10;
static int min_elevation = 40;
static char *api_key = N2YO_API_KEY;
//===============================================//

// static char *test_response_get_string = "info.satid,info.satname,info.transactionscount,info.passescount,passes.0.startAz,passes.0.startAzCompass,passes.0.startUTC,passes.0.maxAz,passes.0.maxAzCompass,passes.0.maxEl,passes.0.maxUTC,passes.0.endAz,passes.0.endAzCompass,passes.0.endUTC,passes.1.startAz,passes.1.startAzCompass,passes.1.startUTC,passes.1.maxAz,passes.1.maxAzCompass,passes.1.maxEl,passes.1.maxUTC,passes.1.endAz,passes.1.endAzCompass,passes.1.endUTC,passes.2.startAz,passes.2.startAzCompass,passes.2.startUTC,passes.2.maxAz,passes.2.maxAzCompass,passes.2.maxEl,passes.2.maxUTC,passes.2.endAz,passes.2.endAzCompass,passes.2.endUTC,passes.3.startAz,passes.3.startAzCompass,passes.3.startUTC,passes.3.maxAz,passes.3.maxAzCompass,passes.3.maxEl,passes.3.maxUTC,passes.3.endAz,passes.3.endAzCompass,passes.3.endUTC,passes.4.startAz,passes.4.startAzCompass,passes.4.startUTC,passes.4.maxAz,passes.4.maxAzCompass,passes.4.maxEl,passes.4.maxUTC,passes.4.endAz,passes.4.endAzCompass,passes.4.endUTC,passes.5.startAz,passes.5.startAzCompass,passes.5.startUTC,passes.5.maxAz,passes.5.maxAzCompass,passes.5.maxEl,passes.5.maxUTC,passes.5.endAz,passes.5.endAzCompass,passes.5.endUTC,passes.6.startAz,passes.6.startAzCompass,passes.6.startUTC,passes.6.maxAz,passes.6.maxAzCompass,passes.6.maxEl,passes.6.maxUTC,passes.6.endAz,passes.6.endAzCompass,passes.6.endUTC,passes.7.startAz,passes.7.startAzCompass,passes.7.startUTC,passes.7.maxAz,passes.7.maxAzCompass,passes.7.maxEl,passes.7.maxUTC,passes.7.endAz,passes.7.endAzCompass,passes.7.endUTC,passes.8.startAz,passes.8.startAzCompass,passes.8.startUTC,passes.8.maxAz,passes.8.maxAzCompass,passes.8.maxEl,passes.8.maxUTC,passes.8.endAz,passes.8.endAzCompass,passes.8.endUTC,passes.9.startAz,passes.9.startAzCompass,passes.9.startUTC,passes.9.maxAz,passes.9.maxAzCompass,passes.9.maxEl,passes.9.maxUTC,passes.9.endAz,passes.9.endAzCompass,passes.9.endUTC,passes.10.startAz,passes.10.startAzCompass,passes.10.startUTC,passes.10.maxAz,passes.10.maxAzCompass,passes.10.maxEl,passes.10.maxUTC,passes.10.endAz,passes.10.endAzCompass,passes.10.endUTC,passes.11.startAz,passes.11.startAzCompass,passes.11.startUTC,passes.11.maxAz,passes.11.maxAzCompass,passes.11.maxEl,passes.11.maxUTC,passes.11.endAz,passes.11.endAzCompass,passes.11.endUTC46494,NORBI,17,12,9,N,1706230745,288.12,WNW,46.56,1706231090,211.11,SW,1706231425,11.08,N,1706316435,289.97,WNW,62.61,1706316785,204.32,SSW,1706317130,175.81,S,1706355230,260.79,W,53.43,1706355580,342.47,NNW,1706355920,13.09,N,1706402130,266.1,W,83.76,1706402485,197.58,SSW,1706402830,169.09,S,1706440925,251.66,WSW,72.82,1706441270,344.81,NNW,1706441620,15.18,NNE,1706487825,99.6,E,72.63,1706488175,190.76,S,1706488520,162.32,SSE,1706526615,75.08,E,83.69,1706526965,346.95,N,1706527310,17.4,NNE,1706573520,103.41,E,53.32,1706573870,183.88,S,1706574210,155.62,SSE,1706612310,69.17,ENE,62.06,1706612660,349.09,N,1706613000,148.82,SE,1706698010,70.77,ENE,46.03,1706698350,351.15,N,1706698690,9.35,N,1706921930,288.12,WNW,48.33,1706922280,210.25,SW,1706922620,182.03,S,1706960730,261.4,W,40.93,1706961070,340.5,NNW,1706961410";
static int event_counter = 0;


// Client
esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt) {
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        event_counter++;
        // event_counter == 1 missed because we don't need this information
        // from get response
        if(event_counter > 1){
            printf("\n");

            printf("Current (int)evt->data_len: %i\n", (int)evt->data_len);
            printf("Current evt->data_len: %i\n", evt->data_len);
            printf("Current sizeof((char*)evt->data): %i\n", sizeof((char*)evt->data));
            printf("Current strlen((char*)evt->data): %i\n",strlen((char*)evt->data));
            printf("\n");

            printf("data value from get response:\n%.*s\n",(int)evt->data_len, (char*)evt->data);
            printf("\n");

            // int sended_bytes = uart_write_bytes(UART_NUM_0, test_response_get_string, strlen(test_response_get_string));
            // ESP_LOGW(tag_http_client, "%i bytes was sended", sended_bytes);

            // uart_send_message();

            // char event_data_buffer[(int)evt->data_len + 1]; 
            // strcat(event_data_buffer, (char*)evt->data);
            // strcat(event_data_buffer, "\n");

            // int sended_bytes = uart_write_bytes(UART_NUM_0, (char*)evt->data, (int)evt->data_len);
            // ESP_LOGW(tag_http_client, "%i bytes sent", sended_bytes);

            // event_counter = 0 so next time when we push the button - we also get
            // only data package instead of something with HTML tags
            // also we could use (int)evt->data_len in this if, but i suppose that solution
            // with event_counter variable is better
            event_counter = 0;

            #ifdef SPIFFS_USE_FUNCTIONALITY
                json_parse_and_write_data_from_http_response_to_spiffs((char*)evt->data);
            #else
                ESP_LOGW(tag_http_client, "You don't use spiffs!");
            #endif
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

void initialize_get_request(){
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
        .buffer_size = HTTP_BUFFER_SIZE,
        .method = HTTP_METHOD_GET,
        .event_handler = client_event_get_handler,
        .skip_cert_common_name_check = true
    };

    esp_http_client_handle_t client = esp_http_client_init(&config_get);

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}