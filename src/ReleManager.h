#pragma once
#include <Arduino.h>
#include "logica/reles.h"

void inicializarReles();
void aplicarEstado();
void alternarRele(int canal);
void setModo(ModoRele modo);
ModoRele getModoAtual();
void salvarEstado();            // novo
void carregarEstadoSalvo();     // novo
void apagarEstadoSalvo();       // Restaurar padrões de fábrica (estado e modo)


extern uint16_t estadoAtual;
