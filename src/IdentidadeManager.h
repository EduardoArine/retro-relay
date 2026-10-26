#pragma once
#include <Arduino.h>

// Nome único do aparelho (retrorelay-a3f2), para vários aparelhos na mesma casa.
// "retrorelay.local" continua respondendo como apelido, para compatibilidade com
// favoritos e versões antigas do app.

void iniciarIdentidade();                // Chamar no início do setup(), antes do WiFi
const char *idDispositivo();             // "a3f2"
const char *nomeDispositivo();           // "retrorelay-a3f2" (roteador e mDNS)
const char *nomePortalConfiguracao();    // "RetroRelay-A3F2" (rede do portal de configuração)
uint8_t grupoDoAparelho();               // 0-99, fixo: define em que etapa da liberação gradual ele recebe
void iniciarMdns(const char *versao);    // Chamar depois de conectar ao WiFi
