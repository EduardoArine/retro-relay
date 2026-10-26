#pragma once
// Nome único de cada aparelho, a partir do MAC (testado em test/test_identidade).
// Os 2 últimos bytes do MAC (4 dígitos hex) bastam para diferenciar aparelhos
// na mesma casa e ficam curtos o suficiente para ler na tela.
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

const size_t ID_TAM = 5;    // "a3f2" + '\0'
const size_t NOME_TAM = 16; // "retrorelay-a3f2" + '\0'

inline void idDoMac(const uint8_t mac[6], char *saida)
{
    snprintf(saida, ID_TAM, "%02x%02x", mac[4], mac[5]);
}

// Nome na rede (hostname do roteador e do mDNS): retrorelay-a3f2
inline void nomeDeRede(const char *id, char *saida)
{
    snprintf(saida, NOME_TAM, "retrorelay-%s", id);
}

// Nome da rede do portal de configuração: RetroRelay-A3F2
inline void nomeDoPortal(const char *id, char *saida)
{
    char maiusculo[ID_TAM];
    for (size_t i = 0; i < ID_TAM; i++)
        maiusculo[i] = (char)toupper((unsigned char)id[i]);
    snprintf(saida, NOME_TAM, "RetroRelay-%s", maiusculo);
}
