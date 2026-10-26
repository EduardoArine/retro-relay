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

## Como liberar uma versão

1. Merge na `main` com mudança de firmware gera uma release **beta** (pre-release), assinada com o secret `OTA_CHAVE_PRIVADA` (sem ele, a release falha e não é publicada). Só aparelhos com **Versões beta** ligado (Config da página) recebem.
2. Depois de testar, em **Actions → Promover versão para estável → Run workflow**, informe a versão e a porcentagem de aparelhos (ex.: 10). Para ampliar, rode de novo com 50 e depois 100.
3. Cada aparelho tem um grupo fixo de 0 a 99 (Diagnóstico da página) e recebe quando a porcentagem passa do grupo dele. Nenhum aparelho volta para uma versão mais antiga.

Chaves de assinatura: as públicas ficam em `certs/ota_*.pub.pem` (embutidas no firmware); a privada principal fica só no secret `OTA_CHAVE_PRIVADA` e a reserva, fora do GitHub.

Certificado e assinatura são obrigatórios: cada aparelho passa a exigir cada um na primeira vez que ele funciona ali (atualização baixada com certificado verificado / assinatura válida) e não desliga mais, nem na restauração de fábrica. O Diagnóstico mostra "obrigatório" quando a exigência está ligada.
