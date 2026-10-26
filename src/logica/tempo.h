#pragma once
#include <stdint.h>

// Tempo entre duas leituras de millis(), correto mesmo quando o contador
// de 32 bits volta a zero (a cada ~49 dias ligado)
inline uint32_t decorrido(uint32_t agora, uint32_t desde)
{
    return agora - desde;
}
