/* SPDX-License-Identifier: GPL-3.0-only
 * ÓRBITA's FM-1 installer against the real Felucca firmware (ANDROMEDA / Dark Funk Durcal / Felucca 1.4.1+), on the host:
 * the exact requests docs/fm1/orbita-fm1.js sends (frames file: cmd, length lo, length hi, args) go to the firmware's own
 * editor handler (editor_fm6.c) over simulated NOR flash. Checks every reply, the bank LIST, SLOT B1..B32 loading the exact
 * records, each voice played through FM6 (level, clipping, DC, release to silence: orbita_voices.wav), and the empty bank
 * (the installer's REMOVE) giving the built-in voices back. Build inside a Felucca tree: see run.sh. */
#define main hostsim_main
#include "hostsim.c"
#undef main

static int32_t fm1_enc_take(uint32_t e) { (void)e; return 0; }
static void fm1_irq_off(void) {}
static void fm1_irq_on(void) {}
static void lcd_sync(void) {}
static void lcd_power(uint32_t s) { (void)s; }
static void lcd_wake_now(void) {}
static void lcd_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint16_t *p)
{ (void)x; (void)y; (void)w; (void)h; (void)p; }
#define FELUCCA_FLASH 1
#include "../firmware/src/gfx.c"
#include "../firmware/src/panel.c"
#include "../firmware/src/ui.c"
#include "../firmware/src/menu_items.c"
static void panel_setup(void) {}

static uint8_t nor[0x100000], flash_ok = 1;
static long cut_bytes = -1, cut_erase = -1;     /* the power goes after that many programmed bytes / at that erase */
static int dead;                                 /* .. and stays off: nothing more is written */
static uint32_t progs, erases;
static int st_read(uint32_t off, void *dst, uint32_t n) { memcpy(dst, nor + off, n); return 0; }
static int st_erase(uint32_t off)
{
    if (dead)
        return -8;
    erases++;
    if (cut_erase >= 0 && cut_erase-- == 0) {    /* cut during the erase: half of it done */
        memset(nor + off, 0xFF, 2048);
        dead = 1;
        return -8;
    }
    memset(nor + off, 0xFF, 4096);
    return 0;
}
static int st_prog(uint32_t off, const void *src, uint32_t n)
{
    const uint8_t *s = src;
    uint32_t i;
    if ((off & 0xFFu) + n > 256u) {
        printf("page wrap at %#x\n", off);
        exit(1);
    }
    for (i = 0; i < n; i++) {
        if (dead || (cut_bytes >= 0 && cut_bytes-- == 0)) {
            dead = 1;
            return -9;
        }
        nor[off + i] &= s[i];
        progs++;
    }
    return 0;
}
static uint32_t irq_save(void) { return 0; }
static void irq_restore(uint32_t f) { (void)f; }
static uint32_t fl_jedec_ram(void) { return 0; }
static void fl_plain_window_init(void) {}
#define FL_FAR(fn) (fn)
#include "../firmware/src/storage.c"
#include "../firmware/src/upreset.c"
#include "../firmware/src/project.c"

static uint8_t rep[1024];                         /* the editor's reply */
static uint32_t rep_n;
static void ed_b(uint32_t v) { if (rep_n < sizeof rep) rep[rep_n++] = (uint8_t)(v & 127u); }
static void ed_str(const char *s, uint32_t max)
{
    uint32_t i;
    for (i = 0; s && s[i] && i < max; i++)
        ed_b((uint8_t)s[i]);
    ed_b(0);
}
static int ed_flash_stop(void) { return transport_busy(); }
#include "../firmware/src/editor_fm6.c"

static int check(const char *what, int ok)
{
    printf("fm6bank: %-96s %s\n", what, ok ? "ok" : "FAIL");
    return ok ? 0 : 1;
}

static uint32_t request(uint32_t cmd, const uint8_t *a, uint32_t n)
{
    rep_n = 0;
    if (!ed_fm6_handle(cmd, a, n))
        return 0;
    return rep_n;
}

static void reset(void)
{
    uint32_t k;
    memset(nor, 0xFF, sizeof nor);
    memset(&song, 0, sizeof song);
    memset(trk, 0, sizeof trk);
    memset(&chain, 0, sizeof chain);
    chain_defaults(&chain_config);
    memset(proj_slot, 0, sizeof proj_slot);
    memset(up_bank, 0, sizeof up_bank);
    memset(&upf, 0, sizeof upf);
    memset(&ui, 0, sizeof ui);
    memset(&motion, 0, sizeof motion);
    memset(&fvb_tx, 0, sizeof fvb_tx);
    host_tracks_init();
    fm6_init();
    for (k = 0; k < NTRK; k++) {
        trk[k].eng_req = trk[k].engine = (uint8_t)ENGI_FM6;
        for (uint32_t i = 0; i < 8u; i++)
            trk[k].p[P_E0 + i] = ENG_FM6.edit[i].def;
        trk[k].p[P_E7] = FM6_OWN;
        fm6_slot[k] = FM6_OWN;
    }
    fm6_own_ok = 0;
    flash_ok = 1;
    dead = 0;
    cut_bytes = cut_erase = -1;
    fm1_ms = 0;
    transport_req = 0;
    up_boot();
}
static void slot_to(uint32_t tr, int32_t s)       /* SLOT set (the editor, MIDI, motion), then the main loop */
{
    trk[tr].p[P_E7] = (int16_t)s;
    fm6_poll();
}
static int patch_is(uint32_t tr, const uint8_t *pk)
{
    uint8_t got[FM6_PACKED];
    fm6_pack(fm6_patch[tr], got);
    return !memcmp(got, pk, FM6_PACKED);
}

static uint8_t frames[64 * 1024];
static uint32_t nframes;
/* feed a frames file: every request's reply byte(s) checked; returns the number of failures */
static int feed(const char *path, uint32_t *last_rc)
{
    FILE *f = fopen(path, "rb");
    uint32_t p = 0, bad = 0;
    if (!f) { printf("cannot read %s\n", path); return 1; }
    nframes = (uint32_t)fread(frames, 1, sizeof frames, f);
    fclose(f);
    while (p + 3 <= nframes) {
        uint32_t cmd = frames[p], n = frames[p + 1] | (uint32_t)frames[p + 2] << 7, rc;
        request(cmd, frames + p + 3, n);
        rc = cmd == ED_FM6B_WRITE ? rep[1] : rep[0];
        if (rep_n == 0 || rc != 0) { printf("  cmd %u: rc %u (reply %u bytes)\n", cmd, rc, rep_n); bad++; }
        *last_rc = rc;
        p += 3 + n;
    }
    return bad + (p != nframes);
}

int main(int argc, char **argv)
{
    const char *fr = argc > 1 ? argv[1] : "orbita_frames.bin", *em = argc > 2 ? argv[2] : "empty_frames.bin";
    const char *wav = argc > 3 ? argv[3] : "orbita_voices.wav", *bank_syx = argc > 4 ? argv[4] : "Orbita-32.syx";
    static const char *NAMES[32] = {"ORSUB1", "ORSUB2", "ORSUB3", "ORSUB4", "ORBELL1", "ORBELL2", "ORBELL3", "ORBELL4",
        "ORHALO1", "ORHALO2", "ORHALO3", "ORHALO4", "ORREED1", "ORREED2", "ORREED3", "ORREED4", "ORDUST1", "ORDUST2", "ORDUST3", "ORDUST4",
        "ORKEY1", "ORKEY2", "ORKEY3", "ORKEY4", "ORWIRE1", "ORWIRE2", "ORWIRE3", "ORWIRE4", "ORTIDE1", "ORTIDE2", "ORTIDE3", "ORTIDE4"};
    static const uint8_t NOTE[8] = {36, 72, 60, 60, 60, 60, 48, 48};   /* per family: SUB BELL HALO REED DUST KEY WIRE TIDE */
    uint8_t syx[4104], arg[4];
    uint32_t k, rc = 99, ok, bad = 0;
    FILE *f = fopen(bank_syx, "rb"), *w;
    if (!f || fread(syx, 1, sizeof syx, f) != sizeof syx) { printf("cannot read %s\n", bank_syx); return 1; }
    fclose(f);
    reset();
    ok = feed(fr, &rc) == 0 && rc == 0 && rep[1] == 32;
    bad += check("the installer's 34 requests (BEGIN, 32 WRITE, END ORBITA + CRC): every reply rc 0, the device holds 32 voices", ok);
    request(ED_FM6B_LIST, arg, 0);
    ok = rep[0] == 32 && rep[1] == 1 && !strcmp((const char *)rep + 2, "ORBITA");
    {
        uint32_t o = 2 + 7;
        for (k = 0; k < 32u && ok; k++) {
            ok &= rep[o] == 1 && !strcmp((const char *)rep + o + 1, NAMES[k]);
            o += 2u + (uint32_t)strlen((const char *)rep + o + 1);
        }
    }
    bad += check("LIST: bank \"ORBITA\", 32 voices ORSUB1 .. ORTIDE4 in order (what the installer reads back to verify)", ok);
    ok = 1;
    for (k = 0; k < 32u; k++) {
        slot_to(0, 9 + (int32_t)k);
        ok &= patch_is(0, syx + 6 + k * 128u);
    }
    bad += check("SLOT B1..B32 load the exact 128-byte records of Orbita-32.syx", ok);

    /* every voice played: a note held 1.2 s, released, 1.3 s more */
    if (!(w = fopen(wav, "wb"))) return 1;
    {
        const uint32_t per = (uint32_t)(2.5 * FS) / CTL * CTL, total = per * 32u;
        uint32_t quiet = 0, clipped = 0, tails = 0;
        wav_hdr(w, total);
        for (k = 0; k < 32u; k++) {
            uint32_t fpos, i;
            int64_t sum = 0;
            int32_t peak = 0, tailpk = 0;
            uint32_t n = NOTE[k / 4u];
            host_tracks_init();
            trk[0].eng_req = trk[0].engine = (uint8_t)ENGI_FM6;
            for (i = 0; i < 8u; i++) trk[0].p[P_E0 + i] = ENG_FM6.edit[i].def;
            trk[0].p[P_DIST] = trk[0].p[P_CHOR] = trk[0].p[P_DLY] = trk[0].p[P_REV] = 0;
            slot_to(0, 9 + (int32_t)k);
            for (fpos = 0; fpos < per; fpos += CTL) {
                int32_t o[2 * CTL];
                if (fpos == CTL) trk_note_on(&trk[0], n, 100);
                if (fpos == (uint32_t)(1.2 * FS) / CTL * CTL) trk_note_off(&trk[0], n);
                mix_block(o, CTL);
                for (i = 0; i < CTL; i++) {
                    int32_t a = o[2 * i] < 0 ? -o[2 * i] : o[2 * i];
                    if (a > peak) peak = a;
                    if (a >= 32700) clipped++;
                    sum += o[2 * i];
                    if (fpos + i > per - FS / 10u && a > tailpk) tailpk = a;
                    wav_put(w, o[2 * i], o[2 * i + 1]);
                }
            }
            printf("  %-8s note %2u  peak %5.1f dBFS  DC %+7.1f  tail %5.1f dBFS\n", NAMES[k], n, 20 * log10((peak + 1) / 32768.0),
                   (double)sum / per, 20 * log10((tailpk + 1) / 32768.0));
            if (peak < 300) quiet++;
            if (tailpk > 100) tails++;
        }
        fclose(w);
        bad += check("every voice sounds (peak above -40 dBFS)", quiet == 0);
        bad += check("no clipping at full scale on any voice", clipped == 0);
        printf("  (%u voice(s) still ringing 2.4 s after the note: long tails, by design or not)\n", tails);
    }
    reset();
    feed(fr, &rc);
    ok = feed(em, &rc) == 0 && rc == 0;
    request(ED_FM6B_LIST, arg, 0);
    printf("  remove: rc %u, LIST bank %u, used voices %u\n", rc, rep[1], fm6_bank_used ? (unsigned)__builtin_popcount(fm6_bank_used) : 0u);
    ok &= fm6_bank_used == 0;
    trk[0].p[P_E7] = FM6_OWN; fm6_poll();
    slot_to(0, 9);
    ok &= patch_is(0, DFD_BANK[0]);
    bad += check("REMOVE (BEGIN, END with no voice): no user bank; B1 plays the built-in voice again (ACID FM)", ok);
    printf("orbita_install_test: %s\n", bad ? "FAILED" : "all ok");
    return bad != 0;
}
