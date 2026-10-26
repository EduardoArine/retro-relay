#pragma once
// Canais de atualização e liberação gradual (testados em test/test_canais).
//
// - Beta: recebe toda versão nova assim que o CI publica (seus aparelhos, testes).
// - Estável: só recebe versões promovidas (workflow "Promover versão"), e só se
//   o aparelho estiver dentro da porcentagem liberada ("liberacao: 25" nas notas).
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum class CanalOta
{
    ESTAVEL,
    BETA
};

inline CanalOta canalSalvo(uint8_t valor)
{
    return valor == (uint8_t)CanalOta::BETA ? CanalOta::BETA : CanalOta::ESTAVEL;
}

inline const char *nomeCanal(CanalOta canal)
{
    return canal == CanalOta::BETA ? "beta" : "estavel";
}

// Grupo fixo do aparelho na liberação gradual (0-99), a partir do MAC (FNV-1a)
inline uint8_t grupoLiberacao(const uint8_t mac[6])
{
    uint32_t h = 2166136261u;
    for (int i = 0; i < 6; i++)
    {
        h ^= mac[i];
        h *= 16777619u;
    }
    return (uint8_t)(h % 100);
}

// Porcentagem liberada, lida das notas da release: "liberacao: 25" (ou "25%").
// Sem a linha, vale para todos (100). Valor ilegível não libera para ninguém (0):
// erro de digitação não deve mandar a versão para todos os aparelhos.
inline int lerLiberacao(const char *notas)
{
    const char *chave = strstr(notas ? notas : "", "liberacao:");
    if (chave == nullptr)
        return 100;

    const char *p = chave + strlen("liberacao:");
    while (*p == ' ')
        p++;
    if (*p < '0' || *p > '9')
        return 0;

    long valor = strtol(p, nullptr, 10);
    return valor > 100 ? 100 : (int)valor;
}

// O aparelho deve receber esta versão? Beta recebe sempre; estável, se estiver no grupo liberado
inline bool liberadaParaAparelho(CanalOta canal, int liberacao, uint8_t grupo)
{
    return canal == CanalOta::BETA || grupo < liberacao;
}
