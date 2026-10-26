// Nome único por aparelho (src/logica/identidade.h).
#include <string.h>
#include <unity.h>

#include "logica/identidade.h"

void setUp() {}
void tearDown() {}

void test_id_usa_os_dois_ultimos_bytes_do_mac()
{
    const uint8_t mac[6] = {0x24, 0x6F, 0x28, 0x1B, 0xA3, 0xF2};
    char id[ID_TAM];
    idDoMac(mac, id);
    TEST_ASSERT_EQUAL_STRING("a3f2", id);
}

void test_id_mantem_zeros_a_esquerda()
{
    const uint8_t mac[6] = {0, 0, 0, 0, 0x00, 0x0A};
    char id[ID_TAM];
    idDoMac(mac, id);
    TEST_ASSERT_EQUAL_STRING("000a", id);
}

void test_nome_de_rede_em_minusculas()
{
    char nome[NOME_TAM];
    nomeDeRede("a3f2", nome);
    TEST_ASSERT_EQUAL_STRING("retrorelay-a3f2", nome);
    // Começa com "retrorelay": o app filtra o mDNS por esse prefixo
    TEST_ASSERT_EQUAL_INT(0, strncmp(nome, "retrorelay", 10));
}

void test_nome_do_portal_em_maiusculas()
{
    char nome[NOME_TAM];
    nomeDoPortal("a3f2", nome);
    TEST_ASSERT_EQUAL_STRING("RetroRelay-A3F2", nome);
    nomeDoPortal("0b9c", nome);
    TEST_ASSERT_EQUAL_STRING("RetroRelay-0B9C", nome);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_id_usa_os_dois_ultimos_bytes_do_mac);
    RUN_TEST(test_id_mantem_zeros_a_esquerda);
    RUN_TEST(test_nome_de_rede_em_minusculas);
    RUN_TEST(test_nome_do_portal_em_maiusculas);
    return UNITY_END();
}
