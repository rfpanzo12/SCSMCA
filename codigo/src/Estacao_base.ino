// --- [ PARTE PRINCIPAL 1: CREDENCIAIS BLYNK E WIFI ] ---
// Informações para conectar o seu dispositivo (ESP32) à plataforma Blynk.
// Estes dados são únicos para cada projeto no Blynk.
#define BLYNK_TEMPLATE_ID "TMPL2MlXyft4h"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "05XF2HZ58QrlQXTgLeXn-9HmotH4cior"

// Para depuração via Monitor Serial. Comente para economizar memória.
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"

// --- Configurações do Rádio LoRa (Receptor) ---
#define RF_FREQUENCY 915000000 // Frequência deve ser a MESMA do nó transmissor.
#define TX_OUTPUT_POWER 14     // Potência não é usada para receber, mas é definida.
#define LORA_BANDWIDTH 0
#define LORA_SPREADING_FACTOR 7 // Parâmetros devem ser os MESMOS do transmissor.
#define LORA_CODINGRATE 1
#define LORA_PREAMBLE_LENGTH 8
#define LORA_SYMBOL_TIMEOUT 0
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 30

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;

int16_t txNumber;
int16_t rssi, rxSize;
bool lora_idle = true;

// [PONTO-CHAVE] Credenciais da sua rede Wi-Fi para que o gateway possa se conectar à internet.
char ssid[] = "iPhone";
char pass[] = "redeiot12";

BlynkTimer timer;

// ... (Função BLYNK_CONNECTED e outras) ...

// --- [ PARTE PRINCIPAL 2: LÓGICA DE RECEPÇÃO ] ---
// Esta função é chamada a cada 2 segundos pelo 'timer' para garantir que o rádio
// esteja sempre no modo de recepção (ouvindo por pacotes LoRa).
void myTimerEvent()
{
    if (lora_idle)
    {
        lora_idle = false;
        Serial.println("Entrando no modo RX...");
        Radio.Rx(0); // Coloca o rádio para receber continuamente.
    }
    Radio.IrqProcess(); // Processa interrupções do rádio.
}

SSD1306Wire factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

// --- [ PARTE PRINCIPAL 3: SETUP DO GATEWAY ] ---
void setup()
{
    Serial.begin(115200);

    // [PONTO-CHAVE] Conecta ao Wi-Fi e ao servidor Blynk usando as credenciais definidas.
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // [PONTO-CHAVE] Configura o 'timer' para chamar a função 'myTimerEvent' a cada 2 segundos.
    timer.setInterval(2000L, myTimerEvent);

    Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

    // [PONTO-CHAVE] Associa a função 'OnRxDone' ao evento de recepção de dados.
    // Esta é a função mais importante do gateway.
    RadioEvents.RxDone = OnRxDone;
    Radio.Init(&RadioEvents);

    // Configura o rádio para o modo de RECEPÇÃO com os parâmetros LoRa definidos.
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

    VextON();
    delay(100);
    factory_display.init();
    factory_display.clear();
    factory_display.display();
}

// --- [ PARTE PRINCIPAL 4: LOOP PRINCIPAL ] ---
// O loop de um projeto Blynk é muito simples.
// As funções .run() cuidam de toda a comunicação com o servidor e da execução dos timers.
void loop()
{
    Blynk.run();
    timer.run();
}

// --- [ PARTE PRINCIPAL 5: MANIPULAÇÃO DOS DADOS RECEBIDOS ] ---
// ESTA É A FUNÇÃO MAIS IMPORTANTE DO GATEWAY.
// Ela é chamada AUTOMATICAMENTE pela biblioteca do rádio sempre que um pacote LoRa é recebido com sucesso.
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    // 1. Copia os dados recebidos (payload) para o buffer 'rxpacket'.
    memcpy(rxpacket, payload, size);
    rxpacket[size] = '\0'; // Adiciona um finalizador de string.
    Radio.Sleep(); // Coloca o rádio em modo de baixo consumo após receber.
    Serial.printf("Pacote recebido: %s\n", rxpacket);

    // 2. [PONTO-CHAVE] Extrai (faz o "parse") dos dados da string recebida.
    // A string "TX1;55.3;28.7;NAO" é dividida em partes usando o ';' como delimitador.
    String pacote = String(rxpacket);
    String tx = getValue(pacote, ';', 0); // Pega a primeira parte: "TX1"
    String h = getValue(pacote, ';', 1);  // Pega a segunda parte: "55.3"
    String t = getValue(pacote, ';', 2);  // Pega a terceira parte: "28.7"
    String c = getValue(pacote, ';', 3);  // Pega a quarta parte: "NAO"

    // 3. [PONTO-CHAVE] Direciona os dados para os Pinos Virtuais corretos no Blynk
    // com base no ID do transmissor ("TX1" ou "TX2").
    int vN, vH, vT, vC;
    if (tx == "TX1")
    { // Se a mensagem veio do Nó 1
        vN = V0; vH = V1; vT = V2; vC = V3;
    }
    else if (tx == "TX2")
    { // Se a mensagem veio do Nó 2
        vN = V4; vH = V5; vT = V6; vC = V7;
    }

    // 4. Atualiza o display OLED com as informações de status.
    factory_display.clear();
    factory_display.setFont(ArialMT_Plain_10);
    // ... (código para desenhar no display) ...
    factory_display.display();

    // 5. [PONTO-CHAVE] Envia os valores extraídos para a plataforma Blynk.
    // Os widgets no seu dashboard do Blynk serão atualizados com estes valores.
    Blynk.virtualWrite(vN, tx);
    Blynk.virtualWrite(vH, h.toFloat());
    Blynk.virtualWrite(vT, t.toFloat());
    Blynk.virtualWrite(vC, c);

    // 6. Libera a flag para que o rádio possa voltar ao modo de recepção.
    lora_idle = true;
}

// Função auxiliar para extrair valores de uma string dividida por um caractere.
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (data.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return data.substring(strIndex[0], strIndex[1]);
}
