#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include "DHT.h"
#include "LoRaWan_APP.h"
#include "Arduino.h"

// --- [ PARTE PRINCIPAL 1: CONFIGURAÇÕES LORA ] ---
// Parâmetros da comunicação sem fio. Devem ser os mesmos no transmissor e no receptor.
#define RF_FREQUENCY 915000000         // Frequência de 915 MHz.
#define TX_OUTPUT_POWER 5              // Potência de transmissão baixa (5 dBm).
#define LORA_SPREADING_FACTOR 7        // SF7, para transmissões mais rápidas e de menor alcance.
#define LORA_CODINGRATE 1
// ... (outras configurações padrão) ...
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define BUFFER_SIZE 50

// --- Configurações dos pinos dos sensores ---
#define pino_dht 45   // GPIO onde está o DHT11
#define tipo_dht DHT11
#define pino_chuva 46 // GPIO onde está o sensor de chuva

char txpacket[BUFFER_SIZE];
bool lora_idle = true; // Flag de controle para o rádio. 'true' = pronto para enviar.
double txNumber;
static RadioEvents_t RadioEvents;

void OnTxDone(void);
void OnTxTimeout(void);

SSD1306Wire factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);
DHT dht(pino_dht, tipo_dht);

void VextON(void) {
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
}
void VextOFF(void) {
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, HIGH);
}

// --- [ PARTE PRINCIPAL 2: SETUP ] ---
// Prepara o hardware e as configurações iniciais do nó sensor.
void setup() {
    Serial.begin(115200);
    Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

    // [PONTO-CHAVE] Associa as funções de callback aos eventos de transmissão do rádio.
    // O programa saberá o que fazer quando a transmissão terminar.
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;

    // [PONTO-CHAVE] Inicia o rádio e o configura para o modo de TRANSMISSÃO (Tx).
    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

    // Inicia os sensores e periféricos.
    dht.begin();
    VextON();
    delay(100);
    factory_display.init();
    factory_display.clear();
    factory_display.display();
    pinMode(LED, OUTPUT);
    pinMode(pino_chuva, INPUT);
}

// --- [ PARTE PRINCIPAL 3: LOOP PRINCIPAL ] ---
// Lógica que roda continuamente: ler sensores, mostrar no display e transmitir.
void loop() {
    // [PONTO-CHAVE] A transmissão só ocorre se o rádio estiver livre.
    if (lora_idle) {
        delay(4000); // Pausa de 4 segundos entre cada envio.

        // 1. Lê os dados dos sensores.
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        String chuva = (digitalRead(pino_chuva) == HIGH) ? "NAO" : "SIM";

        // 2. Atualiza o display OLED com as leituras atuais.
        factory_display.clear();
        factory_display.setFont(ArialMT_Plain_10);
        // ... (código de desenho dos dados no display)
        factory_display.display();

        // 3. [PONTO-CHAVE] Formata a string de dados para envio.
        // O "TX2" no início identifica que esta mensagem é do Nó Sensor 2.
        // Isso permite que o gateway saiba de onde os dados vieram.
        sprintf(txpacket, "TX2;%.1f;%.1f;%s", h, t, chuva.c_str());

        // 4. Envia o pacote de dados via LoRa e marca o rádio como ocupado.
        Serial.printf("\r\nenviando pacote: \"%s\"\r\n", txpacket);
        Radio.Send((uint8_t *)txpacket, strlen(txpacket));
        lora_idle = false;
    }
    // [PONTO-CHAVE] Processa eventos do rádio. Essencial para o funcionamento das callbacks.
    Radio.IrqProcess();
}

// --- [ PARTE PRINCIPAL 4: FUNÇÕES DE CALLBACK ] ---
// Respondem ao resultado da tentativa de transmissão.

// Chamada automaticamente quando a transmissão é concluída com SUCESSO.
void OnTxDone(void) {
    Serial.println("TX concluído");
    lora_idle = true; // Libera o rádio para um novo envio.
}

// Chamada automaticamente se a transmissão FALHAR por tempo esgotado (timeout).
void OnTxTimeout(void) {
    Radio.Sleep();
    Serial.println("TX timeout");
    lora_idle = true; // Libera o rádio para tentar um novo envio.
}
