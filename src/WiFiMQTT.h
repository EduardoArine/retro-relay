#pragma once
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>  // versão compatível

#include "ConexaoManager.h"
#include "IdentidadeManager.h"

extern AsyncWebServer server;  // Apenas declara que será usada de fora

void conectarWiFi() {
  DNSServer dns;
  AsyncWiFiManager wm(&server, &dns);

  // Nome do dispositivo na lista do roteador (padrão seria "esp32-XXXXXX")
  WiFi.setHostname(nomeDispositivo());

  // Sem economia de energia: em modo sleep o rádio perde pacotes multicast/broadcast
  // (mDNS e ARP) e o dispositivo some da rede. A placa é alimentada pela tomada.
  WiFi.setSleep(false);

  wm.setConfigPortalTimeout(300);
  bool res = wm.autoConnect(nomePortalConfiguracao(), "admin");

  if (!res) {
    Serial.println("⚠️ Falha ao conectar ao WiFi");
    reiniciar("wifi_nao_conectou_no_boot");
  }

  Serial.printf("✅ Conectado à rede %s, IP: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}
