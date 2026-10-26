#pragma once
// Leitura da resposta da API de releases do GitHub (testada em test/test_release
// com respostas reais). /releases/latest devolve um objeto; /releases?per_page=1
// devolve uma lista com um objeto.
#include <ArduinoJson.h>
#include <string.h>
#include <string>

// Arquivos da release: retrorelay-X.Y.Z.bin e retrorelay-X.Y.Z.bin.sig
inline bool terminaCom(const char *texto, const char *sufixo)
{
    size_t t = strlen(texto), s = strlen(sufixo);
    return t >= s && strcmp(texto + t - s, sufixo) == 0;
}
inline bool ehArquivoFirmware(const char *nome) { return terminaCom(nome, ".bin"); }
inline bool ehArquivoAssinatura(const char *nome) { return terminaCom(nome, ".bin.sig"); }

struct InfoRelease
{
    std::string versao;
    std::string notas; // Texto da release: pode trazer "liberacao: 25"
    std::string firmwareUrl;
    size_t firmwareTamanho = 0;
    std::string assinaturaUrl; // "" = release sem assinatura
};

// Retorna false se a resposta não for uma release (erro da API, lista vazia...)
inline bool lerRelease(const char *json, bool lista, InfoRelease &info)
{
    // Só os campos usados: a resposta completa traz autor, contadores, etc.
    // Em filtro, o item [0] de uma lista vale para todos os itens.
    JsonDocument filtroRelease;
    filtroRelease["tag_name"] = true;
    filtroRelease["body"] = true;
    filtroRelease["assets"][0]["name"] = true;
    filtroRelease["assets"][0]["browser_download_url"] = true;
    filtroRelease["assets"][0]["size"] = true;
    JsonDocument filtro;
    if (lista)
        filtro[0] = filtroRelease;
    else
        filtro.set(filtroRelease);

    JsonDocument doc;
    if (deserializeJson(doc, json, DeserializationOption::Filter(filtro)))
        return false;
    JsonObjectConst release = lista ? doc[0].as<JsonObjectConst>() : doc.as<JsonObjectConst>();
    if (release.isNull() || !release["tag_name"].is<const char *>())
        return false;

    info = InfoRelease();
    info.versao = release["tag_name"].as<const char *>();
    info.notas = release["body"] | "";
    for (JsonObjectConst arquivo : release["assets"].as<JsonArrayConst>())
    {
        const char *nome = arquivo["name"] | "";
        const char *url = arquivo["browser_download_url"] | "";
        if (ehArquivoAssinatura(nome))
            info.assinaturaUrl = url;
        else if (ehArquivoFirmware(nome) && info.firmwareUrl.empty())
        {
            info.firmwareUrl = url;
            info.firmwareTamanho = arquivo["size"] | 0;
        }
    }
    return true;
}
