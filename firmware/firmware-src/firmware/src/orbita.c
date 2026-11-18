/* SPDX-License-Identifier: GPL-3.0-only
 * ÓRBITA for the M-VAVE FM-1 (a generative instrument; Dark Funk Durcal design, Polaris Sound Lab, Mérida), on Felucca 1.8.2's
 * drivers (Hügelton Instruments): the keys, knobs, buttons, screen, audio, flash, USB and the FM6 engine are Felucca's; what is
 * seen and played is ÓRBITA's own, as the web version (docs/): its screen replaces Felucca's pages (main.c calls orb_input,
 * orb_draw and orb_leds instead of ui_input, ui_draw and ui_leds).
 *
 * As on the web: three FM voices (SUB, PRISM, CRYSTAL: tracks 1..3, FM6 with ÓRBITA's voices built in, dfd_bank.h) play
 * euclidean rhythms over a scale; DENSITY spreads the hits (the sub three fewer, the crystal two more), PHASE turns each
 * voice's pattern and the figure, FM RATIO colours the sound, GEOMETRY (LISSAJOUS, POLAR ROSE, EPICYCLE) and SCALE choose
 * the melody: the notes are the web version's own (orbita_tables.h, generated from docs/app.js). Four SCENES set it all.
 *
 *   PRESETS knob  SCENE          K1 DENSITY 3..14     FX  mute / unmute SUB      OCT- / OCT+  the scene before / after
 *   ALGORITHM     GEOMETRY       K2 PHASE 0..15       SCL mute / unmute PRISM    PLAY         play / stop
 *   SELECT        TEMPO 40..220  K3 FM RATIO 0.5..8   ENV mute / unmute CRYSTAL  HOME         about ÓRBITA (and back)
 *                                K4 SCALE             LFO the voices (4 sets)    EDIT         English / Español
 *   The keys play the PRISM voice over the pattern. Nothing is stored apart from the music itself: at power-on the settings
 *   are read back from the patterns Felucca's autosave restored (orb_recover), or the first scene is loaded.
 * Integer maths, bounded loops; the patterns are written in the main loop, between the sequencer's steps. */
#include "orbita_tables.h"

static const uint8_t ORB_SCALE_LEN[3] = {7, 7, 5};
static const char *const ORB_GEO[3] = {"LISSAJOUS", "POLAR ROSE", "EPICYCLE"};
static const char *const ORB_SCALE[3] = {"D DORIAN", "F MINOR", "A PENTATONIC"};
static const char *const ORB_VOICE[3] = {"SUB", "PRISM", "CRYSTAL"};
static const char *const ORB_SET[5] = {"ORIGINAL", "ORBITA", "GLASS", "DUST", "TIDE"};
/* the voices: set 0 ORIGINAL, the web version's own (eng_orbita.c); sets 1..4: bank voices B1..B32 (0-based) for SUB, PRISM, CRYSTAL */
#define ORB_NSETS 5u
static const uint8_t ORB_SETV[ORB_NSETS][3] = {{0, 1, 2}, {0, 21, 4}, {1, 6, 10}, {2, 17, 24}, {3, 13, 28}};
static const uint8_t ORB_SCENE_SET[ORB_NSCENES] = {0, 0, 0, 0};   /* (as on the web: every scene, the original voices) */
static const int8_t ORB_DOFS[3] = {-3, 0, 2};

static struct {
    uint8_t scene, custom, geo, scale, dens, phase, ratio2, set, mutes, info, booted;
    uint32_t hot_ms, hot_what, sig;
} orb = {0, 0, 0, 0, 7, 2, 4, 0, 0, 0, 0, 0, 0, ~0u};

/* ------------------------------------------------------------------ the web version's pattern --- */
static uint32_t orb_hits(uint32_t v) { return (uint32_t)clamp((int32_t)orb.dens + ORB_DOFS[v % 3u], 1, 16); }
static int orb_on(uint32_t v, uint32_t s, uint32_t dens, uint32_t phase)
{
    uint32_t k = (uint32_t)clamp((int32_t)dens + ORB_DOFS[v % 3u], 1, 16);
    return ((s + phase * v) * k) % 16u < k;
}
static uint32_t orb_note(uint32_t v, uint32_t s) { return ORB_NOTE[orb.geo % 3u][orb.scale % 3u][orb.phase & 15u][v % 3u][s & 15u]; }

static void orb_write_steps(void)                 /* tracks 1..3: the 16 steps; track 4: silent */
{
    uint32_t v, s;
    for (v = 0; v < 3u; v++) {
        track_t *t = &trk[v];
        t->p[P_SLEN] = 16;
        t->p[P_SDIV] = 2;                         /* 1/16 */
        for (s = 0; s < NSTEP; s++) {
            step_t *st = &t->step[s];
            step_clear(st);
            if (s < 16u && orb_on(v, s, orb.dens, orb.phase)) {
                st->time = ST_NOTE;
                st->note[0] = (uint8_t)orb_note(v, s);
                st->n = 1;
                st->vel = 88;                     /* (the web version's velocity) */
            }
        }
    }
    for (s = 0; s < NSTEP; s++)
        step_clear(&trk[3].step[s]);
}
static void orb_ratio_set(void)                   /* FM RATIO: the original voices' RATIO, the bank voices' modulators (FM6 MRAT, 2.0 = as designed) */
{
    uint32_t v;
    for (v = 0; v < 3u; v++) {
        if (trk[v].eng_req == ENGI_ORBITA) trk[v].p[P_E1] = (int16_t)orb.ratio2;
        else trk[v].p[P_E3] = (int16_t)clamp((int32_t)orb.ratio2 - 4, -16, 16);
    }
}
static void orb_sounds(void)                      /* the set's three voices on tracks 1..3 (FM6, bank slots) */
{
    uint32_t v;
    for (v = 0; v < 3u; v++) {
        track_t *t = &trk[v];
        if (orb.set == 0u) {                      /* the web version's own voices */
            set_engine_of(t, ENGI_ORBITA);
            apply_preset_to(t, v);
        } else {
            set_engine_of(t, ENGI_FM6);
            apply_preset_to(t, 0);
            t->p[P_E7] = (int16_t)(FM6_OWN + 1u + ORB_SETV[orb.set % ORB_NSETS][v]);
        }
        t->p[P_DLY] = 0;
        t->p[P_REV] = (int16_t)(v == 0u ? 6 : v == 1u ? 14 : 22);
        t->p[P_VOICE] = V_POLY;
    }
    trk[3].p[P_LEVEL] = 0;
    orb_ratio_set();
}
static void orb_apply(uint32_t sounds)            /* everything written, one undo step */
{
    uint32_t v;
    for (v = 0; v < NTRK; v++)
        load_begin(&trk[v], (sounds ? UNDO_SOUND : 0u) | UNDO_PAT);
    if (sounds)
        orb_sounds();
    orb_write_steps();
    orb_ratio_set();
    for (v = 0; v < NTRK; v++)
        load_end(&trk[v]);
    song.sel = 1;                                 /* the keys play the PRISM voice */
}
static void orb_scene(uint32_t i)
{
    const orb_scene_t *c = &ORB_SCENES[i % ORB_NSCENES];
    orb.scene = (uint8_t)(i % ORB_NSCENES);
    orb.custom = 0;
    orb.geo = c->geo; orb.scale = c->scale; orb.dens = c->dens; orb.phase = c->phase; orb.ratio2 = c->ratio2;
    orb.set = ORB_SCENE_SET[orb.scene];
    orb.mutes = 0;
    song.g[G_BPM] = c->bpm;
    orb_apply(1);
}
/* power-on: the settings the restored patterns were written with (no store of their own); else the first scene */
static int orb_recover(void)
{
    uint32_t d, p, g, c, v, s, k, ok;
    for (v = 0; v < 3u; v++)
        if ((trk[v].eng_req != ENGI_FM6 && trk[v].eng_req != ENGI_ORBITA) || trk[v].p[P_SLEN] != 16)
            return 0;
    for (d = 3; d <= 14u; d++)
        for (p = 0; p < 16u; p++) {
            ok = 1;
            for (v = 0; v < 3u && ok; v++)
                for (s = 0; s < 16u && ok; s++)
                    ok = (trk[v].step[s].time == ST_NOTE) == orb_on(v, s, d, p);
            if (!ok)
                continue;
            for (g = 0; g < 3u; g++)
                for (c = 0; c < 3u; c++) {
                    ok = 1;
                    for (v = 0; v < 3u && ok; v++)
                        for (s = 0; s < 16u && ok; s++)
                            if (trk[v].step[s].time == ST_NOTE)
                                ok = trk[v].step[s].note[0] == ORB_NOTE[g][c][p][v][s];
                    if (!ok)
                        continue;
                    orb.dens = (uint8_t)d; orb.phase = (uint8_t)p; orb.geo = (uint8_t)g; orb.scale = (uint8_t)c;
                    if (trk[0].eng_req == ENGI_ORBITA) {
                        orb.set = 0;
                        orb.ratio2 = (uint8_t)clamp(trk[0].p[P_E1], 1, 16);
                    } else {
                        orb.ratio2 = (uint8_t)clamp(trk[0].p[P_E3] + 4, 1, 16);
                        for (k = 1; k < ORB_NSETS; k++)
                            if (trk[0].p[P_E7] == (int16_t)(FM6_OWN + 1u + ORB_SETV[k][0])) orb.set = (uint8_t)k;
                    }
                    orb.custom = 1;
                    for (k = 0; k < ORB_NSCENES; k++) {
                        const orb_scene_t *e = &ORB_SCENES[k];
                        if (e->geo == g && e->scale == c && e->dens == d && e->phase == p && e->ratio2 == orb.ratio2) {
                            orb.scene = (uint8_t)k;
                            orb.custom = 0;
                        }
                    }
                    return 1;
                }
        }
    return 0;
}

/* ------------------------------------------------------------------ the hooks Felucca's pages call (none shown) --- */
static int an_page(uint32_t g) { return g >= GR_AN_ALIGN && g <= GR_AN_VISION; }   /* (ANDROMEDA's page slots, kept: never shown) */
static void an_knob(uint32_t g, uint32_t slot, int32_t steps) { (void)g; (void)slot; (void)steps; }
static void an_oct(void) {}
static const char *an_act_name(void) { return ""; }
static void an_graph(uint32_t g) { (void)g; }
static void an_columns(uint32_t g) { uint32_t c; (void)g; for (c = 0; c < 4u; c++) draw_column(c, "-", "", "", VAL(c), -1, ICON_AUTO); }
static uint32_t an_sig(void) { return 0; }
static void an_home_vision(void) {}

/* ------------------------------------------------------------------ the main loop --- */
static void andro_poll(void)
{
    lang_es = (uint8_t)(ui_idle >> 7);            /* the language, as the settings hold it (EDIT) */
    if (!orb.booted) {
        orb.booted = 1;
        if (!orb_recover())
            orb_scene(0);
        else
            song.sel = 1;
    }
    an_live_fx = (uint32_t)(orb.mutes & 7u) << PF_M1;   /* the voices muted, as on the web (a tap on a name) */
}

static void orb_hot(uint32_t what) { orb.hot_ms = fm1_ms | 1u; orb.hot_what = what; }
static void orb_input(void)
{
    uint32_t pressed = fm1_input_edges(0), id, b;
    int32_t s, k;
    midi_take();                                  /* (MIDI IN, as ui_input does) */
    midi_pc_poll();
    fm6_poll();                                   /* the bank voices into the tracks' patches */
    settings_later_poll();
    if ((s = panel_enc(EN_PRESET)) != 0) {        /* SCENE */
        orb_scene((uint32_t)((int32_t)orb.scene + (s > 0 ? 1 : (int32_t)ORB_NSCENES - 1)) % ORB_NSCENES);
        orb_hot(10);
    }
    if ((s = panel_enc(EN_ALGO)) != 0) {          /* GEOMETRY */
        orb.geo = (uint8_t)(((int32_t)orb.geo + (s > 0 ? 1 : 2)) % 3);
        orb.custom = 1; orb_write_steps(); orb_hot(11);
    }
    if ((s = panel_enc(EN_SELECT)) != 0) {        /* TEMPO */
        song.g[G_BPM] = (int16_t)clamp(song.g[G_BPM] + s, 40, 220);
        orb_hot(12);
    }
    for (k = 0; k < 4; k++) {
        if ((s = panel_enc(EN_K1 + (uint32_t)k)) == 0)
            continue;
        if (k == 0) orb.dens = (uint8_t)clamp(orb.dens + s, 3, 14);
        if (k == 1) orb.phase = (uint8_t)clamp(orb.phase + s, 0, 15);
        if (k == 2) { orb.ratio2 = (uint8_t)clamp(orb.ratio2 + s, 1, 16); orb_ratio_set(); }
        if (k == 3) orb.scale = (uint8_t)(((int32_t)orb.scale + (s > 0 ? 1 : 2)) % 3);
        orb.custom = 1;
        if (k != 2) orb_write_steps();
        orb_hot((uint32_t)k);
    }
    for (id = 0; id < 14u; id++) {
        if (!((pressed >> id) & 1u))
            continue;
        b = panel_btn_of(id);
        switch (b) {
        case B_PLAY: transport_req = song.playing || chain_busy() || seq_counting() ? 2u : 1u; break;
        case B_FX: orb.mutes ^= 1u; orb_hot(20); break;
        case B_SCL: orb.mutes ^= 2u; orb_hot(21); break;
        case B_ENV: orb.mutes ^= 4u; orb_hot(22); break;
        case B_LFO:
            orb.set = (uint8_t)((orb.set + 1u) % ORB_NSETS);
            orb_apply(1); orb_hot(13);
            break;
        case B_EDIT:                              /* the language: the whole screen (lang.c), kept in the settings */
            lang_es ^= 1u;
            ui_idle = (uint8_t)(lang_es ? ui_idle | 0x80u : ui_idle & 0x7Fu);
            settings_later();
            break;
        case B_HOME: orb.info ^= 1u; break;
        case B_OCTDN: orb_scene((orb.scene + ORB_NSCENES - 1u) % ORB_NSCENES); orb_hot(10); break;
        case B_OCTUP: orb_scene((orb.scene + 1u) % ORB_NSCENES); orb_hot(10); break;
        default: break;
        }
        orb.sig = ~0u;
    }
}

/* ------------------------------------------------------------------ the screen: as the web version --- */
#define ORB_BG RGB(233, 232, 223)                 /* the web version's paper */
#define ORB_INK RGB(26, 28, 25)
#define ORB_DIM RGB(96, 100, 87)
#define ORB_PANEL RGB(25, 29, 25)                 /* the phase plane */
#define ORB_GRID RGB(48, 55, 47)
#define ORB_OFF RGB(213, 215, 203)
#define ORB_LIME RGB(206, 254, 101)
static const uint16_t ORB_COL[3] = {RGB(206, 255, 120), RGB(175, 154, 241), RGB(241, 160, 125)};

static int32_t orb_s(int32_t a) { return AN_S(a); }          /* sin, 256 steps a turn, Q14 */
static int32_t orb_c(int32_t a) { return AN_C(a); }
/* a point of voice v's orbit at angle a (256 steps a turn): the web version's pt(a, 0), x and y in Q14 */
static void orb_pt(uint32_t v, int32_t a, int32_t *x, int32_t *y)
{
    int32_t p = (int32_t)orb.phase * 16 + (int32_t)v * 18;    /* phase * tau / 16 + v * 0.44 */
    if (orb.geo == 0u) {
        *x = orb_s(2 * a + p);
        *y = orb_s(3 * a + (int32_t)v * 12);                  /* + v * 0.3 */
    } else if (orb.geo == 1u) {
        int32_t r = orb_c(5 * a + p);
        *x = r * orb_c(a) / 16384;
        *y = r * orb_s(a) / 16384;
    } else {
        *x = (orb_c(a) * 7 + orb_c(4 * a + p) * 3) / 10;
        *y = (orb_s(a) * 7 - orb_s(4 * a + p) * 3) / 10;
    }
}
static void orb_plane(int32_t x0, int32_t y0, int32_t w, int32_t h, int32_t cur)   /* the phase plane at (x0, y0), w x h */
{
    int32_t v, j, cx = x0 + w / 2, cy = y0 + h / 2, R = (w * 35 / 100 < h * 33 / 100 ? w * 35 / 100 : h * 33 / 100), x, y, px = 0, py = 0;
    for (x = x0 + (cx - x0) % 20; x < x0 + w; x += 20) cv_rect(x, y0, 1, h, ORB_GRID);
    for (y = y0 + (cy - y0) % 20; y < y0 + h; y += 20) cv_rect(x0, y, w, 1, ORB_GRID);
    for (v = 0; v < 3; v++) {
        uint16_t c = (orb.mutes >> v) & 1u ? ux_mix(ORB_PANEL, ORB_COL[v], 18) : ORB_COL[v];
        int32_t sx = R * 138 / 100, sy = R * (1000 - v * 65) / 1000;
        for (j = 0; j <= 96; j++) {               /* the orbit: 96 segments */
            orb_pt((uint32_t)v, j * 256 / 96, &x, &y);
            x = cx + x * sx / 16384; y = cy + y * sy / 16384;
            if (j) an_line(px, py, x, y, 1, c);
            px = x; py = y;
        }
        for (j = 0; j < 16; j++) {                /* the hits: dots on the orbit; the step now: a ring */
            if (!orb_on((uint32_t)v, (uint32_t)j, orb.dens, orb.phase))
                continue;
            orb_pt((uint32_t)v, j * 16, &x, &y);
            x = cx + x * sx / 16384; y = cy + y * sy / 16384;
            an_disc(x, y, 2, c);
            if (j == cur && !((orb.mutes >> v) & 1u)) an_ring(x, y, 6, 256, 0, 0, 1, c);
        }
    }
}
static void orb_seq(int32_t y0, int32_t cur)      /* the sequencer: three rows of 16, as on the web */
{
    int32_t v, s;
    for (v = 0; v < 3; v++) {
        int32_t y = y0 + v * 15, muted = (orb.mutes >> v) & 1;
        cv_text_on(4, y + 1, &AF_S, ORB_VOICE[v], muted ? ux_mix(ORB_BG, ORB_INK, 40) : ORB_INK, ORB_BG);
        for (s = 0; s < 16; s++) {
            int32_t x = 58 + s * 11, on = orb_on((uint32_t)v, (uint32_t)s, orb.dens, orb.phase);
            uint16_t c = on ? (muted ? ux_mix(ORB_OFF, ORB_COL[v], 45) : ORB_COL[v]) : ORB_OFF;
            cv_rect(x, y, 10, 12, c);
            if (s == cur) { cv_rect(x - 1, y - 1, 12, 1, ORB_INK); cv_rect(x - 1, y + 12, 12, 1, ORB_INK);
                            cv_rect(x - 1, y, 1, 12, ORB_INK); cv_rect(x + 10, y, 1, 12, ORB_INK); }
        }
    }
}
static void orb_num(char *b, uint32_t v) { fmt_int(b, v); }
static void orb_ratio_txt(char *b)                /* 0.5 .. 8.0 */
{
    fmt_int(b, orb.ratio2 / 2u);
    str_cpy(b + str_len(b), orb.ratio2 & 1u ? ".5" : ".0", 3);
}
/* the line under the sequencer: the knob just turned in large, else the four settings */
static void orb_status(int32_t y)
{
    char b[24];
    const char *label = 0;
    int hot = orb.hot_ms && fm1_ms - (orb.hot_ms & ~1u) < 1500u;
    b[0] = 0;
    if (hot) {
        switch (orb.hot_what) {
        case 0: label = "DENSITY"; orb_num(b, orb.dens); break;
        case 1: label = "PHASE"; orb_num(b, orb.phase); break;
        case 2: label = "FM RATIO"; orb_ratio_txt(b); break;
        case 3: label = "SCALE"; str_cpy(b, ORB_SCALE[orb.scale % 3u], sizeof b); break;
        case 10: label = "SCENE"; str_cpy(b, ORB_SCENES[orb.scene].name, sizeof b); break;
        case 11: label = "GEOMETRY"; str_cpy(b, ORB_GEO[orb.geo % 3u], sizeof b); break;
        case 12: label = "TEMPO"; orb_num(b, (uint32_t)song.g[G_BPM]); break;
        case 13: label = "VOICES"; str_cpy(b, ORB_SET[orb.set % ORB_NSETS], sizeof b); break;
        default: label = ORB_VOICE[(orb.hot_what - 20u) % 3u]; str_cpy(b, (orb.mutes >> ((orb.hot_what - 20u) % 3u)) & 1u ? "MUTED" : "ON", sizeof b); break;
        }
        cv_text_on(4, y, &AF_S, label, ORB_DIM, ORB_BG);
        cv_text_on(4, y + 12, &AF_M, b, ORB_INK, ORB_BG);
        return;
    }
    {   /* DENSITY  PHASE  FM RATIO  SCALE: four columns */
        static const char *const L[4] = {"DENSITY", "PHASE", "FM RATIO", "SCALE"};
        int32_t c;
        for (c = 0; c < 4; c++) {
            int32_t x = 4 + c * 59 + (c == 1 ? 6 : 0);
            cv_text_on(x, y, &AF_S, L[c], ORB_DIM, ORB_BG);
            if (c == 0) orb_num(b, orb.dens);
            if (c == 1) orb_num(b, orb.phase);
            if (c == 2) orb_ratio_txt(b);
            if (c == 3) str_cpy(b, orb.scale == 0u ? "D DOR" : orb.scale == 1u ? "F MIN" : "A PENT", sizeof b);
            cv_text_on(x, y + 12, &AF_M, b, ORB_INK, ORB_BG);
        }
    }
}
static void orb_info_screen(void)                 /* HOME: about ÓRBITA, the controls, the credits */
{
    static const char *const LINES[8] = {"PRESETS  SCENE", "ALGORITHM  GEOMETRY", "SELECT  TEMPO", "K1 DENSITY  K2 PHASE",
                                         "K3 FM RATIO  K4 SCALE", "FX SCL ENV  MUTE A VOICE", "LFO VOICES  EDIT LANG", "OCT- OCT+  SCENE"};
    int32_t i;
    cv_begin(240, 120, ORB_BG);
    cv_text_on(8, 6, &AF_M, "ORBITA", ORB_INK, ORB_BG);
    cv_text_on(8, 30, &AF_S, "A GENERATIVE INSTRUMENT", ORB_DIM, ORB_BG);
    for (i = 0; i < 5; i++) cv_text_on(8, 48 + i * 14, &AF_S, LINES[i], ORB_INK, ORB_BG);
    cv_blit(0, 0);
    cv_begin(240, 120, ORB_BG);
    for (i = 5; i < 8; i++) cv_text_on(8, 2 + (i - 5) * 14, &AF_S, LINES[i], ORB_INK, ORB_BG);
    cv_rect(8, 46, 224, 1, ORB_OFF);
    cv_text_on(8, 52, &AF_S, "DARK FUNK DURCAL DESIGN", ORB_INK, ORB_BG);
    cv_text_on(8, 66, &AF_S, "POLARIS SOUND LAB - MERIDA", ORB_INK, ORB_BG);
    cv_text_on(8, 80, &AF_S, "@polaris.sound.lab", ORB_DIM, ORB_BG);
    cv_text_on(8, 94, &AF_S, "drophlex.github.io/ORBITA", ORB_DIM, ORB_BG);
    cv_text_on(170, 94, &AF_S, "HOME", ORB_INK, ORB_BG);
    cv_blit(0, 120);
}
static void orb_draw(void)
{
    int32_t cur = song.playing ? (int32_t)(trk[1].seq_idx % 16u) : -1;
    int hot = orb.hot_ms && fm1_ms - (orb.hot_ms & ~1u) < 1500u;
    uint32_t sig = (uint32_t)cur * 3u + orb.geo * 7u + orb.scale * 11u + orb.dens * 13u + orb.phase * 17u + orb.ratio2 * 19u +
                   orb.mutes * 23u + orb.scene * 29u + orb.set * 31u + (uint32_t)song.g[G_BPM] * 37u + lang_es * 41u +
                   (uint32_t)hot * 43u + orb.hot_what * 47u + orb.info * 53u + (uint32_t)song.playing * 59u + orb.custom * 61u;
    char b[24];
    if (!scr_frame())                             /* MENU > SCREEN OFF (kept from the settings): dark */
        return;
    if (sig == orb.sig && !ui.force)
        return;
    orb.sig = sig;
    ui.force = 0;
    if (orb.info) { orb_info_screen(); scr_shown(); return; }
    /* band 1 (y 0..37): the name, the scene, the tempo and the transport */
    cv_begin(240, 38, ORB_BG);
    cv_text_on(6, 3, &AF_M, "ORBITA", ORB_INK, ORB_BG);
    cv_text_on(6, 22, &AF_S, orb.custom ? "MY ORBIT" : ORB_SCENES[orb.scene].name, ORB_DIM, ORB_BG);
    cv_rrect(174, 6, 60, 24, 3, song.playing ? ORB_LIME : ORB_BG, ORB_BG);
    cv_rect(174, 6, 60, 1, ORB_INK); cv_rect(174, 29, 60, 1, ORB_INK); cv_rect(174, 6, 1, 24, ORB_INK); cv_rect(233, 6, 1, 24, ORB_INK);
    if (song.playing) cv_rect(181, 13, 9, 9, ORB_INK);
    else { int32_t r; for (r = 0; r < 10; r++) cv_rect(181 + r / 2, 13 + r / 2, 1, 10 - r, ORB_INK); }
    orb_num(b, (uint32_t)song.g[G_BPM]);
    cv_text_on(198, 12, &AF_S, b, ORB_INK, song.playing ? ORB_LIME : ORB_BG);
    cv_text_on(120, 12, &AF_S, "BPM", ORB_DIM, ORB_BG);
    cv_blit(0, 0);
    /* band 2 (y 38..151): the phase plane */
    cv_begin(240, 114, ORB_BG);
    cv_rrect(4, 0, 232, 114, 6, ORB_PANEL, ORB_BG);
    orb_plane(6, 2, 228, 110, cur);
    str_cpy(b, ORB_GEO[orb.geo % 3u], sizeof b);
    cv_text_on(12, 5, &AF_S, b, ux_mix(ORB_PANEL, RGB(232, 235, 225), 70), ORB_PANEL);
    if (cur >= 0) {
        fmt_int(b, (uint32_t)cur + 1u);
        str_cpy(b + str_len(b), " / 16", 6);
        cv_text_on(228 - text_w(&AF_S, b), 5, &AF_S, b, ux_mix(ORB_PANEL, RGB(232, 235, 225), 70), ORB_PANEL);
    }
    cv_blit(0, 38);
    /* band 3 (y 152..239): the sequencer, then the settings */
    cv_begin(240, 88, ORB_BG);
    orb_seq(2, cur);
    orb_status(50);
    cv_blit(0, 152);
    scr_shown();
}
static void orb_leds(void)                        /* PLAY green while playing; FX SCL ENV lit = the voice sounds; HOME on its page */
{
    uint8_t nl[FM1_NCOL] = {0}, nd[FM1_NCOL] = {0};
    uint32_t c, k;
    static uint8_t ready;
    if (fm1_led_anim_on())
        return;
    if (!ready) { led_pos_init(); ready = 1; }
    led_put(nl, panel.btn[B_FX], !(orb.mutes & 1u));
    led_put(nl, panel.btn[B_SCL], !(orb.mutes & 2u));
    led_put(nl, panel.btn[B_ENV], !(orb.mutes & 4u));
    led_put(nl, panel.btn[B_HOME], orb.info);
    for (k = 0; k < NB; k++)
        led_put(nd, panel.btn[k], 1);
    if (song.playing)
        led_clear(nd, panel.btn[B_PLAY]);
    if (song.playing)
        nl[LED_PLAY_GREEN >> 3] |= (uint8_t)(1u << (LED_PLAY_GREEN & 7u));
    for (c = 0; c < FM1_NCOL; c++) {
        fm1_led_dim[c] = nd[c];
        fm1_led_breath[c] = 0;
        fm1_led_mid[c] = 0;
        fm1_led[c] = nl[c];
    }
}
