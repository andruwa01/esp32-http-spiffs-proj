#include "main.h"

static const char *time_converter_tag = "satellite_data_parser";
static const char time_format[] = "%d.%m.%Y %H:%M";

static int pass_number = 0;

void get_info_values_write_to_spiffs(char* spiffs_file_path, int sat_id, char* sat_name, int transactions_count, int passes_count){
	char info_values[256];
	sprintf(info_values, "sat_id: %i\nsat_name: %s\ntransactions_count: %i\npasses_count: %i\n", sat_id, sat_name, transactions_count, passes_count);

	#ifdef SPIFFS_USE_FUNCTIONALITY
		add_line_to_spiffs(spiffs_file_path, info_values);
	#else	
		ESP_LOGW(time_converter_tag, "You don't use spiffs!");
	#endif
}

void get_max_values_write_to_spiffs(char *spiffs_file_path, double max_az_value, char *max_az_compass_value, double max_el_value, int max_utc){
	char max_values[128];
	char max_utc_converted[32];

	time_t time_to_convert = (int)max_utc;
	time_converter(&time_to_convert, (char*)time_format, max_utc_converted, sizeof(max_utc_converted));

	sprintf(max_values, "maxAz: %lf\nmaxAzCompass: %s\nmaxEl: %lf\nmaxUTC: %s\n", max_az_value, max_az_compass_value, max_el_value, max_utc_converted);

	#ifdef SPIFFS_USE_FUNCTIONALITY
		add_line_to_spiffs(spiffs_file_path, max_values);
	#else	
		ESP_LOGW(time_converter_tag, "You don't use spiffs!");
	#endif
}

void get_az_compass_values_write_to_spiffs(char *spiffs_file_path ,char *start_az_compass_value, char *end_az_compass_value){
	char az_compass_values[128];
	sprintf(az_compass_values, "startAzCompass: %s\nendAzCompass: %s\n", start_az_compass_value, end_az_compass_value);

	#ifdef SPIFFS_USE_FUNCTIONALITY
		add_line_to_spiffs(spiffs_file_path, az_compass_values);
	#else
		ESP_LOGW(time_converter_tag, "You don't use spiffs!");
	#endif
}

void get_az_values_write_to_spiffs(char *spiffs_file_path, double start_az_value, double end_az_value){
	char az_values[128];
	sprintf(az_values, "startAz: %lf\nendAz: %lf\n", start_az_value, end_az_value);

	#ifdef SPIFFS_USE_FUNCTIONALITY
		add_line_to_spiffs(spiffs_file_path, az_values);
	#else
		ESP_LOGW(time_converter_tag, "You don't use spiffs!");
	#endif
}

void calculate_time_write_to_spiffs(int passes_counter, char *spiffs_file_path, int start_utc, int end_utc){
	char utc_converted_start[32];
	char time_human_readable[128];
	char utc_converted_end[32];

	time_t time_to_convert_start = (int)start_utc;
	time_t time_to_convert_end = (int)end_utc;

	time_converter(&time_to_convert_start, time_format, utc_converted_start, sizeof(utc_converted_start));
	time_converter(&time_to_convert_end, time_format, utc_converted_end, sizeof(utc_converted_end));

	// algorithm of indexing -> to count the passes and printing them into files in right way
	int local_pass_number = 0;

	if(++pass_number == passes_counter){
		local_pass_number = pass_number;
		pass_number = 0;
	} else {
		local_pass_number = pass_number;
	}

	sprintf(time_human_readable, "#%i start %s end %s\n", local_pass_number, utc_converted_start, utc_converted_end);
	#ifdef SPIFFS_USE_FUNCTIONALITY
		add_line_to_spiffs(spiffs_file_path, time_human_readable);
	#else
		ESP_LOGW(time_converter_tag, "You don't use spiffs!");
	#endif
}

void json_parse_and_write_data_from_http_response_to_spiffs(char *spiffs_file_path, char* http_data_response){
	ESP_LOGW(time_converter_tag, "start parsing data");

	cJSON *json_object = cJSON_Parse(http_data_response);

	if(json_object == NULL){
		const char* error_ptr = cJSON_GetErrorPtr();
		if(error_ptr != NULL){
			ESP_LOGE(time_converter_tag, "error_ptr on symbol  %s", error_ptr);
		}
		cJSON_Delete(json_object);
	}

	cJSON *info_elements 		= cJSON_GetObjectItemCaseSensitive(json_object, "info");
	cJSON *sat_id 				= cJSON_GetObjectItemCaseSensitive(info_elements, "satid");
	cJSON *sat_name 			= cJSON_GetObjectItemCaseSensitive(info_elements, "satname");
	cJSON *transactions_count   = cJSON_GetObjectItemCaseSensitive(info_elements, "transactionscount");
	cJSON *passes_count 		= cJSON_GetObjectItemCaseSensitive(info_elements, "passescount");

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


	get_info_values_write_to_spiffs(
		(char*)spiffs_file_path,
		(int)cJSON_GetNumberValue(sat_id),
		cJSON_GetStringValue(sat_name),
		(int)cJSON_GetNumberValue(transactions_count),
		(int)cJSON_GetNumberValue(passes_count)
	);

	cJSON *pass = NULL;
	cJSON *passes = cJSON_GetObjectItemCaseSensitive(json_object, "passes");
	cJSON_ArrayForEach(pass, passes){
		cJSON *start_az 		= cJSON_GetObjectItemCaseSensitive(pass, "startAz"); 
		cJSON *start_az_compass = cJSON_GetObjectItemCaseSensitive(pass, "startAzCompass");
		cJSON *start_utc 		= cJSON_GetObjectItemCaseSensitive(pass, "startUTC");
		cJSON *max_az 			= cJSON_GetObjectItemCaseSensitive(pass, "maxAz");
		cJSON *max_az_compass 	= cJSON_GetObjectItemCaseSensitive(pass, "maxAzCompass");
		cJSON *max_el 			= cJSON_GetObjectItemCaseSensitive(pass, "maxEl");
		cJSON *max_utc 			= cJSON_GetObjectItemCaseSensitive(pass, "maxUTC");
		cJSON *end_az 			= cJSON_GetObjectItemCaseSensitive(pass, "endAz");
		cJSON *end_az_compass 	= cJSON_GetObjectItemCaseSensitive(pass, "endAzCompass");
		cJSON *end_utc 			= cJSON_GetObjectItemCaseSensitive(pass, "endUTC");

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

		calculate_time_write_to_spiffs(
			(int)cJSON_GetNumberValue(passes_count),
			(char*)spiffs_file_path,
			(int)cJSON_GetNumberValue(start_utc), 
			(int)cJSON_GetNumberValue(end_utc)
		);

		get_az_values_write_to_spiffs(
			(char*)spiffs_file_path,
			cJSON_GetNumberValue(start_az), 
			cJSON_GetNumberValue(end_az)
		);

		get_az_compass_values_write_to_spiffs(
			(char*)spiffs_file_path,
			cJSON_GetStringValue(start_az_compass), 
			cJSON_GetStringValue(end_az_compass)
		);

		get_max_values_write_to_spiffs(
			(char*)spiffs_file_path,
			cJSON_GetNumberValue(max_az),
			cJSON_GetStringValue(max_az_compass), 
			cJSON_GetNumberValue(max_el), 
			(int)cJSON_GetNumberValue(max_utc)
		);
	}

	cJSON_Delete(json_object);
	ESP_LOGW(time_converter_tag, "finish parsing data");
}