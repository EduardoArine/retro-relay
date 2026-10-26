#pragma once
// Pedidos de reset feitos pela página (testados em test/test_reset).
#include <stdint.h>

enum class TipoReset
{
    NENHUM,
    WIFI,   // Esquece só a rede Wi-Fi (trocar de roteador)
    FABRICA // Esquece Wi-Fi, cadastro de consoles e modo
};

// Retorna nullptr se o reset pode ser feito, ou o motivo da recusa.
// O reset reinicia o aparelho, o que desliga os relés por alguns segundos.
inline const char *validarReset(bool confirmado, uint16_t estadoReles)
{
    if (!confirmado)
        return "Confirmação ausente";
    if (estadoReles != 0)
        return "Desligue todos os consoles antes: o aparelho vai reiniciar";
    return nullptr;
}
