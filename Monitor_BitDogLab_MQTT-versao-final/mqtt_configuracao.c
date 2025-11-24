#include "mqtt_configuracao.h"
#include <stdio.h>
#include <string.h>
#include "lwip/init.h"
#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include <stdbool.h>
#include "pico/stdlib.h"

// --- CREDENCIAIS MQTT DA ATIVIDADE ---
#define MQTT_USUARIO "desafio18"     // Usuário 
#define MQTT_SENHA "desafio18.laica" // Senha 

// Instancia do cliente MQTT
mqtt_client_t *cliente_mqtt_instancia;

// Ponteiro para a função de callback do usuário
static void (*_callback_usuario_mqtt)(char *topico, char *mensagem) = NULL;

static char topico_entrada_atual[100];
bool mqtt_esta_conectado = false;

// Variável global para armazenar o IP resolvido do broker
static ip_addr_t ip_broker_remoto;

// Callback de conexão MQTT (inalterado)
void callback_conexao_mqtt(mqtt_client_t *cliente, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT: Conectado ao servidor!\n");
        mqtt_esta_conectado = true;
    } else {
        printf("MQTT: Erro na conexao: %d\n", status);
        mqtt_esta_conectado = false;
    }
}

// Callbacks de recebimento de mensagens (inalterados)
void callback_publicacao_chegando_mqtt(void *arg, const char *topico, u32_t comprimento_total) {
    printf("MQTT: Publicacao recebida no topico: %s\n", topico);
    strncpy(topico_entrada_atual, topico, sizeof(topico_entrada_atual) - 1);
    topico_entrada_atual[sizeof(topico_entrada_atual) - 1] = '\0';
}

void callback_dados_chegando_mqtt(void *arg, const u8_t *dados, u16_t comprimento, u8_t flags) {
    char mensagem[256];
    strncpy(mensagem, (const char *)dados, comprimento);
    mensagem[comprimento] = '\0';
    printf("MQTT: Dados recebidos: '%s'\n", mensagem);
    if (_callback_usuario_mqtt) {
        _callback_usuario_mqtt(topico_entrada_atual, mensagem);
    }
}

// Callback de conclusão de publicação (inalterado)
void callback_publicacao_concluida_mqtt(void *arg, err_t erro) {
    if (erro == ERR_OK) {
        printf("MQTT: Publicacao concluida com sucesso.\n");
    } else {
        printf("MQTT: Erro na publicacao: %d\n", erro);
    }
}

// Callback de subscrição (inalterado)
void callback_solicitacao_sub_mqtt(void *arg, err_t erro) {
    const char *topico = (const char *)arg;
    if (erro == ERR_OK) {
        printf("MQTT: Subscricao ao topico '%s' bem-sucedida!\n", topico);
    } else {
        printf("MQTT: Falha na subscricao ao topico '%s': %d\n", topico, erro);
    }
}

// --- FUNÇÃO DE CONEXÃO ---
// Esta função é chamada quando o DNS resolve o IP do broker
static void conectar_ao_broker(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
    if (ipaddr != NULL) {
        printf("MQTT_INIT: DNS resolvido. IP de %s: %s\n", hostname, ipaddr_ntoa(ipaddr));
        ip_broker_remoto = *ipaddr;

        // Configura as informações do cliente, incluindo autenticação
        static struct mqtt_connect_client_info_t info_cliente;
        memset(&info_cliente, 0, sizeof(info_cliente));
        info_cliente.client_id = "pico_w_desafio18_aluno"; // ID de cliente
        info_cliente.keep_alive = 60;
        info_cliente.client_user = MQTT_USUARIO;
        info_cliente.client_pass = MQTT_SENHA;

        printf("MQTT_INIT: Tentando conectar ao broker...\n");
        err_t erro = mqtt_client_connect(cliente_mqtt_instancia, &ip_broker_remoto, MQTT_BROKER_PORTA, callback_conexao_mqtt, NULL, &info_cliente);
        
        if (erro != ERR_OK) {
            printf("MQTT_INIT: Erro ao iniciar conexao: %d\n", erro);
        }
    } else {
        printf("MQTT_INIT: ERRO FATAL: Falha ao resolver DNS para %s\n", hostname);
    }
}

// --- FUNÇÃO DE INICIALIZAÇÃO ---
void inicializar_mqtt() {
    cliente_mqtt_instancia = mqtt_client_new();
    if (cliente_mqtt_instancia == NULL) {
        printf("MQTT_INIT: ERRO FATAL: Falha ao criar cliente MQTT!\n");
        return;
    }

    mqtt_set_inpub_callback(cliente_mqtt_instancia, callback_publicacao_chegando_mqtt, callback_dados_chegando_mqtt, NULL);

    printf("MQTT_INIT: Resolvendo DNS para o broker '%s'...\n", NOME_HOST_BROKER_MQTT);
    err_t erro_dns = dns_gethostbyname(NOME_HOST_BROKER_MQTT, &ip_broker_remoto, conectar_ao_broker, NULL);

    if (erro_dns == ERR_OK) {
        // O IP já estava em cache, podemos conectar diretamente
        conectar_ao_broker(NOME_HOST_BROKER_MQTT, &ip_broker_remoto, NULL);
    } else if (erro_dns != ERR_INPROGRESS) {
        printf("MQTT_INIT: Erro ao iniciar consulta DNS: %d\n", erro_dns);
    }
}

// --- FUNÇÃO DE PUBLICAÇÃO---
void publicar_mensagem(const char *topico, const char *mensagem) {
    if (cliente_mqtt_instancia != NULL && mqtt_esta_conectado) {
        // O terceiro '1' ativa a flag de retenção da mensagem, conforme requisito
        mqtt_publish(cliente_mqtt_instancia, topico, mensagem, strlen(mensagem), 1, 0, callback_publicacao_concluida_mqtt, NULL);
        printf("MQTT: Mensagem (retida) enviada: %s -> %s\n", topico, mensagem);
    } else {
        printf("MQTT: Aviso: Cliente nao conectado. Nao foi possivel publicar.\n");
    }
}

// Funções de subscrição e definição de callback 
void definir_callback_usuario_mqtt(void (*callback)(char *topico, char *mensagem)) {
    _callback_usuario_mqtt = callback;
}

void subscrever_topico_mqtt(const char *topico) {
    if (cliente_mqtt_instancia != NULL && mqtt_esta_conectado) {
        err_t erro = mqtt_subscribe(cliente_mqtt_instancia, topico, 0, callback_solicitacao_sub_mqtt, (void *)topico);
        if (erro != ERR_OK) {
            printf("MQTT: Erro ao subscrever ao topico %s: %d\n", topico, erro);
        } else {
            printf("MQTT: Tentando subscrever ao topico: %s\n", topico);
        }
    } else {
        printf("MQTT: Aviso: Cliente nao conectado. Nao foi possivel subscrever.\n");
    }
}