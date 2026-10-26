#pragma once
#include <Arduino.h>

#include "logica/reset.h"

// Reset pedido pela página. O pedido só é aceito com todos os consoles desligados
// e é executado no loop() depois que a resposta HTTP já saiu.

bool pedirReset(TipoReset tipo, bool confirmado, String &erro);
void executarResetPendente(); // Chamar no loop()
