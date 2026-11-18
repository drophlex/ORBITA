/* SPDX-License-Identifier: GPL-3.0-only */
#include "hb_module.h"
#include "tables.h"

static const hb_host_t *host;
static volatile uint32_t ratio = 3, pulses = 5, rotation = 0, bpm = 108;
static volatile uint32_t playing, step_increment, restart;
static uint32_t english;
static uint32_t current_key, carrier, modulator, clock_left, envelope, target;
static volatile uint32_t step;
static uint32_t last_geometry = ~0u, last_step = ~0u;

static uint32_t bounded(int32_t v, uint32_t lo, uint32_t hi) {
    if (v < (int32_t)lo) return lo;
    if (v > (int32_t)hi) return hi;
    return (uint32_t)v;
}
static int init(const hb_host_t *h) {
    host = h;
    return 0;
}
static void release(void) {
    playing = 0;
    target = 0;
    envelope = 0;
}
static void note(uint32_t channel, uint32_t key, uint32_t velocity) {
    (void)channel;
    if (key > 127 || velocity > 127) return;
    if (velocity) {
        current_key = key;
        step_increment = increments[key];
        playing = velocity;
        restart = 1;
    } else if (current_key == key) {
        playing = 0;
    }
}
static void control(uint32_t channel, uint32_t cc, uint32_t value) {
    (void)channel;
    if (cc == 120 || cc == 123) playing = 0;
    if (value > 127) return;
    if (cc == 20) ratio = 1 + value * 7 / 127;
    if (cc == 21) pulses = 1 + value * 15 / 127;
    if (cc == 22) rotation = value * 15 / 127;
    if (cc == 23) bpm = 40 + value * 160 / 127;
}
static void panel(uint32_t event, int32_t delta) {
    if (HB_EV_KIND(event) == HB_EV_BTN_DOWN && HB_EV_ID(event) == HB_BTN_EDIT) {
        english ^= 1u;
        last_geometry = ~0u;
        return;
    }
    if (HB_EV_KIND(event) != HB_EV_ENC) return;
    switch (HB_EV_ID(event)) {
        case HB_ENC_KNOB1: ratio = bounded((int32_t)ratio + delta, 1, 8); break;
        case HB_ENC_KNOB2: pulses = bounded((int32_t)pulses + delta, 1, 16); break;
        case HB_ENC_KNOB3: rotation = bounded((int32_t)rotation + delta, 0, 15); break;
        case HB_ENC_KNOB4: bpm = bounded((int32_t)bpm + delta, 40, 200); break;
    }
}
static uint32_t active_step(uint32_t index, uint32_t count, uint32_t shift) {
    return (((index + shift) * count) & 15u) < count;
}
static void audio(int32_t *out, uint32_t frames) {
    uint32_t velocity = playing, increment = step_increment, multiplier = ratio;
    if (restart) {
        restart = 0; step = 15; clock_left = 0;
    }
    if (!velocity) target = 0;
    for (uint32_t i = 0; i < frames; ++i) {
        if (velocity && !clock_left) {
            step = (step + 1) & 15u;
            clock_left = 661500u / bpm; /* 44.1 kHz times 15/BPM */
            target = active_step(step, pulses, rotation) ? velocity * 140u : 0;
        }
        if (clock_left) --clock_left;
        if (clock_left == 1024u) target = 0;
        if (envelope < target) {
            uint32_t distance = target - envelope;
            envelope += (distance + 63u) >> 6;
        } else if (envelope > target) {
            envelope -= (envelope - target + 255u) >> 8;
        }
        carrier += increment;
        modulator += increment * multiplier;
        int32_t offset = cycle[modulator >> 24] * (int32_t)(7000u + 2u * envelope);
        int32_t sample = cycle[(carrier + (uint32_t)offset) >> 24] * (int32_t)envelope / 32768;
        out[2*i] += sample;
        out[2*i+1] += sample;
    }
}
static void number(char *dest, uint32_t v, uint32_t digits) {
    while (digits) {
        dest[--digits] = (char)('0' + v % 10); v /= 10;
    }
}
static void draw(uint32_t full) {
    uint32_t r = ratio, p = pulses, rot = rotation, speed = bpm;
    uint32_t geometry = r | (p<<4) | (rot<<9) | (speed<<13);
    if (full || geometry != last_geometry) {
        host->fill(0,HB_TOP,240,240-HB_TOP,0);
        host->text(0,34,240,1,"POLARIS / ORBITA",0xffff,1);
        host->text(0,52,240,0,english ? "DARK FUNK DURCAL DESIGN" : "DISENO DE DARK FUNK DURCAL",HB_RGB(120,245,180),1);
        host->text(0,182,240,0,"POLARIS SOUND LAB / MERIDA YUC",HB_RGB(120,245,180),1);
        for (uint32_t i = 0; i < 128; ++i) {
            int x = 119 + cycle[(2*i+rot*16)&255] * 87 / 32768;
            int y = 112 + cycle[(2*i*r)&255] * 41 / 32768;
            host->fill(x,y,2,2,HB_RGB(120,245,180));
        }
        char label[] = "R 0   P 00   F 00   BPM 000";
        number(label+2,r,1); number(label+8,p,2);
        number(label+15,rot,2); number(label+24,speed,3);
        host->text(0,170,240,0,label,0xffff,1);
        host->text(0,148,240,0,english ? "RATIO / PULSES / PHASE" : "RELACION / PULSOS / FASE",0xffff,1);
        host->text(0,218,240,0,english ? "EDIT: LANGUAGE (EN)" : "EDIT: IDIOMA (ES)",0xffff,1);
        last_geometry = geometry; last_step = ~0u;
    }
    uint32_t marker = playing ? step : 16;
    if (marker == last_step) return;
    for (uint32_t i = 0; i < 16; ++i) {
        uint16_t color = active_step(i,p,rot) ? HB_RGB(100,230,178) : HB_RGB(38,52,49);
        host->fill(9+14*i,199,10,10,i==marker ? 0xffff : color);
    }
    last_step = marker;
}
HB_DESC = {
    .abi = HB_ABI, .name = "ORBITA", .init = init, .release = release, .note = note,
    .cc = control, .render = audio, .ui = panel, .draw = draw
};
