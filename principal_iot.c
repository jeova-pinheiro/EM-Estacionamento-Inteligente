// Simulador de Estacionamento Inteligente com BitDogLab e Pico W
// Projeto: Atividade 1 - Comunicação em IoT
// Autor: Jeová Pinheiro

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

#include "generated/ws2812.pio.h"

// Remover antes de subir ao GIT
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"

#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7

bool led_buffer[NUM_PIXELS] = {0};
uint8_t led_r = 0, led_g = 0, led_b = 200;

static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void atualizar_matriz()
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (led_buffer[i])
            put_pixel(urgb_u32(led_r, led_g, led_b));
        else
            put_pixel(0);
    }
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
    for (int i = 0; i < NUM_PIXELS; i++) // Itera por todas as 25 vagas
    {
        char vaga_str[16];
        sprintf(vaga_str, "GET /vaga%d", i); // Cria a requisição para a vaga i
        if (strstr(request, vaga_str))       // Verifica se a vaga foi requisitada
        {
            // Alterna o estado da vaga (reserva ou cancelamento)
            led_buffer[i] = !led_buffer[i];
            atualizar_matriz(); // Atualiza a matriz de LEDs
            sprintf(resposta_json,
                    "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n\r\n"
                    "{\"vaga\": %d, \"ocupada\": %s}",
                    i, led_buffer[i] ? "true" : "false");
            return;
        }
    }

    // Caso a requisição seja para o status
    if (strstr(request, "GET /status"))
    {
        strcpy(resposta_json,
               "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n\r\n{");
        strcat(resposta_json, "\"vagas\":[");

        // Concatena o status das vagas (ocupadas ou não) para todas as 25 vagas
        for (int i = 0; i < NUM_PIXELS; i++)
        {
            char item[8];
            sprintf(item, "%s%d", (i == 0 ? "" : ","), led_buffer[i]);
            strcat(resposta_json, item);
        }
        strcat(resposta_json, "]}");
        return;
    }

    // Caso a requisição seja inválida
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
    processar_requisicao(request, resposta);

    tcp_write(tpcb, resposta, strlen(resposta), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    free(request);
    pbuf_free(p);
    return ERR_OK;
}

#define BOTAO_A_PIN 5
#define BOTAO_B_PIN 6

#include "pico/bootrom.h"

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

    gpio_init(BOTAO_A_PIN);
    gpio_set_dir(BOTAO_A_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_A_PIN);

    gpio_init(BOTAO_B_PIN);
    gpio_set_dir(BOTAO_B_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_B_PIN);

    gpio_set_irq_enabled_with_callback(BOTAO_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_ambos_callback);
    gpio_set_irq_enabled(BOTAO_B_PIN, GPIO_IRQ_EDGE_FALL, true);

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    atualizar_matriz();

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
