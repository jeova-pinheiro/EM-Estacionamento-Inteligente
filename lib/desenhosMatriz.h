#include "matrizled.c"
#include <time.h>
#include <stdlib.h>

#include <stdbool.h>
#include <stdint.h>

// Intensidade padrão para as animações
#define intensidade 1

// Estado atual de cada LED: false = apagado, true = aceso
extern bool estado_leds[25];

void printNum(void)
{
    npWrite();
    npClear();
}

void apagarTodosLeds()
{
    npClear(); // Zera todos os LEDs no buffer
    npWrite(); // Envia os dados para os LEDs (desligando fisicamente)
}

// Recuperar index x e y na matriz 5x5
void get_xy_from_index(int index, uint8_t *x, uint8_t *y)
{
    if (index < 1 || index > 25)
    {
        *x = -1;
        *y = -1;
        return;
    }

    index -= 1;     // Ajusta para começar do 0
    *y = index / 5; // linha (y)
    *x = index % 5; // coluna (x)
}

// Alterna o estado do LED na posição index (1–25)
void alternaLEDNaMatriz(uint8_t index)
{
    // npClear();

    if (index < 1 || index > 25)
        return;

    uint8_t x, y;
    get_xy_from_index(index, &x, &y);

    int pos = getIndex(x, y);
    int idx = index - 1; // índice para o array de estado

    if (estado_leds[idx])
    {
        // Estava aceso → apagar
        npSetLED(pos, 0, 0, 0);
        estado_leds[idx] = false;
    }
    else
    {
        // Estava apagado → acender
        npSetLED(pos, 0, 0, 80); // Azul
        estado_leds[idx] = true;
    }
    npWrite();
}
