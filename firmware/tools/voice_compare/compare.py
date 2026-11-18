#!/usr/bin/env python3
"""The FM-1's original voices against the web version's: the level's shape over time (10 ms frames, each normalised to its
peak), the brightness (spectral centroid over the note's first 60 ms) and the pitch. Usage: compare.py <dir>"""
import sys, numpy as np
d = sys.argv[1]
names = ['SUB', 'PRISM', 'CRYSTAL']
ok = True
for v in range(3):
    w = np.fromfile(f'{d}/web_{v}.f32', dtype='<f4').astype(float)
    c = np.fromfile(f'{d}/orbv_{v}.raw', dtype='<i2').astype(float) / 32768
    n = min(len(w), len(c)); w, c = w[:n], c[:n]
    def env(x):
        f = 441; return np.sqrt(np.array([np.mean(x[i:i + f] ** 2) for i in range(0, len(x) - f, f)]) + 1e-12)
    ew, ec = env(w), env(c)
    sw, sc = ew / ew.max(), ec / ec.max()
    def db(x): return 20 * np.log10(np.maximum(x, 1e-5))
    k = (db(sw) > -60) | (db(sc) > -60)
    err = np.abs(db(sw)[k] - db(sc)[k])
    def onset(x): return int(np.argmax(np.abs(x) > 1e-4))
    def cent(x, a):
        seg = x[a:a + 2646] * np.hanning(2646); s = np.abs(np.fft.rfft(seg, 16384)); fr = np.fft.rfftfreq(16384, 1 / 44100)
        return (s * fr).sum() / s.sum(), fr[np.argmax(s)]
    (cw, pw), (cc, pc) = cent(w, onset(w)), cent(c, onset(c))
    def harm(x, a):                                    # the first 10 partials (multiples of the note) over the first 60 ms, dB
        seg = x[a:a + 2646] * np.hanning(2646); s = np.abs(np.fft.rfft(seg, 65536)); fr = np.fft.rfftfreq(65536, 1 / 44100); f0 = 440 * 2 ** ((int(__import__('os').environ.get('KEY', 50)) - 69) / 12)
        h = np.array([s[np.argmin(np.abs(fr - k * f0))] for k in range(1, 11)]); return 20 * np.log10(h / h.max() + 1e-6)
    hw, hc = harm(w, onset(w)), harm(c, onset(c)); hk = (hw > -40) | (hc > -40); herr = np.abs(hw - hc)[hk]
    def ring(x): e = env(x); return np.argmax(db(e / e.max())[::-1] > -60) and (len(e) - np.argmax(db(e / e.max())[::-1] > -60)) * 10
    good = np.median(err) < 3 and np.median(herr) < 3 and abs(pc - pw) < 3
    ok &= good
    print(f'{names[v]:8s} level shape: median {np.median(err):4.1f} dB, 90% {np.percentile(err, 90):4.1f} dB | '
          f'partials: median {np.median(herr):4.1f} dB | pitch web {pw:5.1f} Hz FM-1 {pc:5.1f} Hz | rings web {ring(w)} ms FM-1 {ring(c)} ms  {"ok" if good else "DIFFERENT"}')
print('voices: the same' if ok else 'voices: DIFFERENT'); sys.exit(0 if ok else 1)
