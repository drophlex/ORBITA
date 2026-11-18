#!/bin/bash
# ORBITA installer — copies the plug-ins to your user plug-in folders and clears the download quarantine.
cd "$(dirname "$0")"
echo ""; echo "  O R B I T A  ·  Dark Funk Durcal design · built at Polaris Sound Lab"; echo ""
V="$HOME/Library/Audio/Plug-Ins/VST3"; C="$HOME/Library/Audio/Plug-Ins/Components"; mkdir -p "$V" "$C" "$HOME/Applications"
rm -rf "$V/Polaris Orbita.vst3" "$C/Polaris Orbita.component" "$HOME/Applications/Polaris Orbita.app"
cp -R "Polaris Orbita.vst3" "$V/" && cp -R "Polaris Orbita.component" "$C/" && cp -R "Polaris Orbita.app" "$HOME/Applications/"
xattr -dr com.apple.quarantine "$V/Polaris Orbita.vst3" "$C/Polaris Orbita.component" "$HOME/Applications/Polaris Orbita.app" 2>/dev/null
killall -9 AudioComponentRegistrar 2>/dev/null
echo "  ✓ VST3 → $V"; echo "  ✓ AU   → $C"; echo "  ✓ App  → ~/Applications"; echo ""
echo "  Ableton Live: Settings → Plug-ins → turn on 'Use VST3 Plug-in System Folders' → Rescan."
echo "  Ableton Live: Ajustes → Plug-ins → activa 'Usar carpetas de sistema VST3' → Volver a escanear."
echo ""; read -n 1 -s -r -p "  Press any key / Presiona una tecla…"
