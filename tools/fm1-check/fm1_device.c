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


/* the firmware as a device on stdin / stdout: a request frame F0 7D 46 4C cmd .. F7 per line of bytes; the reply frame back */
int main(void)
{
    static uint8_t b[8192];
    uint32_t n = 0;
    int c;
    reset();
    setvbuf(stdout, 0, _IONBF, 0);
    while ((c = getchar()) != EOF) {
        if (c == 0xF0) n = 0;
        if (n < sizeof b) b[n++] = (uint8_t)c;
        if (c == 0xF7 && n >= 6 && b[1] == 0x7D && b[2] == 0x46 && b[3] == 0x4C) {
            uint32_t i;
            if (b[4] == 1) {                      /* INFO: the parts the installer reads (version, FM6 bank tag) */
                const char *v = "FELUCCA 1.8.2-an2 (host)";
                rep_n = 0; for (i = 0; v[i]; i++) ed_b((uint8_t)v[i]); ed_b(0); ed_b(0x56); ed_b(1); ed_b(32);
            } else if (!request(b[4], b + 5, n - 6)) { n = 0; continue; }
            putchar(0xF0); putchar(0x7D); putchar(0x46); putchar(0x4C); putchar(b[4]);
            for (i = 0; i < rep_n; i++) putchar(rep[i]);
            putchar(0xF7);
            n = 0;
        }
    }
    return 0;
}
