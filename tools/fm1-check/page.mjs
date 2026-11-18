// ÓRBITA's FM-1 installer page in Chrome, its Web MIDI wired to the firmware's editor handler (out/fm1_device):
// press the button, wait for the check, then Remove; screenshots in out/
import { chromium } from '/opt/npm-tools/node_modules/playwright/index.mjs';
import { spawn } from 'child_process'; import http from 'http'; import fs from 'fs'; import path from 'path';
const [docs, out] = process.argv.slice(2);
const srv = http.createServer((q, r) => { const f = path.join(docs, decodeURIComponent(q.url.split('?')[0]).replace(/\/$/, '/index.html')); fs.readFile(f, (e, d) => { r.writeHead(e ? 404 : 200, { 'content-type': f.endsWith('.js') ? 'text/javascript' : f.endsWith('.html') ? 'text/html' : 'application/octet-stream' }); r.end(d); }); }).listen(8799);
const dev = spawn(path.join(out, 'fm1_device')); let rx = [];
const b = await chromium.launch({ executablePath: '/usr/bin/google-chrome' });
let fails = 0;
for (const lang of ['en', 'es']) {
  const pg = await b.newPage({ viewport: { width: 980, height: 1500 } });
  pg.on('pageerror', e => { console.log('PAGE ERROR', e.message); fails++; });
  pg.on('dialog', d => d.accept());
  await pg.exposeFunction('fmSend', bytes => dev.stdin.write(Buffer.from(bytes)));
  dev.stdout.removeAllListeners('data');
  dev.stdout.on('data', d => { for (const x of d) { rx.push(x); if (x === 0xF7) { const f = rx; rx = []; pg.evaluate(f => window.fmRx(f), f).catch(() => {}); } } });
  await pg.addInitScript(l => {
    localStorage.setItem('orbita-language', l);
    const input = { name: 'FM-1', state: 'connected', onmidimessage: null }, output = { name: 'FM-1', state: 'connected', send: d => window.fmSend(Array.from(d)) };
    window.fmRx = f => input.onmidimessage && input.onmidimessage({ data: new Uint8Array(f) });
    navigator.requestMIDIAccess = async () => ({ inputs: new Map([['i', input]]), outputs: new Map([['o', output]]) });
  }, lang);
  await pg.goto('http://localhost:8799/fm1/');
  await pg.screenshot({ path: path.join(out, `installer_${lang}_start.png`), fullPage: true });
  await pg.click('#go');
  await pg.waitForFunction(() => /✓|Stopped|Se detuvo|No FM-1|No se encontr/.test(document.getElementById('status').textContent), null, { timeout: 30000 });
  const st = await pg.textContent('#status'), chk = await pg.$$eval('.v.chk', v => v.length);
  console.log(lang, 'install:', st, '| voices checked:', chk, '| device:', await pg.textContent('#dev'));
  if (!/✓/.test(st) || chk !== 32) fails++;
  await pg.screenshot({ path: path.join(out, `installer_${lang}_done.png`), fullPage: true });
  await pg.click('#remove'); await pg.waitForFunction(() => /✓.*(built-in|fábrica)|Stopped|Se detuvo/.test(document.getElementById('status').textContent), null, { timeout: 15000 });
  const st2 = await pg.textContent('#status'); console.log(lang, 'remove:', st2); if (!/✓/.test(st2)) fails++;
  await pg.close();
}
await b.close(); dev.stdin.end(); srv.close();
console.log(fails ? 'page check: FAILED' : 'page check: all ok'); process.exit(fails ? 1 : 0);
