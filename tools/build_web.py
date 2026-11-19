#!/usr/bin/env python3
"""ÓRBITA 2: builds the engine (WebAssembly) and one self-contained page, for the web (docs/index.html) and the plug-in (plugin/ui/index.html)"""
import subprocess, base64, pathlib
root = pathlib.Path(__file__).resolve().parents[1]
eng = root / 'engine'
subprocess.run(['clang++', '--target=wasm32', '-std=c++20', '-O3', '-ffast-math', '-nostdlib', '-fno-exceptions', '-fno-rtti', '-fno-threadsafe-statics',
    '-fvisibility=hidden', '-mbulk-memory', '-Wl,--no-entry', '-Wl,--export-dynamic', '-Wl,--initial-memory=33554432', '-Wl,--max-memory=33554432',
    '-Wl,--allow-undefined', '-I.', 'wasm.cpp', '-o', 'orbita.wasm'], cwd=eng, check=True)
wasm = (eng / 'orbita.wasm').read_bytes()
ui = root / 'ui'
js = 'const WASM_B64 = "' + base64.b64encode(wasm).decode() + '";\n'
for f in ['i18n.js', 'bridge.js', 'app.js']:
    js += f'\n/* ---- {f} ---- */\n' + (ui / f).read_text()
def esc_js(t): return ''.join(c if ord(c) < 128 else ('\\u%04x' % ord(c) if ord(c) < 0x10000 else '\\u%04x\\u%04x' % (0xD800 + ((ord(c) - 0x10000) >> 10), 0xDC00 + ((ord(c) - 0x10000) & 0x3FF))) for c in t)
def esc_html(t): return ''.join(c if ord(c) < 128 else '&#x%x;' % ord(c) for c in t)
a, b = (ui / 'template.html').read_text().split('/*JS*/')
html = esc_html(a) + esc_js(js.replace('</script', '<\\/script')) + esc_html(b)
for out in [root / 'docs/index.html', root / 'plugin/ui/index.html']:
    out.parent.mkdir(parents=True, exist_ok=True); out.write_text(html)
print('ok', len(html) // 1024, 'KB, wasm', len(wasm) // 1024, 'KB')
