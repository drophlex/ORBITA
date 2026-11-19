/* bridge.js — the ÓRBITA 2 engine runs as WebAssembly in an AudioWorklet (web) or natively in the plug-in (JUCE).
   The page talks to it with the same small messages either way. */
'use strict';
const WORKLET_CODE = `
class OrbitaProc extends AudioWorkletProcessor {
  constructor(o) {
    super({});
    this.X = new WebAssembly.Instance(new WebAssembly.Module(o.processorOptions.wasm), {}).exports;
    this.X.ox_init(sampleRate); this.n = 0; this.evOn = false;
    this.port.onmessage = e => this.msg(e.data);
  }
  msg(d) {
    const X = this.X;
    switch (d.t) {
      case 'param': X.ox_param(d.id, d.v); break;
      case 'params': for (const [id, v] of d.list) X.ox_param(id, v); break;
      case 'target': for (const [id, v] of d.list) X.ox_target(id, v); break;
      case 'clearTarget': X.ox_clear_target(); break;
      case 'play': X.ox_play(); break;
      case 'stop': X.ox_stop(); break;
      case 'key': X.ox_key(d.note, d.vel); break;
      case 'custom': X.ox_custom(d.lane, d.mask); break;
      case 'audition': X.ox_audition(d.lane, d.note, d.vel); break;
      case 'events': this.evOn = d.on; break;
      case 'evo': X.ox_evo(d.p, d.d, d.r); break;
    }
  }
  process(inputs, outputs) {
    const X = this.X, o = outputs[0], n = o[0].length, mem = X.memory.buffer;
    X.ox_process(n);
    o[0].set(new Float32Array(mem, X.ox_out_l(), n)); if (o[1]) o[1].set(new Float32Array(mem, X.ox_out_r(), n));
    if (this.evOn) { const c = X.ox_ev_count(); if (c) { const dv = new DataView(mem), b = X.ox_ev(), list = [];
      for (let k = 0; k < c; k++) { const p = b + k * 8; list.push([dv.getUint8(p), dv.getUint8(p + 1), dv.getUint8(p + 2), dv.getUint8(p + 3), dv.getInt32(p + 4, true)]); }
      this.port.postMessage({ t: 'ev', list, time: currentTime }); } }
    if (++this.n % 4 === 0) {
      const tele = new Float32Array(mem, X.ox_tele(), 64).slice(), w = X.ox_scope_w(), sl = new Float32Array(mem, X.ox_scope_l(), 512), sr = new Float32Array(mem, X.ox_scope_r(), 512);
      const L = new Float32Array(256), R = new Float32Array(256); for (let k = 0; k < 256; k++) { const j = (w - 256 + k + 512) & 511; L[k] = sl[j]; R[k] = sr[j]; }
      this.port.postMessage({ t: 'tele', tele, L, R }, [tele.buffer, L.buffer, R.buffer]);
    }
    return true;
  }
}
registerProcessor('orbita-proc', OrbitaProc);`;

function b64bytes(s) { const b = atob(s), u = new Uint8Array(b.length); for (let i = 0; i < b.length; i++) u[i] = b.charCodeAt(i); return u; }

const Bridge = {
  kind: 'web', ready: false, onTele: null, onEv: null, onParams: null, _id: 1,
  isPlugin() { return !!(window.__JUCE__ && window.__JUCE__.backend); },
  // the web: the audio starts on the first press (browsers need a gesture)
  async startWeb() {
    if (this.ready) { await this.ctx.resume(); return; }
    this.ctx = new (window.AudioContext || window.webkitAudioContext)({ latencyHint: 'interactive' });
    const url = URL.createObjectURL(new Blob([WORKLET_CODE], { type: 'application/javascript' }));
    await this.ctx.audioWorklet.addModule(url);
    this.node = new AudioWorkletNode(this.ctx, 'orbita-proc', { numberOfInputs: 0, outputChannelCount: [2], processorOptions: { wasm: b64bytes(WASM_B64).buffer } });
    this.node.connect(this.ctx.destination);
    this.node.port.onmessage = e => { const d = e.data; if (d.t === 'tele' && this.onTele) this.onTele(d.tele, d.L, d.R); else if (d.t === 'ev' && this.onEv) this.onEv(d.list, d.time); };
    this.ready = true; await this.ctx.resume();
    if (this.queue) { for (const m of this.queue) this.node.port.postMessage(m); this.queue = null; }
  },
  startPlugin() {
    this.kind = 'plugin'; this.ready = true; const B = window.__JUCE__.backend;
    B.addEventListener('tele', d => this.onTele && this.onTele(Float32Array.from(d.tele), Float32Array.from(d.L), Float32Array.from(d.R)));
    B.addEventListener('params', d => this.onParams && this.onParams(d.list));
    B.addEventListener('state', d => this.onState && this.onState(d));
  },
  call(name, ...args) {
    if (!this.isPlugin()) return Promise.resolve(null);
    const B = window.__JUCE__.backend, id = this._id++;
    return new Promise(res => { const tok = B.addEventListener('__juce__complete', e => { if (e.promiseId === id) { B.removeEventListener(tok); res(e.result); } }); B.emitEvent('__juce__invoke', { name, params: args, resultId: id }); });
  },
  send(m) {
    if (this.kind === 'plugin') { this.call('msg', JSON.stringify(m)); return; }
    if (this.node) this.node.port.postMessage(m); else (this.queue = this.queue || []).push(m);
  }
};

// a second, silent copy of the engine on the page: it answers "which steps play, which notes" for the drawing and the MIDI export
const Mirror = {
  X: null,
  init() { const X = new WebAssembly.Instance(new WebAssembly.Module(b64bytes(WASM_B64)), {}).exports; X.ox_init(48000); this.X = X; this.P = new Float32Array(X.memory.buffer, X.ox_params(), X.ox_param_count()); },
  range(id) { return [this.X.ox_range(id, 0), this.X.ox_range(id, 1), this.X.ox_range(id, 2)]; },
  eff() { this.X.ox_compute(); return new Float32Array(this.X.memory.buffer, this.X.ox_eff(), this.X.ox_param_count()); }
};
