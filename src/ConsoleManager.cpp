#include "ConsoleManager.h"
#include <ArduinoJson.h>
#include <Preferences.h>

#define TOTAL_CANAIS 16
#define NOME_MAX 48
#define IMAGEM_MAX 256
#define NVS_STRING_MAX 4000 // Limite de string do NVS

static Preferences consolePrefs;

String carregarConsoles()
{
    consolePrefs.begin("consoles", true);
    String json = consolePrefs.getString("lista", "[]");
    consolePrefs.end();
    return json;
}

bool salvarConsoles(const char *json, String &erro)
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

    // Normaliza: um console por canal, ordenado, só com os campos conhecidos
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

    JsonDocument saida;
    JsonArray lista = saida.to<JsonArray>();
    for (int canal = 0; canal < TOTAL_CANAIS; canal++)
    {
        if (porCanal[canal].isNull())
            continue;
        JsonObject obj = lista.add<JsonObject>();
        obj["canal"] = canal;
        obj["nome"] = porCanal[canal]["nome"];
        obj["imagem"] = porCanal[canal]["imagem"] | "";
    }

    String normalizado;
    serializeJson(saida, normalizado);
    if (normalizado.length() > NVS_STRING_MAX)
    {
        erro = "Cadastro grande demais";
        return false;
    }

    consolePrefs.begin("consoles", false);
    bool ok = consolePrefs.putString("lista", normalizado) == normalizado.length();
    consolePrefs.end();

    if (!ok)
        erro = "Falha ao gravar na memória";
    return ok;
}
