# Sistema de Comunicação Sem Fio para Monitoramento de um Campo Agrícola

## Descrição
Este projeto tem como objetivo desenvolver uma rede de sensores sem fio, baseada em módulos ESP32 com rádio LoRa, para monitoramento em tempo real de parâmetros ambientais (temperatura, umidade e chuva) em áreas agrícolas. Dois nós sensores coletarão dados locais e um nó concentrador (estação base) centralizará as informações, retransmitindo-as para a nuvem, viabilizando visualização remota e alertas.

## Metodologia

**Arquitetura**
 - Nós Sensores (Nó A e Nó B): Cada um equipado com ESP32 LoRa, DHT11 e sensor de chuva FC-37.
 - Estação Base (Nó C): ESP32 LoRa + módulo Wi-Fi para uplink.
 
 **Estação Base e Uplink**
 - Recebe pacotes em LoRa, decodifica.
 - Envia via TCP (Blynk protocolo

## Componentes de Hardware e Software
- ESP32 LoRa Heltec V3
- DHT11
- Sensor de Chuva FC-37
- Arduino IDE
- Plataforma Blynk

## Estrutura do Repositório
- [Relatório Final](./relatorio/relatorio_final.pdf)
- [Código Fonte](./codigo/src/)
- [Slides da Apresentação](./apresentacao/slides_apresentacao.pdf)
