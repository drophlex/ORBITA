#!/usr/bin/env python3
"""ÓRBITA for the FM-1: the one-file installer (Felucca's web installer, the package inside it).
   tools/make_installer.py <package.fwsc> <version> <identity> <out.html>"""
import base64, json, sys, pathlib
pkg, ver, ident, out = sys.argv[1:5]
root = pathlib.Path(__file__).resolve().parent
tpl = (root / "installer_template.html").read_text()
meta = {"version": f"{ver} · ÓRBITA", "product": ident,
        "pkg": "data:application/octet-stream;base64," + base64.b64encode(pathlib.Path(pkg).read_bytes()).decode()}
html = tpl.replace("/*META*/", json.dumps(meta)).replace("/*VERSION*/", ver)
pathlib.Path(out).write_text(html)
print(f"installer {out}: {len(html) // 1024} KB, {ident} {ver}")
