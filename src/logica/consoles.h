#pragma once
// Validação do cadastro de consoles recebido no POST /consoles, sem dependência
// de hardware (testada em test/test_consoles). Formato igual ao do app:
// [{"canal":0,"nome":"Super Nintendo","imagem":"https://..."}]
#include <ArduinoJson.h>
#include <cstring>
#include <string>

#include "reles.h"

const size_t NOME_MAX = 96;          // Bytes UTF-8: comporta os 32 caracteres da interface mesmo com acentos
const size_t IMAGEM_MAX = 256;
const size_t NVS_STRING_MAX = 4000;  // Limite de string do NVS

// Valida e normaliza: um console por canal, ordenado, só com os campos conhecidos.
// Retorna false e preenche erro se o cadastro for inválido.
inline bool normalizarConsoles(const char *json, std::string &saida, std::string &erro)
{
    JsonDocument entrada;
    if (deserializeJson(entrada, json))
    {
        erro = "JSON inválido";
        return false;
    }
    if (!entrada.is<JsonArray>())
    {
        erro = "Esperado uma lista";
        return false;
    }

    JsonVariantConst porCanal[TOTAL_CANAIS];
    for (JsonVariantConst item : entrada.as<JsonArrayConst>())
    {
        if (!item["canal"].is<int>() || !item["nome"].is<const char *>())
        {
            erro = "Cada item precisa de canal e nome";
            return false;
        }

        int canal = item["canal"];
        const char *nome = item["nome"];
        const char *imagem = item["imagem"] | "";

        if (canal < 0 || canal >= TOTAL_CANAIS)
        {
            erro = "Canal fora do intervalo 0-15";
            return false;
        }
        if (strlen(nome) == 0 || strlen(nome) > NOME_MAX || strlen(imagem) > IMAGEM_MAX)
        {
            erro = "Nome ou imagem com tamanho inválido";
            return false;
        }
        porCanal[canal] = item;
    }

    JsonDocument doc;
    JsonArray lista = doc.to<JsonArray>();
    for (int canal = 0; canal < TOTAL_CANAIS; canal++)
    {
        if (porCanal[canal].isNull())
            continue;
        JsonObject obj = lista.add<JsonObject>();
        obj["canal"] = canal;
        obj["nome"] = porCanal[canal]["nome"];
        obj["imagem"] = porCanal[canal]["imagem"] | "";
    }

    saida.clear();
    serializeJson(doc, saida);
    if (saida.size() > NVS_STRING_MAX)
    {
        erro = "Cadastro grande demais";
        return false;
    }
    return true;
}
