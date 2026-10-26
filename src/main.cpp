#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <esp_task_wdt.h>

#include "ConsoleManager.h"
#include "FirmwareController.h"
#include "ReleManager.h"
#include "WiFiMQTT.h"
#include "web_index.h" // Gerado por scripts/embed_web.py a partir de web/index.html

AsyncWebServer server(80);

void setupWebServer()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (request->header("If-None-Match") == WEB_INDEX_ETAG) {
      request->send(304);
      return;
    }
    // Servida direto da flash, já comprimida
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", WEB_INDEX_GZ, WEB_INDEX_GZ_LEN);
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "no-cache");
    response->addHeader("ETag", WEB_INDEX_ETAG);
    request->send(response); });

  server.on("/estado", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    JsonDocument doc;
    doc["estado"] = estadoAtual;
    doc["modo"] = getModoAtual() == ModoRele::MULTI ? "multi" : "unico";
    doc["versao"] = firmwareVersion;
    String body;
    serializeJson(doc, body);
    request->send(200, "application/json", body); });

  server.on("/consoles", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "application/json", carregarConsoles()); });

  server.on(
      "/consoles", HTTP_POST,
      [](AsyncWebServerRequest *request)
      {
        // O corpo foi acumulado em _tempObject pelo handler abaixo (liberado pela própria lib)
        if (!request->_tempObject) {
          request->send(400, "text/plain", "Corpo ausente ou grande demais");
          return;
        }
        String erro;
        if (salvarConsoles((const char *)request->_tempObject, erro)) {
          request->send(200, "application/json", carregarConsoles());
        } else {
          request->send(400, "text/plain", erro);
        }
      },
      nullptr,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
      {
        if (index == 0) {
          if (total > CONSOLES_MAX_BYTES)
            return;
          request->_tempObject = malloc(total + 1);
        }
        if (!request->_tempObject)
          return;
        memcpy((uint8_t *)request->_tempObject + index, data, len);
        if (index + len == total)
          ((char *)request->_tempObject)[total] = '\0';
      });

  server.on("/ligar", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (request->hasParam("canal")) {
      int canal = request->getParam("canal")->value().toInt();
      alternarRele(canal);
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Faltou o canal");
    } });

  server.on("/modo", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (request->hasParam("valor")) {
      String v = request->getParam("valor")->value();
      setModo(v == "multi" ? ModoRele::MULTI : ModoRele::UNICO);
      request->send(200, "text/plain", "Modo atualizado");
    } else {
      request->send(400, "text/plain", "Faltou o valor");
    } });

  server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "pong"); });

  server.on("/check-ota", HTTP_GET, [](AsyncWebServerRequest *request){
    firmwareForceCheck = true;
    request->send(200, "text/plain", "Forçando verificação OTA...");
  });

  server.begin();
}

void setup()
{
  Serial.begin(115200);

  inicializarReles();
  carregarEstadoSalvo();
  conectarWiFi();

  firmwareForceCheck = true;

  if (!MDNS.begin("retrorelay")) {
      Serial.println("⚠️ Erro ao iniciar mDNS");
  } else {
      MDNS.addService("http", "tcp", 80);
      Serial.println("🌐 mDNS iniciado: http://retrorelay.local");
  }

  esp_task_wdt_init(5, true);
  esp_task_wdt_add(NULL);

  setupWebServer();

  // Só valida um firmware recém-atualizado depois que WiFi e servidor subiram:
  // se ele travar ou não conectar antes disso, o bootloader volta para a versão anterior
  verifyFirmware();
}

void loop()
{
  esp_task_wdt_reset();
  loopOTA();
  delay(100);
}
