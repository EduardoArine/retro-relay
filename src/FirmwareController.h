#pragma once

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>

extern String firmwareVersion;

String firmwareVersion = "#{Deploy_Version}#";
String latestVersion = "";
String firmwareUrl = "";

bool forceCheck = false;
unsigned long lastCheckMillis = 0;
const unsigned long intervalMillis = 30UL * 60UL * 1000UL;

void loopOTA() {
    unsigned long now = millis();

    if (forceCheck || (now - lastCheckMillis >= intervalMillis)) {
        forceCheck = false;
        lastCheckMillis = now;

        Serial.println("🚀 Executando verificação OTA");
        checkAndUpdateFirmware();
    }
}


String getLatestFirmwareVersion() {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient https;
    https.begin(client, "https://api.github.com/repos/<owner>/<repo>/releases/latest");
    https.addHeader("User-Agent", "ESP32-Agent");

    int httpCode = https.GET();
    if (httpCode != 200) return "";

    String payload = https.getString();
    DynamicJsonDocument doc(4096);
    DeserializationError err = deserializeJson(doc, payload);
    if (err) return "";

    latestVersion = doc["tag_name"].as<String>();
    firmwareUrl = doc["assets"][0]["browser_download_url"].as<String>();
    return latestVersion;
}

void checkAndUpdateFirmware() {
    String remoteVersion = getLatestFirmwareVersion();
    if (remoteVersion == "" || remoteVersion == firmwareVersion) {
        Serial.println("ℹ️ Firmware já está atualizado.");
        return;
    }

    Serial.println("🆕 Nova versão detectada: " + remoteVersion);
    Serial.println("⬇️ Iniciando OTA a partir de: " + firmwareUrl);

    WiFiClientSecure client;
    client.setInsecure();

    t_httpUpdate_return result = httpUpdate.update(client, firmwareUrl);

    switch (result) {
        case HTTP_UPDATE_FAILED:
            Serial.println("Atualização falhou: " + String(httpUpdate.getLastError()) + " - " + httpUpdate.getLastErrorString());
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("Nenhuma atualização disponível.");
            break;
        case HTTP_UPDATE_OK:
            Serial.println("Atualização concluída com sucesso. Reiniciando...");
            break;
    }
}
