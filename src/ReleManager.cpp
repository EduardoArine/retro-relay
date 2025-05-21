#include "ReleManager.h"

#define HC595_SI_PIN 14
#define HC595_SCK_PIN 13
#define HC595_RCK_PIN 12
#define HC595_G_PIN 5

uint16_t estadoAtual = 0;
ModoRele modoAtual = ModoRele::UNICO;

void inicializarReles()
{
    pinMode(HC595_SI_PIN, OUTPUT);
    pinMode(HC595_SCK_PIN, OUTPUT);
    pinMode(HC595_RCK_PIN, OUTPUT);
    pinMode(HC595_G_PIN, OUTPUT);
    digitalWrite(HC595_G_PIN, HIGH);
    digitalWrite(HC595_SI_PIN, LOW);
    digitalWrite(HC595_SCK_PIN, LOW);
    digitalWrite(HC595_RCK_PIN, LOW);
}

void aplicarEstado()
{
    digitalWrite(HC595_RCK_PIN, LOW);
    for (int i = 15; i >= 0; i--)
    {
        digitalWrite(HC595_SCK_PIN, LOW);
        digitalWrite(HC595_SI_PIN, (estadoAtual >> i) & 0x01);
        digitalWrite(HC595_SCK_PIN, HIGH);
    }
    digitalWrite(HC595_RCK_PIN, HIGH);
    digitalWrite(HC595_G_PIN, LOW);
}

void alternarRele(int canal)
{
    if (canal < 0 || canal > 15)
        return;

    if (modoAtual == ModoRele::UNICO)
    {
        // Liga somente o canal solicitado, desliga todos os outros
        if ((estadoAtual & (1 << canal)) != 0)
        {
            estadoAtual = 0;
        }
        else
        {
            estadoAtual = (1 << canal);
        }
    }
    else
    {
        // Liga ou desliga individualmente (toggle)
        estadoAtual ^= (1 << canal);
    }

    aplicarEstado();
}

void setModo(ModoRele modo)
{
    modoAtual = modo;
}

ModoRele getModoAtual()
{
    return modoAtual;
}

void carregarEstadoSalvo()
{
    // TODO: usar Preferences futuramente
}
