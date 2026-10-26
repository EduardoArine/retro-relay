#include "IdentidadeManager.h"
#include <ESPmDNS.h>
#include <WiFi.h>
#include <esp_system.h>
#include <mdns.h>

#include "logica/canais.h"
#include "logica/identidade.h"

#define APELIDO "retrorelay" // Nome antigo, ainda respondido via mDNS

static char id[ID_TAM];
static char nome[NOME_TAM];
static char portal[NOME_TAM];
static uint8_t grupo = 0;
static bool mdnsAtivo = false;

// O apelido é um "hostname delegado" no mDNS: precisa ser recadastrado com o IP
// atual sempre que o aparelho ganha um IP novo
static void atualizarApelido()
{
    if (!mdnsAtivo)
        return;

    mdns_ip_addr_t endereco = {};
    endereco.addr.type = ESP_IPADDR_TYPE_V4;
    endereco.addr.u_addr.ip4.addr = static_cast<uint32_t>(WiFi.localIP());
    endereco.next = nullptr;

    if (mdns_hostname_exists(APELIDO))
        mdns_delegate_hostname_remove(APELIDO);
    if (mdns_delegate_hostname_add(APELIDO, &endereco) != ESP_OK)
        Serial.println("⚠️ Não foi possível registrar o apelido " APELIDO ".local");
}

void iniciarIdentidade()
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    idDoMac(mac, id);
    nomeDeRede(id, nome);
    nomeDoPortal(id, portal);
    grupo = grupoLiberacao(mac);
    Serial.printf("🏷️ Aparelho: %s (grupo de liberação %u)\n", nome, grupo);

    WiFi.onEvent([](WiFiEvent_t, WiFiEventInfo_t)
                 { atualizarApelido(); },
                 ARDUINO_EVENT_WIFI_STA_GOT_IP);
}

const char *idDispositivo() { return id; }
const char *nomeDispositivo() { return nome; }
const char *nomePortalConfiguracao() { return portal; }
uint8_t grupoDoAparelho() { return grupo; }

void iniciarMdns(const char *versao)
{
    if (!MDNS.begin(nome))
    {
        Serial.println("⚠️ Erro ao iniciar mDNS");
        return;
    }
    mdnsAtivo = true;

    // Instância "retrorelay-a3f2._http._tcp": o app procura pelo prefixo "retrorelay"
    MDNS.setInstanceName(nome);
    MDNS.addService("http", "tcp", 80);
    MDNS.addServiceTxt("http", "tcp", "modelo", "retrorelay");
    MDNS.addServiceTxt("http", "tcp", "id", (const char *)id);
    MDNS.addServiceTxt("http", "tcp", "versao", versao);
    atualizarApelido();

    Serial.printf("🌐 mDNS: http://%s.local (apelido http://" APELIDO ".local)\n", nome);
}
