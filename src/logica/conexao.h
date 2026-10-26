#pragma once
// Decisões do vigia de WiFi, sem dependência de hardware (testadas em test/test_ota_conexao).
#include <stdint.h>

#include "tempo.h"

const uint32_t RECONECTAR_A_CADA_MS = 30UL * 1000UL;    // Sem WiFi: força reconexão a cada 30 s
const uint32_t REINICIAR_APOS_MS = 5UL * 60UL * 1000UL; // Sem WiFi por 5 min: reinicia

enum class AcaoWiFi
{
    NADA,
    RECONECTAR,
    REINICIAR
};

// O que fazer com o WiFi desconectado. Reiniciar desliga os relés por alguns
// segundos, então com algum console ligado só tenta reconectar.
inline AcaoWiFi proximaAcaoWiFi(uint32_t desconectadoHaMs, uint32_t desdeTentativaMs, uint16_t estadoReles)
{
    if (desconectadoHaMs >= REINICIAR_APOS_MS && estadoReles == 0)
        return AcaoWiFi::REINICIAR;
    if (desdeTentativaMs >= RECONECTAR_A_CADA_MS)
        return AcaoWiFi::RECONECTAR;
    return AcaoWiFi::NADA;
}
