#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

#include "logica/assinatura.h"

// Assinatura do firmware baixado pelo OTA. O HTTPUpdate grava o .bin na partição
// livre e já a marca para o próximo boot; antes de reiniciar, a assinatura é
// conferida sobre o que foi gravado. Se for recusada, o boot volta para a versão atual.

// tamanho = bytes do .bin (informado pela API do GitHub)
ResultadoAssinatura conferirFirmwareGravado(size_t tamanho, const uint8_t *assinatura, size_t tamanhoAssinatura);
bool descartarFirmwareGravado();
void registrarAssinatura(ResultadoAssinatura resultado, const String &versao); // Sobrevive ao reinício (NVS)
void preencherAssinatura(JsonObject ota);                                      // Para o /diag
