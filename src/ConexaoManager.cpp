#include "ConexaoManager.h"
#include "ReleManager.h"
#include <WiFi.h>
#include <esp_system.h>

#include "logica/conexao.h"
#define MOTIVO_VALIDO 0x52524C59                 // Marca de que motivoReinicio foi gravado

// Sobrevive a reinícios por software (não a falta de energia)
static RTC_NOINIT_ATTR uint32_t motivoMarca;
static RTC_NOINIT_ATTR char motivoReinicio[32];

static String ultimoReinicio; // Motivo gravado pelo firmware antes do último reinício
static volatile uint32_t desconexoes = 0;
static volatile uint8_t ultimoMotivoDesconexao = 0;
static uint32_t reconexoesForcadas = 0;
static unsigned long desconectadoDesde = 0; // 0 = conectado
static unsigned long ultimaTentativa = 0;

void iniciarMonitorWiFi()
{
    if (motivoMarca == MOTIVO_VALIDO)
        ultimoReinicio = String(motivoReinicio);
    motivoMarca = 0;

    WiFi.onEvent([](WiFiEvent_t, WiFiEventInfo_t info)
                 {
        uint8_t motivo = info.wifi_sta_disconnected.reason;
        if (motivo == WIFI_REASON_ASSOC_LEAVE)
            return; // Desconexão voluntária (feita por WiFi.reconnect)
        desconexoes++;
        ultimoMotivoDesconexao = motivo; },
                 ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
}

void monitorarWiFi()
{
    unsigned long agora = millis();

    if (WiFi.status() == WL_CONNECTED)
    {
        if (desconectadoDesde != 0)
            Serial.println("✅ WiFi reconectado");
        desconectadoDesde = 0;
        return;
    }

    if (desconectadoDesde == 0)
    {
        Serial.println("⚠️ WiFi desconectado");
        desconectadoDesde = agora;
        ultimaTentativa = agora;
        return;
    }

    switch (proximaAcaoWiFi(decorrido(agora, desconectadoDesde), decorrido(agora, ultimaTentativa), estadoAtual))
    {
    case AcaoWiFi::REINICIAR:
        reiniciar("wifi_sem_conexao");
        break;
    case AcaoWiFi::RECONECTAR:
        Serial.println("🔁 Forçando reconexão do WiFi");
        ultimaTentativa = agora;
        reconexoesForcadas++;
        WiFi.reconnect();
        break;
    case AcaoWiFi::NADA:
        break;
    }
}

String motivoUltimoReinicio()
{
    return ultimoReinicio;
}

void reiniciar(const char *motivo)
{
    strlcpy(motivoReinicio, motivo, sizeof(motivoReinicio));
    motivoMarca = MOTIVO_VALIDO;
    Serial.printf("🔄 Reiniciando: %s\n", motivo);
    delay(100);
    ESP.restart();
}

static const char *motivoReset(esp_reset_reason_t motivo)
{
    switch (motivo)
    {
    case ESP_RST_POWERON:
        return "energia";
    case ESP_RST_SW:
        return "software";
    case ESP_RST_PANIC:
        return "crash";
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:
        return "watchdog";
    case ESP_RST_BROWNOUT:
        return "queda_de_tensao";
    case ESP_RST_EXT:
        return "botao_reset";
    default:
        return "outro";
    }
}

void preencherDiagnostico(JsonDocument &doc)
{
    doc["ligado_ha_s"] = millis() / 1000;
    doc["reset"] = motivoReset(esp_reset_reason());
    doc["ultimo_reinicio"] = ultimoReinicio;

    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["conectado"] = WiFi.status() == WL_CONNECTED;
    wifi["rssi"] = WiFi.RSSI();
    wifi["ip"] = WiFi.localIP().toString();
    wifi["desconexoes"] = desconexoes;
    wifi["ultimo_motivo"] = ultimoMotivoDesconexao
                                ? WiFi.disconnectReasonName((wifi_err_reason_t)ultimoMotivoDesconexao)
                                : "";
    wifi["reconexoes_forcadas"] = reconexoesForcadas;

    JsonObject memoria = doc["memoria"].to<JsonObject>();
    memoria["livre"] = ESP.getFreeHeap();
    memoria["minima"] = ESP.getMinFreeHeap();
    memoria["maior_bloco"] = ESP.getMaxAllocHeap();
}
