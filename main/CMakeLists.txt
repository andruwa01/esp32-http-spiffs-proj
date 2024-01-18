idf_component_register(
    SRCS  "main.c" "button.c" "uart.c" "wifi.c" "http_client.c" "satellite_data_parser.c" "time_converter.c" "spiffs_storage.c" 
    INCLUDE_DIRS "." 
    # EMBED_TXTFILES certificate.pem
)

spiffs_create_partition_image(data_storage ../spiffs_image FLASH_IN_PROJECT)