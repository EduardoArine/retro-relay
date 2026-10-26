#pragma once
// Regras dos relés, sem dependência de hardware (testadas em test/test_reles).
#include <stdint.h>

const int TOTAL_CANAIS = 16;

enum class ModoRele
{
    UNICO, // Apenas um pode estar ligado (exclusivo)
    MULTI  // Vários podem estar ligados (toggle individual)
};

// Ao entrar no modo único: mantém ligado só o primeiro canal ativo (mesma regra do app)
inline uint16_t manterPrimeiroAtivo(uint16_t estado)
{
    return estado & (uint16_t)(-estado);
}

// Novo estado ao tocar em um canal. Canal fora de 0-15 não muda nada.
inline uint16_t alternarCanal(uint16_t estado, int canal, ModoRele modo)
{
    if (canal < 0 || canal >= TOTAL_CANAIS)
        return estado;

    uint16_t bit = (uint16_t)(1u << canal);
    if (modo == ModoRele::MULTI)
        return estado ^ bit;

    // Modo único: liga só este canal; tocar no que já está ligado desliga tudo
    return (estado & bit) ? 0 : bit;
}

// Modo salvo na memória: qualquer valor desconhecido (memória corrompida) vira único
inline ModoRele modoSalvo(uint8_t valor)
{
    return valor == (uint8_t)ModoRele::MULTI ? ModoRele::MULTI : ModoRele::UNICO;
}

// Estado restaurado no boot, coerente com o modo
inline uint16_t estadoRestaurado(uint32_t salvo, ModoRele modo)
{
    uint16_t estado = (uint16_t)salvo;
    return modo == ModoRele::UNICO ? manterPrimeiroAtivo(estado) : estado;
}

// Estado dos relés ao ligar a placa. Se a energia acabou e voltou (ou houve queda
// de tensão), tudo começa desligado: consoles religando sozinhos assustam.
// Nos outros reinícios (firmware, travamento, watchdog) o estado é restaurado.
inline uint16_t estadoAoLigar(uint32_t salvo, ModoRele modo, bool energiaVoltou)
{
    return energiaVoltou ? 0 : estadoRestaurado(salvo, modo);
}
