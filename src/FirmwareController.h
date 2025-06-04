#pragma once

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>

extern String firmwareVersion;

String firmwareVersion = "#{Deploy_Version}#";
String latestVersion = "";
String firmwareUrl = "";

bool firmwareForceCheck = false;
unsigned long lastCheckMillis = 0;
const unsigned long intervalMillis = 30UL * 60UL * 1000UL;

void checkAndUpdateFirmware();
void loopOTA();
String getLatestFirmwareVersion();

void loopOTA() {
    unsigned long now = millis();

    if (firmwareForceCheck || (now - lastCheckMillis >= intervalMillis)) {
        firmwareForceCheck = false;
        lastCheckMillis = now;

        Serial.println("🚀 Executando verificação OTA");

        esp_task_wdt_delete(NULL);

        checkAndUpdateFirmware();

        esp_task_wdt_add(NULL);
    }
}


String getLatestFirmwareVersion() {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient https;
    https.begin(client, "https://api.github.com/repos/EduardoArine/retro-relay/releases/latest");
    https.addHeader("User-Agent", "ESP32-Agent");

    int httpCode = https.GET();
    if (httpCode != 200) return "";

    String payload = https.getString();
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) return "";

    latestVersion = doc["tag_name"].as<String>();
    firmwareUrl = doc["assets"][0]["browser_download_url"].as<String>();
    return latestVersion;
}

void checkAndUpdateFirmware() {
    String remoteVersion = getLatestFirmwareVersion();
    if (remoteVersion == "" || remoteVersion == firmwareVersion) {
        Serial.println("Firmware já está atualizado.");
        return;
    }

    Serial.println("Nova versão detectada: " + remoteVersion);
    Serial.println("Iniciando OTA a partir de: " + firmwareUrl);

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.begin(client, firmwareUrl);

    httpUpdate.rebootOnUpdate(false);
    httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    t_httpUpdate_return result = httpUpdate.update(http, firmwareVersion);

    switch (result) {
        case HTTP_UPDATE_FAILED:
            Serial.println("Atualização falhou: " + String(httpUpdate.getLastError()) + " - " + httpUpdate.getLastErrorString());
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("Nenhuma atualização disponível.");
            break;
        case HTTP_UPDATE_OK:
            Serial.println("Atualização concluída com sucesso. Reiniciando...");
            ESP.restart();
            break;
    }
}
