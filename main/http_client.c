#include "main.h"

const static char* tag_http_client = "http_client";

//==========GET request data NORBI===============//
// static int id = 46494;
const static char temp_file_path[SPIFFS_MAX_FILE_NAME_LENGTH];
const static float observer_lat = 51.671667;
const static float observer_lng = 39.210556;
const static float observer_alt = 99;
const static int days = 10;
const static int min_elevation = 40;
const static char *api_key = N2YO_API_KEY;
//===============================================//

static int event_counter = 0;

satellite satellites[SPIFFS_NUMBER_OF_FILES] = {
    {.name = "norbi.txt",       .id = 46494},
    {.name = "2023-091t.txt",   .id = 57183},
    {.name = "cstp-1.1.txt",    .id = 57202},
    {.name = "cstp-1.2.txt",    .id = 57186},
    {.name = "fees_sat.txt",    .id = 48082},
    {.name = "fossasat.txt",    .id = 52776},
    {.name = "jilingaofen.txt", .id = 49338},
    {.name = "mdqubesat-2.txt", .id = 58665}, 
    {.name = "polytech_un.txt", .id = 57191},
    {.name = "rs52sb.txt",      .id = 57200},
    {.name = "rs52sd.txt",      .id = 57208},
    {.name = "rs52se.txt",      .id = 57205},
    {.name = "rs52sg.txt",      .id = 57170},
    {.name = "rs52sv.txt",      .id = 57169},
    {.name = "tianqi-7.txt",    .id = 54687}
};

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

            printf("\n");
 
            printf("data value from get response:%.*s",(int)evt->data_len, (char*)evt->data);

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

            #if defined(SPIFFS_USE_FUNCTIONALITY)
                json_parse_and_write_data_from_http_response_to_spiffs((char*)temp_file_path, (char*)evt->data);
                // json_parse_and_write_data_from_http_response_to_spiffs("/spiffs/jilin-01_gaofen_2f.txt", (char*)evt->data);
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



void initialize_get_request(int sat_id, char *file_name){

    char spiffs_temp_file_path[strlen(SPIFFS_BASE_PATH) + strlen("/") + strlen(file_name)];
    sprintf(spiffs_temp_file_path, "%s/%s", SPIFFS_BASE_PATH, file_name);

    // now we work with temp_file_name in client_event_get_handler 
    strcpy(temp_file_path, spiffs_temp_file_path);

    char url_buffer[256];

    sprintf(url_buffer, "http://api.n2yo.com/rest/v1/satellite/radiopasses/%i/%f/%f/%f/%i/%i/&apiKey=%s", 
        sat_id,
        observer_lat,
        observer_lng,
        observer_alt, 
        days,
        min_elevation,
        api_key
    );

    esp_http_client_config_t config_get = {
        // use .host + .path or only .url
        .url = url_buffer,
        .buffer_size = HTTP_BUFFER_SIZE,
        .method = HTTP_METHOD_GET,
        .event_handler = client_event_get_handler,
        .skip_cert_common_name_check = true,
        .timeout_ms = HTTP_TIMOUT_MS
    };

    esp_http_client_handle_t client = esp_http_client_init(&config_get);

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

void initialize_get_requests_for_all_satellites(){
    for(int satellite_index = 0; satellite_index < SPIFFS_NUMBER_OF_FILES; satellite_index++){
        initialize_get_request(satellites[satellite_index].id, satellites[satellite_index].name);
    }
}