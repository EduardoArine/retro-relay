// Decisões do OTA e do vigia de WiFi (src/logica/ota.h e conexao.h).
// A regra mais importante: nunca reiniciar com console ligado.
#include <unity.h>

#include "logica/conexao.h"
#include "logica/ota.h"

void setUp() {}
void tearDown() {}

const uint16_t DESLIGADOS = 0;
const uint16_t UM_LIGADO = 1 << 4;

void test_decorrido_atravessa_a_volta_do_millis()
{
    TEST_ASSERT_EQUAL_UINT32(1000, decorrido(5000, 4000));
    // millis() de 32 bits volta a zero a cada ~49 dias
    TEST_ASSERT_EQUAL_UINT32(200, decorrido(100, 0xFFFFFF9C));
}

void test_build_de_desenvolvimento()
{
    TEST_ASSERT_TRUE(ehBuildDesenvolvimento("#{Deploy_Version}#"));
    TEST_ASSERT_FALSE(ehBuildDesenvolvimento("1.6.1"));
}

void test_versao_nova()
{
    TEST_ASSERT_TRUE(haVersaoNova("1.7.0", "1.6.1"));
    TEST_ASSERT_FALSE(haVersaoNova("1.6.1", "1.6.1"));
    TEST_ASSERT_FALSE(haVersaoNova("", "1.6.1")); // Falha ao consultar o GitHub
}

void test_ota_nunca_reinicia_com_console_ligado()
{
    TEST_ASSERT_TRUE(proximaAcaoOta(true, false, false, UM_LIGADO) == AcaoOta::NADA);
    TEST_ASSERT_TRUE(proximaAcaoOta(true, false, true, UM_LIGADO) == AcaoOta::NADA);
    TEST_ASSERT_TRUE(proximaAcaoOta(true, false, false, DESLIGADOS) == AcaoOta::REINICIAR);
}

void test_ota_nunca_instala_com_console_ligado()
{
    TEST_ASSERT_TRUE(proximaAcaoOta(false, true, false, UM_LIGADO) == AcaoOta::NADA);
    TEST_ASSERT_TRUE(proximaAcaoOta(false, true, false, DESLIGADOS) == AcaoOta::INSTALAR);
}

void test_ota_verifica_mesmo_com_console_ligado()
{
    // Só consulta a versão: não reinicia nem desliga nada
    TEST_ASSERT_TRUE(proximaAcaoOta(false, false, true, UM_LIGADO) == AcaoOta::VERIFICAR);
    TEST_ASSERT_TRUE(proximaAcaoOta(false, false, true, DESLIGADOS) == AcaoOta::VERIFICAR);
}

void test_ota_sem_nada_pendente()
{
    TEST_ASSERT_TRUE(proximaAcaoOta(false, false, false, DESLIGADOS) == AcaoOta::NADA);
}

void test_wifi_espera_antes_de_reconectar()
{
    TEST_ASSERT_TRUE(proximaAcaoWiFi(10000, 10000, DESLIGADOS) == AcaoWiFi::NADA);
    TEST_ASSERT_TRUE(proximaAcaoWiFi(RECONECTAR_A_CADA_MS, RECONECTAR_A_CADA_MS, DESLIGADOS) == AcaoWiFi::RECONECTAR);
}

void test_wifi_reinicia_apos_5_minutos_com_consoles_desligados()
{
    TEST_ASSERT_TRUE(proximaAcaoWiFi(REINICIAR_APOS_MS - 1, 0, DESLIGADOS) == AcaoWiFi::NADA);
    TEST_ASSERT_TRUE(proximaAcaoWiFi(REINICIAR_APOS_MS, 0, DESLIGADOS) == AcaoWiFi::REINICIAR);
}

void test_wifi_nunca_reinicia_com_console_ligado()
{
    TEST_ASSERT_TRUE(proximaAcaoWiFi(REINICIAR_APOS_MS * 10, 0, UM_LIGADO) == AcaoWiFi::NADA);
    TEST_ASSERT_TRUE(proximaAcaoWiFi(REINICIAR_APOS_MS * 10, RECONECTAR_A_CADA_MS, UM_LIGADO) == AcaoWiFi::RECONECTAR);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_decorrido_atravessa_a_volta_do_millis);
    RUN_TEST(test_build_de_desenvolvimento);
    RUN_TEST(test_versao_nova);
    RUN_TEST(test_ota_nunca_reinicia_com_console_ligado);
    RUN_TEST(test_ota_nunca_instala_com_console_ligado);
    RUN_TEST(test_ota_verifica_mesmo_com_console_ligado);
    RUN_TEST(test_ota_sem_nada_pendente);
    RUN_TEST(test_wifi_espera_antes_de_reconectar);
    RUN_TEST(test_wifi_reinicia_apos_5_minutos_com_consoles_desligados);
    RUN_TEST(test_wifi_nunca_reinicia_com_console_ligado);
    return UNITY_END();
}
