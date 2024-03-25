# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly

# SDKCONFIG_DEFAULTS=${SDKCONFIG_DEFAULTS_UDP}

cmake_minimum_required(VERSION 3.5)

set(ENV{SDKCONFIG_DEFAULTS_UDP} "sdkconfig.defaults_udp")
set(ENV{SDKCONFIG_DEFAULTS_UART} "sdkconfig.defaults_uart")

set(ENV{SDKCONFIG_DEFAULTS} $ENV{SDKCONFIG_DEFAULTS_UDP})
# set(ENV{SDKCONFIG_DEFAULTS} $ENV{SDKCONFIG_DEFAULTS_UART})

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32s3-http-spiffs-proj)