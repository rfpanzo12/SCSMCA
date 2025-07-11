## Como Executar
1. Faça upload dos códigos em `codigo/src/` nos respectivos ESP32.
2. Configure Wi-Fi no código da base.
3. Acesse os dados em [Plataforma Blynk](https://blynk.io/)
   OBS: Será necessário criar uma conta, configurar um projeto e atribuir os pinos virtuais apropriados.

## Compilação
Use a Arduino IDE com as seguintes configurações:
- Placa: Heltec WiFi LoRa 32(V3)
- Frequência LoRa: 915 MHz
- Bibliotecas necessárias:
  - Wire.h: Essa biblioteca permite a comunicação via I2C. Usou-se para comunicação com displays OLED.
  - HT_SSD1306Wire.h: Essa biblioteca é específica para displays OLED com controlador SSD1306, usados com a placa Heltec LoRa.
  - DHT.h: Biblioteca que gerencia sensores da família DHT (DHT11, DHT22, etc.), que medem temperatura e umidade.
  - LoRaWan_APP.h: Biblioteca fornecida por fabricantes como Heltec ou TTGO para gerenciar o LoRaWAN.
  - WiFi.h: Permite ao ESP32 se conectar a redes Wi-Fi.
  - WiFiClient.h: Cria e gerencia conexões cliente TCP/IP sobre Wi-Fi.
  - BlynkSimpleEsp32.h: Biblioteca oficial do Blynk para ESP32. Permite conectar o ESP32 ao Blynk usando Wi-Fi.
  - Arduino.h: É o cabeçalho principal do framework Arduino, e inclui definições básicas como pinMode(), digitalWrite() e delay().

## Pastas
- `src/`: Códigos dos nós sensores e estação base
- `lib/`: Bibliotecas externas, caso necessário