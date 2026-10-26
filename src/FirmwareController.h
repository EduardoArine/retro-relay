#pragma once

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>
#include <esp_ota_ops.h>
#include <Preferences.h>

#include "AssinaturaOta.h"
#include "ConexaoManager.h"
#include "IdentidadeManager.h"
#include "ReleManager.h"
#include "logica/canais.h"
#include "logica/ota.h"
#include "logica/release.h"

extern String firmwareVersion;

String firmwareVersion = "#{Deploy_Version}#";
String latestVersion = "";
String firmwareUrl = "";

bool firmwareForceCheck = false;
unsigned long lastCheckMillis = 0;
const unsigned long intervalMillis = 30UL * 60UL * 1000UL;

// Raízes confiáveis para o HTTPS do OTA, geradas por scripts/gen_crt_bundle.py
// e embutidas no firmware via board_build.embed_files
extern const uint8_t x509_crt_bundle_start[] asm("_binary_certs_x509_crt_bundle_bin_start");

// Fase de validação do certificado: com false, se a conexão verificada falhar mas
// a sem verificação funcionar, o OTA segue sem verificar (como nas versões
// anteriores) e registra "falhou" no /diag. Depois de confirmar "ok" no
// dispositivo, mudar para true: aí o OTA só acontece com certificado válido.
const bool OTA_EXIGIR_CERTIFICADO = false;

String statusCertificadoOta = "nao_testado"; // ok | falhou | nao_testado (exposto no /diag)

// Fase de validação da assinatura do firmware (chaves em certs/*.pub.pem): com
// false, o resultado só é registrado no /diag e o firmware é instalado mesmo sem
// assinatura válida. Depois de ver "valida" no dispositivo, mudar para true: aí
// firmware sem assinatura válida é descartado antes do reinício.
const bool OTA_EXIGIR_ASSINATURA = false;

String assinaturaUrl = "";  // .bin.sig da release ("" = release sem assinatura)
size_t firmwareTamanho = 0; // Bytes do .bin, para conferir a assinatura do que foi gravado
String versaoRecusada = ""; // Versão descartada por assinatura: não baixa de novo

// O reinício desliga os relés por alguns segundos (pino de enable do 74HC595 em
// nível alto durante o reset), então a atualização espera os consoles desligarem.
String versaoPendente = "";          // Versão nova encontrada, ainda não baixada
bool reinicioPendente = false;       // Versão nova já gravada, falta reiniciar
const char *motivoReinicioOta = "ota";

// Desliga a validação automática do core Arduino: quem valida é verifyFirmware()
extern "C" bool verifyRollbackLater(){
    return true;
}

// Chamar no fim do setup(), depois que o dispositivo estiver acessível.
// Até lá, um firmware novo fica PENDING_VERIFY e qualquer reinício
// (crash, watchdog ou falha de WiFi) faz o bootloader voltar à versão anterior.
void verifyFirmware()
{
    /* Captura o ponteiro da partição em execução */
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;

    /* Verifica se a partição possui bloco 'otadata' */
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) 
    {
        const char* otaState = ota_state == ESP_OTA_IMG_NEW ? "ESP_OTA_IMG_NEW"
            : ota_state == ESP_OTA_IMG_PENDING_VERIFY ? "ESP_OTA_IMG_PENDING_VERIFY"
            : ota_state == ESP_OTA_IMG_VALID ? "ESP_OTA_IMG_VALID"
            : ota_state == ESP_OTA_IMG_INVALID ? "ESP_OTA_IMG_INVALID"
            : ota_state == ESP_OTA_IMG_ABORTED ? "ESP_OTA_IMG_ABORTED"
            : "ESP_OTA_IMG_UNDEFINED";

        Serial.println("Estado do Firmware: " + String(otaState));

        /* Verifica se a imagem do OTA está pendente de verificação */
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) 
        {
            /* Marca a imagem como válida e tenta cancelar o rollback */
            if (esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) 
            {
                Serial.println("Firmware válido, rollback cancelado com sucesso!");
            } 
            else 
            {
                Serial.println("Firmware inválido, realizando rollback!");
                /* Marca a imagem como inválida e reinicia o device (rollback) */
                esp_ota_mark_app_invalid_rollback_and_reboot();
            }
        }
    }
    else
    {
        Serial.println("Partição OTA não possui registro de dados!");
    }
}

static void configurarTls(WiFiClientSecure &client, bool verificarCertificado)
{
    if (verificarCertificado)
        client.setCACertBundle(x509_crt_bundle_start);
    else
        client.setInsecure();
}

CanalOta canalAtual = CanalOta::ESTAVEL;
int ultimaLiberacao = 100; // Porcentagem liberada da última versão estável consultada (para o /diag)

void carregarCanal()
{
    Preferences prefs;
    prefs.begin("ota", true);
    canalAtual = canalSalvo(prefs.getUChar("canal", (uint8_t)CanalOta::ESTAVEL));
    prefs.end();
}

void definirCanal(CanalOta canal)
{
    canalAtual = canal;
    Preferences prefs;
    prefs.begin("ota", false);
    prefs.putUChar("canal", (uint8_t)canal);
    prefs.end();
    firmwareForceCheck = true; // Consulta já no canal novo
}

// Consulta a versão disponível para este aparelho. Retorna false se a consulta
// falhou; com true, versaoDisponivel fica "" quando não há versão liberada para ele.
//   Estável: /releases/latest (só versões promovidas) + porcentagem liberada
//   Beta:    /releases?per_page=1 (a mais recente, promovida ou não)
static bool consultarVersao(bool verificarCertificado, String &versaoDisponivel)
{
    bool beta = canalAtual == CanalOta::BETA;
    WiFiClientSecure client;
    configurarTls(client, verificarCertificado);

    HTTPClient https;
    https.begin(client, beta ? "https://api.github.com/repos/EduardoArine/retro-relay/releases?per_page=1"
                             : "https://api.github.com/repos/EduardoArine/retro-relay/releases/latest");
    https.addHeader("User-Agent", "ESP32-Agent");

    int httpCode = https.GET();
    if (httpCode != 200) return false;

    InfoRelease release;
    if (!lerRelease(https.getString().c_str(), beta, release)) return false;

    latestVersion = release.versao.c_str();
    firmwareUrl = release.firmwareUrl.c_str();
    firmwareTamanho = release.firmwareTamanho;
    assinaturaUrl = release.assinaturaUrl.c_str();
    versaoDisponivel = firmwareUrl == "" ? "" : latestVersion;

    if (!beta)
    {
        ultimaLiberacao = lerLiberacao(release.notas.c_str());
        if (versaoDisponivel != "" && !liberadaParaAparelho(canalAtual, ultimaLiberacao, grupoDoAparelho()))
        {
            Serial.printf("Versão %s liberada para %d%% dos aparelhos; este é do grupo %u: aguardando\n",
                          latestVersion.c_str(), ultimaLiberacao, grupoDoAparelho());
            versaoDisponivel = "";
        }
    }
    return true;
}

String getLatestFirmwareVersion()
{
    String versao;
    if (consultarVersao(true, versao))
    {
        statusCertificadoOta = "ok";
        return versao;
    }
    if (OTA_EXIGIR_CERTIFICADO)
        return "";

    if (consultarVersao(false, versao))
        statusCertificadoOta = "falhou"; // Só funcionou sem verificar: o problema é o certificado
    return versao;
}

void verificarAtualizacao()
{
    // Build local (gravado pelo cabo) não tem versão: sem isso, seria trocado pela release
    if (ehBuildDesenvolvimento(firmwareVersion.c_str()))
    {
        Serial.println("Build de desenvolvimento: OTA desativado");
        return;
    }

    String remoteVersion = getLatestFirmwareVersion();
    if (!haVersaoNova(remoteVersion.c_str(), firmwareVersion.c_str()) || remoteVersion == versaoRecusada)
    {
        versaoPendente = "";
        Serial.println("Firmware já está atualizado.");
        return;
    }

    Serial.println("Nova versão detectada: " + remoteVersion);
    versaoPendente = remoteVersion;
}

static t_httpUpdate_return baixarFirmware(bool verificarCertificado)
{
    WiFiClientSecure client;
    configurarTls(client, verificarCertificado);

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.begin(client, firmwareUrl);

    httpUpdate.rebootOnUpdate(false);
    httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    return httpUpdate.update(http, firmwareVersion);
}

// A assinatura (DER, ~72 bytes) vem do mesmo servidor que o .bin
static size_t baixarAssinatura(bool verificarCertificado, uint8_t *destino, size_t capacidade)
{
    if (assinaturaUrl == "")
        return 0;

    WiFiClientSecure client;
    configurarTls(client, verificarCertificado);

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.begin(client, assinaturaUrl);
    if (http.GET() != 200)
        return 0;

    int tamanho = http.getSize();
    if (tamanho <= 0 || (size_t)tamanho > capacidade)
        return 0;
    size_t lido = http.getStreamPtr()->readBytes(destino, tamanho);
    return lido == (size_t)tamanho ? lido : 0;
}

void aplicarAtualizacao()
{
    Serial.println("Iniciando OTA a partir de: " + firmwareUrl);
    versaoPendente = "";

    bool verificado = true;
    t_httpUpdate_return result = baixarFirmware(true);
    if (result == HTTP_UPDATE_FAILED && !OTA_EXIGIR_CERTIFICADO)
    {
        Serial.println("Download verificado falhou (" + httpUpdate.getLastErrorString() + "), tentando sem verificar");
        verificado = false;
        result = baixarFirmware(false);
    }

    switch (result)
    {
    case HTTP_UPDATE_FAILED:
        Serial.println("Atualização falhou: " + String(httpUpdate.getLastError()) + " - " + httpUpdate.getLastErrorString());
        break;
    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("Nenhuma atualização disponível.");
        break;
    case HTTP_UPDATE_OK:
    {
        Serial.println("Atualização gravada.");

        static uint8_t assinatura[128];
        size_t tamanhoAssinatura = baixarAssinatura(verificado, assinatura, sizeof(assinatura));
        ResultadoAssinatura resultado = conferirFirmwareGravado(firmwareTamanho, assinatura, tamanhoAssinatura);
        registrarAssinatura(resultado, latestVersion);
        Serial.printf("Assinatura do firmware: %s\n", nomeResultado(resultado));

        if (!aceitarFirmware(resultado, OTA_EXIGIR_ASSINATURA))
        {
            descartarFirmwareGravado();
            versaoRecusada = latestVersion;
            Serial.println("⛔ Firmware descartado: assinatura não confere");
            break;
        }

        motivoReinicioOta = verificado ? "ota" : "ota_sem_certificado";
        reinicioPendente = true;
        break;
    }
    }
}

void loopOTA()
{
    unsigned long now = millis();
    bool horaDeVerificar = firmwareForceCheck || decorrido(now, lastCheckMillis) >= intervalMillis;

    switch (proximaAcaoOta(reinicioPendente, versaoPendente != "", horaDeVerificar, estadoAtual))
    {
    case AcaoOta::REINICIAR:
        reiniciar(motivoReinicioOta);
        break;

    case AcaoOta::VERIFICAR:
        firmwareForceCheck = false;
        lastCheckMillis = now;
        Serial.println("🚀 Executando verificação OTA");
        esp_task_wdt_delete(NULL);
        verificarAtualizacao();
        esp_task_wdt_add(NULL);
        // Se achou versão nova, a próxima volta do loop instala (consoles desligados)
        if (versaoPendente != "" && estadoAtual != 0)
            Serial.println("⏸️ Atualização " + versaoPendente + " adiada até os consoles serem desligados");
        break;

    case AcaoOta::INSTALAR:
        esp_task_wdt_delete(NULL);
        aplicarAtualizacao();
        esp_task_wdt_add(NULL);
        break;

    case AcaoOta::NADA:
        break;
    }
}
