#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include "mqtt_configuracao.h"
#include "inc/leitura_sensor.h"

// --- DEFINIÇÕES DA REDE WIFI  ---
#define SSID_WIFI "PROXXIMA-178171" // Nome da rede wifi
#define SENHA_WIFI "17041991"      // Senha da rede wifi

// --- TÓPICOS MQTT PERSONALIZADOS ---
#define PRIMEIRO_ULTIMO_NOME "marcelo.junior" 
#define TOPICO_TEMP_MQTT "ha/desafio18/" PRIMEIRO_ULTIMO_NOME "/temp"
#define TOPICO_JOYSTICK_MQTT "ha/desafio18/" PRIMEIRO_ULTIMO_NOME "/joy"

// --- DEFINA AQUI OS PINOS DO SEU JOYSTICK ---
#define PINO_JOY 26


// --- DEFINA AQUI OS PINOS DO SEU JOYSTICK ---
#define PINO_JOY_CIMA    2
#define PINO_JOY_BAIXO   3
#define PINO_JOY_ESQ     4
#define PINO_JOY_DIR     5

// --- Pinos GPIO ---
#define PINO_LED 11       // Pino GPIO do LED verde
const uint I2C_SDA = 14;  // Pino SDA do I2C (dados)
const uint I2C_SCL = 15;  // Pino SCL do I2C (clock)

// --- Display OLED (sem alterações) ---
uint8_t buffer_display[ssd1306_buffer_length];
struct render_area area_renderizacao = {
    .start_column = 0, .end_column = ssd1306_width - 1,
    .start_page = 0, .end_page = ssd1306_n_pages - 1};

// Callback MQTT para debug.
void callback_mqtt_recebido(char *topico, char *mensagem) {
    printf("Monitor: Mensagem recebida no topico '%s': '%s'\n", topico, mensagem);
}

// --- FUNÇÃO PARA PISCAR O LED ---
void piscar_led_publicacao() {
    gpio_put(PINO_LED, 1);
    sleep_ms(1000); // Pisca rápido para indicar publicação 
    gpio_put(PINO_LED, 0);
}

// --- FUNÇÃO PRINCIPAL ---
int main() {
    stdio_init_all();

    printf("Monitor: Iniciando Wi-Fi...\n");
    if (cyw43_arch_init()) {
        printf("Monitor: Falha na inicializacao do Wi-Fi.\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();
    printf("Monitor: Conectando ao Wi-Fi %s...\n", SSID_WIFI);
    if (cyw43_arch_wifi_connect_timeout_ms(SSID_WIFI, SENHA_WIFI, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0) {
        printf("Monitor: Falha na conexao Wi-Fi.\n");
        return 1;
    }
    printf("Monitor: Wi-Fi conectado com sucesso!\n");

    // ---- Inicializa LED, Sensor e Joystick ----
    gpio_init(PINO_LED);
    gpio_set_dir(PINO_LED, GPIO_OUT);
    inicializar_pinos(); 

    
    // --- Inicialize aqui os pinos do Joystick como entrada com pull-up ---
    gpio_init(PINO_JOY);
    gpio_set_dir(PINO_JOY, GPIO_IN);

    // --- Inicialização do Display OLED ---
    i2c_init(i2c1, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();
    calculate_render_area_buffer_length(&area_renderizacao);

    // ---- Conexão com MQTT ----
    printf("Monitor: Iniciando conexao MQTT...\n");
    inicializar_mqtt();
    definir_callback_usuario_mqtt(callback_mqtt_recebido);

    // Aguarda conexão MQTT
    uint32_t tempo_inicio = to_ms_since_boot(get_absolute_time());
    while (!mqtt_esta_conectado && (to_ms_since_boot(get_absolute_time()) - tempo_inicio < 20000)) {
        printf("Monitor: Aguardando conexao MQTT...\n");
        sleep_ms(1000);
    }

    if (!mqtt_esta_conectado) {
        printf("Monitor: Falha na conexao MQTT apos timeout.\n");
    } else {
        printf("Monitor: MQTT conectado com sucesso!\n");
    }
    
    char ultima_direcao_joy[15] = "nenhuma";

    printf("Monitor: Entrando no loop principal...\n");
    
    char direcao_atual_joy[15] = "nenhuma";
    // ---- LOOP PRINCIPAL ----
    while (true) {
        // --- LÓGICA DO JOYSTICK ---
        // Verifique o estado dos pinos do joystick (a lógica !gpio_get assume pull-up)
        adc_gpio_init(26);
        uint16_t joy_y, joy_x;
        
        adc_select_input(0);
        joy_y = adc_read();
        adc_select_input(1);
        joy_x = adc_read();
        
        if (joy_y > 3900) {            
            strcpy(direcao_atual_joy, "cima");
        } else if (joy_y < 100) {            
            strcpy(direcao_atual_joy, "baixo");
        } else if (joy_x > 3900) {            
            strcpy(direcao_atual_joy, "direita");
        } else if (joy_x < 100) {            
            strcpy(direcao_atual_joy, "esquerda");
        }


        // Publica apenas se houver mudança de estado
        if (strcmp(direcao_atual_joy, ultima_direcao_joy) != 0 && strcmp(direcao_atual_joy, "nenhuma") != 0) {
            piscar_led_publicacao();
            publicar_mensagem(TOPICO_JOYSTICK_MQTT, direcao_atual_joy);
            strcpy(ultima_direcao_joy, direcao_atual_joy);
        }

        // --- PUBLICAÇÃO PERIÓDICA DA TEMPERATURA ---
        float temperatura_atual = ler_temperatura();
        char temp_str[10];
        
        // Publica a temperatura como um valor inteiro
        sprintf(temp_str, "%d", (int)temperatura_atual);
        
        piscar_led_publicacao();
        publicar_mensagem(TOPICO_TEMP_MQTT, temp_str);

        // --- ATUALIZAÇÃO DO DISPLAY OLED ---
        memset(buffer_display, 0, ssd1306_buffer_length);
        ssd1306_draw_string(buffer_display, 0, 0, "Desafio 18 ");
        
        char status_str[20];
        sprintf(status_str, "MQTT: %s", mqtt_esta_conectado ? "Conectado" : "Desconectado");
        ssd1306_draw_string(buffer_display, 0, 16, status_str);

        char temp_display_str[20];
        sprintf(temp_display_str, "Temp: %s C", temp_str);
        ssd1306_draw_string(buffer_display, 0, 32, temp_display_str);

        char joy_display_str[25];
        sprintf(joy_display_str, "Joy: %s", ultima_direcao_joy);
        ssd1306_draw_string(buffer_display, 0, 48, joy_display_str);
        
        render_on_display(buffer_display, &area_renderizacao);

        // Publicação periódica a cada 30 segundos
        sleep_ms(30000); // Aguarda 30 segundos antes da próxima iteração
    }

    cyw43_arch_deinit();
    return 0;
}