#!/usr/bin/env python3
"""ÓRBITA FM-1: the built-in FM6 voice bank (B1..B32) from Orbita-FM1/Orbita-32.syx -> firmware-src/firmware/src/dfd_bank.h
(the file and its names are the ones Dark Funk Durcal's firmware gives the built-in bank: eng_fm6.c reads DFD_BANK)."""
import pathlib
ROOT = pathlib.Path(__file__).resolve().parents[2]
b = (ROOT / 'Orbita-FM1/Orbita-32.syx').read_bytes()
assert len(b) == 4104 and b[:6] == bytes([0xF0, 0x43, 0, 9, 0x20, 0]) and ((-sum(b[6:4102])) & 0x7F) == b[4102]
v = [b[6 + k * 128:6 + (k + 1) * 128] for k in range(32)]
names = [r[118:128].decode().rstrip() for r in v]
out = ['/* SPDX-License-Identifier: GPL-3.0-only',
       ' * the built-in FM6 voice bank: ÓRBITA\'s 32 voices (Polaris Sound Lab), from Orbita-FM1/Orbita-32.syx (tools/gen_bank.py; do not',
       ' * edit). FM6 SLOT B1..B32 plays these while the user\'s bank (fm6_vbank.c, loaded from the editor) has no voice in that slot. */',
       '#pragma once',
       'static const char *const DFD_BANK_NAMES[32] = {' + ', '.join('"%s"' % n for n in names) + '};',
       'static const uint8_t DFD_BANK[32][128] = {']
out += ['    {' + ', '.join(str(x) for x in r) + '},' for r in v]
out.append('};')
(ROOT / 'firmware/firmware-src/firmware/src/dfd_bank.h').write_text('\n'.join(out) + '\n')
print('dfd_bank.h: ÓRBITA', ' '.join(names))
