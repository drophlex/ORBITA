// renders the manual's HTML (manual.py) to PDF and reports pages whose content runs past the footer
import { chromium } from '/opt/npm-tools/node_modules/playwright/index.mjs';
const D = process.argv[2] || '/home/claude/man/orbita2';
const b = await chromium.launch({ executablePath: '/usr/bin/google-chrome' });
const p = await b.newPage();
for (const [src, out] of [['manual_en.html', 'Orbita-EN.pdf'], ['manual_es.html', 'Orbita-ES.pdf'], ['manual_en_es.html', 'Orbita-EN-ES.pdf']]) {
  await p.goto('file://' + D + '/' + src); await p.waitForTimeout(800);
  const over = await p.evaluate(() => [...document.querySelectorAll('.page')].map((pg, i) => { const r = pg.getBoundingClientRect(); let m = 0; pg.querySelectorAll('*').forEach(e => { if (e.closest('footer')) return; const q = e.getBoundingClientRect(); if (q.height) m = Math.max(m, q.bottom - r.top); }); return [i + 1, Math.round(m)]; }).filter(x => x[1] > 990));
  await p.pdf({ path: D + '/' + out, width: '8.5in', height: '11in', printBackground: true });
  console.log(out, 'overflow:', JSON.stringify(over));
}
await b.close();
