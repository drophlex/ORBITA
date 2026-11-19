// ÓRBITA 2 engine test: classic notes and rhythms = ÓRBITA 1's, levels (no clipping), sync, MIDI out events
#include "../engine/orbita.h"
#include <cstdio>
#include <cmath>
#include <vector>
using namespace orbita;
static int fails = 0;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)
// ÓRBITA 1 (docs/app.js): note(v,s) and pattern()
static int webNote(int v, int s, int scIdx, int mode, int phase) {
  static const int sc[3][7] = {{0,2,3,5,7,9,10},{0,2,3,5,7,8,10},{0,3,5,7,10}}; static const int len[3] = {7,7,5}, roots[3] = {38,41,45};
  double a = s*M_PI/8 + phase*v*M_PI/8; double f[3] = {sin(a*(1+v)), cos(a*2)*sin(a), sin(a+sin(3*a))};
  int n = (int)floor((1+f[mode])*len[scIdx]*.48); return roots[scIdx] + v*12 + sc[scIdx][n];
}
static bool webHit(int v, int s, int den, int phase) { int k = std::min(16, std::max(1, den + (v==0?-3:v==1?0:2))); return ((s+phase*v)*k)%16 < k; }
static Engine E;
int main() {
  E.init(48000);
  // 1. classic compatibility: all scales x geometries x phases x densities, lanes 0..2
  int noteMiss = 0, hitMiss = 0, cases = 0;
  const int rootOf[3] = {2, 5, 9};
  for (int sc = 0; sc < 3; sc++) for (int g = 0; g < 3; g++) for (int ph = 0; ph < 16; ph++) for (int den = 3; den <= 14; den++) {
    E.setParam(G_SCALE, sc); E.setParam(G_GEOM, g); E.setParam(G_PHASE, ph); E.setParam(G_DENSITY, den); E.setParam(G_ROOT, rootOf[sc]); E.computeEff();
    for (int v = 0; v < 3; v++) for (int s = 0; s < 16; s++) { cases++;
      if (E.laneNote(v, s) != webNote(v, s, sc, g, ph)) noteMiss++;
      if (E.laneHit(v, s) != webHit(v, s, den, ph)) hitMiss++; }
  }
  printf("classic: %d cases, %d note differences, %d rhythm differences\n", cases, noteMiss, hitMiss);
  CHECK(hitMiss == 0, "rhythms differ from ÓRBITA 1");
  CHECK(noteMiss * 1000 < cases, "too many notes differ (%d)", noteMiss);   // the sine table can round a note at an exact boundary
  // 2. levels: render 16 bars of the default sound and stressed settings
  auto render = [&](const char* name, void (*setup)(Engine&), float& peak, float& rms) {
    E.init(48000); setup(E); E.play(); float L[128], R[128]; double acc = 0; long n = 0; peak = 0;
    for (int b = 0; b < 48000 * 30 / 128; b++) { E.process(L, R, 128); for (int i = 0; i < 128; i++) { peak = std::max(peak, std::max(fabsf(L[i]), fabsf(R[i]))); acc += L[i]*L[i] + R[i]*R[i]; n += 2; CHECK(std::isfinite(L[i]) && std::isfinite(R[i]), "%s: not finite", name); if (!std::isfinite(L[i])) return; } }
    rms = 10 * log10(acc / n + 1e-12); printf("%-26s peak %6.2f dBFS  rms %6.1f dBFS  gr %5.1f dB\n", name, 20*log10(peak+1e-9), rms, E.lastGR);
  };
  float pk, rm;
  render("default", [](Engine&) {}, pk, rm); CHECK(pk < 0.95f, "default clips"); CHECK(rm > -30 && rm < -10, "default level %f", rm);
  render("dense + bloom + warmth", [](Engine& e) { e.setParam(G_DENSITY, 14); e.setParam(G_BLOOM, 1); e.setParam(G_WARMTH, 1); e.setParam(G_MASTER, 1); for (int l = 0; l < NL; l++) e.setParam(LPID(l, L_LEVEL), 1); }, pk, rm); CHECK(pk < 0.95f, "stress clips");
  render("space + echo max", [](Engine& e) { e.setParam(G_SPACE, 1); e.setParam(G_SIZE, 1); e.setParam(G_ECHO, 1); e.setParam(G_ECHOFB, 0.95f); e.setParam(G_DAMP, 0); }, pk, rm); CHECK(pk < 0.95f, "fx clips");
  render("chaos + evolve", [](Engine& e) { e.setParam(G_CHAOS, 1); e.setParam(G_EVOLVE, 1); e.setParam(G_HUMAN, 1); }, pk, rm);
  render("glass + dust models", [](Engine& e) { e.setParam(LPID(1, L_MODEL), M_GLASS); e.setParam(LPID(2, L_MODEL), M_DUST); e.setParam(LPID(1, L_FB), 1); }, pk, rm);
  // 3. silence after stop (the tails fade, nothing hangs)
  { E.init(48000); E.play(); float L[128], R[128]; for (int b = 0; b < 2000; b++) E.process(L, R, 128); E.stop(); E.setParam(G_SPACE, 0); E.setParam(G_ECHO, 0);
    for (int b = 0; b < 48000 * 8 / 128; b++) E.process(L, R, 128); float p = 0; for (int i = 0; i < 128; i++) p = std::max(p, fabsf(L[i])); CHECK(p < 1e-3f, "not silent after stop: %f", p);
    int on = 0; for (auto& l : E.v) for (auto& x : l) on += x.on; CHECK(on == 0, "%d voices still on", on); }
  // 4. events: every note on gets its note off; host sync starts on the bar
  { E.init(48000); E.play(); float L[128], R[128]; int ons = 0, offs = 0;
    for (int b = 0; b < 3000; b++) { E.nev = 0; E.process(L, R, 128); for (int k = 0; k < E.nev; k++) { if (E.ev[k].kind) ons++; else offs++; CHECK(E.ev[k].at >= 0 && E.ev[k].at < 128, "event out of the block"); } }
    E.nev = 0; E.stop(); offs += E.nev; printf("events: %d note on, %d note off\n", ons, offs); CHECK(ons > 50 && ons == offs, "on/off mismatch"); }
  { E.init(48000); E.play(); float L[128], R[128]; double ppq = 0; const double bpm = 120, ppqPerBlock = 128.0 / 48000 * bpm / 60; std::vector<long> at;
    for (int b = 0; b < 400; b++) { E.host(true, bpm, ppq, true); long before = E.gstep; E.nev = 0; E.process(L, R, 128); if (E.gstep != before) at.push_back(b); ppq += ppqPerBlock; }
    // 120 bpm: a 16th = 6000 samples = 46.875 blocks
    int bad = 0; for (size_t i = 1; i < at.size(); i++) { long d = at[i] - at[i-1]; if (d < 46 || d > 48) bad++; }
    printf("sync: %zu steps, %d irregular\n", at.size(), bad); CHECK(at.size() >= 8 && bad == 0, "sync irregular"); CHECK(!at.empty() && at[0] == 0, "first step not at the start"); }
  // 4b. XO transport: ÓRBITA's own play survives the DAW's stop; the DAW's own start stops with it
  { E.init(48000); float L[128], R[128]; E.play(); E.host(true, 120, 0, true); E.process(L, R, 128); E.host(false, 120, 1, true); E.process(L, R, 128);
    CHECK(E.playing, "own play stopped by the DAW"); E.stop(); E.host(true, 120, 2, true); E.process(L, R, 128); CHECK(!E.playing, "the DAW's play started it");
    CHECK(E.bpm() == 120, "tempo not the DAW's"); }
  // 5. gravity morph
  { E.init(48000); E.setTarget(G_DENSITY, 13); E.setParam(G_GRAVITY, 1); E.computeEff(); CHECK((int)E.eff[G_DENSITY] == 13, "gravity"); E.setParam(G_GRAVITY, 0); E.computeEff(); CHECK((int)E.eff[G_DENSITY] == 7, "gravity back"); }
  printf(fails ? "%d FAILED\n" : "all ok\n", fails); return fails ? 1 : 0;
}
