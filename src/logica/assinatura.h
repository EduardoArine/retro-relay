#pragma once
// Assinatura do firmware: ECDSA P-256 sobre o SHA-256 do .bin, gerada no CI com
// `openssl dgst -sha256 -sign`. Usa só o mbedTLS (o mesmo 2.28 do ESP32), então
// é testada no computador em test/test_assinatura com assinaturas reais.
#include <mbedtls/pk.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum class ResultadoAssinatura
{
    VALIDA,
    INVALIDA,
    SEM_ASSINATURA // Release sem arquivo .sig
};

// true se a assinatura (DER) do hash confere com a chave pública (PEM com '\0' no fim)
inline bool assinaturaConfere(const uint8_t hash[32], const uint8_t *assinatura, size_t tamanho, const char *chavePem)
{
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    bool ok = mbedtls_pk_parse_public_key(&pk, (const unsigned char *)chavePem, strlen(chavePem) + 1) == 0 &&
              mbedtls_pk_can_do(&pk, MBEDTLS_PK_ECDSA) &&
              mbedtls_pk_verify(&pk, MBEDTLS_MD_SHA256, hash, 32, assinatura, tamanho) == 0;
    mbedtls_pk_free(&pk);
    return ok;
}

// Confere contra todas as chaves aceitas: a principal (usada pelo CI) e a reserva
// (guardada fora do CI, para trocar de chave se a principal vazar ou se perder)
inline ResultadoAssinatura verificarAssinatura(const uint8_t hash[32], const uint8_t *assinatura, size_t tamanho,
                                               const char *const chaves[], size_t quantidadeChaves)
{
    if (assinatura == nullptr || tamanho == 0)
        return ResultadoAssinatura::SEM_ASSINATURA;
    for (size_t i = 0; i < quantidadeChaves; i++)
        if (assinaturaConfere(hash, assinatura, tamanho, chaves[i]))
            return ResultadoAssinatura::VALIDA;
    return ResultadoAssinatura::INVALIDA;
}

// Fase de validação (exigir = false): instala mesmo sem assinatura válida e só
// registra o resultado. Exigindo, só firmware assinado é instalado.
inline bool aceitarFirmware(ResultadoAssinatura resultado, bool exigirAssinatura)
{
    return resultado == ResultadoAssinatura::VALIDA || !exigirAssinatura;
}

inline const char *nomeResultado(ResultadoAssinatura resultado)
{
    switch (resultado)
    {
    case ResultadoAssinatura::VALIDA:
        return "valida";
    case ResultadoAssinatura::INVALIDA:
        return "invalida";
    default:
        return "sem_assinatura";
    }
}

// Arquivos da release: retrorelay-X.Y.Z.bin e retrorelay-X.Y.Z.bin.sig
inline bool terminaCom(const char *texto, const char *sufixo)
{
    size_t t = strlen(texto), s = strlen(sufixo);
    return t >= s && strcmp(texto + t - s, sufixo) == 0;
}
inline bool ehArquivoFirmware(const char *nome) { return terminaCom(nome, ".bin"); }
inline bool ehArquivoAssinatura(const char *nome) { return terminaCom(nome, ".bin.sig"); }
