#include "AssinaturaOta.h"
#include <Preferences.h>
#include <esp_ota_ops.h>
#include <mbedtls/sha256.h>

// Chaves públicas embutidas via board_build.embed_txtfiles (certs/*.pub.pem)
extern const char chavePrincipal[] asm("_binary_certs_ota_principal_pub_pem_start");
extern const char chaveReserva[] asm("_binary_certs_ota_reserva_pub_pem_start");
static const char *const CHAVES[] = {chavePrincipal, chaveReserva};

static bool hashParticao(const esp_partition_t *particao, size_t tamanho, uint8_t hash[32])
{
    static uint8_t bloco[1024];
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);

    bool ok = mbedtls_sha256_starts_ret(&ctx, 0) == 0;
    for (size_t lido = 0; ok && lido < tamanho;)
    {
        size_t n = min(sizeof(bloco), tamanho - lido);
        ok = esp_partition_read(particao, lido, bloco, n) == ESP_OK &&
             mbedtls_sha256_update_ret(&ctx, bloco, n) == 0;
        lido += n;
    }
    ok = ok && mbedtls_sha256_finish_ret(&ctx, hash) == 0;

    mbedtls_sha256_free(&ctx);
    return ok;
}

ResultadoAssinatura conferirFirmwareGravado(size_t tamanho, const uint8_t *assinatura, size_t tamanhoAssinatura)
{
    if (assinatura == nullptr || tamanhoAssinatura == 0)
        return ResultadoAssinatura::SEM_ASSINATURA;

    // Depois do HTTPUpdate, a partição de boot é a recém-gravada (ainda não em uso)
    const esp_partition_t *nova = esp_ota_get_boot_partition();
    if (nova == nullptr || nova == esp_ota_get_running_partition() || tamanho == 0 || tamanho > nova->size)
        return ResultadoAssinatura::INVALIDA;

    uint8_t hash[32];
    if (!hashParticao(nova, tamanho, hash))
        return ResultadoAssinatura::INVALIDA;

    return verificarAssinatura(hash, assinatura, tamanhoAssinatura, CHAVES, sizeof(CHAVES) / sizeof(CHAVES[0]));
}

bool descartarFirmwareGravado()
{
    return esp_ota_set_boot_partition(esp_ota_get_running_partition()) == ESP_OK;
}

void registrarAssinatura(ResultadoAssinatura resultado, const String &versao)
{
    Preferences prefs;
    prefs.begin("ota", false);
    prefs.putString("assinatura", nomeResultado(resultado));
    prefs.putString("assin_versao", versao);
    prefs.end();
}

void preencherAssinatura(JsonObject ota)
{
    Preferences prefs;
    prefs.begin("ota", true);
    ota["assinatura"] = prefs.getString("assinatura", "nao_testado");
    ota["assinatura_versao"] = prefs.getString("assin_versao", "");
    prefs.end();
}
