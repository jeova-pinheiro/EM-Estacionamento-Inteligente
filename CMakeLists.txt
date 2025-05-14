cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(led_control_webserver C CXX ASM)

pico_sdk_init()

add_executable(led_control_webserver principal_iot.c)

# Adicionando arquivo pio
file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(led_control_webserver ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

pico_set_program_name(led_control_webserver "led_control_webserver")
pico_set_program_version(led_control_webserver "0.1")

# Configuração do UART/USB
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

# Add the standard library to the build
target_link_libraries(led_control_webserver
        pico_stdlib
        hardware_gpio
        hardware_adc
        pico_cyw43_arch_lwip_threadsafe_background
        hardware_pio
        hardware_clocks
        hardware_uart  
)

# Add the standard include files to the build
target_include_directories(led_control_webserver PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PICO_SDK_PATH}/lib/lwip/src/include
    ${PICO_SDK_PATH}/lib/lwip/src/include/arch
    ${PICO_SDK_PATH}/lib/lwip/src/include/lwip
)

target_sources(led_control_webserver PRIVATE
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/httpd.c
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/fs.c
)

pico_add_extra_outputs(led_control_webserver)
