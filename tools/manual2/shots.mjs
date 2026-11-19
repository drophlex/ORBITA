// screenshots of ÓRBITA 2's web edition for its manual (serve docs/ on :8812 first)
import pw from '/opt/npm-tools/node_modules/playwright/index.js'; const { chromium } = pw;
const OUT = process.argv[2] || '/home/claude/man/orbita2/shots';
const b = await chromium.launch({ executablePath: '/usr/bin/google-chrome', args: ['--autoplay-policy=no-user-gesture-required'] });
for (const L of ['en', 'es']) {
  const pg = await b.newPage({ viewport: { width: 1440, height: 940 }, deviceScaleFactor: 2 });
  await pg.addInitScript(l => localStorage.setItem('orbita2', JSON.stringify({ lang: l, coach: true })), L);
  await pg.goto('http://127.0.0.1:8812/'); await pg.waitForTimeout(500);
  await pg.click('#play'); await pg.waitForTimeout(2600);
  await pg.screenshot({ path: `${OUT}/${L}_full.png` });
  await pg.locator('.top').screenshot({ path: `${OUT}/${L}_top.png` });
  await pg.locator('.screen').screenshot({ path: `${OUT}/${L}_screen.png` });
  await pg.locator('.side .card').nth(0).screenshot({ path: `${OUT}/${L}_scenes.png` });
  await pg.locator('.side .card').nth(1).screenshot({ path: `${OUT}/${L}_macros.png` });
  await pg.locator('.side .card').nth(2).screenshot({ path: `${OUT}/${L}_harmony.png` });
  await pg.locator('#lanes').screenshot({ path: `${OUT}/${L}_lanes.png` });
  for (let i = 1; i < 4; i++) { await pg.click(`.sc[data-i="${[1, 4, 6][i - 1]}"]`); await pg.waitForTimeout(1100); await pg.locator('.screen').screenshot({ path: `${OUT}/${L}_geo${i}.png` }); }
  // gravity: Nebula -> Cenote
  await pg.click('.sc[data-i="5"]'); await pg.waitForTimeout(300); await pg.click('#bMode'); await pg.click('.sc[data-i="4"]');
  await pg.evaluate(() => setP(G.GRAVITY, 0.35)); await pg.waitForTimeout(1500);
  await pg.locator('.screen').screenshot({ path: `${OUT}/${L}_gravity.png` });
  await pg.evaluate(() => { setP(G.GRAVITY, 0); clearTarget(); });
  await pg.click('.lane[data-l="2"] .nm'); await pg.waitForTimeout(400);
  await pg.locator('#drawer').screenshot({ path: `${OUT}/${L}_drawer.png` });
  await pg.click('#dClose');
  await pg.evaluate(() => { newOrbit(); }); await pg.waitForTimeout(500); await pg.evaluate(() => { mutate(); }); await pg.waitForTimeout(300); await pg.evaluate(() => { newOrbit(); }); await pg.waitForTimeout(400);
  await pg.locator('.bottom').screenshot({ path: `${OUT}/${L}_bottom.png` });
  await pg.click('#helpBtn'); await pg.waitForTimeout(200); await pg.close();
  const c = await b.newPage({ viewport: { width: 1440, height: 940 }, deviceScaleFactor: 2 });
  await c.addInitScript(l => localStorage.setItem('orbita2', JSON.stringify({ lang: l })), L);
  await c.goto('http://127.0.0.1:8812/'); await c.waitForTimeout(500); await c.locator('#coach .sheet').screenshot({ path: `${OUT}/${L}_coach.png` }); await c.close();
  const ph = await b.newPage({ viewport: { width: 390, height: 844 }, deviceScaleFactor: 2 });
  await ph.addInitScript(l => localStorage.setItem('orbita2', JSON.stringify({ lang: l, coach: true })), L);
  await ph.goto('http://127.0.0.1:8812/'); await ph.click('#play'); await ph.waitForTimeout(1800); await ph.screenshot({ path: `${OUT}/${L}_phone.png` }); await ph.close();
}
await b.close(); console.log('shots ok');
