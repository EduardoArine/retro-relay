#pragma once
// Decisões do OTA, sem dependência de hardware (testadas em test/test_ota_conexao).
#include <stdint.h>
#include <string.h>

#include "tempo.h"

// Build local (gravado pelo cabo): o CI troca "#{Deploy_Version}#" pela versão real
inline bool ehBuildDesenvolvimento(const char *versao)
{
    return versao[0] == '#';
}

// Há versão para instalar: a remota existe e é diferente da atual
inline bool haVersaoNova(const char *remota, const char *atual)
{
    return remota[0] != '\0' && strcmp(remota, atual) != 0;
}

enum class AcaoOta
{
    NADA,
    REINICIAR, // Firmware novo já gravado: reiniciar para usá-lo
    VERIFICAR, // Consultar se há versão nova
    INSTALAR   // Baixar e gravar a versão nova
};

// O que o OTA deve fazer agora. O reinício desliga os relés por alguns segundos
// (pino de enable do 74HC595 em nível alto durante o reset), então reiniciar e
// instalar só acontecem com todos os consoles desligados.
inline AcaoOta proximaAcaoOta(bool reinicioPendente, bool temVersaoPendente, bool horaDeVerificar, uint16_t estadoReles)
{
    bool consolesDesligados = estadoReles == 0;

    if (reinicioPendente)
        return consolesDesligados ? AcaoOta::REINICIAR : AcaoOta::NADA;
    if (horaDeVerificar)
        return AcaoOta::VERIFICAR;
    if (temVersaoPendente && consolesDesligados)
        return AcaoOta::INSTALAR;
    return AcaoOta::NADA;
}
