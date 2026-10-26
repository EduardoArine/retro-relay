// Leitura da API de releases do GitHub (src/logica/release.h), com respostas
// reais capturadas da API (campos irrelevantes reduzidos).
#include <unity.h>

#include "logica/release.h"

void setUp() {}
void tearDown() {}

// GET /releases/latest em 2026-10-26 (release 1.8.0, sem assinatura)
static const char *LATEST_REAL = R"json({"url": "https://api.github.com/repos/EduardoArine/retro-relay/releases/424619409", "id": 424619409, "tag_name": "1.8.0", "target_commitish": "main", "name": "1.8.0", "draft": false, "prerelease": false, "created_at": "2026-10-26T18:32:12Z", "published_at": "2026-10-26T18:34:33Z", "body": "", "assets": [{"url": "https://api.github.com/repos/EduardoArine/retro-relay/releases/assets/679086346", "id": 679086346, "name": "retrorelay-1.8.0.bin", "label": "", "content_type": "application/octet-stream", "state": "uploaded", "size": 1148800, "download_count": 1, "browser_download_url": "https://github.com/EduardoArine/retro-relay/releases/download/1.8.0/retrorelay-1.8.0.bin", "uploader": {"login": "github-actions[bot]", "type": "Bot"}}], "author": {"login": "github-actions[bot]", "type": "Bot"}})json";

// GET /releases?per_page=1: mesma release, dentro de uma lista
static const char *LISTA_REAL = R"json([{"url": "https://api.github.com/repos/EduardoArine/retro-relay/releases/424619409", "id": 424619409, "tag_name": "1.8.0", "draft": false, "prerelease": false, "body": "", "assets": [{"id": 679086346, "name": "retrorelay-1.8.0.bin", "size": 1148800, "browser_download_url": "https://github.com/EduardoArine/retro-relay/releases/download/1.8.0/retrorelay-1.8.0.bin", "uploader": {"login": "github-actions[bot]"}}]}])json";

// Formato de uma release assinada e promovida com liberação gradual
static const char *ASSINADA = R"json({"tag_name": "1.10.0", "prerelease": false, "body": "liberacao: 25", "assets": [
  {"name": "retrorelay-1.10.0.bin", "size": 1150000, "browser_download_url": "https://github.com/x/releases/download/1.10.0/retrorelay-1.10.0.bin"},
  {"name": "retrorelay-1.10.0.bin.sig", "size": 72, "browser_download_url": "https://github.com/x/releases/download/1.10.0/retrorelay-1.10.0.bin.sig"}]})json";

// Resposta real da API quando o limite de consultas acaba
static const char *LIMITE_DA_API = R"json({"message": "API rate limit exceeded for 177.0.0.1.", "documentation_url": "https://docs.github.com/rest/overview/resources-in-the-rest-api#rate-limiting"})json";

void test_le_release_latest_real()
{
    InfoRelease info;
    TEST_ASSERT_TRUE(lerRelease(LATEST_REAL, false, info));
    TEST_ASSERT_EQUAL_STRING("1.8.0", info.versao.c_str());
    TEST_ASSERT_EQUAL_STRING("https://github.com/EduardoArine/retro-relay/releases/download/1.8.0/retrorelay-1.8.0.bin", info.firmwareUrl.c_str());
    TEST_ASSERT_EQUAL_UINT32(1148800, info.firmwareTamanho);
    TEST_ASSERT_TRUE(info.assinaturaUrl.empty());
    TEST_ASSERT_TRUE(info.notas.empty());
}

void test_le_lista_do_canal_beta()
{
    InfoRelease info;
    TEST_ASSERT_TRUE(lerRelease(LISTA_REAL, true, info));
    TEST_ASSERT_EQUAL_STRING("1.8.0", info.versao.c_str());
    TEST_ASSERT_EQUAL_UINT32(1148800, info.firmwareTamanho);
}

void test_formato_errado_nao_e_lido()
{
    InfoRelease info;
    TEST_ASSERT_FALSE(lerRelease(LISTA_REAL, false, info)); // Lista lida como objeto
    TEST_ASSERT_FALSE(lerRelease(LATEST_REAL, true, info)); // Objeto lido como lista
}

void test_le_assinatura_e_notas()
{
    InfoRelease info;
    TEST_ASSERT_TRUE(lerRelease(ASSINADA, false, info));
    TEST_ASSERT_EQUAL_STRING("1.10.0", info.versao.c_str());
    TEST_ASSERT_EQUAL_STRING("https://github.com/x/releases/download/1.10.0/retrorelay-1.10.0.bin", info.firmwareUrl.c_str());
    TEST_ASSERT_EQUAL_STRING("https://github.com/x/releases/download/1.10.0/retrorelay-1.10.0.bin.sig", info.assinaturaUrl.c_str());
    TEST_ASSERT_EQUAL_STRING("liberacao: 25", info.notas.c_str());
}

void test_erro_da_api_e_lista_vazia()
{
    InfoRelease info;
    TEST_ASSERT_FALSE(lerRelease(LIMITE_DA_API, false, info));
    TEST_ASSERT_FALSE(lerRelease("[]", true, info));
    TEST_ASSERT_FALSE(lerRelease("<html>erro</html>", false, info));
}

void test_release_sem_firmware()
{
    InfoRelease info;
    TEST_ASSERT_TRUE(lerRelease(R"json({"tag_name": "2.0.0", "assets": []})json", false, info));
    TEST_ASSERT_TRUE(info.firmwareUrl.empty());
}

void test_nomes_de_arquivo()
{
    TEST_ASSERT_TRUE(ehArquivoFirmware("retrorelay-1.9.0.bin"));
    TEST_ASSERT_FALSE(ehArquivoFirmware("retrorelay-1.9.0.bin.sig"));
    TEST_ASSERT_TRUE(ehArquivoAssinatura("retrorelay-1.9.0.bin.sig"));
    TEST_ASSERT_FALSE(ehArquivoAssinatura("retrorelay-1.9.0.bin"));
    TEST_ASSERT_FALSE(ehArquivoFirmware("bin"));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_le_release_latest_real);
    RUN_TEST(test_le_lista_do_canal_beta);
    RUN_TEST(test_formato_errado_nao_e_lido);
    RUN_TEST(test_le_assinatura_e_notas);
    RUN_TEST(test_erro_da_api_e_lista_vazia);
    RUN_TEST(test_release_sem_firmware);
    RUN_TEST(test_nomes_de_arquivo);
    return UNITY_END();
}
