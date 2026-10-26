// Assinatura do firmware (src/logica/assinatura.h), com assinaturas reais
// geradas pelo OpenSSL (mesmo comando do CI) e uma chave só de teste.
#include <mbedtls/sha256.h>
#include <string.h>
#include <unity.h>

#include "logica/assinatura.h"

void setUp() {}
void tearDown() {}

// Chave só de teste (a privada foi descartada) e assinatura de DADOS feita com
// `openssl dgst -sha256 -sign teste.pem -out dados.sig dados.bin`
static const char *CHAVE_TESTE =
    "-----BEGIN PUBLIC KEY-----\n"
    "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE3nwoNcdQAjlgrEebxcRSmMfJ8tu+\n"
    "mCEfE3+AzN4mnvUxKZpQ8HDj3bpsXcZyFZKxQcOeKLv1a8EAsPEcp4GwbA==\n"
    "-----END PUBLIC KEY-----\n";
static const char *OUTRA_CHAVE =
    "-----BEGIN PUBLIC KEY-----\n"
    "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEeMM4GakQJ0ehm9EwokCWudlzKeGn\n"
    "oAw3hzyxD7g5NKwN9CbA/9z5az/R2oyjfvrtjA21mDV1bjSp6Z7vcOPy5g==\n"
    "-----END PUBLIC KEY-----\n";
static const char *DADOS = "retro-relay firmware de teste";
static const uint8_t ASSINATURA[] = {
    0x30, 0x45, 0x02, 0x21, 0x00, 0xb3, 0x3a, 0x76, 0x70, 0xb2, 0xdc, 0x4a, 0x34, 0x2c, 0x47, 0x83,
    0xbc, 0x95, 0xba, 0x3c, 0x4e, 0x4d, 0x13, 0x3e, 0xa0, 0x01, 0x58, 0xdc, 0x85, 0xd1, 0xf8, 0x5f,
    0xc1, 0xe3, 0x3a, 0xd7, 0xd1, 0x02, 0x20, 0x6b, 0x0e, 0xbc, 0xdc, 0x12, 0x85, 0xb1, 0xc8, 0x86,
    0xcc, 0xe9, 0xb6, 0xb5, 0xa2, 0x83, 0x50, 0xd5, 0x08, 0xeb, 0xb5, 0xef, 0xc8, 0x0b, 0x63, 0xa9,
    0xa7, 0x6e, 0xbc, 0x6c, 0xce, 0x21, 0x8c};

static void sha256(const char *texto, uint8_t hash[32])
{
    mbedtls_sha256_ret((const unsigned char *)texto, strlen(texto), hash, 0);
}

void test_assinatura_valida()
{
    uint8_t hash[32];
    sha256(DADOS, hash);
    TEST_ASSERT_TRUE(assinaturaConfere(hash, ASSINATURA, sizeof(ASSINATURA), CHAVE_TESTE));
}

void test_firmware_alterado_e_recusado()
{
    uint8_t hash[32];
    sha256("retro-relay firmware de teste!", hash);
    TEST_ASSERT_FALSE(assinaturaConfere(hash, ASSINATURA, sizeof(ASSINATURA), CHAVE_TESTE));
}

void test_assinatura_de_outra_chave_e_recusada()
{
    uint8_t hash[32];
    sha256(DADOS, hash);
    TEST_ASSERT_FALSE(assinaturaConfere(hash, ASSINATURA, sizeof(ASSINATURA), OUTRA_CHAVE));
}

void test_assinatura_corrompida_e_recusada()
{
    uint8_t hash[32];
    sha256(DADOS, hash);
    uint8_t corrompida[sizeof(ASSINATURA)];
    memcpy(corrompida, ASSINATURA, sizeof(ASSINATURA));
    corrompida[20] ^= 0x01;
    TEST_ASSERT_FALSE(assinaturaConfere(hash, corrompida, sizeof(corrompida), CHAVE_TESTE));
    TEST_ASSERT_FALSE(assinaturaConfere(hash, ASSINATURA, 10, CHAVE_TESTE));
}

void test_aceita_qualquer_chave_da_lista()
{
    uint8_t hash[32];
    sha256(DADOS, hash);
    const char *principalErrada[] = {OUTRA_CHAVE, CHAVE_TESTE}; // vale pela reserva
    TEST_ASSERT_TRUE(verificarAssinatura(hash, ASSINATURA, sizeof(ASSINATURA), principalErrada, 2) == ResultadoAssinatura::VALIDA);
    const char *nenhuma[] = {OUTRA_CHAVE};
    TEST_ASSERT_TRUE(verificarAssinatura(hash, ASSINATURA, sizeof(ASSINATURA), nenhuma, 1) == ResultadoAssinatura::INVALIDA);
}

void test_release_sem_assinatura()
{
    uint8_t hash[32];
    sha256(DADOS, hash);
    const char *chaves[] = {CHAVE_TESTE};
    TEST_ASSERT_TRUE(verificarAssinatura(hash, nullptr, 0, chaves, 1) == ResultadoAssinatura::SEM_ASSINATURA);
}

void test_fase_de_validacao_e_fase_obrigatoria()
{
    TEST_ASSERT_TRUE(aceitarFirmware(ResultadoAssinatura::VALIDA, true));
    TEST_ASSERT_FALSE(aceitarFirmware(ResultadoAssinatura::INVALIDA, true));
    TEST_ASSERT_FALSE(aceitarFirmware(ResultadoAssinatura::SEM_ASSINATURA, true));
    TEST_ASSERT_TRUE(aceitarFirmware(ResultadoAssinatura::INVALIDA, false));
    TEST_ASSERT_TRUE(aceitarFirmware(ResultadoAssinatura::SEM_ASSINATURA, false));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_assinatura_valida);
    RUN_TEST(test_firmware_alterado_e_recusado);
    RUN_TEST(test_assinatura_de_outra_chave_e_recusada);
    RUN_TEST(test_assinatura_corrompida_e_recusada);
    RUN_TEST(test_aceita_qualquer_chave_da_lista);
    RUN_TEST(test_release_sem_assinatura);
    RUN_TEST(test_fase_de_validacao_e_fase_obrigatoria);
    return UNITY_END();
}
