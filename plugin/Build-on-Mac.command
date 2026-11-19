#!/bin/bash
set -eu
cd "$(dirname "$0")"
if [[ "$(uname -s)" != Darwin ]]; then echo "Run this on macOS · Ejecuta este archivo en macOS."; exit 1; fi
if ! xcrun --find clang++ >/dev/null 2>&1; then echo "Instala las herramientas de Apple con xcode-select --install y vuelve a abrir este archivo."; exit 1; fi
if ! command -v cmake >/dev/null 2>&1; then
    if [[ -x /Applications/CMake.app/Contents/bin/cmake ]]; then export PATH="/Applications/CMake.app/Contents/bin:$PATH"
    elif [[ -x /opt/homebrew/bin/cmake ]]; then export PATH="/opt/homebrew/bin:$PATH"
    else echo "Instala CMake 3.22 o posterior desde https://cmake.org/download/ y vuelve a ejecutar."; exit 1; fi
fi
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --config Release --target Orbita_VST3 --parallel 3
bundle="$(find build -type d -name 'Polaris Orbita.vst3' -print -quit)"
test -n "$bundle"
codesign --force --deep --sign - "$bundle"
codesign --verify --deep --strict "$bundle"
target="$HOME/Library/Audio/Plug-Ins/VST3"
mkdir -p "$target"
ditto "$bundle" "$target/Polaris Orbita.vst3"
echo "Installed · Instalado. Live: Settings → Plug-ins → Rescan · Ajustes → Plug-ins → Volver a escanear."
