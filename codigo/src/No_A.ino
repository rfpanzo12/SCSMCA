#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include "DHT.h"
#include "LoRaWan_APP.h"
#include "Arduino.h"

// --- [ PARTE PRINCIPAL 1: CONFIGURAÇÕES LORA ] ---
// Define os parâmetros essenciais da comunicação sem fio.
// A combinação de Frequência, Potência e Spreading Factor determina o alcance e a velocidade da transmissão.
#define RF_FREQUENCY 915000000         // 915 MHz, padrão para as Américas.
#define TX_OUTPUT_POWER 5              // Potência baixa (5 dBm), ideal para testes e economia de bateria.
#define LORA_SPREADING_FACTOR 7        // SF7: mais rápido, menor alcance. Aumentar para 12 aumenta o alcance.
#define LORA_CODINGRATE 1
#define LORA_PREAMBLE_LENGTH 8
// ... (outras configurações padrão) ...
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define BUFFER_SIZE 50

#define pino_dht 45
#define tipo_dht DHT11
#define pino_chuva 46

char txpacket[BUFFER_SIZE];
bool lora_idle = true; // Flag de controle: 'true' significa que o rádio está livre para enviar.
double txNumber;
static RadioEvents_t RadioEvents;

void OnTxDone(void);
void OnTxTimeout(void);

SSD1306Wire factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);
DHT dht(pino_dht, tipo_dht);

void VextON(void)
{
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
}
void VextOFF(void)
{
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, HIGH);
}

// --- [ PARTE PRINCIPAL 2: SETUP ] ---
// Prepara todo o hardware e software para a execução.
void setup()
{
    Serial.begin(115200);
    Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

    // [PONTO-CHAVE] Associa as funções de callback aos eventos do rádio.
    // O programa saberá o que fazer quando uma transmissão terminar (com sucesso ou falha).
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;

    // [PONTO-CHAVE] Inicializa e configura o rádio LoRa com os parâmetros definidos acima.
    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
    
    // Inicia os periféricos.
    dht.begin();
    VextON();
    delay(100);
    factory_display.init();
    factory_display.clear();
    factory_display.display();
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    pinMode(pino_chuva, INPUT);
}

// --- [ PARTE PRINCIPAL 3: LOOP PRINCIPAL ] ---
// Contém a lógica principal que é executada repetidamente.
void loop()
{
    // [PONTO-CHAVE] Condição que controla o fluxo de envio.
    // O código dentro deste 'if' só executa se o rádio estiver livre.
    if (lora_idle)
    {
        delay(2500); // Pausa entre os envios.

        // 1. Lê os dados dos sensores.
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        String chuva = (digitalRead(pino_chuva) == HIGH) ? "NAO" : "SIM";

        // 2. Mostra os dados no display OLED.
        factory_display.clear();
        factory_display.setFont(ArialMT_Plain_10);
        factory_display.setTextAlignment(TEXT_ALIGN_CENTER);
        factory_display.drawString(64, 0, "MONITORAMENTO DE");
        factory_display.drawString(64, 10, "CAMPO AGRÍCOLA");
        // ... (código de desenho dos dados)
        factory_display.display();

        // 3. [PONTO-CHAVE] Formata os dados em uma única string para transmissão.
        // O formato "TX1;umidade;temperatura;chuva" é definido aqui.
        sprintf(txpacket, "TX1;%.1f;%.1f;%s", h, t, chuva.c_str());

        // 4. Inicia o envio do pacote e bloqueia novos envios.
        Serial.printf("\r\nenviando pacote: \"%s\"\r\n", txpacket);
        Radio.Send((uint8_t *)txpacket, strlen(txpacket));
        lora_idle = false; // Rádio agora está ocupado.
    }

    // [PONTO-CHAVE] Processa eventos pendentes do rádio.
    // É ESSENCIAL chamar esta função constantemente para que as callbacks (OnTxDone/Timeout) funcionem.
    Radio.IrqProcess();
}

// --- [ PARTE PRINCIPAL 4: FUNÇÕES DE CALLBACK ] ---
// Lógica assíncrona que responde aos resultados da transmissão.

// Chamada automaticamente pela biblioteca quando a transmissão é CONCLUÍDA.
void OnTxDone(void)
{
    Serial.println("TX concluído");
    lora_idle = true; // Libera o rádio para o próximo envio no loop.
}

// Chamada automaticamente pela biblioteca se a transmissão FALHAR por timeout.
void OnTxTimeout(void)
{
    Radio.Sleep(); // Coloca o rádio em modo de baixo consumo.
    Serial.println("TX timeout");
    lora_idle = true; // Libera o rádio para tentar um novo envio.
}
