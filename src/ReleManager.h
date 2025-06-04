#pragma once
#include <Arduino.h>

enum class ModoRele
{
    UNICO, // Apenas um pode estar ligado (exclusivo)
    MULTI  // Vários podem estar ligados (toggle individual)
};

void inicializarReles();
void aplicarEstado();
void alternarRele(int canal);
void setModo(ModoRele modo);
ModoRele getModoAtual();
void salvarEstado();            // novo
void carregarEstadoSalvo();     // novo


extern uint16_t estadoAtual;
