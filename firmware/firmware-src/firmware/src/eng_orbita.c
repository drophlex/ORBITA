/* SPDX-License-Identifier: GPL-3.0-only
 * ORBITA: the web version's own voices (docs/app.js voice()), for ÓRBITA on the FM-1 (Polaris Sound Lab).
 * A sine carrier whose frequency a sine modulator at RATIO x the note sweeps (frequency modulation, as the browser's
 * oscillator.frequency input does), the sweep starting at the note's frequency x DEPTH; an 8 ms rise to the voice's level,
 * then the level and the sweep die away together by 80 dB over the TAIL, which, as on the web, is the step's length
 * (15 / BPM s) times the voice's own factor: the notes ring longer at slower tempi.
 *   VOICE  SUB      DEPTH 0.45  TAIL 1.9 steps  LEVEL 0.26
 *          PRISM          1.7        2.8              0.13
 *          CRYSTAL        2.7        3.6              0.10
 *   RATIO  0.5 .. 8 (the web's FM ratio; 2.0: as the scenes' Mérida Ring)
 * Hits: the release does not shorten a note (the web's notes play out their tail). Integer maths: Q30 envelopes, one
 * 64-bit product per sample for the sweep. State: the voice's s[] (no engine memory). */

static const char *const N_ORB_VOICE[] = {"SUB", "PRISM", "CRYSTAL"};
static const int32_t ORBV_DEPTH[3] = {1843, 6963, 11059};   /* 0.45 1.7 2.7, Q12 */
static const int32_t ORBV_TAIL[3] = {30, 45, 58};           /* 1.9 2.8 3.6 steps, Q4 */
static const int32_t ORBV_LEVEL[3] = {13000, 6500, 5000};  /* 0.26 0.13 0.10 (x 100000) as Q15 levels: the same ratios */
static const int32_t ORBV_LDB[3] = {4476109, 4082893, 3932160};   /* the level's fall over the tail: 68.3 62.3 60.0 dB (.26 .13 .1 -> .0001), Q16 */
static const int32_t ORBV_LOGD[3] = {-75497, 50168, 93913};      /* log2 of DEPTH, Q16 */
#define ORBV_ATTACK (FS / 125u)                                  /* 8 ms */

/* an exponential fall of db (Q16 dB) over n samples: the factor per sample, Q30 (exp(-a) ~ 1 - a + a^2 / 2) */
static int32_t orbv_coef(int32_t db, int32_t n)
{
    int32_t q = db / n, x = q * 1886 + (db - q * n) * 1886 / n;  /* a = ln(10) / 20 x dB / n, Q30 (32-bit division only) */
    return (int32_t)((1LL << 30) - x + ((x * x) >> 31));
}
/* s[0] the level (Q30) and s[1] its factor, s[4] the sweep (Q30) and s[5] its factor (each falls as the web's exponential ramp:
 * the level from the voice's own to .0001, the sweep from the note's frequency x DEPTH to 0.01 Hz), s[2] samples into the rise,
 * s[6] the tail's samples left, s[3] the voice */
static void orbv_note_on(track_t *t, voice_t *v)
{
    uint32_t r = (uint32_t)clamp(t->p[P_E0], 0, 2), bpm = (uint32_t)clamp(song.g[G_BPM], 20, 300);
    int32_t n = (int32_t)(15u * (uint32_t)ORBV_TAIL[r] * FS / 16u / bpm);   /* the tail in samples: d x TAIL, d = 15 / BPM s */
    int32_t oct = 575493 + ((int32_t)v->note - 69) * 65536 / 12 + ORBV_LOGD[r] + 435440;   /* log2(f x DEPTH x 100), Q16 */
    if (n < 64) n = 64;
    v->s[0] = 1 << 30;
    v->s[1] = orbv_coef(ORBV_LDB[r], n);
    v->s[4] = 1 << 30;
    v->s[5] = orbv_coef(oct > 0 ? (int32_t)(((int64_t)oct * 3083) >> 9) : 0, n);    /* 20 log10(f D / .01) dB */
    v->s[2] = 0;
    v->s[6] = n;
    v->s[3] = (int32_t)r;
    v->ph[0] = v->ph[1] = 0;
}
static int orbv_done(track_t *t, voice_t *v)
{
    (void)t;
    return v->s[2] >= (int32_t)ORBV_ATTACK && v->s[6] <= 0;     /* (as the web's oscillators stop at the tail's end) */
}
static void orbv_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    uint32_t i, r = (uint32_t)v->s[3] % 3u, inc = m->inc, minc;
    int32_t ratio2 = clamp(t->p[P_E1], 1, 16), e = v->s[0], k = v->s[1], at = v->s[2], dv = v->s[4], dk = v->s[5], left = v->s[6];
    int64_t dmax = (int64_t)inc * ORBV_DEPTH[r] >> 12;      /* the sweep at its start, as a phase increment */
    minc = (uint32_t)(((uint64_t)inc * (uint32_t)ratio2) >> 1);
    for (i = 0; i < n; i++) {
        int32_t g, s, y, rise;
        v->ph[1] += minc;
        s = sine_i(v->ph[1]);
        g = (int32_t)((dmax * (dv >> 15)) >> 15);           /* the sweep now */
        v->ph[0] += inc + (uint32_t)(((int64_t)g * s) >> 15);
        rise = at < (int32_t)ORBV_ATTACK ? 3 + at * (at * 32764 / (int32_t)ORBV_ATTACK) / (int32_t)ORBV_ATTACK : 32767;
        y = mulq15(sine_i(v->ph[0]), mulq15(ORBV_LEVEL[r], rise));
        y = mulq15(y, e >> 15);
        out[i] += left > 0 || at < (int32_t)ORBV_ATTACK ? voice_amp(y, m, i) << 1 : 0;
        if (at < (int32_t)ORBV_ATTACK) {
            at++;
        } else {
            e = (int32_t)(((int64_t)e * k) >> 30);
            left--;
        }
        dv = (int32_t)(((int64_t)dv * dk) >> 30);             /* (the web's sweep falls from the start) */
    }
    v->s[0] = e; v->s[2] = at; v->s[4] = dv; v->s[6] = left;
}

static const preset_t ORBV_PRESETS[] = {
    {"SUB", {0, 4, 0, 0, 0, 0, 0, 0}, {0, 0, 127, 0}, 0, 0, FX(0, 0, 0, 6), PAT(0)},
    {"PRISM", {1, 4, 0, 0, 0, 0, 0, 0}, {0, 0, 127, 0}, 0, 0, FX(0, 0, 0, 14), PAT(0)},
    {"CRYSTAL", {2, 4, 0, 0, 0, 0, 0, 0}, {0, 0, 127, 0}, 0, 0, FX(0, 0, 0, 22), PAT(0)},
};

static const engine_t ENG_ORBITA = {
    .name = "ORBITA",
    .page_title = {"VOICE", "VOICE 2"},
    .edit = {
        {"VOICE", F_ENUM, 0, 2, 0, N_ORB_VOICE, 0},
        {"RATIO", F_INT, 1, 16, 4, 0, 0},
        {"-", F_INT, 0, 0, 0, 0, 0},
        {"-", F_INT, 0, 0, 0, 0, 0},
        {"-", F_INT, 0, 0, 0, 0, 0},
        {"-", F_INT, 0, 0, 0, 0, 0},
        {"-", F_INT, 0, 0, 0, 0, 0},
        {"-", F_INT, 0, 0, 0, 0, 0},
    },
    .presets = ORBV_PRESETS,
    .npresets = NELEM(ORBV_PRESETS),
    .note_on = orbv_note_on,
    .render = orbv_render,
    .knob = {P_E0, P_E1, P_E2, P_E3},
    .poly = 6,
    .oneshot = 1,
    .ownenv = 1,
    .done = orbv_done,
    .mem = 0,
};
