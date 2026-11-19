/* app.js — ÓRBITA 2's interface: the phase plane, scenes, GRAVITY, the lanes, the time machine, the keys */
'use strict';
const $ = s => document.querySelector(s), $$ = s => [...document.querySelectorAll(s)];
const G = { BPM: 0, SWING: 1, ROOT: 2, SCALE: 3, GEOM: 4, DENSITY: 5, PHASE: 6, RATIO: 7, GRAVITY: 8, EVOLVE: 9, CHAOS: 10, ECHO: 11, ECHOTIME: 12, ECHOFB: 13, ECHOTONE: 14, SPACE: 15,
  SIZE: 16, DAMP: 17, CHORUS: 18, WIDTH: 19, WARMTH: 20, TILT: 21, MASTER: 22, KEYMODE: 23, PLAYLANE: 24, HUMAN: 25, BLOOM: 26 }, NG = 32;
const L = { ON: 0, LEVEL: 1, PAN: 2, PULSES: 3, STEPS: 4, ROT: 5, OCT: 6, PROB: 7, GATE: 8, RATIO: 9, INDEX: 10, FB: 11, ENVIDX: 12, ATK: 13, DEC: 14, SUS: 15, REL: 16,
  CUT: 17, RES: 18, ECHO: 19, SPACE: 20, DETUNE: 21, VEL: 22, MODEL: 23 }, NLP = 24, NL = 4;
const LP = (l, k) => NG + l * NLP + k;
const TAU = Math.PI * 2;
const COLORS = ['#ceff78', '#af9af1', '#f1a07d', '#7fd6ee'];
const MODELS = ['SUB', 'PRISM', 'CRYSTAL', 'HALO', 'GLASS', 'DUST'];
const LANE_NAMES = ['Sub bass', 'Prism', 'Crystal', 'Halo'];
const SCALES = ['Dorian', 'Minor', 'Minor pentatonic', 'Phrygian', 'Lydian', 'Major pentatonic', 'Phrygian dominant', 'Whole tone', 'Harmonic minor', 'Hirajoshi'];
const GEOMS = ['Lissajous', 'Polar rose', 'Epicycle', 'Spiral'];
const NOTES = ['C', 'C♯', 'D', 'E♭', 'E', 'F', 'F♯', 'G', 'A♭', 'A', 'B♭', 'B'];
const ECHO_DIV = ['1/16', '1/8', '3/16', '1/4', '3/8', '1/2', '1/8T', '1/4T'];

Mirror.init();
const NP = Mirror.X.ox_param_count();
const DEF = Float32Array.from(Mirror.P);
let P = Float32Array.from(DEF);
let custom = [null, null, null, null];           // a lane's own steps (bit mask) when you have clicked them
let target = null, targetName = '', bMode = false;
let curScene = 0, sceneTitle = '', sceneText = '';
let playing = false, tele = new Float32Array(64), scope = [new Float32Array(256), new Float32Array(256)];
let selLane = -1, userScenes = [null, null, null, null], solo = -1;
const plugin = Bridge.isPlugin();
if (plugin) document.documentElement.classList.add('isplug');

// ------------------------------------------------------------------ scenes (the first four are ÓRBITA 1's, note for note)
function S(name, es, desc, descEs, g, lanes) { return { name, es, desc, descEs, g, lanes: lanes || [] }; }
const SCENES = [
  S('Mérida Ring', 'Anillo de Mérida', 'A spaced sub bass holds two bells that cross.', 'Un grave espaciado sostiene dos campanas que se cruzan.',
    { BPM: 112, GEOM: 0, SCALE: 0, ROOT: 2, DENSITY: 7, PHASE: 2, RATIO: 2 }, [{}, {}, {}, { LEVEL: 0.32 }]),
  S('Skewed Crystal', 'Cristal sesgado', 'Metallic off-beat attacks in a five-petal rose.', 'Ataques metálicos a contratiempo en una rosa de cinco pétalos.',
    { BPM: 136, GEOM: 1, SCALE: 1, ROOT: 5, DENSITY: 10, PHASE: 5, RATIO: 4, ECHO: 0.28, ECHOTIME: 2 }, [{}, {}, {}, { LEVEL: 0.28, MODEL: 4 }]),
  S('Lunar Drift', 'Deriva lunar', 'A slow pulse and open resonances on a pentatonic scale.', 'Un pulso lento y resonancias abiertas sobre una escala pentatónica.',
    { BPM: 74, GEOM: 2, SCALE: 2, ROOT: 9, DENSITY: 4, PHASE: 3, RATIO: 1.5, SPACE: 0.5, SIZE: 0.75 }, [{}, {}, {}, { LEVEL: 0.45 }]),
  S('Discrete Acid', 'Ácido discreto', 'A dense web of sixteenths with bright modulation.', 'Una trama densa de semicorcheas con modulación brillante.',
    { BPM: 148, GEOM: 0, SCALE: 0, ROOT: 2, DENSITY: 12, PHASE: 9, RATIO: 3.5, WARMTH: 0.3, SPACE: 0.18 }, [{ CUT: 0.55, RES: 0.35 }, {}, {}, { ON: 0 }]),
  S('Cenote', 'Cenote', 'Glass drops falling into deep water, a spiral that never repeats.', 'Gotas de vidrio cayendo en agua profunda, una espiral que nunca se repite.',
    { BPM: 92, GEOM: 3, SCALE: 5, ROOT: 7, DENSITY: 6, PHASE: 3, RATIO: 1, SPACE: 0.62, SIZE: 0.86, ECHO: 0.3, ECHOTIME: 3, EVOLVE: 0.35, CHORUS: 0.4 },
    [{ LEVEL: 0.78 }, { MODEL: 4, INDEX: 0.7 }, { MODEL: 2, STEPS: 12 }, { LEVEL: 0.55, STEPS: 10 }]),
  S('Nebula Polymeter', 'Nebulosa polimétrica', 'Four cycles of 16, 12, 10 and 7 steps: they meet again every few minutes.', 'Cuatro ciclos de 16, 12, 10 y 7 pasos: se vuelven a encontrar cada pocos minutos.',
    { BPM: 120, GEOM: 2, SCALE: 4, ROOT: 0, DENSITY: 7, PHASE: 1, RATIO: 2.5, ECHO: 0.34, ECHOTIME: 1, ECHOFB: 0.5, CHORUS: 0.5, SPACE: 0.4 },
    [{}, { STEPS: 12 }, { STEPS: 10, MODEL: 4 }, { STEPS: 7, PULSES: -3, LEVEL: 0.48 }]),
  S('Hijaz Ritual', 'Ritual Hijaz', 'Phrygian dominant, dust and a rose that turns: a ceremony at night.', 'Frigio dominante, polvo y una rosa que gira: una ceremonia nocturna.',
    { BPM: 104, GEOM: 1, SCALE: 6, ROOT: 4, DENSITY: 9, PHASE: 6, RATIO: 3, CHAOS: 0.2, SPACE: 0.42, WARMTH: 0.25 },
    [{}, { DETUNE: 0.3 }, { MODEL: 5, LEVEL: 0.6 }, { MODEL: 3, LEVEL: 0.4 }]),
  S('Deep Orbit', 'Órbita profunda', 'Sub-heavy, 126 BPM, echoes in eighths: a techno orbit for the club.', 'Grave al frente, 126 BPM, ecos en corcheas: una órbita techno para el club.',
    { BPM: 126, GEOM: 0, SCALE: 1, ROOT: 9, DENSITY: 8, PHASE: 4, RATIO: 0.5, WARMTH: 0.35, ECHO: 0.3, ECHOTIME: 1, ECHOFB: 0.48, SPACE: 0.25 },
    [{ PULSES: -4, LEVEL: 0.9, CUT: 0.5, RES: 0.3 }, { RATIO: 2 }, { RATIO: 3 }, { LEVEL: 0.3 }])
];
function buildScene(sc) {
  const p = Float32Array.from(DEF);
  for (const k in sc.g) p[G[k]] = sc.g[k];
  sc.lanes.forEach((o, l) => { for (const k in o) p[LP(l, L[k])] = o[k]; });
  // keep what belongs to the player, not to the scene
  for (const id of [G.MASTER, G.KEYMODE, G.PLAYLANE, G.GRAVITY]) p[id] = P[id];
  if (plugin && syncOn) p[G.BPM] = P[G.BPM];
  return p;
}

// ------------------------------------------------------------------ parameters
function rng(id) { return Mirror.range(id); }
function clampP(id, v) { const [a, b] = rng(id); return Math.min(b, Math.max(a, v)); }
let saveT = 0, histT = 0;
function setP(id, v, opt = {}) {
  v = clampP(id, v); if (P[id] === v && !opt.force) return;
  P[id] = v; Mirror.X.ox_param(id, v);
  if (!opt.quiet) Bridge.send({ t: 'param', id, v });
  const lk = id >= NG ? (id - NG) % NLP : -1;
  if (id === G.DENSITY || id === G.PHASE) custom = custom.map(() => null);
  if (lk === L.PULSES || lk === L.STEPS || lk === L.ROT) custom[(id - NG) / NLP | 0] = null;
  if (!opt.noUi) refreshSoon();
  if (!opt.noHist && id !== G.GRAVITY) { clearTimeout(histT); histT = setTimeout(() => commit(), 1400); }
  saveSoon();
}
function pushAll() {
  for (let i = 0; i < NP; i++) Mirror.X.ox_param(i, P[i]);
  Bridge.send({ t: 'params', list: [...P].map((v, i) => [i, v]) });
  custom.forEach((m, l) => { if (m !== null) { Mirror.X.ox_custom(l, m); Bridge.send({ t: 'custom', lane: l, mask: m }); } });
}
function loadScene(i, opt = {}) {
  if (bMode && !opt.force) { setTarget(i); return; }
  const sc = i < SCENES.length ? SCENES[i] : null;
  if (!sc) { const u = userScenes[i - SCENES.length]; if (!u) return; P = Float32Array.from(u.P); custom = u.custom.slice(); sceneTitle = u.name; sceneText = T('slot') + ' ' + (i - SCENES.length + 1); }
  else { P = buildScene(sc); custom = [null, null, null, null]; sceneTitle = LANG === 'es' ? sc.es : sc.name; sceneText = LANG === 'es' ? sc.descEs : sc.desc; }
  curScene = i; P[G.GRAVITY] = target ? P[G.GRAVITY] : 0;
  pushAll(); Bridge.send({ t: 'evo', p: 0, d: 0, r: 0 });
  refresh(); commit(); saveSoon();
}
function sceneParams(i) { if (i < SCENES.length) return buildScene(SCENES[i]); const u = userScenes[i - SCENES.length]; return u ? Float32Array.from(u.P) : null; }
function setTarget(i) {
  bMode = false; $('#bMode').classList.remove('on');
  const p = sceneParams(i); if (!p) return;
  target = i; targetName = i < SCENES.length ? (LANG === 'es' ? SCENES[i].es : SCENES[i].name) : userScenes[i - SCENES.length].name;
  const list = [...p].map((v, id) => [id, v]);
  for (const [id, v] of list) Mirror.X.ox_target(id, v);
  Bridge.send({ t: 'target', list });
  toast(T('Gravity target') + ': ' + targetName); refresh(); saveSoon();
}
function clearTarget() { target = null; Mirror.X.ox_clear_target(); Bridge.send({ t: 'clearTarget' }); setP(G.GRAVITY, 0); toast(T('Target cleared')); refresh(); }

// ------------------------------------------------------------------ knobs
const knobs = [];
function fmtDefault(id, v) { const [a, b] = rng(id); return (b - a) > 20 || Number.isInteger(a) && Number.isInteger(b) && (b - a) <= 16 && id !== G.RATIO ? String(Math.round(v)) : Math.round((v - a) / (b - a) * 100) + ''; }
function makeKnob(parent, id, label, o = {}) {
  const el = document.createElement('div'); el.className = 'knob' + (o.hero ? ' hero' : ''); el.tabIndex = 0;
  el.innerHTML = `<svg viewBox="-30 -30 60 60"><circle r="22" fill="none" stroke="currentColor" stroke-opacity=".14" stroke-width="5"/><path class="arc" fill="none" stroke-width="5" stroke-linecap="round"/><circle class="cap" r="15" fill="var(--paper)" stroke="currentColor" stroke-opacity=".28"/><line class="ptr" x1="0" y1="-6" x2="0" y2="-14" stroke="currentColor" stroke-width="2.4" stroke-linecap="round"/></svg><div class="lbl">${T(label)}</div><div class="val"></div>`;
  el.dataset.label = label; parent.append(el);
  const k = { el, id: typeof id === 'function' ? id : () => id, label, fmt: o.fmt, col: o.col, step: o.step };
  const get = () => P[k.id()], [lo, hi] = rng(k.id());
  let startY = 0, startV = 0, drag = false;
  el.addEventListener('pointerdown', e => { drag = true; startY = e.clientY; startV = get(); el.setPointerCapture(e.pointerId); e.preventDefault(); });
  el.addEventListener('pointermove', e => { if (!drag) return; const [a, b] = rng(k.id()); const sens = (e.shiftKey ? 0.15 : 1) * (b - a) / 180; let v = startV + (startY - e.clientY) * sens; if (k.step) v = Math.round(v / k.step) * k.step; setP(k.id(), v); });
  el.addEventListener('pointerup', () => drag = false);
  el.addEventListener('dblclick', () => setP(k.id(), k.reset ? k.reset() : DEF[k.id()]));
  el.addEventListener('wheel', e => { e.preventDefault(); const [a, b] = rng(k.id()); let d = (b - a) / 100 * (e.deltaY < 0 ? 1 : -1); if (k.step) d = Math.sign(d) * k.step; setP(k.id(), get() + d); }, { passive: false });
  el.addEventListener('keydown', e => { const [a, b] = rng(k.id()); const d = k.step || (b - a) / 50; if (e.key === 'ArrowUp' || e.key === 'ArrowRight') { setP(k.id(), get() + d); e.preventDefault(); } if (e.key === 'ArrowDown' || e.key === 'ArrowLeft') { setP(k.id(), get() - d); e.preventDefault(); } });
  k.update = () => {
    const id = k.id(), [a, b] = rng(id), v = P[id], t = (v - a) / (b - a), A0 = -135, A1 = A0 + 270 * t;
    const pt = d => [22 * Math.sin(d * Math.PI / 180), -22 * Math.cos(d * Math.PI / 180)];
    const [x0, y0] = pt(A0), [x1, y1] = pt(A1);
    el.querySelector('.arc').setAttribute('d', t <= 0.001 ? '' : `M${x0} ${y0} A22 22 0 ${A1 - A0 > 180 ? 1 : 0} 1 ${x1} ${y1}`);
    el.querySelector('.arc').setAttribute('stroke', k.col ? k.col() : 'var(--ink)');
    el.querySelector('.ptr').setAttribute('transform', `rotate(${A1})`);
    el.querySelector('.val').textContent = k.fmt ? k.fmt(v) : fmtDefault(id, v);
    el.querySelector('.lbl').textContent = T(k.label);
    el.title = T(k.label);
  };
  knobs.push(k); k.update(); return k;
}
const pct = v => Math.round(v * 100) + '%';
function buildMacros() {
  const m = $('#macros'); m.innerHTML = '';
  makeKnob(m, G.DENSITY, 'DENSITY', { step: 1, fmt: v => v.toFixed(0) });
  makeKnob(m, G.PHASE, 'PHASE', { step: 1, fmt: v => v.toFixed(0) });
  makeKnob(m, G.RATIO, 'FM RATIO', { step: 0.25, fmt: v => v.toFixed(2) });
  makeKnob(m, G.GRAVITY, 'GRAVITY', { hero: true, fmt: pct, col: () => target !== null ? '#8466aa' : 'var(--mute)' });
  makeKnob(m, G.EVOLVE, 'EVOLVE', { fmt: pct });
  makeKnob(m, G.CHAOS, 'CHAOS', { fmt: pct });
  makeKnob(m, G.BLOOM, 'BLOOM', { fmt: pct });
  makeKnob(m, G.SPACE, 'SPACE', { fmt: pct });
  makeKnob(m, G.ECHO, 'ECHO', { fmt: pct });
  makeKnob(m, G.WARMTH, 'WARMTH', { fmt: pct });
  const more = document.createElement('details'); more.style.gridColumn = '1/-1'; more.innerHTML = `<summary class="mono" style="font-size:10.5px;cursor:pointer;margin-top:4px">${T('FX & MIX')}</summary><div class="knobs" style="margin-top:8px"></div>`;
  m.append(more); const f = more.querySelector('.knobs');
  makeKnob(f, G.ECHOTIME, 'ECHO TIME', { step: 1, fmt: v => ECHO_DIV[Math.round(v)] });
  makeKnob(f, G.ECHOFB, 'FEEDBACK', { fmt: pct }); makeKnob(f, G.ECHOTONE, 'ECHO TONE', { fmt: pct });
  makeKnob(f, G.SIZE, 'SIZE', { fmt: pct }); makeKnob(f, G.DAMP, 'DAMP', { fmt: pct });
  makeKnob(f, G.CHORUS, 'CHORUS', { fmt: pct }); makeKnob(f, G.WIDTH, 'WIDTH', { fmt: v => Math.round(v * 200) + '%' });
  makeKnob(f, G.SWING, 'SWING', { fmt: pct }); makeKnob(f, G.HUMAN, 'HUMAN', { fmt: pct });
  makeKnob(f, G.TILT, 'TILT', { fmt: v => (v > 0 ? '+' : '') + v.toFixed(2) }); makeKnob(f, G.MASTER, 'MASTER', { fmt: pct });
}
function buildDrawer() {
  const d = $('#dKnobs'); d.innerHTML = ''; const lid = k => () => LP(Math.max(0, selLane), k), col = () => COLORS[Math.max(0, selLane)];
  const list = [['RATIO', L.RATIO, v => '×' + v.toFixed(2)], ['INDEX', L.INDEX, pct], ['FEEDBACK', L.FB, pct], ['MOD ENV', L.ENVIDX, pct], ['ATTACK', L.ATK, pct], ['DECAY', L.DEC, pct],
    ['SUSTAIN', L.SUS, pct], ['RELEASE', L.REL, pct], ['GATE', L.GATE, v => v.toFixed(1)], ['CUTOFF', L.CUT, pct], ['RESONANCE', L.RES, pct], ['DETUNE', L.DETUNE, pct],
    ['ECHO SEND', L.ECHO, pct], ['SPACE SEND', L.SPACE, pct], ['CHANCE', L.PROB, pct], ['ACCENT', L.VEL, pct], ['PAN', L.PAN, v => v === 0 ? 'C' : (v < 0 ? 'L' : 'R') + Math.round(Math.abs(v) * 100)], ['LEVEL', L.LEVEL, pct]];
  for (const [lab, k, fmt] of list) { const kn = makeKnob(d, lid(k), lab, { fmt, col }); kn.reset = () => DEF[LP(Math.max(0, selLane), k)]; }
}

// ------------------------------------------------------------------ lanes
const minis = [];
function makeMini(parent, idf, label, fmt) {
  const el = document.createElement('div'); el.className = 'mini'; el.tabIndex = 0; el.innerHTML = `<span>${T(label)}</span><b></b>`; parent.append(el);
  let sy = 0, sv = 0, drag = false, moved = false;
  el.addEventListener('pointerdown', e => { drag = true; moved = false; sy = e.clientY; sv = P[idf()]; el.setPointerCapture(e.pointerId); });
  el.addEventListener('pointermove', e => { if (!drag) return; const dy = sy - e.clientY; if (Math.abs(dy) > 3) moved = true; const [a, b] = rng(idf()); const step = (b - a) > 3 ? 1 : 0.05; setP(idf(), Math.round((sv + dy / (b - a > 3 ? 8 : 120) * ((b - a) > 3 ? 1 : 1)) / step) * step); });
  el.addEventListener('pointerup', () => { drag = false; });
  el.addEventListener('wheel', e => { e.preventDefault(); const [a, b] = rng(idf()); const step = (b - a) > 3 ? 1 : 0.05; setP(idf(), P[idf()] + (e.deltaY < 0 ? step : -step)); }, { passive: false });
  el.addEventListener('dblclick', () => setP(idf(), DEF[idf()]));
  el.addEventListener('keydown', e => { const [a, b] = rng(idf()); const step = (b - a) > 3 ? 1 : 0.05; if (e.key === 'ArrowUp') setP(idf(), P[idf()] + step); if (e.key === 'ArrowDown') setP(idf(), P[idf()] - step); });
  const m = { el, update: () => { el.querySelector('b').textContent = fmt(P[idf()]); el.querySelector('span').textContent = T(label); } }; minis.push(m); m.update(); return m;
}
function laneMask(l) { return Mirror.X.ox_mask(l); }
function buildLanes() {
  const box = $('#lanes'); box.innerHTML = ''; minis.length = 0;
  for (let l = 0; l < NL; l++) {
    const row = document.createElement('div'); row.className = 'lane'; row.style.setProperty('--col', COLORS[l]); row.dataset.l = l;
    row.innerHTML = `<div class="lname"><button class="mbtn m" title="${T('mute')}">M</button><button class="mbtn s" title="${T('solo')}">S</button><button class="nm"><i></i><span><span class="lnm"></span><br><small class="mdl"></small></span></button></div><div class="cells"></div><div class="lctl"></div>`;
    const cells = row.querySelector('.cells');
    for (let s = 0; s < 16; s++) { const c = document.createElement('button'); c.className = 'cell' + (s % 4 === 0 ? ' beat' : ''); c.setAttribute('aria-label', T(LANE_NAMES[l]) + ' ' + (s + 1));
      c.onclick = () => toggleStep(l, s); cells.append(c); }
    row.querySelector('.m').onclick = () => setP(LP(l, L.ON), P[LP(l, L.ON)] > 0.5 ? 0 : 1);
    row.querySelector('.s').onclick = () => doSolo(l);
    row.querySelector('.nm').onclick = () => openDrawer(selLane === l && $('#drawer').classList.contains('open') ? -1 : l);
    const ctl = row.querySelector('.lctl');
    makeMini(ctl, () => LP(l, L.PULSES), 'PULSES', v => String(Math.max(1, Math.min(P[LP(l, L.STEPS)], Math.round(P[G.DENSITY] + v)))));
    makeMini(ctl, () => LP(l, L.STEPS), 'STEPS', v => v.toFixed(0));
    makeMini(ctl, () => LP(l, L.ROT), 'ROTATE', v => (v > 0 ? '+' : '') + v.toFixed(0));
    makeMini(ctl, () => LP(l, L.OCT), 'OCT', v => (v > 0 ? '+' : '') + v.toFixed(0));
    makeMini(ctl, () => LP(l, L.LEVEL), 'LEVEL', v => Math.round(v * 100));
    box.append(row);
  }
}
function toggleStep(l, s) {
  const steps = P[LP(l, L.STEPS)]; if (s >= steps) return;
  const m = (custom[l] !== null ? custom[l] : laneMask(l)) ^ (1 << s);
  custom[l] = m; Mirror.X.ox_custom(l, m); Bridge.send({ t: 'custom', lane: l, mask: m });
  if (!playing) Bridge.send({ t: 'audition', lane: l, note: Mirror.X.ox_note(l, s), vel: 100 });
  refreshSoon(); clearTimeout(histT); histT = setTimeout(() => commit(), 1400); saveSoon();
}
let soloSaved = null;
function doSolo(l) {
  if (solo === l) { if (soloSaved) soloSaved.forEach((v, k) => setP(LP(k, L.ON), v)); solo = -1; soloSaved = null; }
  else { if (!soloSaved) soloSaved = [0, 1, 2, 3].map(k => P[LP(k, L.ON)]); solo = l; for (let k = 0; k < NL; k++) setP(LP(k, L.ON), k === l ? 1 : 0); }
  refresh();
}
function openDrawer(l) {
  selLane = l; const d = $('#drawer');
  if (l < 0) { d.classList.remove('open'); refresh(); return; }
  d.classList.add('open'); $('#dDot').style.background = COLORS[l]; $('#dName').textContent = T(LANE_NAMES[l]).toUpperCase();
  refresh();
}

// ------------------------------------------------------------------ harmony controls
function buildSelects() {
  const fill = (sel, arr, tr) => { sel.innerHTML = ''; arr.forEach((n, i) => sel.add(new Option(tr ? T(n) : n, i))); };
  fill($('#root'), NOTES); fill($('#scale'), SCALES, true); fill($('#geom'), GEOMS, true); fill($('#playLane'), LANE_NAMES, true); fill($('#dModel'), MODELS, true);
}
$('#root').onchange = e => setP(G.ROOT, +e.target.value);
$('#scale').onchange = e => setP(G.SCALE, +e.target.value);
$('#geom').onchange = e => setP(G.GEOM, +e.target.value);
$('#playLane').onchange = e => setP(G.PLAYLANE, +e.target.value);
$('#dModel').onchange = e => { if (selLane >= 0) setP(LP(selLane, L.MODEL), +e.target.value); };
$('#dReset').onclick = () => { if (selLane < 0) return; for (let k = 1; k < NLP; k++) if (![L.PULSES, L.STEPS, L.ROT, L.OCT].includes(k)) setP(LP(selLane, k), DEF[LP(selLane, k)]); };
$('#dClose').onclick = () => openDrawer(-1);
$$('#keyMode button').forEach(b => b.onclick = () => setP(G.KEYMODE, +b.dataset.v));

// ------------------------------------------------------------------ refresh
let refT = 0;
function refreshSoon() { if (!refT) refT = requestAnimationFrame(() => { refT = 0; refresh(); }); }
function refresh() {
  knobs.forEach(k => k.update()); minis.forEach(m => m.update());
  $('#bpm').value = Math.round(P[G.BPM]);
  $('#root').value = P[G.ROOT]; $('#scale').value = P[G.SCALE]; $('#geom').value = P[G.GEOM]; $('#playLane').value = P[G.PLAYLANE];
  $$('#keyMode button').forEach(b => b.classList.toggle('on', +b.dataset.v === Math.round(P[G.KEYMODE])));
  const km = Math.round(P[G.KEYMODE]);
  $('#keyHint').textContent = km === 0 ? T('The keys play') + ': ' + T(LANE_NAMES[Math.round(P[G.PLAYLANE])]) : km === 1 ? T('A key sets the root: the whole orbit follows it.') : T('Hold notes: they become the scale the orbit plays.');
  $('#geoName').textContent = T(GEOMS[Math.round(P[G.GEOM])]).toUpperCase() + ' · ' + NOTES[Math.round(P[G.ROOT])] + ' ' + T(SCALES[Math.round(P[G.SCALE])]).toUpperCase();
  $('#sceneName').textContent = sceneTitle; $('#sceneDesc').textContent = sceneText;
  $$('.lane').forEach(row => {
    const l = +row.dataset.l, on = P[LP(l, L.ON)] > 0.5, steps = Math.round(P[LP(l, L.STEPS)]), m = laneMask(l);
    row.classList.toggle('off', !on); row.classList.toggle('sel', l === selLane && $('#drawer').classList.contains('open'));
    row.querySelector('.m').classList.toggle('on', !on); row.querySelector('.s').classList.toggle('on', solo === l);
    row.querySelector('.lnm').textContent = T(LANE_NAMES[l]); row.querySelector('.mdl').textContent = T(MODELS[Math.round(P[LP(l, L.MODEL)])]) + (custom[l] !== null ? ' · ✎' : '');
    [...row.querySelectorAll('.cell')].forEach((c, s) => { c.classList.toggle('x', s >= steps); c.classList.toggle('on', s < steps && !!(m >> s & 1)); });
  });
  if (selLane >= 0) $('#dModel').value = Math.round(P[LP(selLane, L.MODEL)]);
  // chips on the screen
  const chips = $('#chips'); if (!chips.children.length) for (let l = 0; l < NL; l++) { const c = document.createElement('button'); c.className = 'chip'; c.style.setProperty('--col', COLORS[l]); c.innerHTML = '<i></i><span></span>'; c.onclick = () => setP(LP(l, L.ON), P[LP(l, L.ON)] > 0.5 ? 0 : 1); chips.append(c); }
  [...chips.children].forEach((c, l) => { c.querySelector('span').textContent = T(LANE_NAMES[l]); c.classList.toggle('off', P[LP(l, L.ON)] < 0.5); });
  // scenes
  $$('.sc').forEach(b => { const i = +b.dataset.i; b.classList.toggle('cur', i === curScene); b.classList.toggle('tgt', i === target); });
  $('#gravbar').classList.toggle('show', target !== null); $('#gravVal').textContent = pct(P[G.GRAVITY]); $('#gravTgt').textContent = targetName;
  $('#play').classList.toggle('on', playing); $('#play').innerHTML = (playing ? '■&nbsp; ' : '▶&nbsp; ') + '<span>' + T(playing ? 'Stop' : 'Play') + '</span>';
}
function buildScenes() {
  const box = $('#scenes'); box.innerHTML = '';
  const all = SCENES.length + userScenes.length;
  for (let i = 0; i < all; i++) {
    const b = document.createElement('button'), u = i >= SCENES.length ? userScenes[i - SCENES.length] : null;
    b.className = 'sc' + (i >= SCENES.length ? ' user' + (u ? ' full' : '') : ''); b.dataset.i = i;
    const nm = i < SCENES.length ? (LANE_NAMES && LANG === 'es' ? SCENES[i].es : SCENES[i].name) : (u ? u.name : '★ ' + T('slot') + ' ' + (i - SCENES.length + 1));
    const sub = i < SCENES.length ? Math.round(buildScene(SCENES[i])[G.BPM]) + ' BPM' : (u ? Math.round(u.P[G.BPM]) + ' BPM' : T('empty'));
    b.innerHTML = `<canvas width="52" height="52"></canvas><b>${nm}</b><small>${sub}</small>`;
    b.onclick = () => loadScene(i);
    b.oncontextmenu = e => { e.preventDefault(); setTarget(i); };
    box.append(b);
    const p = sceneParams(i); if (p) thumb(b.querySelector('canvas'), p, true);
  }
}
$('#bMode').onclick = () => { bMode = !bMode; $('#bMode').classList.toggle('on', bMode); if (bMode) toast(T('Pick the scene for B')); };
$('#clearB').onclick = clearTarget;
$('#saveScene').onclick = () => {
  let k = userScenes.findIndex(u => !u); if (k < 0) k = (saveRot = (saveRot + 1) % userScenes.length);
  const name = (sceneTitle || 'ÓRBITA').replace(/^★ /, '') + ' ★';
  userScenes[k] = { name, P: [...P], custom: custom.slice() }; buildScenes(); refresh(); saveSoon(); toast(T('Saved in slot') + ' ' + (k + 1));
};
let saveRot = -1;

// ------------------------------------------------------------------ the orbit geometry (ÓRBITA 1's curves, plus the spiral)
function orbitPt(geom, a, v, p, k) {
  switch (geom) {
    case 0: return [Math.sin(2 * a + p + k), Math.sin(3 * a + v * 0.3)];
    case 1: { const r = Math.cos(5 * a + p + k); return [r * Math.cos(a), r * Math.sin(a)]; }
    case 2: return [0.7 * Math.cos(a) + 0.3 * Math.cos(4 * a + p + k), 0.7 * Math.sin(a) - 0.3 * Math.sin(4 * a + p + k)];
    default: { const r = 0.55 + 0.45 * Math.sin(3 * a + p + k); return [r * Math.cos(a + 0.4 * Math.sin(2 * a)), r * Math.sin(a + 0.4 * Math.sin(2 * a)) * 0.95]; }
  }
}
function thumb(cv, p, small) {
  const g = cv.getContext('2d'), w = cv.width, h = cv.height; g.clearRect(0, 0, w, h);
  if (!small) { g.fillStyle = '#121512'; g.fillRect(0, 0, w, h); }
  const geom = Math.round(p[G.GEOM]), ph = Math.round(p[G.PHASE]), R = w * 0.36;
  for (let v = 0; v < NL; v++) { if (p[LP(v, L.ON)] < 0.5) continue; g.strokeStyle = small ? (v === 0 ? '#3d4a2a' : v === 1 ? '#6a5aa0' : v === 2 ? '#b4704c' : '#3d8aa0') : COLORS[v]; g.globalAlpha = small ? 0.9 : 0.85; g.lineWidth = small ? 1.4 : 1.2;
    g.beginPath(); for (let j = 0; j <= 120; j++) { const [x, y] = orbitPt(geom, j * TAU / 120, v, ph * TAU / 16 + v * 0.44, 0); const X = w / 2 + x * R * 1.2, Y = h / 2 + y * R * (1 - v * 0.065); j ? g.lineTo(X, Y) : g.moveTo(X, Y); } g.stroke(); }
  g.globalAlpha = 1;
}

// ------------------------------------------------------------------ the stage
const cv = $('#stage'), gx = cv.getContext('2d');
const stars = Array.from({ length: 140 }, () => [Math.random(), Math.random(), Math.random() * 1.2 + 0.2, Math.random() * TAU]);
const parts = [], rings = [], MONO_FONT = '10px ui-monospace, SFMono-Regular, Menlo, monospace';
let lastStep = [-1, -1, -1, -1], dots = [];
function draw(ts) {
  const w = cv.clientWidth, h = cv.clientHeight, px = Math.min(devicePixelRatio || 1, 2);
  if (cv.width !== Math.round(w * px) || cv.height !== Math.round(h * px)) { cv.width = Math.round(w * px); cv.height = Math.round(h * px); }
  gx.setTransform(px, 0, 0, px, 0, 0); gx.clearRect(0, 0, w, h);
  const E = Mirror.eff(), geom = Math.round(E[G.GEOM]), t = ts / 1000;
  const phaseE = Math.round(E[G.PHASE]) + (playing ? tele[5] : 0), ratioE = E[G.RATIO] + (playing ? tele[7] : 0);
  const cx = w / 2 + (w > 700 ? w * 0.06 : 0), cy = h / 2 + 8, R = Math.min(w * 0.33, h * 0.36);
  // the sky: stars and polar rings
  for (const s of stars) { gx.globalAlpha = 0.18 + 0.22 * Math.sin(t * s[2] + s[3]) ** 2; gx.fillStyle = '#dfe6d4'; gx.fillRect(s[0] * w, s[1] * h, s[2], s[2]); }
  gx.globalAlpha = 1; gx.strokeStyle = '#ffffff10'; gx.lineWidth = 1;
  for (let r = 1; r <= 4; r++) { gx.beginPath(); gx.arc(cx, cy, R * r * 0.34, 0, TAU); gx.stroke(); }
  gx.save(); gx.translate(cx, cy); gx.rotate(t * 0.02); gx.beginPath(); for (let k = 0; k < 16; k++) { const a = k * TAU / 16; gx.moveTo(Math.cos(a) * R * 0.2, Math.sin(a) * R * 0.2); gx.lineTo(Math.cos(a) * R * 1.45, Math.sin(a) * R * 1.45); } gx.strokeStyle = '#ffffff07'; gx.stroke(); gx.restore();
  // the core: the sound itself, drawn as an XY figure
  gx.globalCompositeOperation = 'lighter';
  gx.beginPath(); const sL = scope[0], sR = scope[1];
  for (let k = 0; k < 256; k++) { const x = cx + (sL[k] - sR[k]) * R * 0.9, y = cy - (sL[k] + sR[k]) * R * 0.9; k ? gx.lineTo(x, y) : gx.moveTo(x, y); }
  gx.strokeStyle = '#e8f4d855'; gx.lineWidth = 1; gx.stroke();
  // the four orbits
  dots = [];
  for (let v = 0; v < NL; v++) {
    const on = E[LP(v, L.ON)] > 0.5, steps = Math.round(E[LP(v, L.STEPS)]), m = laneMask(v), env = playing ? Math.min(1, tele[16 + v] * 1.5) * (v === 3 ? 0.55 : 1) : 0;
    const p = phaseE * TAU / 16 + v * 0.44, sy = 1 - v * 0.065, col = COLORS[v];
    const pt = (a, k) => { const [x, y] = orbitPt(geom, a, v, p, k); return [cx + x * R * 1.38, cy + y * R * sy]; };
    gx.strokeStyle = col;
    for (let k = 6; k >= 0; k--) {
      gx.globalAlpha = !on ? 0.03 : k === 0 ? 0.55 + env * 0.45 : 0.06 + env * 0.06;
      gx.lineWidth = k === 0 ? 1.3 + env * 1.4 : 1;
      gx.beginPath(); for (let j = 0; j <= 300; j++) { const [x, y] = pt(j * TAU / 300, k * 0.032 * ratioE); j ? gx.lineTo(x, y) : gx.moveTo(x, y); } gx.stroke();
    }
    if (on && env > 0.02) { gx.globalAlpha = env * 0.12; gx.lineWidth = 7; gx.beginPath(); for (let j = 0; j <= 160; j++) { const [x, y] = pt(j * TAU / 160, 0); j ? gx.lineTo(x, y) : gx.moveTo(x, y); } gx.stroke(); }
    // the notes
    const cur = playing ? tele[12 + v] : -1;
    gx.fillStyle = col;
    for (let s = 0; s < steps; s++) {
      const hit = (m >> s) & 1, [x, y] = pt(s * TAU / steps, 0); dots.push([x, y, v, s]);
      if (!hit) { gx.globalAlpha = on ? 0.16 : 0.05; gx.beginPath(); gx.arc(x, y, 1.2, 0, TAU); gx.fill(); continue; }
      gx.globalAlpha = on ? 1 : 0.12; gx.beginPath(); gx.arc(x, y, s === cur ? 6 : 2.8, 0, TAU); gx.fill();
    }
    if (on && cur >= 0) {
      const [x, y] = pt(cur * TAU / steps, 0); gx.globalAlpha = 0.9; gx.lineWidth = 1.2; gx.beginPath(); gx.arc(x, y, 9, 0, TAU); gx.stroke();
      if (cur !== lastStep[v]) { lastStep[v] = cur; if ((m >> cur) & 1) { rings.push({ x, y, r: 6, a: 0.8, col }); for (let q = 0; q < 7; q++) { const a = Math.random() * TAU, sp = 0.4 + Math.random() * 1.3; parts.push({ x, y, vx: Math.cos(a) * sp, vy: Math.sin(a) * sp, a: 1, col }); } }
        const note = tele[20 + v]; if ((m >> cur) & 1 && note > 0) { gx.globalAlpha = 0.85; gx.font = MONO_FONT; gx.fillText(NOTES[note % 12] + (Math.floor(note / 12) - 1), x + 12, y - 10); } }
    } else lastStep[v] = -1;
  }
  // GRAVITY: the target orbit, dashed
  if (target !== null && E[G.GRAVITY] < 0.999) {
    const tp = sceneParams(target), tg = Math.round(tp[G.GEOM]), tph = Math.round(tp[G.PHASE]);
    gx.setLineDash([3, 6]); gx.lineWidth = 1;
    for (let v = 0; v < NL; v++) { if (tp[LP(v, L.ON)] < 0.5) continue; gx.strokeStyle = COLORS[v]; gx.globalAlpha = 0.18 * (1 - E[G.GRAVITY]) + 0.05;
      gx.beginPath(); for (let j = 0; j <= 200; j++) { const [x, y] = orbitPt(tg, j * TAU / 200, v, tph * TAU / 16 + v * 0.44, 0); const X = cx + x * R * 1.38, Y = cy + y * R * (1 - v * 0.065); j ? gx.lineTo(X, Y) : gx.moveTo(X, Y); } gx.stroke(); }
    gx.setLineDash([]);
  }
  // sparks
  for (let i = rings.length - 1; i >= 0; i--) { const r = rings[i]; r.r += 1.4; r.a *= 0.93; gx.globalAlpha = r.a; gx.strokeStyle = r.col; gx.lineWidth = 1; gx.beginPath(); gx.arc(r.x, r.y, r.r, 0, TAU); gx.stroke(); if (r.a < 0.03) rings.splice(i, 1); }
  for (let i = parts.length - 1; i >= 0; i--) { const q = parts[i]; q.x += q.vx; q.y += q.vy; q.vx *= 0.97; q.vy *= 0.97; q.a *= 0.95; gx.globalAlpha = q.a; gx.fillStyle = q.col; gx.fillRect(q.x, q.y, 1.6, 1.6); if (q.a < 0.03) parts.splice(i, 1); }
  gx.globalAlpha = 1; gx.globalCompositeOperation = 'source-over';
  if (rings.length > 80) rings.splice(0, rings.length - 80); if (parts.length > 500) parts.splice(0, parts.length - 500);
  requestAnimationFrame(draw);
}
cv.addEventListener('click', e => { const r = cv.getBoundingClientRect(), x = e.clientX - r.left, y = e.clientY - r.top; let best = null, bd = 14 * 14;
  for (const d of dots) { const dd = (d[0] - x) ** 2 + (d[1] - y) ** 2; if (dd < bd) { bd = dd; best = d; } } if (best) toggleStep(best[2], best[3]); });

// telemetry from the engine
let lastEvo = '';
Bridge.onTele = (t, l, r) => {
  tele = t; scope[0] = l; scope[1] = r;
  const was = playing; playing = t[1] > 0.5; if (was !== playing) refresh();
  const gs = t[0]; $('#pos').textContent = gs < 0 || !playing ? '01 · 16' : String((gs % 16) + 1).padStart(2, '0') + ' · ' + String(Math.floor(gs / 16) + 1).padStart(3, '0');
  if (plugin && syncOn) $('#bpm').value = Math.round(t[2]);
  const evo = t[5] + '/' + t[6] + '/' + t[7].toFixed(2);
  Mirror.X.ox_evo(t[5] | 0, t[6] | 0, t[7]);
  $('#evo').textContent = playing && P[G.EVOLVE] > 0.001 ? '✦ ' + T('evolving') + (t[5] ? ' Φ' + (t[5] > 0 ? '+' : '') + t[5] : '') : '';
  if (playing && evo !== lastEvo) { const first = lastEvo === ''; lastEvo = evo; if (!first && P[G.EVOLVE] > 0.001) commit(true); }
  if (t[9] !== heldMask) { heldMask = t[9]; paintKeys(); }
  if (target !== null && P[G.GRAVITY] !== undefined) $('#gravVal').textContent = pct(P[G.GRAVITY]);
};
Bridge.onParams = list => { for (const [id, v] of list) { P[id] = v; Mirror.X.ox_param(id, v); } refreshSoon(); };

// ------------------------------------------------------------------ time machine
const TM = []; let tmIdx = -1;
function commit(evolved) {
  const p = Float32Array.from(P);
  if (evolved) { p[G.PHASE] = Math.max(0, Math.min(15, p[G.PHASE] + tele[5])); p[G.DENSITY] = Math.max(1, Math.min(16, p[G.DENSITY] + tele[6])); p[G.RATIO] = Math.max(0.5, Math.min(8, p[G.RATIO] + tele[7])); }
  const last = TM[TM.length - 1];
  if (last && last.P.every((v, i) => v === p[i]) && JSON.stringify(last.custom) === JSON.stringify(custom)) return;
  TM.push({ P: p, custom: custom.slice(), name: sceneTitle, evolved: !!evolved, at: new Date() });
  if (TM.length > 40) TM.shift(); tmIdx = TM.length - 1; drawTM();
}
function drawTM() {
  const box = $('#tm'); box.innerHTML = '';
  TM.forEach((s, i) => { const b = document.createElement('button'); b.className = 'snap' + (i === tmIdx ? ' cur' : ''); b.title = s.name + ' · ' + s.at.toLocaleTimeString();
    b.innerHTML = `<canvas width="136" height="136"></canvas><small>${s.evolved ? '✦ ' : ''}${s.at.toLocaleTimeString([], { hour12: false, minute: '2-digit', second: '2-digit' })}</small>`;
    thumb(b.querySelector('canvas'), s.P, false); b.onclick = () => restore(i); box.append(b); });
  if (!TM.length) box.innerHTML = '<div class="snap empty">✦</div>';
  box.scrollLeft = box.scrollWidth;
}
function restore(i) {
  const s = TM[i]; if (!s) return; tmIdx = i;
  const keep = [G.MASTER, G.KEYMODE, G.PLAYLANE, G.GRAVITY].map(id => P[id]);
  P = Float32Array.from(s.P); [G.MASTER, G.KEYMODE, G.PLAYLANE, G.GRAVITY].forEach((id, k) => P[id] = keep[k]);
  P[G.EVOLVE] = 0;                       // you came back to this moment: it stays (turn EVOLVE up to let it wander again)
  custom = s.custom.slice(); sceneTitle = s.name; pushAll(); Bridge.send({ t: 'evo', p: 0, d: 0, r: 0 }); lastEvo = '';
  refresh(); drawTM(); toast(T('Back in time') + ' · ' + s.at.toLocaleTimeString()); saveSoon();
}
$('#undo').onclick = () => restore(Math.max(0, tmIdx - 1));

// ------------------------------------------------------------------ new orbit / mutate
const pick = a => a[Math.floor(Math.random() * a.length)], ri = (a, b) => a + Math.floor(Math.random() * (b - a + 1)), rf = (a, b) => a + Math.random() * (b - a);
const WORDS = ['Perihelion', 'Cenote', 'Sascab', 'Equinox', 'Henequén', 'Puuc', 'Nadir', 'Zenith', 'Albedo', 'Umbra', 'Selva', 'Halo', 'Kin', 'Sisal', 'Aphelion', 'Ceiba', 'Corona', 'Penumbra'];
function newOrbit() {
  const p = Float32Array.from(DEF);
  for (const id of [G.BPM, G.MASTER, G.KEYMODE, G.PLAYLANE, G.EVOLVE, G.SWING, G.HUMAN]) p[id] = P[id];
  p[G.ROOT] = ri(0, 11); p[G.SCALE] = pick([0, 0, 1, 1, 2, 2, 3, 4, 5, 5, 6, 8, 9]); p[G.GEOM] = ri(0, 3); p[G.DENSITY] = ri(4, 11); p[G.PHASE] = ri(0, 15);
  p[G.RATIO] = pick([0.5, 1, 1.5, 2, 2, 2.5, 3, 3.5, 4, 5, 7]); p[G.SPACE] = rf(0.2, 0.6); p[G.SIZE] = rf(0.4, 0.9); p[G.ECHO] = rf(0.08, 0.38); p[G.ECHOTIME] = pick([1, 2, 3, 3, 4, 6]);
  p[G.ECHOFB] = rf(0.25, 0.55); p[G.CHORUS] = rf(0, 0.5); p[G.CHAOS] = Math.random() < 0.3 ? rf(0.05, 0.25) : 0; p[G.BLOOM] = Math.random() < 0.4 ? rf(0.05, 0.4) : 0; p[G.WARMTH] = rf(0.05, 0.35);
  const poly = Math.random() < 0.35;
  const models = [[0, 0, 0, 5], [1, 4, 2, 1], [2, 4, 5, 2, 1], [3, 3, 3, 1, 4]], offs = [[-4, -1], [-1, 1], [0, 3], [-5, -2]];
  for (let l = 0; l < NL; l++) {
    p[LP(l, L.MODEL)] = pick(models[l]); p[LP(l, L.PULSES)] = ri(...offs[l]); p[LP(l, L.INDEX)] = rf(0.5, 1.3); p[LP(l, L.FB)] = Math.random() < 0.25 ? rf(0.1, 0.5) : 0;
    if (poly && l > 0) p[LP(l, L.STEPS)] = pick([16, 12, 10, 9, 7, 14, 15]);
    if (Math.random() < 0.2) p[LP(l, L.ROT)] = ri(-3, 3);
    if (l === 0) p[LP(l, L.CUT)] = rf(0.45, 0.72);
    if (p[LP(l, L.MODEL)] === 3) { p[LP(l, L.ATK)] = rf(0.3, 0.6); p[LP(l, L.SUS)] = rf(0.4, 0.7); p[LP(l, L.REL)] = rf(0.4, 0.7); p[LP(l, L.GATE)] = pick([2, 3, 4, 6]); p[LP(l, L.DETUNE)] = rf(0.2, 0.6); }
  }
  p[LP(3, L.LEVEL)] = rf(0.3, 0.5); if (Math.random() < 0.2) p[LP(3, L.ON)] = 0;
  P = p; custom = [null, null, null, null]; curScene = -1;
  sceneTitle = pick(WORDS) + ' ' + ['I', 'II', 'III', 'IV', 'V', 'VI', 'VII', 'VIII', 'IX'][ri(0, 8)];
  sceneText = NOTES[p[G.ROOT]] + ' ' + T(SCALES[p[G.SCALE]]) + ' · ' + T(GEOMS[p[G.GEOM]]) + (poly ? ' · ' + [0, 1, 2, 3].map(l => p[LP(l, L.STEPS)]).join('/') : '');
  pushAll(); Bridge.send({ t: 'evo', p: 0, d: 0, r: 0 }); refresh(); commit(); saveSoon();
}
function mutate() {
  const moves = [
    () => setP(G.PHASE, (P[G.PHASE] + pick([-3, -2, -1, 1, 2, 3]) + 16) % 16, { noHist: true }),
    () => setP(G.DENSITY, P[G.DENSITY] + pick([-1, 1]), { noHist: true }),
    () => setP(G.RATIO, pick([0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 5]), { noHist: true }),
    () => { const l = ri(0, 3); setP(LP(l, L.ROT), P[LP(l, L.ROT)] + pick([-1, 1]), { noHist: true }); },
    () => { const l = ri(1, 3); setP(LP(l, L.PULSES), P[LP(l, L.PULSES)] + pick([-1, 1]), { noHist: true }); },
    () => { if (Math.random() < 0.35) setP(G.GEOM, (P[G.GEOM] + 1) % 4, { noHist: true }); },
    () => { const l = ri(1, 3); setP(LP(l, L.INDEX), P[LP(l, L.INDEX)] + rf(-0.25, 0.25), { noHist: true }); }
  ];
  for (let k = 0; k < 3; k++) pick(moves)();
  refresh(); commit();
}
$('#newOrbit').onclick = newOrbit; $('#mutate').onclick = mutate;

// ------------------------------------------------------------------ MIDI clip and WAV loop (4 bars), dragged onto the DAW or downloaded
function vlq(n) { const a = [n & 127]; while ((n >>= 7) > 0) a.unshift((n & 127) | 128); return a; }
function midiClip() {   // one track, one channel (channel 1): every voice together, as Live expects on one MIDI track
  const ppq = 480, st = 120, tempo = Math.round(60000000 / curBpm()), ev = [];
  for (let l = 0; l < NL; l++) {
    if (P[LP(l, L.ON)] < 0.5) continue;
    const steps = Math.round(P[LP(l, L.STEPS)]), gate = Math.max(30, Math.round(P[LP(l, L.GATE)] * st * 0.92));
    for (let gs = 0; gs < 64; gs++) { const s = gs % steps; if (!Mirror.X.ox_hit(l, s)) continue; const n = Mirror.X.ox_note(l, s), v = Mirror.X.ox_vel(l, s);
      const sw = (gs & 1) ? Math.round(P[G.SWING] * 0.33 * st) : 0; ev.push([gs * st + sw, 0x90, n, v], [Math.min(64 * st - 1, gs * st + sw + gate), 0x80, n, 0]); }
  }
  ev.sort((a, b) => a[0] - b[0] || a[1] - b[1]);
  const name = [...new TextEncoder().encode('ORBITA ' + (sceneTitle || ''))];
  const d = [0, 255, 3, name.length, ...name, 0, 255, 81, 3, (tempo >> 16) & 255, (tempo >> 8) & 255, tempo & 255, 0, 255, 88, 4, 4, 2, 24, 8];
  let last = 0; for (const e of ev) { d.push(...vlq(e[0] - last), e[1], e[2], e[3]); last = e[0]; }
  d.push(...vlq(Math.max(0, 64 * st - last)), 255, 47, 0);
  return new Uint8Array([77, 84, 104, 100, 0, 0, 0, 6, 0, 0, 0, 1, ppq >> 8, ppq & 255, 77, 84, 114, 107, (d.length >>> 24) & 255, (d.length >> 16) & 255, (d.length >> 8) & 255, d.length & 255, ...d]);
}
function curBpm() { return plugin && syncOn && tele[2] > 1 ? tele[2] : P[G.BPM]; }
// the loop as audio: a separate copy of the engine plays one bar to fill the echo and the reverb, then four bars are kept,
// so the file loops without a gap
function wavLoop() {
  const X = new WebAssembly.Instance(new WebAssembly.Module(b64bytes(WASM_B64)), {}).exports, sr = 48000;
  X.ox_init(sr); for (let i = 0; i < NP; i++) X.ox_param(i, P[i]); X.ox_param(G.BPM, curBpm());
  if (target !== null) { const tp = sceneParams(target); for (let i = 0; i < NP; i++) X.ox_target(i, tp[i]); X.ox_param(G.GRAVITY, P[G.GRAVITY]); }
  custom.forEach((m, l) => { if (m !== null) X.ox_custom(l, m); });
  X.ox_param(G.CHAOS, P[G.CHAOS]); X.ox_play();
  const bar = Math.round(sr * 60 / curBpm() * 4), pre = bar, len = bar * 4, L = new Float32Array(len), R = new Float32Array(len);
  let n = 0; const mem = () => X.memory.buffer;
  while (n < pre + len) { const k = Math.min(128, pre + len - n); X.ox_process(k); const a = new Float32Array(mem(), X.ox_out_l(), k), b = new Float32Array(mem(), X.ox_out_r(), k);
    for (let i = 0; i < k; i++) { const j = n + i - pre; if (j >= 0) { L[j] = a[i]; R[j] = b[i]; } } n += k; }
  // the first bar of the loop also gets the tail of the bar before it (it was rendered), so the seam is the music's own
  const out = new DataView(new ArrayBuffer(44 + len * 4)); const w = (o, s) => { for (let i = 0; i < s.length; i++) out.setUint8(o + i, s.charCodeAt(i)); };
  w(0, 'RIFF'); out.setUint32(4, 36 + len * 4, true); w(8, 'WAVEfmt '); out.setUint32(16, 16, true); out.setUint16(20, 1, true); out.setUint16(22, 2, true); out.setUint32(24, sr, true); out.setUint32(28, sr * 4, true); out.setUint16(32, 4, true); out.setUint16(34, 16, true); w(36, 'data'); out.setUint32(40, len * 4, true);
  for (let i = 0; i < len; i++) { out.setInt16(44 + i * 4, Math.max(-1, Math.min(1, L[i])) * 32767, true); out.setInt16(46 + i * 4, Math.max(-1, Math.min(1, R[i])) * 32767, true); }
  return new Uint8Array(out.buffer);
}
function clipName(ext) { return 'ORBITA-' + (sceneTitle || 'orbit').normalize('NFD').replace(/[^\w]+/g, '-').replace(/-+$/, '') + '-' + Math.round(curBpm()) + 'bpm.' + ext; }
function toB64(u) { let s = ''; for (let i = 0; i < u.length; i += 0x8000) s += String.fromCharCode.apply(null, u.subarray(i, i + 0x8000)); return btoa(s); }
function download(bytes, name, type) { const a = document.createElement('a'), url = URL.createObjectURL(new Blob([bytes], { type })); a.href = url; a.download = name; a.click(); setTimeout(() => URL.revokeObjectURL(url), 3000); }
function hookDrag(id, make, ext, type) {
  if (plugin) $(id).addEventListener('pointerdown', () => { Bridge.call('dragFile', clipName(ext), toB64(make())); toast(T('Drag it onto a track in your DAW')); });
  else $(id).onclick = () => { download(make(), clipName(ext), type); toast(T(ext === 'mid' ? 'MIDI clip saved' : 'Audio loop saved')); };
}
hookDrag('#drag', midiClip, 'mid', 'audio/midi');
hookDrag('#dragWav', wavLoop, 'wav', 'audio/wav');

// a MIDI clip dropped on ÓRBITA: its key becomes the scale and the root, its rhythm becomes the steps of the lane under it
function readMidiNotes(buf) {
  const b = new Uint8Array(buf), notes = []; let p = 0; const u32 = () => (b[p++] << 24 | b[p++] << 16 | b[p++] << 8 | b[p++]) >>> 0, u16 = () => b[p++] << 8 | b[p++];
  if (String.fromCharCode(...b.subarray(0, 4)) !== 'MThd') throw Error('not a MIDI file');
  p = 4; const hl = u32(); u16(); const ntr = u16(), div = u16(); p = 8 + hl;
  for (let t = 0; t < ntr && p < b.length; t++) {
    p += 4; const len = u32(), end = p + len; let tick = 0, run = 0;
    const vlq2 = () => { let v = 0, c; do { c = b[p++]; v = (v << 7) | (c & 127); } while (c & 128 && p < end); return v; };
    while (p < end) { tick += vlq2(); let st = b[p]; if (st < 0x80) st = run; else p++;
      if (st === 0xff) { p++; const l = vlq2(); p += l; } else if (st === 0xf0 || st === 0xf7) { const l = vlq2(); p += l; }
      else { run = st; const hi = st & 0xf0, d1 = b[p++], d2 = (hi === 0xc0 || hi === 0xd0) ? 0 : b[p++]; if (hi === 0x90 && d2 > 0) notes.push([tick / div * 4, d1, d2]); } }
    p = end;
  }
  return notes;   // [sixteenth, note, velocity]
}
function learnClip(notes, lane) {
  if (!notes.length) return;
  const pcs = new Array(12).fill(0); notes.forEach(([, n]) => pcs[n % 12]++);
  let best = [-1e9, 0, 0];
  for (let r = 0; r < 12; r++) for (let sc = 0; sc < SCALE_SEMIS.length; sc++) {
    const set = SCALE_SEMIS[sc].map(x => (x + r) % 12); let sc2 = 0; pcs.forEach((c, pc) => sc2 += set.includes(pc) ? c : -2 * c);
    sc2 += pcs[r] * 0.5 - set.length * 0.05; if (sc2 > best[0]) best = [sc2, r, sc]; }
  setP(G.ROOT, best[1], { noHist: true }); setP(G.SCALE, best[2], { noHist: true });
  const steps = Math.round(P[LP(lane, L.STEPS)]); let m = 0; notes.forEach(([s]) => { const q = Math.round(s); if (q < 16) m |= 1 << (q % steps); });
  if (m) { custom[lane] = m; Mirror.X.ox_custom(lane, m); Bridge.send({ t: 'custom', lane, mask: m }); }
  refresh(); commit(); toast(T('From the clip') + ': ' + NOTES[best[1]] + ' ' + T(SCALES[best[2]]) + ' · ' + T(LANE_NAMES[lane]));
}
const SCALE_SEMIS = [[0, 2, 3, 5, 7, 9, 10], [0, 2, 3, 5, 7, 8, 10], [0, 3, 5, 7, 10], [0, 1, 3, 5, 7, 8, 10], [0, 2, 4, 6, 7, 9, 11], [0, 2, 4, 7, 9], [0, 1, 4, 5, 7, 8, 10], [0, 2, 4, 6, 8, 10], [0, 2, 3, 5, 7, 8, 11], [0, 2, 3, 7, 8]];
addEventListener('dragover', e => { e.preventDefault(); document.body.classList.add('dropping'); });
addEventListener('dragleave', e => { if (!e.relatedTarget) document.body.classList.remove('dropping'); });
addEventListener('drop', async e => {
  e.preventDefault(); document.body.classList.remove('dropping');
  const f = [...(e.dataTransfer.files || [])].find(x => /\.midi?$/i.test(x.name)); if (!f) { toast(T('Drop a .mid file')); return; }
  const row = e.target.closest && e.target.closest('.lane'); const lane = row ? +row.dataset.l : (selLane >= 0 ? selLane : 0);
  try { learnClip(readMidiNotes(await f.arrayBuffer()), lane); } catch (err) { toast(err.message); }
});

// ------------------------------------------------------------------ keys (on screen, the computer's keyboard)
let heldMask = 0, kbOct = 4; const downKeys = new Set();
function buildKeys() {
  const box = $('#keys'); box.innerHTML = ''; const whites = [0, 2, 4, 5, 7, 9, 11], blacks = { 1: 0, 3: 1, 6: 3, 8: 4, 10: 5 };
  for (let o = 0; o < 2; o++) for (const s of whites) { const k = document.createElement('div'); k.className = 'k'; k.dataset.n = 12 * (kbOct + o) + 12 + s; if (s === 0) k.innerHTML = `<span>C${kbOct + o}</span>`; box.append(k); }
  for (let o = 0; o < 2; o++) for (const s in blacks) { const k = document.createElement('div'); k.className = 'k b'; k.dataset.n = 12 * (kbOct + o) + 12 + +s; k.style.left = ((o * 7 + blacks[s] + 1) / 14 * 100 - 2.6) + '%'; box.append(k); }
  box.onpointerdown = e => { const k = e.target.closest('.k'); if (!k) return; box.setPointerCapture(e.pointerId); press(+k.dataset.n, true); box._n = +k.dataset.n; };
  box.onpointerup = box.onpointercancel = () => { if (box._n) { press(box._n, false); box._n = 0; } };
  paintKeys();
}
async function press(n, down) {
  if (down && !plugin && !Bridge.ready) await startAudio();
  if (down) downKeys.add(n); else downKeys.delete(n);
  Bridge.send({ t: 'key', note: n, vel: down ? 100 : 0 }); paintKeys();
}
function paintKeys() { $$('#keys .k').forEach(k => { const n = +k.dataset.n; k.classList.toggle('dn', downKeys.has(n)); k.classList.toggle('held', !downKeys.has(n) && Math.round(P[G.KEYMODE]) === 2 && !!(heldMask >> (n % 12) & 1)); }); }
const KB = 'awsedftgyhujkolp;'.split('');
addEventListener('keydown', e => {
  if (e.target.matches('input,select,textarea') || e.metaKey || e.ctrlKey || e.altKey) return;
  if (e.code === 'Space') { e.preventDefault(); togglePlay(); return; }
  if (e.key === 'z') { kbOct = Math.max(1, kbOct - 1); buildKeys(); return; } if (e.key === 'x') { kbOct = Math.min(7, kbOct + 1); buildKeys(); return; }
  const i = KB.indexOf(e.key.toLowerCase()); if (i < 0 || e.repeat) return; press(12 * kbOct + 12 + i, true);
});
addEventListener('keyup', e => { const i = KB.indexOf(e.key.toLowerCase()); if (i >= 0) press(12 * kbOct + 12 + i, false); });

// ------------------------------------------------------------------ transport, web MIDI out, plug-in switches
async function startAudio() { try { await Bridge.startWeb(); pushAll(); if (target !== null) setTarget(target); if (midiPort) Bridge.send({ t: 'events', on: true }); } catch (e) { toast(T('The audio could not start.') + ' ' + e.message); throw e; } }
async function togglePlay() {
  if (!plugin && !Bridge.ready) await startAudio();
  playing = !playing; Bridge.send({ t: playing ? 'play' : 'stop' }); if (!playing && midiPort) for (let c = 0; c < 4; c++) midiPort.send([0xb0 | c, 123, 0]);
  lastEvo = ''; refresh();
}
$('#play').onclick = togglePlay;
$('#bpm').onchange = e => setP(G.BPM, +e.target.value);
let midiAccess = null, midiPort = null;
$('#connect').onclick = async () => {
  try { if (!navigator.requestMIDIAccess) throw Error('Open the web edition in Chrome or Edge for Web MIDI.');
    midiAccess = await navigator.requestMIDIAccess();
    const fill = () => { const sel = $('#out'); sel.length = 1; for (const o of midiAccess.outputs.values()) if (o.state === 'connected') sel.add(new Option(o.name, o.id)); sel.value = midiPort ? midiPort.id : ''; $('#msg').textContent = T(sel.length > 1 ? 'Pick the FM-1 in MIDI output.' : 'No MIDI outputs found. Check the USB data cable.'); };
    fill(); midiAccess.onstatechange = fill;
  } catch (e) { $('#msg').textContent = T(e.message); }
};
$('#out').onchange = () => { midiPort = midiAccess && midiAccess.outputs.get($('#out').value) || null; Bridge.send({ t: 'events', on: !!midiPort }); };
Bridge.onEv = (list, time) => {
  if (!midiPort || !Bridge.ctx) return; const base = performance.now() + Math.max(0, (time - Bridge.ctx.currentTime)) * 1000 + (Bridge.ctx.outputLatency || 0.02) * 1000, sr = Bridge.ctx.sampleRate;
  for (const [kind, lane, note, vel, at] of list) midiPort.send(kind ? [0x90 | lane, note, Math.max(1, vel)] : [0x80 | lane, note, 0], base + at / sr * 1000);
};
let syncOn = false, midiOutOn = false;
$('#sync').onclick = () => { syncOn = !syncOn; Bridge.send({ t: 'sync', on: syncOn }); $('#sync').classList.toggle('on', syncOn); toast(T(syncOn ? 'Following the DAW' : 'Own tempo')); saveSoon(); };
$('#midiOut').onclick = () => { midiOutOn = !midiOutOn; Bridge.send({ t: 'midiOut', on: midiOutOn }); $('#midiOut').classList.toggle('on', midiOutOn); toast(T(midiOutOn ? 'MIDI out on' : 'MIDI out off')); saveSoon(); };
if (plugin) $$('a[target=_blank]').forEach(a => a.addEventListener('click', e => { e.preventDefault(); Bridge.call('openUrl', a.href); }));

// ------------------------------------------------------------------ language, theme, help, coach, toast
function setLang(l) { LANG = l; $('#lang').value = l; applyLang(); buildSelects(); buildScenes(); knobs.forEach(k => k.update()); if (curScene >= 0 && curScene < SCENES.length) { sceneTitle = l === 'es' ? SCENES[curScene].es : SCENES[curScene].name; sceneText = l === 'es' ? SCENES[curScene].descEs : SCENES[curScene].desc; }
  buildLanes(); $('#manual').href = l === 'es' ? 'Orbita-ES.pdf' : 'Orbita-EN.pdf'; helpHtml(); refresh(); saveSoon(); }
$('#lang').onchange = e => setLang(e.target.value);
$('#theme').onclick = () => { document.documentElement.classList.toggle('night'); saveSoon(); };
function helpHtml() {
  const en = `<small>QUICK GUIDE</small><h2>ÓRBITA 2</h2><p>Four FM voices play euclidean rhythms over a scale. Each voice draws its orbit: every bright dot is a note, the ring is where it is now.</p>
  <h4>PLAY</h4><ol><li><b>▶ Play</b> (or the space bar). Pick a <b>scene</b>, or press <b>✦ NEW ORBIT</b> for a fresh, musical one; <b>≈ MUTATE</b> changes a few things and keeps the feel.</li><li><b>DENSITY</b> is how many notes in a cycle, <b>PHASE</b> where they fall (and which notes), <b>FM RATIO</b> warm to metallic.</li><li>Click a dot on the screen or a cell in a lane to add or remove that note.</li></ol>
  <h4>THE THINGS NOBODY ELSE HAS</h4><ol><li><b>GRAVITY</b>: press <b>B</b> and pick a second scene (or right-click it). Turn GRAVITY and the whole sound glides from one world to the other: rhythm, harmony, timbre and space. Automate it in your DAW for transitions.</li><li><b>EVOLVE</b>: the orbit wanders by itself, a little every bar, and never far from home.</li><li><b>TIME MACHINE</b>: every change and every evolution is kept. Heard something great a minute ago? Click it and it's back.</li><li><b>HARMONY keys</b>: in HARMONY, hold a chord on your keyboard and the orbit plays only those notes. TRANSPOSE moves everything to the key you press. PLAY plays a voice like a synth.</li><li><b>Polymeter</b>: give each voice its own STEPS (7, 10, 12…) and the four cycles drift and meet again.</li><li><b>⇣ MIDI</b>: drag 4 bars (all the voices on one channel) onto your DAW, or turn on <b>MIDI OUT</b> to play your other instruments live (channel 1). <b>⇣ WAV</b> drags a 4-bar audio loop. Drop a MIDI clip on ÓRBITA (on a lane) and it takes the clip's key and rhythm.</li><li><b>PLAY and STOP are ÓRBITA's own</b>, like XO: it plays without starting your DAW. With SYNC on it follows the DAW's tempo and, when both play, the DAW's bar; the DAW's play never starts it.</li></ol>
  <h4>SOUND</h4><p>Click a voice's name to open its sound: FM ratio and index, envelopes, filter, detune, sends. Six models: SUB, PRISM, CRYSTAL, HALO, GLASS, DUST.</p>
  <p class="hint">Free and open source · Dark Funk Durcal design · built at Polaris Sound Lab, Mérida, Yucatán, México.</p>`;
  const es = `<small>GUÍA RÁPIDA</small><h2>ÓRBITA 2</h2><p>Cuatro voces FM tocan ritmos euclidianos sobre una escala. Cada voz dibuja su órbita: cada punto brillante es una nota, el anillo es donde está ahora.</p>
  <h4>TOCAR</h4><ol><li><b>▶ Tocar</b> (o la barra espaciadora). Elige una <b>escena</b> o pulsa <b>✦ NUEVA ÓRBITA</b> para una nueva y musical; <b>≈ MUTAR</b> cambia algunas cosas y conserva la sensación.</li><li><b>DENSIDAD</b> es cuántas notas hay en un ciclo, <b>FASE</b> dónde caen (y qué notas), <b>RELACIÓN FM</b> de cálido a metálico.</li><li>Haz clic en un punto de la pantalla o en una celda de un carril para poner o quitar esa nota.</li></ol>
  <h4>LO QUE NADIE MÁS TIENE</h4><ol><li><b>GRAVEDAD</b>: pulsa <b>B</b> y elige una segunda escena (o clic derecho sobre ella). Gira GRAVEDAD y todo el sonido se desliza de un mundo al otro: ritmo, armonía, timbre y espacio. Automatízala en tu DAW para las transiciones.</li><li><b>EVOLUCIÓN</b>: la órbita vaga sola, un poco cada compás, sin alejarse de casa.</li><li><b>MÁQUINA DEL TIEMPO</b>: cada cambio y cada evolución se guarda. ¿Oíste algo genial hace un minuto? Haz clic y vuelve.</li><li><b>Teclas de ARMONÍA</b>: en ARMONÍA, mantén un acorde en tu teclado y la órbita toca sólo esas notas. TRANSPONER lleva todo a la tecla que pulses. TOCAR toca una voz como un sinte.</li><li><b>Polimetría</b>: dale a cada voz sus propios PASOS (7, 10, 12…) y los cuatro ciclos se desfasan y se vuelven a encontrar.</li><li><b>⇣ MIDI</b>: arrastra 4 compases (todas las voces en un canal) a tu DAW, o activa <b>MIDI OUT</b> para tocar tus otros instrumentos en vivo (canal 1). <b>⇣ WAV</b> arrastra un loop de audio de 4 compases. Suelta un clip MIDI sobre ÓRBITA (sobre un carril) y toma la tonalidad y el ritmo del clip.</li><li><b>TOCAR y DETENER son propios de ÓRBITA</b>, como XO: suena sin arrancar tu DAW. Con SYNC sigue el tempo del DAW y, cuando ambos tocan, su compás; el play del DAW nunca lo arranca.</li></ol>
  <h4>SONIDO</h4><p>Haz clic en el nombre de una voz para abrir su sonido: relación e índice FM, envolventes, filtro, desafinación, envíos. Seis modelos: SUBGRAVE, PRISMA, CRISTAL, HALO, VIDRIO, POLVO.</p>
  <p class="hint">Libre y gratuito · diseño de Dark Funk Durcal · desarrollado en Polaris Sound Lab, Mérida, Yucatán, México.</p>`;
  $('#helpBody').innerHTML = (LANG === 'es' ? es : en) + `<div class="go"><button class="tb" onclick="document.getElementById('help').classList.remove('show')">${T('Close')}</button></div>`;
}
$('#helpBtn').onclick = () => $('#help').classList.add('show');
$('#help').onclick = e => { if (e.target.id === 'help') $('#help').classList.remove('show'); };
$('#coachGo').onclick = async () => { $('#coach').classList.remove('show'); coachSeen = true; saveSoon(); if (!playing) await togglePlay(); };
$('#coachSkip').onclick = () => { $('#coach').classList.remove('show'); coachSeen = true; saveSoon(); };
let toastT = 0; function toast(s) { const t = $('#toast'); t.textContent = s; t.classList.add('show'); clearTimeout(toastT); toastT = setTimeout(() => t.classList.remove('show'), 1800); }

// ------------------------------------------------------------------ saving (the browser's storage on the web, the project in the plug-in)
let coachSeen = false;
function uiState() { return { v: 2, lang: LANG, night: document.documentElement.classList.contains('night'), coach: coachSeen, user: userScenes, scene: curScene, title: sceneTitle, text: sceneText, target, sync: syncOn, midiOut: midiOutOn, custom, P: plugin ? undefined : [...P] }; }
function saveSoon() { clearTimeout(saveT); saveT = setTimeout(() => { const s = JSON.stringify(uiState()); if (plugin) Bridge.call('setUi', s); else try { localStorage.setItem('orbita2', s); } catch (e) {} }, 400); }
function applyUi(s) {
  if (!s) return; if (s.lang) LANG = s.lang; if (s.night) document.documentElement.classList.add('night'); coachSeen = !!s.coach;
  if (Array.isArray(s.user)) userScenes = s.user.slice(0, 4).concat([null, null, null, null]).slice(0, 4);
  curScene = s.scene ?? 0; sceneTitle = s.title || ''; sceneText = s.text || ''; syncOn = !!s.sync; midiOutOn = !!s.midiOut;
  if (Array.isArray(s.custom)) custom = s.custom.slice(0, 4);
  if (s.P && s.P.length === NP) P = Float32Array.from(s.P);
  return s.target ?? null;
}

// ------------------------------------------------------------------ boot
async function boot() {
  let tgt = null, fresh = true;
  if (plugin) {
    Bridge.startPlugin();
    const st = await Bridge.call('getState');
    if (st && st.params && st.params.length === NP) { P = Float32Array.from(st.params); fresh = false; }
    if (st && st.ui) { try { tgt = applyUi(JSON.parse(st.ui)); } catch (e) {} }
    if (st) { syncOn = !!st.sync; midiOutOn = !!st.midiOut; playing = !!st.playing; }
    $('#sync').classList.toggle('on', syncOn); $('#midiOut').classList.toggle('on', midiOutOn);
  } else {
    try { const s = JSON.parse(localStorage.getItem('orbita2') || 'null'); if (s) { tgt = applyUi(s); fresh = !s.P; } } catch (e) {}
  }
  for (let i = 0; i < NP; i++) Mirror.X.ox_param(i, P[i]);
  custom.forEach((m, l) => { if (m !== null && m !== undefined) Mirror.X.ox_custom(l, m); else custom[l] = null; });
  $('#lang').value = LANG; applyLang(); buildSelects(); buildMacros(); buildDrawer(); buildLanes(); buildScenes(); buildKeys(); helpHtml();
  if (fresh) loadScene(0, { force: true }); else { if (plugin) pushAll(); refresh(); commit(); }
  if (tgt !== null && sceneParams(tgt)) { const g = P[G.GRAVITY]; setTarget(tgt); P[G.GRAVITY] = g; }
  drawTM(); requestAnimationFrame(draw);
  if (!coachSeen) $('#coach').classList.add('show');
}
boot();
