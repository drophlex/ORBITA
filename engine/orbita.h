// ÓRBITA 2 · a generative FM instrument — engine (header only, no libc, no allocation)
// The same code runs in the browser (WebAssembly in an AudioWorklet) and in the VST3/AU plug-in (JUCE),
// so the web and the plug-in sound exactly alike. Its note and rhythm rules are the ones of ÓRBITA 1
// (the web, the FM-1 firmware), so the four classic scenes still play the same notes.
// Created at Polaris Sound Lab · Mérida, Yucatán · Dark Funk Durcal design
#pragma once
#include "dsp.h"

namespace orbita {

constexpr int NL = 4, POLY = 6, NSCALE = 10, NGEOM = 4, NEV = 256;

// ------------------------------------------------------------------ parameters (one float array, shared with the page and the host)
enum GP {
  G_BPM, G_SWING, G_ROOT, G_SCALE, G_GEOM, G_DENSITY, G_PHASE, G_RATIO,
  G_GRAVITY, G_EVOLVE, G_CHAOS, G_ECHO, G_ECHOTIME, G_ECHOFB, G_ECHOTONE, G_SPACE,
  G_SIZE, G_DAMP, G_CHORUS, G_WIDTH, G_WARMTH, G_TILT, G_MASTER, G_KEYMODE,
  G_PLAYLANE, G_HUMAN, G_BLOOM, G_NG = 32 };
enum LP {
  L_ON, L_LEVEL, L_PAN, L_PULSES, L_STEPS, L_ROT, L_OCT, L_PROB,
  L_GATE, L_RATIO, L_INDEX, L_FB, L_ENVIDX, L_ATK, L_DEC, L_SUS,
  L_REL, L_CUT, L_RES, L_ECHO, L_SPACE, L_DETUNE, L_VEL, L_MODEL, L_NP = 24 };
inline int LPID(int l, int k) { return G_NG + l * L_NP + k; }
constexpr int P_COUNT = G_NG + NL * L_NP;   // 128

// how a parameter morphs under GRAVITY: 0 continuous, 1 rounded, 2 switches half way
inline int morphKind(int id) {
  if (id < G_NG) {
    switch (id) { case G_SCALE: case G_GEOM: case G_KEYMODE: case G_PLAYLANE: case G_ECHOTIME: return 2;
      case G_ROOT: case G_DENSITY: case G_PHASE: return 1; case G_GRAVITY: return 3; default: return 0; }
  }
  switch ((id - G_NG) % L_NP) { case L_ON: case L_MODEL: case L_STEPS: return 2;
    case L_PULSES: case L_ROT: case L_OCT: return 1; default: return 0; }
}

struct Range { float lo, hi, def; };
inline Range range(int id) {
  if (id < G_NG) {
    static const Range g[G_NG] = {
      {40, 220, 112}, {0, 1, 0}, {0, 11, 2}, {0, NSCALE - 1, 0}, {0, NGEOM - 1, 0}, {1, 16, 7}, {0, 15, 2}, {0.5f, 8, 2},
      {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0.22f}, {0, 7, 2}, {0, 0.95f, 0.38f}, {0, 1, 0.55f}, {0, 1, 0.3f},
      {0, 1, 0.55f}, {0, 1, 0.45f}, {0, 1, 0.25f}, {0, 1, 0.6f}, {0, 1, 0.2f}, {-1, 1, 0}, {0, 1, 0.72f}, {0, 2, 0},
      {0, NL - 1, 3}, {0, 1, 0.15f}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0} };
    return g[id];
  }
  static const Range l[L_NP] = {
    {0, 1, 1}, {0, 1, 0.75f}, {-1, 1, 0}, {-8, 8, 0}, {2, 16, 16}, {-15, 15, 0}, {-2, 2, 0}, {0, 1, 1},
    {0.1f, 8, 1}, {0.25f, 4, 1}, {0, 2, 1}, {0, 1, 0}, {0, 1, 0.85f}, {0, 1, 0.05f}, {0, 1, 0.4f}, {0, 1, 0},
    {0, 1, 0.3f}, {0, 1, 1}, {0, 1, 0.1f}, {0, 1, 0.2f}, {0, 1, 0.3f}, {0, 1, 0}, {0, 1, 0.5f}, {0, 5, 0} };
  return l[(id - G_NG) % L_NP];
}

// scales (semitones), the first three are ÓRBITA 1's
struct Scale { uint8_t n; int8_t s[8]; };
inline const Scale& scale(int i) {
  static const Scale S[NSCALE] = {
    {7, {0, 2, 3, 5, 7, 9, 10}}, {7, {0, 2, 3, 5, 7, 8, 10}}, {5, {0, 3, 5, 7, 10}}, {7, {0, 1, 3, 5, 7, 8, 10}},
    {7, {0, 2, 4, 6, 7, 9, 11}}, {5, {0, 2, 4, 7, 9}}, {7, {0, 1, 4, 5, 7, 8, 10}}, {6, {0, 2, 4, 6, 8, 10}},
    {7, {0, 2, 3, 5, 7, 8, 11}}, {5, {0, 2, 3, 7, 8}} };
  return S[clampi(i, 0, NSCALE - 1)];
}
// voice models: SUB, PRISM, CRYSTAL, HALO, GLASS, DUST
enum MODEL { M_SUB, M_PRISM, M_CRYSTAL, M_HALO, M_GLASS, M_DUST, M_COUNT };

// ------------------------------------------------------------------ the default sound of each lane (ÓRBITA 1's three voices, plus HALO)
inline void laneDefaults(float* P, int l) {
  for (int k = 0; k < L_NP; k++) P[LPID(l, k)] = range(LPID(l, k)).def;
  static const float lv[NL] = {0.82f, 0.62f, 0.52f, 0.42f}, pulses[NL] = {-3, 0, 2, -4}, idx[NL] = {0.45f, 0.85f, 0.9f, 0.5f},
    dec[NL] = {0.2f, 0.24f, 0.28f, 0.7f}, pan[NL] = {0, -0.28f, 0.3f, 0}, echo[NL] = {0.02f, 0.28f, 0.34f, 0.2f},
    space[NL] = {0.08f, 0.32f, 0.42f, 0.6f}, cut[NL] = {0.62f, 0.9f, 1, 0.78f}, oct[NL] = {0, 0, 0, 0};
  P[LPID(l, L_LEVEL)] = lv[l]; P[LPID(l, L_PULSES)] = pulses[l]; P[LPID(l, L_INDEX)] = idx[l]; P[LPID(l, L_DEC)] = dec[l];
  P[LPID(l, L_PAN)] = pan[l]; P[LPID(l, L_ECHO)] = echo[l]; P[LPID(l, L_SPACE)] = space[l]; P[LPID(l, L_CUT)] = cut[l];
  P[LPID(l, L_OCT)] = oct[l]; P[LPID(l, L_MODEL)] = (float)l;
  if (l == 3) { P[LPID(l, L_ATK)] = 0.45f; P[LPID(l, L_SUS)] = 0.55f; P[LPID(l, L_REL)] = 0.6f; P[LPID(l, L_GATE)] = 4; P[LPID(l, L_DETUNE)] = 0.45f; P[LPID(l, L_ENVIDX)] = 0.3f; }
  if (l == 1) P[LPID(l, L_DETUNE)] = 0.15f;
}
inline void defaults(float* P) {
  for (int i = 0; i < G_NG; i++) P[i] = range(i).def;
  for (int l = 0; l < NL; l++) laneDefaults(P, l);
}

// ------------------------------------------------------------------ the note and rhythm rules (ÓRBITA 1, generalised)
inline float geomf(int g, float a, int v) {   // a in radians, result in [-1, 1]
  const float t = a / TAU;
  switch (g) {
    case 0: return sinp(t * (1 + v));
    case 1: return cosp(t * 2) * sinp(t);
    case 2: return sinp(t + sinp(3 * t) / TAU);
    default: return sinp(t * 1.5f + v * 0.13f) * cosp(t * 0.5f);   // SPIRAL (new)
  }
}
inline bool euclidHit(int s, int steps, int k, int rot) {
  if (steps < 1) return false; k = clampi(k, 0, steps); if (k <= 0) return false;
  int x = ((s + rot) % steps + steps) % steps; return (x * k) % steps < k;
}

// ------------------------------------------------------------------ one FM voice: carrier + two modulators (+ a sub sine), up to two detuned copies
struct Voice {
  bool on = false; int note = 0, lane = 0; float vel = 0, hz = 0; uint32_t age = 0; int offIn = -1;
  float pc[2] = {0, 0}, pm[2] = {0, 0}, pm2[2] = {0, 0}, ps = 0, prev[2] = {0, 0};
  float menv = 1, mk = 0.999f; Env amp;
};

struct Ev { uint8_t kind, lane, note, vel; int32_t at; };   // kind 1 note on, 0 note off (MIDI out, the page's lights)

struct Engine {
  float sr = 48000, params[P_COUNT], target[P_COUNT], eff[P_COUNT]; bool hasTarget = false;
  float tele[64] = {}; float scopeL[512] = {}, scopeR[512] = {}; int scopeW = 0, scopeDec = 0;
  Voice v[NL][POLY]; uint32_t vAge = 1; Rng rnd;
  // sequencer
  bool playing = false; long gstep = -1; double ctr = 0, nextAt = 0; int lastStepLen = 0;
  bool hostSync = false, hostPlaying = false, ownPlay = false; float hostBpm = 0; double hostStep = 0; bool hostNew = false;
  uint16_t custom[NL] = {0, 0, 0, 0}; uint8_t useCustom[NL] = {0, 0, 0, 0};
  int evoPhase = 0, evoDen = 0; float evoRatio = 0;
  // keyboard
  int keyRoot = -1; uint16_t held = 0; int heldN[16]; int nHeld = 0;
  // events out
  Ev ev[NEV]; int nev = 0; struct Off { int8_t lane, note; int32_t in; } offs[64]; int nOff = 0;
  // effects
  Delay<200000> eL, eR; OnePole eLP[2], eHP[2]; Smooth eTime;
  Delay<4096> chL, chR; float chPh = 0;
  Delay<24000> fdn[8]; OnePole fdnLP[8]; Delay<4800> pre; float apBuf[4][2048]; Allpass ap[4]; float fdnPh = 0;
  SVF lf[NL][2]; float lvlS[NL] = {}, panS[NL] = {}, cutS[NL] = {1, 1, 1, 1};
  Shelf lo[2], hi[2]; DCBlock dc[2]; float limG = 1, rmsAcc = 0; int rmsN = 0; float lastRms = -90, lastGR = 0;
  float laneEnv[NL] = {}; int lastNote[NL] = {}; float flash[NL] = {};

  void init(float rate) {
    sr = rate; defaults(params); for (int i = 0; i < P_COUNT; i++) target[i] = params[i];
    computeEff();
    for (int l = 0; l < NL; l++) for (int k = 0; k < POLY; k++) { v[l][k] = Voice(); v[l][k].amp.setSR(sr); }
    eL.clear(); eR.clear(); chL.clear(); chR.clear(); pre.clear(); for (auto& d : fdn) d.clear();
    for (int i = 0; i < 4; i++) { for (int j = 0; j < 2048; j++) apBuf[i][j] = 0; ap[i].buf = apBuf[i]; ap[i].g = 0.62f; }
    const float apLen[4] = {0.0047f, 0.0036f, 0.0127f, 0.0093f}; for (int i = 0; i < 4; i++) ap[i].n = clampi((int)(apLen[i] * sr), 8, 2047);
    eTime.time(0.12f, sr); eTime.snap(stepLen() * 2);
    for (int c = 0; c < 2; c++) { eHP[c].setLP(140, sr); dc[c] = DCBlock(); }
    rnd.s = 0x0B17A;
  }
  void setParam(int id, float x) { if (id >= 0 && id < P_COUNT) { Range r = range(id); params[id] = clampf(x, r.lo, r.hi);
      if (id >= G_NG && (((id - G_NG) % L_NP) == L_PULSES || ((id - G_NG) % L_NP) == L_STEPS || ((id - G_NG) % L_NP) == L_ROT)) useCustom[(id - G_NG) / L_NP] = 0;
      if (id == G_DENSITY || id == G_PHASE) for (auto& u : useCustom) u = 0; } }
  void setTarget(int id, float x) { if (id >= 0 && id < P_COUNT) { Range r = range(id); target[id] = clampf(x, r.lo, r.hi); hasTarget = true; } }
  void clearTarget() { for (int i = 0; i < P_COUNT; i++) target[i] = params[i]; hasTarget = false; }

  void computeEff() {
    const float g = hasTarget ? clampf(params[G_GRAVITY], 0, 1) : 0;
    for (int i = 0; i < P_COUNT; i++) {
      const float a = params[i], b = target[i];
      switch (morphKind(i)) {
        case 0: eff[i] = a + (b - a) * g; break;
        case 1: eff[i] = floorf_(a + (b - a) * g + 0.5f); break;
        case 2: eff[i] = g < 0.5f ? a : b; break;
        default: eff[i] = a; }
    }
  }
  float bpm() const { return hostSync && hostBpm > 1 ? clampf(hostBpm, 20, 400) : eff[G_BPM]; }   // with SYNC, the DAW's tempo even while it is stopped
  float stepLen() const { return sr * 15.f / bpm(); }
  int lanePulses(int l) const { return clampi((int)eff[G_DENSITY] + evoDen + (int)eff[LPID(l, L_PULSES)], 1, (int)eff[LPID(l, L_STEPS)]); }
  int phaseEff() const { return (int)eff[G_PHASE] + evoPhase; }
  int laneRot(int l) const { return phaseEff() * l + (int)eff[LPID(l, L_ROT)]; }
  bool laneHit(int l, int s) const {
    const int steps = (int)eff[LPID(l, L_STEPS)];
    if (useCustom[l]) return (custom[l] >> (s % steps)) & 1;
    return euclidHit(s, steps, lanePulses(l), laneRot(l));
  }
  uint16_t laneMask(int l) const { uint16_t m = 0; const int steps = (int)eff[LPID(l, L_STEPS)]; for (int s = 0; s < steps; s++) if (laneHit(l, s)) m |= 1 << s; return m; }
  void setCustom(int l, uint16_t m) { if (l >= 0 && l < NL) { custom[l] = m; useCustom[l] = 1; } }

  // the note of lane l at its step s (ÓRBITA 1's rule for lanes 0..2 with the classic scales and roots)
  int laneNote(int l, int s, float* accent = nullptr) const {
    const float a = s * PI / 8 + phaseEff() * l * PI / 8;
    const float f = geomf((int)eff[G_GEOM], a, l);
    if (accent) *accent = absf(f);
    static const int base[NL] = {0, 12, 24, 12};
    if (held && eff[G_KEYMODE] > 1.5f) {   // HARMONY: the notes you hold become the scale
      int n = clampi((int)floorf_((1 + f) * nHeld * 0.48f), 0, nHeld - 1);
      return clampi(36 + (heldN[n] % 12) + base[l] + 12 * (int)eff[LPID(l, L_OCT)] + (l == 3 ? 12 : 0), 0, 127);
    }
    const Scale& sc = scale((int)eff[G_SCALE]);
    int n = clampi((int)floorf_((1 + f) * sc.n * 0.48f), 0, sc.n - 1);
    const int root = keyRoot >= 0 && eff[G_KEYMODE] > 0.5f ? keyRoot : (int)eff[G_ROOT];
    return clampi(36 + root + base[l] + sc.s[n] + 12 * (int)eff[LPID(l, L_OCT)], 0, 127);
  }

  // ---------------------------------------------------------------- voices
  void noteOn(int l, int note, float vel, int gateSamples) {
    Voice* best = &v[l][0];
    for (auto& x : v[l]) { if (!x.on) { best = &x; break; } if (x.age < best->age) best = &x; }
    Voice& x = *best; const bool steal = x.on;
    x.on = true; x.note = note; x.lane = l; x.vel = vel; x.hz = mtof((float)note); x.age = vAge++; x.offIn = gateSamples;
    if (!steal) { x.pc[0] = x.pc[1] = x.pm[0] = x.pm[1] = x.pm2[0] = x.pm2[1] = x.ps = 0; x.prev[0] = x.prev[1] = 0; x.amp.v = 0; }
    const float A = 0.001f + powf_(eff[LPID(l, L_ATK)], 2.2f) * 3.f, D = 0.03f + powf_(eff[LPID(l, L_DEC)], 2.f) * 6.f,
      S = eff[LPID(l, L_SUS)], R = 0.01f + powf_(eff[LPID(l, L_REL)], 2.f) * 6.f;
    x.amp.set(A, D, S, R); x.amp.on();
    x.menv = 1; x.mk = expf_(-1.f / ((0.02f + D * 0.65f) * sr));
  }
  void noteOff(int l, int note) { for (auto& x : v[l]) if (x.on && x.note == note && x.offIn != -2) { x.amp.off(); x.offIn = -2; } }
  void allOff() { for (auto& L : v) for (auto& x : L) if (x.on) { x.amp.off(); x.offIn = -2; } }
  void pushEv(uint8_t kind, int l, int note, int vel, int at) { if (nev < NEV) ev[nev++] = {kind, (uint8_t)l, (uint8_t)note, (uint8_t)clampi(vel, 0, 127), at}; }

  // keyboard in (the plug-in's MIDI in, the page's keys)
  void keyDown(int note, int vel, int at = 0) {
    const int mode = (int)eff[G_KEYMODE];
    if (nHeld < 16) heldN[nHeld++] = note; held |= 1 << (note % 12);
    // keep the held notes sorted (HARMONY)
    for (int i = 1; i < nHeld; i++) for (int j = i; j > 0 && heldN[j] % 12 < heldN[j - 1] % 12; j--) { int t = heldN[j]; heldN[j] = heldN[j - 1]; heldN[j - 1] = t; }
    if (mode == 1) keyRoot = note % 12;
    if (mode == 0) { const int l = clampi((int)eff[G_PLAYLANE], 0, NL - 1); noteOn(l, note, 0.35f + 0.65f * vel / 127.f, -1); pushEv(1, l, note, vel, at); lastNote[l] = note; flash[l] = 1; }
  }
  void keyUp(int note, int at = 0) {
    int k = 0; for (int i = 0; i < nHeld; i++) if (heldN[i] != note) heldN[k++] = heldN[i]; nHeld = k;
    held = 0; for (int i = 0; i < nHeld; i++) held |= 1 << (heldN[i] % 12);
    if ((int)eff[G_KEYMODE] == 0) { const int l = clampi((int)eff[G_PLAYLANE], 0, NL - 1); noteOff(l, note); pushEv(0, l, note, 0, at); }
  }

  // ---------------------------------------------------------------- transport
  // the transport works like XO: ÓRBITA's own PLAY and STOP are independent of the DAW. With SYNC on it runs at the
  // DAW's tempo and, while the DAW plays too, on the DAW's bar.
  void start() { if (!playing) { playing = true; gstep = -1; ctr = 0; nextAt = 0; evoPhase = evoDen = 0; } }
  void play() { ownPlay = true; start(); }
  void halt() { playing = false; allOff(); flushOffs(0); }
  void stop() { ownPlay = false; halt(); }
  void flushOffs(int at) { for (int i = 0; i < nOff; i++) pushEv(0, offs[i].lane, offs[i].note, 0, at); nOff = 0; }
  void host(bool on, float b, double ppq, bool sync) {
    hostSync = sync; hostBpm = b;
    if (!sync) { hostPlaying = false; return; }
    // the DAW's play and stop never start or stop ÓRBITA (only its own PLAY / STOP do); while both play it lines up on the DAW's bar
    hostPlaying = on; hostStep = ppq * 4.0; hostNew = on && playing;
  }

  void step(long gs, int at) {
    if (gs < 0) return;
    const float sl = (float)stepLen();
    // EVOLVE: once a bar, the orbit drifts a little (a random walk that stays near the scene)
    if (gs % 16 == 0 && gs > 0 && eff[G_EVOLVE] > 0.001f && rnd.chance(eff[G_EVOLVE] * 0.85f)) {
      const int r = rnd.range(0, 3);
      if (r <= 1) evoPhase = clampi(evoPhase + (rnd.chance(0.5f) ? 1 : -1), -4, 4);
      else if (r == 2) evoDen = clampi(evoDen + (rnd.chance(0.5f) ? 1 : -1), -2, 2);
      else evoRatio = clampf(evoRatio + rnd.next() * 0.25f, -0.5f, 0.5f);
    }
    for (int l = 0; l < NL; l++) {
      if (eff[LPID(l, L_ON)] < 0.5f) continue;
      const int steps = (int)eff[LPID(l, L_STEPS)], s = (int)(gs % steps);
      bool hit = laneHit(l, s); float velBoost = 0;
      const float chaos = eff[G_CHAOS];
      if (chaos > 0.001f) {
        if (!hit && rnd.chance(chaos * 0.22f)) { hit = true; velBoost = -0.35f; }        // a ghost note
        else if (hit && rnd.chance(chaos * 0.15f)) hit = false;                         // a missing one
      }
      if (!hit || !rnd.chance(eff[LPID(l, L_PROB)])) continue;
      float acc = 0; int note = laneNote(l, s, &acc);
      if (chaos > 0.3f && rnd.chance((chaos - 0.3f) * 0.3f)) note = clampi(note + (rnd.chance(0.5f) ? 12 : -12), 0, 127);
      const float vs = eff[LPID(l, L_VEL)];
      float vel = clampf(0.62f + (acc - 0.5f) * 0.6f * vs + 0.2f + velBoost + rnd.next() * 0.12f * eff[G_HUMAN], 0.08f, 1.f);
      const int gateS = (int)(eff[LPID(l, L_GATE)] * sl * 0.92f);
      noteOn(l, note, vel, gateS);
      // a note that is still held (MIDI out) is closed first, so the new one isn't cut by the old note off
      { int k = 0; for (int i = 0; i < nOff; i++) { if (offs[i].lane == l && offs[i].note == note) pushEv(0, l, note, 0, at); else offs[k++] = offs[i]; } nOff = k; }
      pushEv(1, l, note, (int)(vel * 127), at);
      if (nOff < 64) offs[nOff++] = {(int8_t)l, (int8_t)note, at + gateS};
      lastNote[l] = note; flash[l] = 1;
    }
  }

  // ---------------------------------------------------------------- audio
  void process(float* outL, float* outR, int n) {
    computeEff();
    // tempo-synced echo time
    static const float div[8] = {1, 2, 3, 4, 6, 8, 4.f / 3, 8.f / 3};
    eTime.set(minf(div[clampi((int)eff[G_ECHOTIME], 0, 7)] * (float)stepLen(), 190000.f));
    const float eTone = 900.f * powf_(16.f, eff[G_ECHOTONE]); eLP[0].setLP(eTone, sr); eLP[1].setLP(eTone, sr);
    const float size = eff[G_SIZE], damp = eff[G_DAMP];
    const float t60 = 0.6f + size * size * 9.f;
    static const float fl[8] = {1103, 1279, 1427, 1601, 1789, 1999, 2203, 2411};
    float fdnLen[8], fdnG[8];
    for (int i = 0; i < 8; i++) { fdnLen[i] = clampf(fl[i] * (0.55f + size * 1.6f) * sr / 48000.f, 100, 23000); fdnG[i] = powf_(10.f, -3.f * fdnLen[i] / (t60 * sr)); fdnLP[i].setLP(1800.f + (1 - damp) * 14000.f, sr); }
    for (int c = 0; c < 2; c++) { lo[c].set(true, 250, -eff[G_TILT] * 4.f, sr); hi[c].set(false, 3200, eff[G_TILT] * 4.f, sr); }
    // per-lane filters and voice constants
    float lvlT[NL], panT[NL], ratioL[NL], idxL[NL], fbL[NL], envIdx[NL], det[NL], sendE[NL], sendS[NL]; int model[NL];
    const float gRatio = clampf(eff[G_RATIO] + evoRatio, 0.5f, 8.f);
    for (int l = 0; l < NL; l++) {
      lvlT[l] = eff[LPID(l, L_ON)] > 0.5f ? eff[LPID(l, L_LEVEL)] : 0; panT[l] = eff[LPID(l, L_PAN)];
      model[l] = clampi((int)eff[LPID(l, L_MODEL)], 0, M_COUNT - 1);
      ratioL[l] = gRatio * eff[LPID(l, L_RATIO)];
      static const float kIdx[M_COUNT] = {0.45f, 1.7f, 2.7f, 0.9f, 3.2f, 1.2f};   // ÓRBITA 1's depths, as phase-modulation indexes
      idxL[l] = kIdx[model[l]] * eff[LPID(l, L_INDEX)] * (1.f + eff[G_BLOOM] * 1.2f);
      fbL[l] = eff[LPID(l, L_FB)] * 1.4f; envIdx[l] = eff[LPID(l, L_ENVIDX)];
      det[l] = eff[LPID(l, L_DETUNE)]; sendE[l] = eff[LPID(l, L_ECHO)]; sendS[l] = eff[LPID(l, L_SPACE)];
      cutS[l] += (eff[LPID(l, L_CUT)] - cutS[l]) * 0.25f;
      const float fc = 60.f * powf_(300.f, cutS[l]);
      lf[l][0].set(fc, 0.6f + eff[LPID(l, L_RES)] * 5.f, sr); lf[l][1].set(fc, 0.6f + eff[LPID(l, L_RES)] * 5.f, sr);
    }
    const float echoMix = eff[G_ECHO], echoFb = eff[G_ECHOFB], space = eff[G_SPACE], chorus = eff[G_CHORUS], width = eff[G_WIDTH] * 2.f;
    const float warmth = eff[G_WARMTH], master = eff[G_MASTER] * 1.5f;
    const float drv = 1 + warmth * 1.6f;
    const float sl = (float)stepLen();
    const float ka = 1.f / n;

    // host sync: follow the song position (re-align when it jumps)
    if (hostSync && hostPlaying && hostNew && playing) {
      hostNew = false;
      const double hs = hostStep, swS = eff[G_SWING] * 0.33 * sl;
      double fb = (double)(long)hs; if (fb > hs) fb -= 1; const long base = (long)fb; const double frac = hs - fb;
      long nxt; double inS;
      if (frac * sl < 1.0 && gstep < base) { nxt = base; inS = (base & 1) ? swS : 0; }
      else if ((base & 1) && frac * sl < swS && gstep < base) { nxt = base; inS = swS - frac * sl; }
      else { nxt = base + 1; inS = (1 - frac) * sl + (((base + 1) & 1) ? swS : 0); }
      const double now = nextAt - ctr;
      if (gstep + 1 != nxt || absf((float)(now - inS)) > 0.05f * sl) { gstep = nxt - 1; ctr = 0; nextAt = inS; }
    }
    for (int i = 0; i < n; i++) {
      // the sequencer
      if (playing) {
        while (ctr >= nextAt) {
          gstep++; step(gstep, i);
          const float sw = eff[G_SWING] * 0.33f * sl;
          nextAt += sl + ((gstep & 1) ? -sw : sw);
        }
        ctr += 1;
      }
      float dryL = 0, dryR = 0, eInL = 0, eInR = 0, sIn = 0;
      for (int l = 0; l < NL; l++) {
        float aL = 0, aR = 0, envMax = 0;
        for (auto& x : v[l]) {
          if (!x.on) continue;
          if (x.offIn >= 0) { if (--x.offIn < 0) { x.amp.off(); x.offIn = -2; } }
          const float e = x.amp.tick(); if (!x.amp.active()) { x.on = false; continue; }
          envMax = maxf(envMax, e);
          x.menv *= x.mk;
          const float beta = idxL[l] / maxf(ratioL[l], 0.25f) * (envIdx[l] * x.menv + (1 - envIdx[l]) * 0.5f) * 0.16f;
          const float dt = x.hz / sr;
          const int copies = det[l] > 0.01f ? 2 : 1;
          for (int c = 0; c < copies; c++) {
            const float cents = copies == 2 ? (c ? 1.f : -1.f) * det[l] * 14.f : 0.f;
            const float d = dt * (1.f + cents * 0.000578f);
            float m2 = 0;
            switch (model[l]) {
              case M_PRISM: m2 = sinp(x.pm2[c]) * 0.35f; x.pm2[c] += d * 7.002f; break;
              case M_CRYSTAL: m2 = sinp(x.pm2[c]) * 0.6f; x.pm2[c] += d * 3.53f; break;
              case M_GLASS: m2 = sinp(x.pm2[c]) * 0.8f; x.pm2[c] += d * 9.07f; break;
              case M_DUST: m2 = rnd.next() * 0.25f * x.menv; break;
              default: break;
            }
            const float mod = sinp(x.pm[c] + m2 + fbL[l] * x.prev[c] * 0.08f);
            x.prev[c] = mod;
            float y = sinp(x.pc[c] + beta * mod);
            if (model[l] == M_SUB) { y = y * 0.78f + sinp(x.ps) * 0.38f; }
            x.pc[c] += d; x.pm[c] += d * ratioL[l];
            if (x.pc[c] > 64) x.pc[c] -= 64; if (x.pm[c] > 64) x.pm[c] -= 64; if (x.pm2[c] > 64) x.pm2[c] -= 64;
            y *= e * x.vel;
            if (copies == 2) { if (c) aR += y; else aL += y; } else { aL += y; aR += y; }
          }
          if (model[l] == M_SUB) { x.ps += dt * 0.5f; if (x.ps > 64) x.ps -= 64; }
        }
        laneEnv[l] = maxf(envMax, laneEnv[l] * 0.9995f);
        lvlS[l] += (lvlT[l] - lvlS[l]) * ka; panS[l] += (panT[l] - panS[l]) * ka;
        if (cutS[l] < 0.995f) { aL = lf[l][0].lp(aL); aR = lf[l][1].lp(aR); }
        const float g = lvlS[l] * lvlS[l] * 0.34f, p = (panS[l] + 1) * 0.125f;
        const float gl = cosp(p) * 1.41421f * g, gr = sinp(p) * 1.41421f * g;
        aL *= gl; aR *= gr;
        dryL += aL; dryR += aR;
        eInL += aL * sendE[l]; eInR += aR * sendE[l]; sIn += (aL + aR) * 0.5f * sendS[l];
      }
      // chorus (a slow stereo shimmer)
      if (chorus > 0.01f) {
        chL.push(dryL); chR.push(dryR); chPh += 0.27f / sr; if (chPh > 1) chPh -= 1;
        const float dl = (0.007f + 0.0025f * sinp(chPh)) * sr, dr = (0.007f + 0.0025f * cosp(chPh)) * sr;
        const float cl = chL.read(dl), cr = chR.read(dr);
        dryL += (cr - dryL * 0.2f) * chorus * 0.6f; dryR += (cl - dryR * 0.2f) * chorus * 0.6f;
      }
      // echo: ping-pong, filtered and warm in its loop
      const float et = eTime.v += (eTime.t - eTime.v) * eTime.k;
      float oL = eL.read(et), oR = eR.read(et);
      float fbL2 = eLP[0].lp(oR), fbR2 = eLP[1].lp(oL);
      fbL2 -= eHP[0].lp(fbL2); fbR2 -= eHP[1].lp(fbR2);
      eL.push(tanhf_((eInL + eInR) * 0.7f + fbL2 * echoFb)); eR.push(tanhf_(fbR2 * echoFb + (eInR - eInL) * 0.2f));
      // space: pre-delay, diffusion, an 8-line feedback delay network
      pre.push(sIn + (oL + oR) * 0.15f * echoMix);
      float d = pre.tap((int)(0.018f * sr));
      for (auto& a : ap) d = a.tick(d);
      fdnPh += 0.11f / sr; if (fdnPh > 1) fdnPh -= 1;
      float y[8];
      for (int k = 0; k < 8; k++) { const float mod = (k < 2) ? (k ? cosp(fdnPh) : sinp(fdnPh)) * 0.0011f * sr : 0; y[k] = fdnLP[k].lp(fdn[k].read(fdnLen[k] + mod)) * fdnG[k]; }
      // Hadamard mix
      for (int s2 = 1; s2 < 8; s2 <<= 1) for (int k = 0; k < 8; k += s2 << 1) for (int j = k; j < k + s2; j++) { const float a = y[j], b = y[j + s2]; y[j] = a + b; y[j + s2] = a - b; }
      for (int k = 0; k < 8; k++) fdn[k].push(y[k] * 0.35355f + d * ((k & 1) ? -0.5f : 0.5f));
      const float rL = (y[0] + y[2] + y[4] + y[6]) * 0.18f, rR = (y[1] + y[3] + y[5] + y[7]) * 0.18f;
      // mix
      float L = dryL + oL * echoMix * 1.2f + rL * space * 1.6f, R = dryR + oR * echoMix * 1.2f + rR * space * 1.6f;
      // width (mid/side)
      { const float m = (L + R) * 0.5f, s = (L - R) * 0.5f * width; L = m + s; R = m - s; }
      // tone tilt, DC
      L = dc[0].tick(hi[0].tick(lo[0].tick(L))); R = dc[1].tick(hi[1].tick(lo[1].tick(R)));
      // warmth: a clean soft knee, with tape-like drive blended in (never harsh)
      auto glue = [&](float x) { const float ax = absf(x); const float soft = ax < 0.6f ? x : (x > 0 ? 1.f : -1.f) * (0.6f + 0.4f * tanhf_((ax - 0.6f) / 0.4f));
        const float hot = tanhf_(x * drv) / tanhf_(drv) * 0.92f; return soft + (hot - soft) * minf(1.f, warmth * 1.6f); };
      L = glue(L * master); R = glue(R * master);
      // safety limiter at -1 dBFS
      const float pk = maxf(absf(L), absf(R)), ceil = 0.891f;
      const float want = pk > ceil ? ceil / pk : 1.f;
      limG = want < limG ? want : limG + (1.f - limG) * 0.00012f;
      L *= limG; R *= limG;
      L = softclip(L, 0.98f); R = softclip(R, 0.98f);
      outL[i] = L; outR[i] = R;
      rmsAcc += (L * L + R * R) * 0.5f; rmsN++;
      if (++scopeDec >= 2) { scopeDec = 0; scopeL[scopeW] = L; scopeR[scopeW] = R; scopeW = (scopeW + 1) & 511; }
    }
    // the gates of the sequenced notes (MIDI out)
    int k2 = 0;
    for (int i = 0; i < nOff; i++) { offs[i].in -= n; if (offs[i].in < 0) pushEv(0, offs[i].lane, offs[i].note, 0, clampi(offs[i].in + n, 0, n - 1)); else offs[k2++] = offs[i]; }
    nOff = k2;
    // keep the counters small
    if (ctr > 1e7) { ctr -= 1e6; nextAt -= 1e6; }
    // telemetry for the page
    if (rmsN > 2048) { lastRms = rmsAcc > 0 ? 10.f * log2f_(rmsAcc / rmsN) * 0.30103f : -90; rmsAcc = 0; rmsN = 0; }
    lastGR = limG < 1 ? 20.f * log2f_(limG) * 0.30103f : 0;
    tele[0] = (float)gstep; tele[1] = playing ? 1.f : 0.f; tele[2] = bpm(); tele[3] = lastRms; tele[4] = lastGR;
    tele[5] = (float)evoPhase; tele[6] = (float)evoDen; tele[7] = evoRatio; tele[8] = keyRoot < 0 ? -1.f : (float)keyRoot; tele[9] = (float)held;
    tele[10] = hasTarget ? params[G_GRAVITY] : 0;
    for (int l = 0; l < NL; l++) {
      const int steps = (int)eff[LPID(l, L_STEPS)];
      tele[12 + l] = gstep < 0 ? -1.f : (float)(gstep % steps); tele[16 + l] = laneEnv[l]; tele[20 + l] = (float)lastNote[l];
      tele[24 + l] = flash[l]; flash[l] = 0; tele[28 + l] = (float)laneMask(l); tele[32 + l] = (float)steps; tele[36 + l] = (float)lanePulses(l);
      laneEnv[l] *= 0.92f;
    }
  }
};

} // namespace orbita
