// the web version's voice() (docs/app.js), rendered by Chrome's own WebAudio offline: note 50, ratio 2, 112 BPM, the three voices
import { chromium } from '/opt/npm-tools/node_modules/playwright/index.mjs';
import fs from 'fs';
const out = process.argv[2];
const b = await chromium.launch({ executablePath: '/usr/bin/google-chrome' });
const p = await b.newPage();
const [R2, KEY, BPM] = [+(process.env.R2 || 4), +(process.env.KEY || 50), +(process.env.BPM || 112)];
const res = await p.evaluate(async ([R2, KEY, BPM]) => {
  const outs = [];
  for (let v = 0; v < 3; v++) {
    const ctx = new OfflineAudioContext(1, 44100, 44100), bus = ctx.createGain(); bus.gain.value = .75; bus.connect(ctx.destination);
    const ratio = R2 / 2, key = KEY, t = 128 / 44100, d = 15 / BPM;
    // ---- docs/app.js voice(v,key,t,d), unchanged
    let c=ctx.createOscillator(),m=ctx.createOscillator(),depth=ctx.createGain(),env=ctx.createGain();let f=440*2**((key-69)/12),tail=d*[1.9,2.8,3.6][v];c.frequency.value=f;m.frequency.value=f*ratio;depth.gain.setValueAtTime(f*[.45,1.7,2.7][v],t);depth.gain.exponentialRampToValueAtTime(.01,t+tail);m.connect(depth).connect(c.frequency);c.connect(env).connect(bus);env.gain.setValueAtTime(.0001,t);env.gain.exponentialRampToValueAtTime([.26,.13,.1][v],t+.008);env.gain.exponentialRampToValueAtTime(.0001,t+tail);c.start(t);m.start(t);c.stop(t+tail+.01);m.stop(t+tail+.01);
    const buf = await ctx.startRendering();
    outs.push(Array.from(buf.getChannelData(0)));
  }
  return outs;
}, [R2, KEY, BPM]);
res.forEach((a, v) => fs.writeFileSync(`${out}/web_${v}.f32`, Buffer.from(new Float32Array(a).buffer)));
await b.close();
