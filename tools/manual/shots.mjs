// screenshots of the web edition for the manual (serve docs/ on :8777 first)
import { chromium } from '/opt/npm-tools/node_modules/playwright/index.mjs';
const OUT = process.argv[2] || '/home/claude/man/orbita/shots';
const b = await chromium.launch({ executablePath: '/usr/bin/google-chrome', args: ['--autoplay-policy=no-user-gesture-required'] });
for (const L of ['en', 'es']) {
  const pg = await b.newPage({ viewport: { width: 1320, height: 900 }, deviceScaleFactor: 2 });
  await pg.addInitScript(l => localStorage.setItem('orbita-language', l), L);
  await pg.goto('http://localhost:8777/index.html'); await pg.waitForTimeout(400);
  await pg.click('#play'); await pg.waitForTimeout(2200);
  await pg.screenshot({ path: `${OUT}/${L}_full.png` });
  await pg.locator('header').screenshot({ path: `${OUT}/${L}_header.png` });
  await pg.locator('.mast').screenshot({ path: `${OUT}/${L}_mast.png` });
  await pg.locator('.panel').screenshot({ path: `${OUT}/${L}_panel.png` });
  await pg.locator('.seq').screenshot({ path: `${OUT}/${L}_seq.png` });
  for (const [i, n] of [[0, 'liss'], [1, 'rose'], [2, 'epi']]) {
    await pg.selectOption('#shape', { index: i }); await pg.waitForTimeout(700);
    await pg.locator('.screen').screenshot({ path: `${OUT}/${L}_${n}.png` });
  }
  await pg.selectOption('#shape', { index: 0 });
  for (const [i, n] of [[1, 'scene1'], [2, 'scene2'], [3, 'scene3']]) { await pg.selectOption('#scene', { index: i }); await pg.waitForTimeout(600); await pg.locator('.rack').screenshot({ path: `${OUT}/${L}_${n}.png` }); }
  await pg.selectOption('#scene', { index: 0 }); await pg.waitForTimeout(300);
  await pg.locator('.track').first().locator('button').first().click(); await pg.waitForTimeout(600);
  await pg.locator('.rack').screenshot({ path: `${OUT}/${L}_mute.png` });
  await pg.click('#play'); await pg.click('summary'); await pg.waitForTimeout(300);
  await pg.locator('details').screenshot({ path: `${OUT}/${L}_conn.png` });
  await pg.locator('footer').screenshot({ path: `${OUT}/${L}_footer.png` });
  await pg.close();
  const ph = await b.newPage({ viewport: { width: 390, height: 844 }, deviceScaleFactor: 2 });
  await ph.addInitScript(l => localStorage.setItem('orbita-language', l), L);
  await ph.goto('http://localhost:8777/index.html'); await ph.click('#play'); await ph.waitForTimeout(1800);
  await ph.screenshot({ path: `${OUT}/${L}_phone.png` }); await ph.close();
  const gd = await b.newPage({ viewport: { width: 1100, height: 900 }, deviceScaleFactor: 2 });
  await gd.addInitScript(l => localStorage.setItem('orbita-language', l), L);
  await gd.goto('http://localhost:8777/guide/'); await gd.waitForTimeout(500);
  await gd.locator('#mac').screenshot({ path: `${OUT}/${L}_guide_mac.png` });
  await gd.screenshot({ path: `${OUT}/${L}_guide.png` }); await gd.close();
}
await b.close();
