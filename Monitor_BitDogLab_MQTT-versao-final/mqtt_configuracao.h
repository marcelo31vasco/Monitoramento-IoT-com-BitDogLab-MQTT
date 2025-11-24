#ifndef MQTT_CONFIGURACAO_H
#define MQTT_CONFIGURACAO_H

#include "lwip/apps/mqtt.h" //Biblioteca para suporte ao protocolo MQTT
#include <stdbool.h>       //Biblioteca para manipulacao de valores

// --- CONFIGURAÇÕES DO BROKER MQTT ---
#define NOME_HOST_BROKER_MQTT "mqtt.iot.natal.br" // Broker da atividade
#define MQTT_BROKER_PORTA 1883                    // Porta padrão para MQTT sem criptografia 

// --- FUNÇÕES PARA MANIPULAÇÃO DO MQTT ---
void inicializar_mqtt();
// Publica uma mensagem em um tópico MQTT
void publicar_mensagem(const char *topico, const char *mensagem);
// Define um callback para processar mensagens recebidas via MQTT
void definir_callback_usuario_mqtt(void (*callback)(char *topico, char *mensagem));
// Subscreve a um tópico MQTT para receber mensagens
void subscrever_topico_mqtt(const char *topico);

// Variável para verificar o status da conexão MQTT
extern bool mqtt_esta_conectado; // Declarada como externa para ser acessada em outros arquivos

#endif