// Regras dos relés (src/logica/reles.h).
#include <unity.h>

#include "logica/reles.h"

void setUp() {}
void tearDown() {}

void test_unico_liga_so_o_canal_tocado()
{
    TEST_ASSERT_EQUAL_HEX16(1 << 3, alternarCanal(0, 3, ModoRele::UNICO));
    TEST_ASSERT_EQUAL_HEX16(1 << 5, alternarCanal(1 << 3, 5, ModoRele::UNICO));
}

void test_unico_tocar_no_ligado_desliga_tudo()
{
    TEST_ASSERT_EQUAL_HEX16(0, alternarCanal(1 << 3, 3, ModoRele::UNICO));
}

void test_multi_alterna_so_o_canal_tocado()
{
    uint16_t estado = alternarCanal(0, 0, ModoRele::MULTI);
    estado = alternarCanal(estado, 15, ModoRele::MULTI);
    TEST_ASSERT_EQUAL_HEX16(0x8001, estado);
    TEST_ASSERT_EQUAL_HEX16(0x8000, alternarCanal(estado, 0, ModoRele::MULTI));
}

void test_canal_invalido_nao_muda_nada()
{
    TEST_ASSERT_EQUAL_HEX16(0x0005, alternarCanal(0x0005, -1, ModoRele::MULTI));
    TEST_ASSERT_EQUAL_HEX16(0x0005, alternarCanal(0x0005, 16, ModoRele::UNICO));
    TEST_ASSERT_EQUAL_HEX16(0x0005, alternarCanal(0x0005, 1000, ModoRele::MULTI));
}

void test_modo_unico_mantem_o_primeiro_ativo()
{
    TEST_ASSERT_EQUAL_HEX16(0x0004, manterPrimeiroAtivo(0x0034));
    TEST_ASSERT_EQUAL_HEX16(0x8000, manterPrimeiroAtivo(0x8000));
    TEST_ASSERT_EQUAL_HEX16(0x0001, manterPrimeiroAtivo(0xFFFF));
    TEST_ASSERT_EQUAL_HEX16(0, manterPrimeiroAtivo(0));
}

void test_modo_salvo_corrompido_vira_unico()
{
    TEST_ASSERT_TRUE(modoSalvo(0) == ModoRele::UNICO);
    TEST_ASSERT_TRUE(modoSalvo(1) == ModoRele::MULTI);
    TEST_ASSERT_TRUE(modoSalvo(7) == ModoRele::UNICO);
    TEST_ASSERT_TRUE(modoSalvo(255) == ModoRele::UNICO);
}

void test_estado_restaurado_respeita_o_modo()
{
    // Gravado em 32 bits na NVS: bits acima de 15 são ignorados
    TEST_ASSERT_EQUAL_HEX16(0x0003, estadoRestaurado(0x30003, ModoRele::MULTI));
    TEST_ASSERT_EQUAL_HEX16(0x0001, estadoRestaurado(0x0003, ModoRele::UNICO));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_unico_liga_so_o_canal_tocado);
    RUN_TEST(test_unico_tocar_no_ligado_desliga_tudo);
    RUN_TEST(test_multi_alterna_so_o_canal_tocado);
    RUN_TEST(test_canal_invalido_nao_muda_nada);
    RUN_TEST(test_modo_unico_mantem_o_primeiro_ativo);
    RUN_TEST(test_modo_salvo_corrompido_vira_unico);
    RUN_TEST(test_estado_restaurado_respeita_o_modo);
    return UNITY_END();
}
