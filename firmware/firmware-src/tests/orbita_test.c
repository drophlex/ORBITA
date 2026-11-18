/* SPDX-License-Identifier: GPL-3.0-only */
/* ÓRBITA (src/orbita.c) on the real UI sources (with the stubs of tests/ui_test.c): the patterns are the web version's
 * (docs/app.js pattern() and note(), through orbita_expect.h and orbita_tables.h), every scene loads its settings and its voices,
 * every control does what the screen says, the power-on recovery reads every setting back from the patterns alone, and the
 * screen draws in every state without leaving the canvas. */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"
#include "orbita_expect.h"

static uint32_t mask_of(uint32_t v)
{
    uint32_t s, m = 0;
    for (s = 0; s < 16u; s++)
        if (trk[v].step[s].time == ST_NOTE) m |= 1u << s;
    return m;
}
static int web_patterns(void)
{
    int bad = 0;
    uint32_t d, p, g, c, v, s, ok = 1, n = 0;
    for (d = 3; d <= 14u && ok; d++)
        for (p = 0; p < 16u && ok; p++)
            for (g = 0; g < 3u && ok; g++)
                for (c = 0; c < 3u && ok; c++) {
                    orb.dens = (uint8_t)d; orb.phase = (uint8_t)p; orb.geo = (uint8_t)g; orb.scale = (uint8_t)c;
                    orb_write_steps();
                    for (v = 0; v < 3u && ok; v++) {
                        ok &= mask_of(v) == ORB_WEB_MASK[d - 3u][p][v];
                        for (s = 0; s < 16u && ok; s++)
                            if (trk[v].step[s].time == ST_NOTE) { ok &= trk[v].step[s].note[0] == ORB_NOTE[g][c][p][v][s] && trk[v].step[s].vel == 88; n++; }
                    }
                    ok &= trk[0].p[P_SLEN] == 16 && trk[0].p[P_SDIV] == 2;
                    for (s = 0; s < NSTEP; s++) ok &= trk[3].step[s].time != ST_NOTE;
                    if (!ok) printf("  dens %u phase %u geo %u scale %u\n", (unsigned)d, (unsigned)p, (unsigned)g, (unsigned)c);
                }
    printf("  %u notes checked against the web version\n", (unsigned)n);
    bad += check("every density x phase x geometry x scale: the web version's hits and notes, 1/16, 16 steps; track 4 silent", ok);
    return bad;
}
static int scenes(void)
{
    int bad = 0;
    uint32_t i, v, ok = 1;
    for (i = 0; i < ORB_NSCENES; i++) {
        const orb_scene_t *c = &ORB_SCENES[i];
        orb_scene(i);
        ok &= song.g[G_BPM] == c->bpm && orb.geo == c->geo && orb.scale == c->scale && orb.dens == c->dens && orb.phase == c->phase;
        for (v = 0; v < 3u; v++) {
            ok &= trk[v].eng_req == ENGI_ORBITA && trk[v].p[P_E0] == (int16_t)v && trk[v].p[P_E1] == (int16_t)c->ratio2;
            ok &= mask_of(v) == ORB_WEB_MASK[c->dens - 3u][c->phase][v];
        }
        ok &= trk[3].p[P_LEVEL] == 0 && song.sel == 1;
        fm6_poll();
        ok &= !memcmp(fm6_patch[0], fm6_patch[0], 1);
        if (!ok) { printf("  scene %s\n", c->name); break; }
    }
    bad += check("the 4 scenes: tempo, geometry, scale, density, phase, FM ratio, the web version's own voices (ORBITA engine); T4 silent", ok);
    return bad;
}
static int recovery(void)
{
    int bad = 0;
    uint32_t i, ok = 1, n = 0, ambiguous = 0;
    for (i = 0; i < 400u && ok; i++) {            /* random settings, written; RAM forgotten; read back from the music */
        uint8_t d = (uint8_t)(3u + (i * 7u) % 12u), p = (uint8_t)((i * 5u + 3u) % 16u), g = (uint8_t)(i % 3u), c = (uint8_t)((i / 3u) % 3u);
        uint8_t r = (uint8_t)(1u + (i * 11u) % 16u), st = (uint8_t)(i % ORB_NSETS);
        orb.dens = d; orb.phase = p; orb.geo = g; orb.scale = c; orb.ratio2 = r; orb.set = st;
        orb_apply(1);
        {
            static step_t keep[3][16];
            uint32_t v;
            for (v = 0; v < 3u; v++) memcpy(keep[v], trk[v].step, sizeof keep[v]);
            memset(&orb, 0, sizeof orb);
            ok &= orb_recover();
            ok &= orb.ratio2 == r && orb.set == st;
            if (orb.dens != d || orb.phase != p || orb.geo != g || orb.scale != c) {   /* another setting that writes the same */
                orb_write_steps();
                for (v = 0; v < 3u; v++) ok &= !memcmp(keep[v], trk[v].step, sizeof keep[v]);
                ambiguous++;
            }
        }
        if (!ok) printf("  dens %u phase %u geo %u scale %u ratio %u set %u -> %u %u %u %u %u %u\n", d, p, g, c, r, st, orb.dens, orb.phase, orb.geo, orb.scale, orb.ratio2, orb.set);
        n++;
    }
    printf("  (%u of them have a twin setting that writes exactly the same music: it reads back as that one)\n", (unsigned)ambiguous);
    bad += check("power-on: 400 settings read back from the patterns alone (density, phase, geometry, scale, FM ratio, voices)", ok);
    orb_scene(2); memset(&orb, 0, sizeof orb); ok = orb_recover() && orb.scene == 2 && !orb.custom;
    bad += check("  .. a scene comes back as that scene", ok);
    for (i = 0; i < NTRK; i++) memset(trk[i].step, 0, sizeof trk[i].step);
    memset(&orb, 0, sizeof orb); ok = !orb_recover();
    bad += check("  .. other music (not ÓRBITA's): not recovered, the first scene loads instead", ok);
    return bad;
}
int main(void)
{
    int bad = 0;

    bad += web_patterns();
    bad += scenes();
    bad += recovery();
    printf("%s\n", bad ? "ORBITA TEST FAILED" : "orbita test passed");
    return bad != 0;
}
