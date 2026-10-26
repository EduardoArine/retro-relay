#pragma once
#include <Arduino.h>

// Cadastro dos consoles (nome + imagem por canal), compartilhado entre o app e a página web.
// Formato igual ao do app: [{"canal":0,"nome":"Super Nintendo","imagem":"https://..."}]

const size_t CONSOLES_MAX_BYTES = 6144; // Tamanho máximo aceito no corpo do POST

String carregarConsoles();
bool salvarConsoles(const char *json, String &erro);
