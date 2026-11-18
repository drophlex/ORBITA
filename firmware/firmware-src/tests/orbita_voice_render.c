/* SPDX-License-Identifier: GPL-3.0-only */
/* ÓRBITA's original voices through the firmware's audio path (voice.c, fx.c's mix), one note each: SUB PRISM CRYSTAL at
 * note 50, RATIO 2.0, 112 BPM -> OUT/orbv_<voice>.raw (int16 mono, FS), to compare with the web version's (tools/voice_compare). */
#define main hostsim_main
#include "hostsim.c"
#undef main
int main(int argc, char **argv)
{
    const char *dir = argc > 1 ? argv[1] : ".";
    uint32_t v, f, i;
    for (v = 0; v < 3u; v++) {
        char path[512];
        FILE *w;
        snprintf(path, sizeof path, "%s/orbv_%u.raw", dir, (unsigned)v);
        w = fopen(path, "wb");
        host_tracks_init();
        song.g[G_BPM] = (int16_t)(getenv("BPM") ? atoi(getenv("BPM")) : 112);
        host_preset(&trk[0], ENGI_ORBITA, v);
        trk[0].p[P_DIST] = trk[0].p[P_CHOR] = trk[0].p[P_DLY] = trk[0].p[P_REV] = 0;
        trk[0].p[P_E1] = (int16_t)(getenv("R2") ? atoi(getenv("R2")) : 4);
        for (i = 1; i < NTRK; i++) trk[i].p[P_LEVEL] = 0;
        for (f = 0; f < FS; f += CTL) {
            int32_t o[2 * CTL];
            if (f == CTL) trk_note_on(&trk[0], (uint32_t)(getenv("KEY") ? atoi(getenv("KEY")) : 50), 100);
            if (f == 20u * CTL) trk_note_off(&trk[0], 50);
            mix_block(o, CTL);
            for (i = 0; i < CTL; i++) { int16_t s = (int16_t)clamp(o[2 * i], -32768, 32767); fwrite(&s, 2, 1, w); }
        }
        fclose(w);
        printf("voice %u: FS %u, active voices at the end: %u\n", (unsigned)v, (unsigned)FS, (unsigned)busy_now());
    }
    return 0;
}
