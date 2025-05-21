cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)


project(led_control_webserver C CXX ASM)


pico_sdk_init()

add_executable(led_control_webserver 
    principal_iot.c
    lib/ssd1306.c # Biblioteca para o display OLED    
 )


# Adicionando arquivo pio
file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib)
pico_generate_pio_header(led_control_webserver ${CMAKE_CURRENT_LIST_DIR}/lib/ws2818b.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib)

# Configuração do UART/USB
pico_enable_stdio_uart(led_control_webserver 0)
pico_enable_stdio_usb(led_control_webserver 1)

# Add the standard library to the build
target_link_libraries(led_control_webserver
    pico_stdlib                      # Biblioteca padrão do SDK do Raspberry Pi Pico, funções básicas e utilitários
    hardware_gpio                   # Controle dos pinos GPIO para leitura e escrita digital
    hardware_adc                    # Conversor analógico-digital para leitura de sinais analógicos
    pico_cyw43_arch_lwip_threadsafe_background  # Integração do Wi-Fi CYW43 com a pilha TCP/IP lwIP para comunicação de rede
    hardware_pio                    # Suporte ao periférico PIO para criação de interfaces personalizadas
    hardware_clocks                 # Controle e configuração dos clocks internos do microcontrolador
    hardware_uart                   # Comunicação serial assíncrona via UART
    hardware_i2c                   # Comunicação do display
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
