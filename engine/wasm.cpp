// ÓRBITA 2 · WebAssembly entry (no libc)
#include "orbita.h"
extern "C" {
void* memset(void* d, int c, unsigned long n) { unsigned char* p = (unsigned char*)d; while (n--) *p++ = (unsigned char)c; return d; }
void* memcpy(void* d, const void* s, unsigned long n) { unsigned char* a = (unsigned char*)d; const unsigned char* b = (const unsigned char*)s; while (n--) *a++ = *b++; return d; }
void* memmove(void* d, const void* s, unsigned long n) { unsigned char* a = (unsigned char*)d; const unsigned char* b = (const unsigned char*)s; if (a < b) while (n--) *a++ = *b++; else { a += n; b += n; while (n--) *--a = *--b; } return d; }
}
using namespace orbita;
alignas(16) static unsigned char ENGMEM[sizeof(Engine)];   // zeroed .bss, built in place by ox_init (no load-time constructor)
static Engine* E = nullptr;
static float outL[128], outR[128];
inline void* operator new(unsigned long, void* p) noexcept { return p; }
#define EXPORT extern "C" __attribute__((visibility("default")))
EXPORT void ox_init(float sr) { E = new (ENGMEM) Engine(); E->init(sr); }
EXPORT void ox_process(int n) { E->nev = 0; E->process(outL, outR, n); }
EXPORT float* ox_out_l() { return outL; }
EXPORT float* ox_out_r() { return outR; }
EXPORT float* ox_params() { return E->params; }
EXPORT int ox_param_count() { return P_COUNT; }
EXPORT void ox_param(int id, float v) { E->setParam(id, v); }
EXPORT void ox_target(int id, float v) { E->setTarget(id, v); }
EXPORT void ox_clear_target() { E->clearTarget(); }
EXPORT void ox_play() { E->play(); }
EXPORT void ox_stop() { E->stop(); }
EXPORT void ox_key(int note, int vel) { if (vel > 0) E->keyDown(note, vel); else E->keyUp(note); }
EXPORT void ox_custom(int lane, int mask) { E->setCustom(lane, (uint16_t)mask); }
EXPORT void ox_audition(int lane, int note, int vel) { E->noteOn(lane, note, vel / 127.f, (int)(E->stepLen() * 2)); }
EXPORT float* ox_tele() { return E->tele; }
EXPORT float* ox_scope_l() { return E->scopeL; }
EXPORT float* ox_scope_r() { return E->scopeR; }
EXPORT int ox_scope_w() { return E->scopeW; }
EXPORT int ox_note(int lane, int s) { return E->laneNote(lane, s); }
EXPORT int ox_ev_count() { return E->nev; }
EXPORT void* ox_ev() { return (void*)E->ev; }
EXPORT float ox_range(int id, int which) { Range r = range(id); return which == 0 ? r.lo : which == 1 ? r.hi : r.def; }
EXPORT float* ox_eff() { return E->eff; }
EXPORT void ox_compute() { E->computeEff(); }
EXPORT int ox_hit(int lane, int s) { return E->laneHit(lane, s) ? 1 : 0; }
EXPORT int ox_mask(int lane) { return E->laneMask(lane); }
EXPORT int ox_vel(int lane, int s) { float a = 0; E->laneNote(lane, s, &a); const float vs = E->eff[LPID(lane, L_VEL)]; return clampi((int)((0.82f + (a - 0.5f) * 0.6f * vs) * 127), 10, 127); }
EXPORT void ox_evo(int p, int d, float r) { E->evoPhase = p; E->evoDen = d; E->evoRatio = r; }
EXPORT int ox_custom_on(int lane) { return E->useCustom[lane]; }
EXPORT int ox_custom_mask(int lane) { return E->custom[lane]; }
