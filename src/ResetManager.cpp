#include "ResetManager.h"
#include <WiFi.h>
#include <esp_wifi.h>

#include "ConexaoManager.h"
#include "ConsoleManager.h"
#include "ReleManager.h"
#include "logica/tempo.h"

#define ESPERA_RESPOSTA_MS 500 // Tempo para a resposta HTTP chegar ao navegador

static volatile TipoReset pendente = TipoReset::NENHUM;
static unsigned long pedidoEm = 0;

bool pedirReset(TipoReset tipo, bool confirmado, String &erro)
{
    const char *recusa = validarReset(confirmado, estadoAtual);
    if (recusa)
    {
        erro = recusa;
        return false;
    }
    pedidoEm = millis();
    pendente = tipo;
    return true;
}

void executarResetPendente()
{
    if (pendente == TipoReset::NENHUM || decorrido(millis(), pedidoEm) < ESPERA_RESPOSTA_MS)
        return;

    TipoReset tipo = pendente;
    pendente = TipoReset::NENHUM;

    // Alguém ligou um console depois do pedido: não reinicia
    if (validarReset(true, estadoAtual))
    {
        Serial.println("⚠️ Reset cancelado: há console ligado");
        return;
    }

    if (tipo == TipoReset::FABRICA)
    {
        apagarConsoles();
        apagarEstadoSalvo();
    }

    // Apaga da flash as credenciais e demais configurações de WiFi
    WiFi.disconnect(false, false);
    esp_wifi_restore();

    reiniciar(tipo == TipoReset::FABRICA ? "restaurado_fabrica" : "wifi_esquecido");
}
