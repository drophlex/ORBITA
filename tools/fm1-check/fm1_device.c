/* SPDX-License-Identifier: GPL-3.0-only
 * The FM-1 as a USB-MIDI device for the installer page: the firmware's whole editor (editor.c: its real INFO, every command,
 * the USB-MIDI packet path ep1_take -> ed_service -> the reply queue) over NOR flash in RAM, on stdin / stdout. Built from
 * the Felucca tree's tests/editor_test.c preamble (GPL-3.0). FAULT=drop:N skips the reply to request N; FAULT=lost:N forgets
 * the transfer before request N (as the device does after 10 s idle: rc 5).
 * (editor_test.c: Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
static unsigned char host_samples[3][0x14000];
#define SMP_USER_XIP(k) host_samples[k]
#define FELUCCA_OTA 1
#define FELUCCA_FLASH 1
#define FELUCCA_VERSION "1.8.2-an2"
#define FELUCCA_CDC 1                                    /* (as the firmware: MENU > USB SERIAL switches the console) */
#define main hostsim_main
#include "hostsim.c"
#undef main
#define MENU_N (32u + FELUCCA_CPU2)                  /* 1.6.1 (core.h FELUCCA_CPU2, the default): 2ND CORE, id 32 */

static uint32_t host_progress = 1, host_erases, host_writes;
static uint8_t host_wire[4096];
static uint32_t host_wire_n;
static void host_drain(void)
{
    while (so_r != so_w) {
        uint32_t p = sx_out_q[so_r++ % SXQ], cin = p & 15u, i;
        uint32_t n = cin == 4u || cin == 7u ? 3u : cin == 6u ? 2u : 1u;
        for (i = 0; i < n && host_wire_n < sizeof host_wire; i++)
            host_wire[host_wire_n++] = (uint8_t)(p >> (8u * (i + 1u)));
    }
}
static uint32_t ota_now_ms(void) { return fm1_ms; }
static void ota_idle(void) { host_drain(); fm1_ms++; }
static void fm1_wdt_feed(void)
{
    fm1_ms++;
    if (host_progress && transport_req == 2u) { seq_stop(); transport_req = 0; }
}
static void fm1_irq_off(void) {}
static void fm1_irq_on(void) {}
static int32_t fm1_enc_take(uint32_t e) { (void)e; return 0; }
static void lcd_sync(void) {}
static void lcd_power(uint32_t s) { (void)s; }   /* (MENU > SCREEN OFF: lcd.c) */
static void lcd_wake_now(void) {}
static void lcd_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint16_t *p)
{ (void)x; (void)y; (void)w; (void)h; (void)p; }
#include "../firmware/src/gfx.c"
#include "../firmware/src/panel.c"
#include "../firmware/src/ui.c"
#include "../firmware/src/menu_items.c"
static void panel_setup(void) {}
static uint32_t flash_ok = 1;
static void audio_silence(void) {}
static void fl_inval(uint32_t off, uint32_t n) { (void)off; (void)n; }
static uint8_t *host_flash_ptr(uint32_t off) { return &host_samples[0][0] + off - SMP_USER_BASE; }
static int fl_erase4k(uint32_t off, uint32_t *took) { memset(host_flash_ptr(off), 0xFF, 4096); *took = 0; host_erases++; return 0; }
static int fl_write(uint32_t off, const void *p, uint32_t n) { memcpy(host_flash_ptr(off), p, n); host_writes++; return 0; }
static uint8_t nor[0x100000];
static int st_read(uint32_t off, void *p, uint32_t n) { memcpy(p, nor + off, n); return 0; }
static int st_prog(uint32_t off, const void *p, uint32_t n) { const uint8_t *s = p; for (uint32_t i = 0; i < n; i++) nor[off + i] &= s[i]; return 0; }
static int st_erase(uint32_t off) { memset(nor + off, 0xFF, 4096); return 0; }
static uint32_t irq_save(void) { return 0; }
static void irq_restore(uint32_t f) { (void)f; }
static uint32_t fl_jedec_ram(void) { return 0; }
static void fl_plain_window_init(void) {}
#define FL_FAR(fn) (fn)
#include "../firmware/src/storage.c"
#include "../firmware/src/upreset.c"
#include "../firmware/src/project.c"

#include "../firmware/src/editor.c"

static uint32_t usb_frame(const uint8_t *f, uint32_t n, uint8_t *usbp)   /* F0..F7 -> USB-MIDI event packets */
{
    uint32_t i = 0, o = 0;
    while (i < n) {
        uint32_t k = n - i >= 3u ? 3u : n - i, cin = k == 3u && i + 3u < n ? 4u : k == 3u ? 7u : 4u + k;
        usbp[o++] = (uint8_t)cin;
        usbp[o++] = f[i];
        usbp[o++] = k > 1u ? f[i + 1] : 0;
        usbp[o++] = k > 2u ? f[i + 2] : 0;
        i += k;
    }
    return o;
}
static uint32_t usb_feed(const uint8_t *p, uint32_t n)   /* whole 64-byte USB packets, as the host sends them */
{
    uint32_t o, refused = 0;
    for (o = 0; o < n; o += 64u)
        while (!ep1_take(p + o, n - o > 64u ? 64u : n - o))
            refused++;                                  /* NAK: the host sends it again */
    return refused;
}

int main(void)
{
    static uint8_t f[8192], u[12000];
    const char *fault = getenv("FAULT");
    uint32_t n = 0, t, i, req = 0, drop = 0, lost = 0;
    int c;
    if (fault && !strncmp(fault, "drop:", 5)) drop = (uint32_t)atoi(fault + 5);
    if (fault && !strncmp(fault, "lost:", 5)) lost = (uint32_t)atoi(fault + 5);
    memset(nor, 0xFF, sizeof nor);
    memset(&song, 0, sizeof song); memset(trk, 0, sizeof trk);
    memset(&chain, 0, sizeof chain); chain_defaults(&chain_config);
    memset(&ed_w, 0, sizeof ed_w); memset(&ui, 0, sizeof ui);
    memset(&settings, 0, sizeof settings); settings_init();
    host_progress = 1; transport_req = panic_req = 0; sx_ready = sx_collect = sx_busy = 0;
    so_r = so_w = mi_r = mi_w = 0; usb.config = 1;
    for (i = 0; i < G_COUNT; i++) song.g[i] = GP[i].def;
    for (t = 0; t < NTRK; t++) { track_defaults(&trk[t]); set_engine_of(&trk[t], 0); apply_preset_to(&trk[t], 0); trk[t].engine = trk[t].eng_req; track_defaults_steps(&trk[t]); }
    fm6_init(); fvb_boot();
    setvbuf(stdout, 0, _IONBF, 0);
    while ((c = getchar()) != EOF) {
        if (c == 0xF0) n = 0;
        if (n < sizeof f) f[n++] = (uint8_t)c;
        if (c != 0xF7) continue;
        req++;
        if (lost && req == lost) memset(&fvb_tx, 0, sizeof fvb_tx);
        host_wire_n = 0;
        usb_feed(u, usb_frame(f, n, u));
        ed_service(); host_drain();
        if (drop && req == drop) { n = 0; continue; }
        fwrite(host_wire, 1, host_wire_n, stdout);
        n = 0;
    }
    return 0;
}
