#include "inc/leitura_sensor.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

void inicializar_pinos()
{
    adc_init();                          // Inicializa o ADC
    adc_set_temp_sensor_enabled(true);    // Habilita o sensor de temperatura interno
}

float ler_temperatura()
{
    const int amostras = 10;
    uint32_t soma = 0;

    adc_select_input(4); // Seleciona o canal do sensor interno

    for (int i = 0; i < amostras; i++) {
        sleep_ms(5);
        soma += adc_read();
    }

    float media = soma / (float)amostras;
    const float conversao = 3.3f / (1 << 12); // 3.3V e ADC de 12 bits
    float tensao = media * conversao;

    // Fórmula oficial do datasheet do RP2040
    float temperatura = 27.0f - (tensao - 0.706f) / 0.001721f;

    printf("DEBUG_TEMP: ADC=%.2f | V=%.3f V | TEMP=%.2f C\n", media, tensao, temperatura);

    return temperatura;
}
