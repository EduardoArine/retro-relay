// Confere a sintaxe dos <script> das páginas HTML antes da release: um erro de
// JavaScript em web/index.html iria por OTA para todos os aparelhos.
// Uso: node scripts/checar_web.js web/index.html docs/index.html
const fs = require('fs');
const vm = require('vm');

let falhas = 0;
for (const arquivo of process.argv.slice(2)) {
  const html = fs.readFileSync(arquivo, 'utf8');
  const scripts = [...html.matchAll(/<script(?![^>]*\bsrc=)[^>]*>([\s\S]*?)<\/script>/g)].map(m => m[1]);
  if (!scripts.length) {
    console.error(`✗ ${arquivo}: nenhum <script> encontrado`);
    falhas++;
    continue;
  }
  scripts.forEach((codigo, i) => {
    try {
      new vm.Script(codigo, { filename: `${arquivo}#script${i + 1}` });
    } catch (e) {
      console.error(`✗ ${arquivo} (script ${i + 1}): ${e.message}`);
      falhas++;
    }
  });
  if (!falhas) console.log(`✓ ${arquivo}: ${scripts.length} script(s) ok`);
}
process.exit(falhas ? 1 : 0);
