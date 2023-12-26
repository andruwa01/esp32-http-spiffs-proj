#include "time_converter.h"
#include "satellite_data_parser.h"
#include "spiffsStorage.h"

// #define PRINT_PACKAGE

const static char *time_converter_tag = "time_converter";
static char time_format[] = "%d.%m.%Y %H:%M";


void json_parser(char* string_to_parse){
	ESP_LOGI(time_converter_tag, "Start parsing data . . .");

	cJSON *json_object = cJSON_Parse(string_to_parse);

	if(json_object == NULL){
		const char* error_ptr = cJSON_GetErrorPtr();
		if(error_ptr != NULL){
			ESP_LOGE(time_converter_tag, "error_ptr on symbol  %s", error_ptr);
		}
		cJSON_Delete(json_object);
	}

	cJSON *info_elements = cJSON_GetObjectItemCaseSensitive(json_object, "info");

	cJSON *sat_id = cJSON_GetObjectItemCaseSensitive(info_elements, "satid");
	cJSON *sat_name = cJSON_GetObjectItemCaseSensitive(info_elements, "satname");
	cJSON *transactions_count = cJSON_GetObjectItemCaseSensitive(info_elements, "transactionscount");
	cJSON *passes_count = cJSON_GetObjectItemCaseSensitive(info_elements, "passescount");

	#ifdef PRINT_PACKAGE
	if(cJSON_IsNumber(sat_id) && cJSON_IsString(sat_name) && cJSON_IsNumber(transactions_count) && cJSON_IsNumber(passes_count)){
		printf("\n");
		printf("Value from \"info\":\n");
		printf("satid: %i\n", (int)cJSON_GetNumberValue(sat_id));
		printf("satname: %s\n", cJSON_GetStringValue(sat_name));
		printf("transactionscount: %i\n", (int)cJSON_GetNumberValue(transactions_count));
		printf("passescount: %i\n", (int)cJSON_GetNumberValue(passes_count));
	}
	#endif

	// TODO need to write something or in a file from first part of package or not ?
	
	time_t time_to_convert_start; 
	time_t time_to_convert_end;

	char utc_converted_start[32];
	char utc_converted_end[32];
	char formatted_string[128];

	cJSON *pass = NULL;
	cJSON *passes = cJSON_GetObjectItemCaseSensitive(json_object, "passes");
	int index = 1;
	cJSON_ArrayForEach(pass, passes){
		cJSON *start_az = cJSON_GetObjectItemCaseSensitive(pass, "startAz"); 
		cJSON *start_az_compass = cJSON_GetObjectItemCaseSensitive(pass, "startAzCompass");
		cJSON *start_utc = cJSON_GetObjectItemCaseSensitive(pass, "startUTC");
		cJSON *max_az = cJSON_GetObjectItemCaseSensitive(pass, "maxAz");
		cJSON *max_az_compass = cJSON_GetObjectItemCaseSensitive(pass, "maxAzCompass");
		cJSON *max_el = cJSON_GetObjectItemCaseSensitive(pass, "maxEl");
		cJSON *max_utc = cJSON_GetObjectItemCaseSensitive(pass, "maxUTC");
		cJSON *end_az = cJSON_GetObjectItemCaseSensitive(pass, "endAz");
		cJSON *end_az_compass = cJSON_GetObjectItemCaseSensitive(pass, "endAzCompass");
		cJSON *end_utc = cJSON_GetObjectItemCaseSensitive(pass, "endUTC");

		#ifdef PRINT_PACKAGE
		printf("\n");
		printf("startAz: %lf\n", cJSON_GetNumberValue(start_az));
		printf("startAzCompass: %s\n", cJSON_GetStringValue(start_az_compass));
		printf("startUTC: %i\n", (int)cJSON_GetNumberValue(start_utc));
		printf("maxAz: %lf\n", cJSON_GetNumberValue(max_az));
		printf("maxAzCompass: %s\n", cJSON_GetStringValue(max_az_compass));
		printf("maxEL: %lf\n",cJSON_GetNumberValue(max_el));
		printf("maxUTC: %i\n", (int)cJSON_GetNumberValue(max_utc));
		printf("endAz: %lf\n", cJSON_GetNumberValue(end_az));
		printf("endAzCompass: %s\n", cJSON_GetStringValue(end_az_compass));
		printf("endUTC: %i\n", (int)cJSON_GetNumberValue(end_utc));
		#endif

		time_to_convert_start = (int)cJSON_GetNumberValue(start_utc); 
		time_to_convert_end = (int)cJSON_GetNumberValue(end_utc);

		time_converter(&time_to_convert_start, time_format, utc_converted_start, sizeof(utc_converted_start));
		time_converter(&time_to_convert_end, time_format, utc_converted_end, sizeof(utc_converted_end));

		sprintf(formatted_string, "#%i start %s end %s", index++, utc_converted_start, utc_converted_end);

		add_to_spiffs("/spiffs/data.txt", formatted_string);
	}

	cJSON_Delete(json_object);

	ESP_LOGI(time_converter_tag, "Finish parsing data");
}