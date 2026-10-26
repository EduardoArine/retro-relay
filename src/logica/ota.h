#pragma once
// Decisões do OTA, sem dependência de hardware (testadas em test/test_ota_conexao).
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tempo.h"

// Build local (gravado pelo cabo): o CI troca "#{Deploy_Version}#" pela versão real
inline bool ehBuildDesenvolvimento(const char *versao)
{
    return versao[0] == '#';
}

// Compara versões "X.Y.Z" numericamente (1.10.0 > 1.9.0); o que vier depois
// do terceiro número (ex.: "-beta.1") é ignorado. Retorna <0, 0 ou >0.
inline int compararVersoes(const char *a, const char *b)
{
    for (int parte = 0; parte < 3; parte++)
    {
        char *fimA, *fimB;
        long na = strtol(a, &fimA, 10), nb = strtol(b, &fimB, 10);
        if (na != nb)
            return na < nb ? -1 : 1;
        a = *fimA == '.' ? fimA + 1 : fimA;
        b = *fimB == '.' ? fimB + 1 : fimB;
    }
    return 0;
}

// Há versão para instalar: a remota existe e é mais nova que a atual. Nunca volta
// para uma versão anterior (ex.: ao trocar do canal beta para o estável).
inline bool haVersaoNova(const char *remota, const char *atual)
{
    return remota[0] != '\0' && compararVersoes(remota, atual) > 0;
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
