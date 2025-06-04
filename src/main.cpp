#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <esp_task_wdt.h>

#include "FirmwareController.h"
#include "ReleManager.h"
#include "WiFiMQTT.h"

// Test Versioning


AsyncWebServer server(80);

const char *html = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="UTF-8"><title>Painel de Relés</title>
<style>
body { font-family: sans-serif; text-align: center; background: #f2f2f2; }
h2 { margin: 20px; } .grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 10px; max-width: 600px; margin: auto; }
button { padding: 20px; font-size: 16px; border-radius: 8px; background: #d9d9d9; cursor: pointer; border: none; }
button.ativo { background: #4CAF50; color: white; }
select { margin: 20px auto; padding: 10px; font-size: 16px; }
</style></head><body>
<h2>Painel de Controle</h2>
<label for='modo'>Modo:</label>
<select id="modo" onchange="trocarModo(this.value)">
  <option value="unico">Somente 1 Ligado</option>
  <option value="multi">Vários (toggle)</option>
</select>
<div class="grid" id="grid"></div>
<script>
const grid = document.getElementById('grid');
const nomes = Array.from({ length: 16 }, (_, i) => `Canal ${String(i+1).padStart(2,'0')}`);

function trocarModo(valor) {
  fetch(`/modo?valor=${valor}`);
}

function gerar() {
  for (let i = 0; i < 16; i++) {
    const b = document.createElement('button');
    b.textContent = nomes[i];
    b.onclick = () => {
      fetch(`/ligar?canal=${i}`)
        .then(() => {
          if (document.getElementById("modo").value === "unico") {
            document.querySelectorAll('button').forEach(x => x.classList.remove('ativo'));
            b.classList.add('ativo');
          } else {
            b.classList.toggle('ativo');
          }
        });
    };
    grid.appendChild(b);
  }
}
gerar();
</script>
</body></html>
)rawliteral";

void setupWebServer()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", html); });

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
    forceCheck = true;
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

  if (!MDNS.begin("retrorelay")) {
      Serial.println("⚠️ Erro ao iniciar mDNS");
  } else {
      MDNS.addService("http", "tcp", 80);
      Serial.println("🌐 mDNS iniciado: http://retrorelay.local");
  }

  esp_task_wdt_init(5, true);
  esp_task_wdt_add(NULL);

  setupWebServer();
}

void loop()
{
  esp_task_wdt_reset();
  loopOTA();
  delay(100);
}
