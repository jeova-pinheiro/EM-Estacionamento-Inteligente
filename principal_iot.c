/**
 * Simulador de Estacionamento Inteligente com BitDogLab e Pico W
 * Projeto: Atividade 1 - Comunicação em IoT - Parte 2
 * Autor: Jeová Pinheiro
 * */

// Incluindo bibliotecas necessáriass
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"

#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

#include "desenhosMatriz.h"

#include "pico/bootrom.h"

// Pino PIO para matriz WS2812B
#define MATRIZ_WS2812B 7 // GPIO7 para matriz de LEDs WS2812B 5x5

// Remover antes de subir ao GIT
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"

// Define constantes para botões A e B
#define BOTAO_A_PIN 5
#define BOTAO_B_PIN 6

// === Definições dos pinos ===
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO_OLED 0x3C

// Objeto para controlar o display OLED SSD1306
ssd1306_t ssd;

// bool led_buffer[NUM_PIXELS] = {0};
bool estado_leds[25] = {false};

// Indice da matriz de leds
uint8_t x = 0, y = 0;

// Inicializa pinos para o display OLED
void init_i2c_display(ssd1306_t *ssd)
{
    // Inicializa a comunicação I2C
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa o display OLED
    ssd1306_init(ssd, 128, 64, false, ENDERECO_OLED, I2C_PORT);
    ssd1306_config(ssd);
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
}

void mostrar_vagas_no_display(ssd1306_t *ssd, bool *estado_leds, int total_vagas)
{
    ssd1306_fill(ssd, false); // Limpa o display

    int ocupadas = 0;
    for (int i = 0; i < total_vagas; i++)
    {
        if (estado_leds[i])
            ocupadas++;
    }

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Nº Vagas: %d", total_vagas);
    ssd1306_draw_string(ssd, buffer, 0, 5);

    snprintf(buffer, sizeof(buffer), "Ocupadas: %d", ocupadas);
    ssd1306_draw_string(ssd, buffer, 0, 20);

    ssd1306_send_data(ssd); // Atualiza o display
}

// TCP callbacks
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

// Alterna reserva/cancelamento da vaga correspondente
void processar_requisicao(char *request, char *resposta_json)
{
    // Separando string da vaga do request
    char *inicio = strstr(request, "GET /vaga");
    if (inicio)
    {
        int vaga = 0;

        // Tenta extrair o número após "/vaga"
        if (sscanf(inicio, "GET /vaga%d", &vaga) == 1 && vaga >= 1 && vaga <= 25)
        {
            alternaLEDNaMatriz(vaga);
            mostrar_vagas_no_display(&ssd, estado_leds, 25);

            printf("Vaga %d %s\n", vaga, estado_leds[vaga - 1] ? "ocupada" : "liberada");

            sprintf(resposta_json,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/json\r\n"
                    "Access-Control-Allow-Origin: *\r\n\r\n"
                    "{\"vaga\": %d, \"ocupada\": %s}",
                    vaga, estado_leds[vaga - 1] ? "true" : "false");
            return;
        }
    }

    if (strstr(request, "GET /status"))
    {
        strcpy(resposta_json,
               "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n\r\n{");
        strcat(resposta_json, "\"vagas\":[");

        for (int i = 0; i < 25; i++)
        {
            char item[8];
            sprintf(item, "%s%d", (i == 0 ? "" : ","), estado_leds[i]);
            strcat(resposta_json, item);
        }

        strcat(resposta_json, "]}");
        return;
    }

    strcpy(resposta_json,
           "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nAccess-Control-Allow-Origin: *\r\n\r\nRequisição não reconhecida.");
}

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (!p)
    {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    char *request = (char *)malloc(p->len + 1);
    memcpy(request, p->payload, p->len);
    request[p->len] = '\0';
    printf("Requisição: %s\n", request);

    char resposta[1024];

    printf("\nResposta: %s\n", resposta);

    processar_requisicao(request, resposta);

    tcp_write(tpcb, resposta, strlen(resposta), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    tcp_close(tpcb); // Encerra a conexão

    free(request);
    pbuf_free(p);
    return ERR_OK;
}

void gpio_ambos_callback(uint gpio, uint32_t events)
{
    if (gpio == BOTAO_A_PIN)
    {
        if (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP)
            printf("Conectado! IP: %s\n", ipaddr_ntoa(&netif_default->ip_addr));
        else
            printf("Wi-Fi desconectado.\n");
    }
    else if (gpio == BOTAO_B_PIN)
    {
        printf("Entrando no modo BOOTSEL...\n");
        reset_usb_boot(0, 0);
    }
}

int main()
{
    stdio_init_all();

    init_i2c_display(&ssd);

    mostrar_vagas_no_display(&ssd, estado_leds, 25);

    npInit(MATRIZ_WS2812B); // Inicializar matriz

    gpio_init(BOTAO_A_PIN);
    gpio_set_dir(BOTAO_A_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_A_PIN);

    gpio_init(BOTAO_B_PIN);
    gpio_set_dir(BOTAO_B_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_B_PIN);

    gpio_set_irq_enabled_with_callback(BOTAO_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_ambos_callback);
    gpio_set_irq_enabled(BOTAO_B_PIN, GPIO_IRQ_EDGE_FALL, true);

    apagarTodosLeds(); // Apagar leds da matriz

    if (cyw43_arch_init())
        return -1;
    cyw43_arch_enable_sta_mode();

    printf("Conectando ao Wi-Fi...\n");
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("Falha na conexão Wi-Fi. Tentando novamente...\n");
        sleep_ms(2000);
    }

    printf("Conectado! IP: %s\n", ipaddr_ntoa(&netif_default->ip_addr));

    struct tcp_pcb *server = tcp_new();
    if (!server || tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK)
        return -1;
    server = tcp_listen(server);
    tcp_accept(server, tcp_server_accept);

    while (1)
    {
        cyw43_arch_poll();
        sleep_ms(100);
    }

    cyw43_arch_deinit();
    return 0;
}
