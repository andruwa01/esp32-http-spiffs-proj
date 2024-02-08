idf_component_register(
    # Main compile configuration 
    # SRCS  "main.c" "spiffs_storage.c" "nvs_flash.c" "wifi.c" "http_client.c" "satellite_data_parser.c" "time_converter.c"  
    # INCLUDE_DIRS "." 

    # New compile configuration
    SRCS "command_handler.c"
         "main.c" 
         "spiffs_storage.c" 
         "nvs.c" "wifi.c" 
         "http_client.c" 
         "button.c" 
         "freertos_handler.c" 
         "uart.c" 
         "satellite_data_parser.c" 
         "time_converter.c"
         "command_handler.c"
    INCLUDE_DIRS "."

    # Test compile configuration
    # SRCS "main.c" "button.c" "uart.c"

    # EMBED_TXTFILES certificate.pem
)

# when use spiffs folder to initiate partition
# spiffs_create_partition_image(data_storage ../spiffs_image FLASH_IN_PROJECT)