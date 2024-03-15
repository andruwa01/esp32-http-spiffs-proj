idf_component_register(
    # esp32-uart-python command project configuration 
    # SRCS "udp.c" "wifi.c" "main.c"
    #      "command_handler.c"
    #      "spiffs_storage.c"
    #      "uart.c"
    # INCLUDE_DIRS "."

    # Test udp (wifi-client)
    SRCS "main.c"
         "wifi.c"
         "nvs.c"
         "udp.c"

    INCLUDE_DIRS "."

    # EMBED_TXTFILES certificate.pem
)

# when use spiffs folder to initiate partition
# spiffs_create_partition_image(data_storage ../spiffs_image FLASH_IN_PROJECT)