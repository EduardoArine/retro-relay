# Gera include/web_index.h a partir de web/index.html antes de cada build.
#
# A página vai comprimida (gzip) dentro do firmware, e não no SPIFFS, para
# que toda atualização OTA leve junto a interface web correspondente.
# O ETag é derivado do conteúdo, então o navegador só baixa a página de novo
# quando ela muda.

import gzip
import hashlib
import os

Import("env")

project_dir = env.subst("$PROJECT_DIR")
src_path = os.path.join(project_dir, "web", "index.html")
dst_path = os.path.join(project_dir, "include", "web_index.h")

with open(src_path, "rb") as f:
    html = f.read()

gz = gzip.compress(html, compresslevel=9, mtime=0)
etag = hashlib.sha1(html).hexdigest()[:16]

lines = [
    "// Arquivo gerado por scripts/embed_web.py a partir de web/index.html. Não editar.",
    "#pragma once",
    "#include <Arduino.h>",
    "",
    f'#define WEB_INDEX_ETAG "\\"{etag}\\""',
    f"const size_t WEB_INDEX_GZ_LEN = {len(gz)};",
    "const uint8_t WEB_INDEX_GZ[] PROGMEM = {",
]
for i in range(0, len(gz), 20):
    lines.append("  " + ",".join(f"0x{b:02x}" for b in gz[i:i + 20]) + ",")
lines.append("};")
header = "\n".join(lines) + "\n"

# Só reescreve se mudou, para não forçar recompilação à toa
current = None
if os.path.exists(dst_path):
    with open(dst_path, "r", encoding="utf-8") as f:
        current = f.read()

if current != header:
    with open(dst_path, "w", encoding="utf-8", newline="\n") as f:
        f.write(header)

print(f"web/index.html: {len(html)} bytes -> {len(gz)} bytes gzip")
