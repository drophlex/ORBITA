#!/bin/bash
# ORBITA one-line installer for Mac:  curl -fsSL https://drophlex.github.io/ORBITA/install.sh | bash
set -e
echo ""; echo "  O R B I T A  ·  installing / instalando…"; echo ""
T="$(mktemp -d)"; cd "$T"
curl -fsSL -o O.zip https://github.com/drophlex/ORBITA/releases/latest/download/ORBITA-mac.zip
ditto -x -k O.zip .
V="$HOME/Library/Audio/Plug-Ins/VST3"; C="$HOME/Library/Audio/Plug-Ins/Components"; mkdir -p "$V" "$C" "$HOME/Applications"
rm -rf "$V/Polaris Orbita.vst3" "$C/Polaris Orbita.component" "$HOME/Applications/Polaris Orbita.app"
cp -R "ORBITA/Polaris Orbita.vst3" "$V/"; cp -R "ORBITA/Polaris Orbita.component" "$C/"; cp -R "ORBITA/Polaris Orbita.app" "$HOME/Applications/"
xattr -dr com.apple.quarantine "$V/Polaris Orbita.vst3" "$C/Polaris Orbita.component" "$HOME/Applications/Polaris Orbita.app" 2>/dev/null || true
killall -9 AudioComponentRegistrar 2>/dev/null || true
cd /; rm -rf "$T"
echo "  ✓ VST3 → ~/Library/Audio/Plug-Ins/VST3"
echo "  ✓ AU   → ~/Library/Audio/Plug-Ins/Components"
echo "  ✓ App  → ~/Applications"
echo ""; echo "  ✓ ORBITA installed / instalado. Ableton → Settings/Ajustes → Plug-ins → Rescan/Volver a escanear."; echo ""
