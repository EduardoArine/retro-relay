#pragma once
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>  // versão compatível

extern AsyncWebServer server;  // Apenas declara que será usada de fora

void conectarWiFi() {
  DNSServer dns;
  AsyncWiFiManager wm(&server, &dns);

  wm.setConfigPortalTimeout(300);
  bool res = wm.autoConnect("RetroRelay-Setup", "admin");

  if (!res) {
    Serial.println("⚠️ Falha ao conectar ao WiFi");
    ESP.restart();
  }

  Serial.printf("✅ Conectado à rede %s, IP: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}
