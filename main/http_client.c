#include "main.h"

const static char* tag_http_client = "http_client";

//==========GET request data NORBI===============//
// static int id = 46494;
const static char temp_file_path[SPIFFS_FILE_NAME_LENGTH_MAX];
const static float observer_lat = 51.671667;
const static float observer_lng = 39.210556;
const static float observer_alt = 99;
const static int days = 10;
const static int min_elevation = 40;
const static char *api_key = N2YO_API_KEY;
//===============================================//

static int response_part = 0;

satellite_data satellites[SPIFFS_NUMBER_OF_FILES] = {
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
        response_part++;
        // response_part == 1 missed because we don't need this information
        // from get response
        if(response_part == 2){
            printf("\n");
            printf("Current (int)evt->data_len: %i\n", (int)evt->data_len);
            printf("\n");
            printf("data value from get response:%.*s",(int)evt->data_len, (char*)evt->data);
            printf("\n");

            response_part = 0;

            #if defined(SPIFFS_USE_FUNCTIONALITY)
                json_parse_and_write_data_from_http_response_to_spiffs((char*)temp_file_path, (char*)evt->data);
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