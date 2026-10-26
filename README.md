# retro-relay

📍 **Roadmap:** https://eduardoarine.github.io/retro-relay/ (fonte em [docs/roadmap.json](docs/roadmap.json))

hello git world!!!

[Major | Minor | Patch] = 1.0.0
+semver: major
+semver: minor

## Desenvolvimento

```sh
pio run                                          # compila o firmware (ESP32)
pio test -e native                               # testes da lógica (src/logica), sem placa
python -m unittest discover -s scripts/tests     # pacote de certificados do OTA
node scripts/checar_web.js web/index.html docs/index.html   # sintaxe do JS das páginas
```

O CI (`.github/workflows/testes.yml`) roda tudo isso a cada push; a release só é publicada se os testes passarem.
