#include "ConsoleManager.h"
#include <Preferences.h>

#include "logica/consoles.h"

static Preferences consolePrefs;

String carregarConsoles()
{
    consolePrefs.begin("consoles", true);
    String json = consolePrefs.getString("lista", "[]");
    consolePrefs.end();
    return json;
}

void apagarConsoles()
{
    consolePrefs.begin("consoles", false);
    consolePrefs.clear();
    consolePrefs.end();
}

bool salvarConsoles(const char *json, String &erro)
{
    std::string normalizado, motivo;
    if (!normalizarConsoles(json, normalizado, motivo))
    {
        erro = motivo.c_str();
        return false;
    }

    consolePrefs.begin("consoles", false);
    bool ok = consolePrefs.putString("lista", normalizado.c_str()) == normalizado.size();
    consolePrefs.end();

    if (!ok)
        erro = "Falha ao gravar na memória";
    return ok;
}
