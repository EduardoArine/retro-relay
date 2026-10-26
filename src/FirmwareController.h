#pragma once

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>
#include <esp_ota_ops.h>

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

// Desliga a validação automática do core Arduino: quem valida é verifyFirmware()
extern "C" bool verifyRollbackLater(){
    return true;
}

// Chamar no fim do setup(), depois que o dispositivo estiver acessível.
// Até lá, um firmware novo fica PENDING_VERIFY e qualquer reinício
// (crash, watchdog ou falha de WiFi) faz o bootloader voltar à versão anterior.
void verifyFirmware()
{
    /* Captura o ponteiro da partição em execução */
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;

    /* Verifica se a partição possui bloco 'otadata' */
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) 
    {
        const char* otaState = ota_state == ESP_OTA_IMG_NEW ? "ESP_OTA_IMG_NEW"
            : ota_state == ESP_OTA_IMG_PENDING_VERIFY ? "ESP_OTA_IMG_PENDING_VERIFY"
            : ota_state == ESP_OTA_IMG_VALID ? "ESP_OTA_IMG_VALID"
            : ota_state == ESP_OTA_IMG_INVALID ? "ESP_OTA_IMG_INVALID"
            : ota_state == ESP_OTA_IMG_ABORTED ? "ESP_OTA_IMG_ABORTED"
            : "ESP_OTA_IMG_UNDEFINED";

        Serial.println("Estado do Firmware: " + String(otaState));

        /* Verifica se a imagem do OTA está pendente de verificação */
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) 
        {
            /* Marca a imagem como válida e tenta cancelar o rollback */
            if (esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) 
            {
                Serial.println("Firmware válido, rollback cancelado com sucesso!");
            } 
            else 
            {
                Serial.println("Firmware inválido, realizando rollback!");
                /* Marca a imagem como inválida e reinicia o device (rollback) */
                esp_ota_mark_app_invalid_rollback_and_reboot();
            }
        }
    }
    else
    {
        Serial.println("Partição OTA não possui registro de dados!");
    }
}

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
