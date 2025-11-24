# 📡 Monitoramento IoT com BitDogLab e MQTT

Este projeto foi desenvolvido como parte da atividade da **Unidade 2 - Comunicação em IoT** da trilha de Software Embarcado do Programa de Residência Tecnológica EMBARCATECH. O objetivo é integrar o hardware educacional **BitDogLab (Raspberry Pi Pico W)** a um broker MQTT para a transmissão de dados de telemetria e controle em tempo real.

## 📋 Funcionalidades Implementadas

O sistema conecta-se via Wi-Fi e estabelece comunicação com um broker MQTT público (`mqtt.iot.natal.br`) para realizar as seguintes tarefas:

* **📶 Conectividade:** Conexão automática via Wi-Fi (WPA2) e reconexão ao broker MQTT em caso de queda.
* **🌡️ Monitoramento de Temperatura:** Leitura do sensor interno de temperatura do RP2040 e publicação periódica (a cada 30 segundos) no tópico MQTT.
* **🕹️ Controle via Joystick:** Leitura dos eixos analógicos do Joystick. A publicação ocorre apenas quando há mudança de estado (Cima, Baixo, Esquerda, Direita), otimizando o tráfego de rede.
* **💾 Mensagens Retidas:** Todas as publicações utilizam a flag *Retain*, garantindo que o último estado permaneça disponível no broker para novos assinantes.
* **💡 Feedback Visual:**
    * **LED Verde:** Pisca rapidamente a cada publicação de mensagem confirmada.
    * **Display OLED:** Exibe o status da conexão (Wi-Fi/MQTT), a última temperatura lida e a direção atual do joystick.

## 🛠️ Hardware Necessário

* Placa de desenvolvimento **BitDogLab** (Raspberry Pi Pico W).
* Display OLED SSD1306 (I2C).
* Joystick Analógico.
* LED Integrado (GPIO 11).

## ⚙️ Configurações e Tópicos MQTT

A solução utiliza a biblioteca `lwIP` para a stack TCP/IP e MQTT.

### Estrutura de Tópicos
Os tópicos foram personalizados conforme o desafio, utilizando a estrutura:
`ha/desafio18/marcelo.junior/`

| Função | Tópico | Tipo de Dado | Comportamento |
| :--- | :--- | :--- | :--- |
| **Temperatura** | `.../temp` | Int (ex: `30`) | Publicação a cada 30s |
| **Joystick** | `.../joy` | String (ex: `cima`) | Publicação por interrupção/mudança |

### Credenciais
Configuradas no arquivo `mqtt_configuracao.c`:
* **Broker:** `mqtt.iot.natal.br`
* **Porta:** `1883`
* **Usuário:** `desafio18`

## 🚀 Como Executar

1.  **Clonar o Repositório:**
    ```bash
    git clone <seu-link-do-repo>
    ```
2.  **Configurar Wi-Fi:**
    Edite as definições no arquivo `Tarefa_Unidade_II_MQTT.c`:
    ```c
    #define SSID_WIFI "SUA_REDE"
    #define SENHA_WIFI "SUA_SENHA"
    ```
3.  **Compilar e Carregar:**
    Utilize a extensão do **Raspberry Pi Pico** no VS Code ou o **CMake** via terminal para compilar o projeto e carregar o arquivo `.uf2` na placa.

## 📺 Estrutura do Código

* `Tarefa_Unidade_II_MQTT.c`: Loop principal, lógica de leitura do Joystick, controle do OLED e gerenciamento de tempo.
* `mqtt_configuracao.c`: Implementação das callbacks do cliente MQTT (conexão, subscrição e publicação).
* `inc/`: Bibliotecas auxiliares para o display SSD1306 e leitura do sensor de temperatura.

## 🎥 Demonstração

https://youtu.be/k3HD-kh3B4A

---
**Desenvolvido por:** Marcelo Junior
