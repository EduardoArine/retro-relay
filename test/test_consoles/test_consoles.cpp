// Validação do cadastro de consoles (src/logica/consoles.h).
#include <string>
#include <unity.h>

#include "logica/consoles.h"

void setUp() {}
void tearDown() {}

static std::string saida, erro;

static bool normalizar(const char *json)
{
    saida.clear();
    erro.clear();
    return normalizarConsoles(json, saida, erro);
}

void test_lista_vazia_e_valida()
{
    TEST_ASSERT_TRUE(normalizar("[]"));
    TEST_ASSERT_EQUAL_STRING("[]", saida.c_str());
}

void test_ordena_por_canal_e_descarta_campos_extras()
{
    TEST_ASSERT_TRUE(normalizar(
        "[{\"canal\":5,\"nome\":\"Saturn\",\"imagem\":\"https://x/s.png\",\"extra\":1},"
        "{\"canal\":0,\"nome\":\"SNES\"}]"));
    TEST_ASSERT_EQUAL_STRING(
        "[{\"canal\":0,\"nome\":\"SNES\",\"imagem\":\"\"},"
        "{\"canal\":5,\"nome\":\"Saturn\",\"imagem\":\"https://x/s.png\"}]",
        saida.c_str());
}

void test_canal_repetido_fica_o_ultimo()
{
    TEST_ASSERT_TRUE(normalizar("[{\"canal\":2,\"nome\":\"A\"},{\"canal\":2,\"nome\":\"B\"}]"));
    TEST_ASSERT_EQUAL_STRING("[{\"canal\":2,\"nome\":\"B\",\"imagem\":\"\"}]", saida.c_str());
}

void test_acentos_sao_preservados()
{
    TEST_ASSERT_TRUE(normalizar("[{\"canal\":1,\"nome\":\"Máquina de Ação\"}]"));
    TEST_ASSERT_EQUAL_STRING("[{\"canal\":1,\"nome\":\"Máquina de Ação\",\"imagem\":\"\"}]", saida.c_str());
}

void test_json_invalido()
{
    TEST_ASSERT_FALSE(normalizar("[{\"canal\":1,"));
    TEST_ASSERT_EQUAL_STRING("JSON inválido", erro.c_str());
}

void test_precisa_ser_lista()
{
    TEST_ASSERT_FALSE(normalizar("{\"canal\":1,\"nome\":\"A\"}"));
    TEST_ASSERT_EQUAL_STRING("Esperado uma lista", erro.c_str());
}

void test_campos_obrigatorios()
{
    TEST_ASSERT_FALSE(normalizar("[{\"nome\":\"Sem canal\"}]"));
    TEST_ASSERT_FALSE(normalizar("[{\"canal\":1}]"));
    TEST_ASSERT_FALSE(normalizar("[{\"canal\":\"1\",\"nome\":\"Canal como texto\"}]"));
    TEST_ASSERT_FALSE(normalizar("[{\"canal\":1,\"nome\":7}]"));
}

void test_canal_fora_do_intervalo()
{
    TEST_ASSERT_FALSE(normalizar("[{\"canal\":16,\"nome\":\"A\"}]"));
    TEST_ASSERT_FALSE(normalizar("[{\"canal\":-1,\"nome\":\"A\"}]"));
    TEST_ASSERT_EQUAL_STRING("Canal fora do intervalo 0-15", erro.c_str());
}

void test_limites_de_tamanho()
{
    TEST_ASSERT_FALSE(normalizar("[{\"canal\":1,\"nome\":\"\"}]"));

    std::string nome(NOME_MAX, 'a');
    TEST_ASSERT_TRUE(normalizar(("[{\"canal\":1,\"nome\":\"" + nome + "\"}]").c_str()));
    TEST_ASSERT_FALSE(normalizar(("[{\"canal\":1,\"nome\":\"" + nome + "a\"}]").c_str()));

    std::string imagem(IMAGEM_MAX + 1, 'x');
    TEST_ASSERT_FALSE(normalizar(("[{\"canal\":1,\"nome\":\"A\",\"imagem\":\"" + imagem + "\"}]").c_str()));
}

void test_cadastro_completo_cabe_na_nvs()
{
    // 16 consoles com nome de 32 caracteres e URL típica do CDN
    std::string json = "[";
    for (int canal = 0; canal < TOTAL_CANAIS; canal++)
    {
        if (canal)
            json += ",";
        json += "{\"canal\":" + std::to_string(canal) + ",\"nome\":\"" + std::string(32, 'n') +
                "\",\"imagem\":\"https://raw.githubusercontent.com/EduardoArine/retro-relay-images/refs/heads/main/sega_dreamcast.png\"}";
    }
    json += "]";
    TEST_ASSERT_TRUE(normalizar(json.c_str()));
    TEST_ASSERT_LESS_OR_EQUAL(NVS_STRING_MAX, saida.size());
}

void test_cadastro_grande_demais_e_recusado()
{
    // 16 consoles no limite de nome e imagem passam de 4000 bytes
    std::string json = "[";
    for (int canal = 0; canal < TOTAL_CANAIS; canal++)
    {
        if (canal)
            json += ",";
        json += "{\"canal\":" + std::to_string(canal) + ",\"nome\":\"" + std::string(NOME_MAX, 'n') +
                "\",\"imagem\":\"" + std::string(IMAGEM_MAX, 'i') + "\"}";
    }
    json += "]";
    TEST_ASSERT_FALSE(normalizar(json.c_str()));
    TEST_ASSERT_EQUAL_STRING("Cadastro grande demais", erro.c_str());
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_lista_vazia_e_valida);
    RUN_TEST(test_ordena_por_canal_e_descarta_campos_extras);
    RUN_TEST(test_canal_repetido_fica_o_ultimo);
    RUN_TEST(test_acentos_sao_preservados);
    RUN_TEST(test_json_invalido);
    RUN_TEST(test_precisa_ser_lista);
    RUN_TEST(test_campos_obrigatorios);
    RUN_TEST(test_canal_fora_do_intervalo);
    RUN_TEST(test_limites_de_tamanho);
    RUN_TEST(test_cadastro_completo_cabe_na_nvs);
    RUN_TEST(test_cadastro_grande_demais_e_recusado);
    return UNITY_END();
}
