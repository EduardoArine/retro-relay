// Canais beta/estável e liberação gradual (src/logica/canais.h).
#include <unity.h>

#include "logica/canais.h"

void setUp() {}
void tearDown() {}

void test_canal_salvo_desconhecido_vira_estavel()
{
    TEST_ASSERT_TRUE(canalSalvo(0) == CanalOta::ESTAVEL);
    TEST_ASSERT_TRUE(canalSalvo(1) == CanalOta::BETA);
    TEST_ASSERT_TRUE(canalSalvo(255) == CanalOta::ESTAVEL);
}

void test_grupo_e_fixo_e_fica_entre_0_e_99()
{
    const uint8_t mac[6] = {0x24, 0x6F, 0x28, 0x1B, 0xA3, 0xF2};
    uint8_t grupo = grupoLiberacao(mac);
    TEST_ASSERT_LESS_THAN(100, grupo);
    TEST_ASSERT_EQUAL_UINT8(grupo, grupoLiberacao(mac));
}

void test_grupos_se_espalham_entre_aparelhos()
{
    // 1000 MACs seguidos: cada faixa de 10% deve ter perto de 100 aparelhos
    int faixas[10] = {0};
    for (int i = 0; i < 1000; i++)
    {
        const uint8_t mac[6] = {0x24, 0x6F, 0x28, (uint8_t)(i >> 16), (uint8_t)(i >> 8), (uint8_t)i};
        faixas[grupoLiberacao(mac) / 10]++;
    }
    for (int f = 0; f < 10; f++)
    {
        TEST_ASSERT_GREATER_THAN(60, faixas[f]);
        TEST_ASSERT_LESS_THAN(140, faixas[f]);
    }
}

void test_ler_liberacao_das_notas()
{
    TEST_ASSERT_EQUAL_INT(25, lerLiberacao("liberacao: 25"));
    TEST_ASSERT_EQUAL_INT(50, lerLiberacao("Correções de Wi-Fi\nliberacao: 50%\n"));
    TEST_ASSERT_EQUAL_INT(10, lerLiberacao("liberacao:10"));
    TEST_ASSERT_EQUAL_INT(100, lerLiberacao("liberacao: 250"));
}

void test_sem_liberacao_nas_notas_vale_para_todos()
{
    TEST_ASSERT_EQUAL_INT(100, lerLiberacao(""));
    TEST_ASSERT_EQUAL_INT(100, lerLiberacao(nullptr));
    TEST_ASSERT_EQUAL_INT(100, lerLiberacao("Correções de Wi-Fi"));
}

void test_liberacao_ilegivel_nao_libera_para_ninguem()
{
    TEST_ASSERT_EQUAL_INT(0, lerLiberacao("liberacao: metade"));
    TEST_ASSERT_EQUAL_INT(0, lerLiberacao("liberacao: -5"));
}

void test_estavel_respeita_a_porcentagem()
{
    TEST_ASSERT_TRUE(liberadaParaAparelho(CanalOta::ESTAVEL, 25, 24));
    TEST_ASSERT_FALSE(liberadaParaAparelho(CanalOta::ESTAVEL, 25, 25));
    TEST_ASSERT_FALSE(liberadaParaAparelho(CanalOta::ESTAVEL, 0, 0));
    TEST_ASSERT_TRUE(liberadaParaAparelho(CanalOta::ESTAVEL, 100, 99));
}

void test_beta_recebe_sempre()
{
    TEST_ASSERT_TRUE(liberadaParaAparelho(CanalOta::BETA, 0, 99));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_canal_salvo_desconhecido_vira_estavel);
    RUN_TEST(test_grupo_e_fixo_e_fica_entre_0_e_99);
    RUN_TEST(test_grupos_se_espalham_entre_aparelhos);
    RUN_TEST(test_ler_liberacao_das_notas);
    RUN_TEST(test_sem_liberacao_nas_notas_vale_para_todos);
    RUN_TEST(test_liberacao_ilegivel_nao_libera_para_ninguem);
    RUN_TEST(test_estavel_respeita_a_porcentagem);
    RUN_TEST(test_beta_recebe_sempre);
    return UNITY_END();
}
