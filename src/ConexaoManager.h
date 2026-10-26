#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

// Mantém o WiFi de pé e registra dados para diagnosticar quando o dispositivo "some".
//
// O auto-reconnect do core só tenta de novo para alguns motivos de desconexão
// (ex.: AUTH_FAIL fica de fora) e o dispositivo podia ficar desconectado até
// alguém reiniciá-lo. monitorarWiFi() força a reconexão e, em último caso, reinicia.

void iniciarMonitorWiFi();                   // Chamar no setup(), depois de conectar
void monitorarWiFi();                        // Chamar no loop()
void reiniciar(const char *motivo);          // Reinicia guardando o motivo para o /diag
void preencherDiagnostico(JsonDocument &doc);
