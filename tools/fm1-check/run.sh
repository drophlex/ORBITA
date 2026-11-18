#!/bin/sh
# Runs ÓRBITA's installer check against a Felucca-family firmware tree (ANDROMEDA, Dark Funk Durcal, Felucca 1.4.1+)
# whose build/gen exists (./build.sh, or its gen step):  tools/fm1-check/run.sh <firmware-src>
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"; ROOT="$HERE/../.."; FW="${1:?firmware-src}"
node "$HERE/frames.js" "$ROOT/docs/fm1/orbita-fm1.js" "$HERE/out"
cp "$HERE/orbita_install_test.c" "$FW/tests/orbita_install_test.c"
cd "$FW" && cc -O1 -w -Ifirmware/hal -Ifirmware/bsp/chip -Ifirmware/bsp/boards/fm1 -Ibuild/gen -Ifirmware/src -o "$HERE/out/orbita_install_test" tests/orbita_install_test.c -lm
cc -O1 -w -DDFD_NO_BANK -Ifirmware/hal -Ifirmware/bsp/chip -Ifirmware/bsp/boards/fm1 -Ibuild/gen -Ifirmware/src -o "$HERE/out/orbita_install_test_felucca" tests/orbita_install_test.c -lm
rm -f "$FW/tests/orbita_install_test.c"
cd "$HERE/out"
echo "== with the built-in voice bank (ANDROMEDA, Dark Funk Durcal) =="
./orbita_install_test orbita_frames.bin empty_frames.bin orbita_voices.wav "$ROOT/Orbita-FM1/Orbita-32.syx"
echo "== without it (Felucca 1.4.1+ as released) =="
./orbita_install_test_felucca orbita_frames.bin empty_frames.bin orbita_voices_felucca.wav "$ROOT/Orbita-FM1/Orbita-32.syx" | grep -v "^  OR"
# the installer page itself, in Chrome, against the firmware as a USB-MIDI device (fm1_device.c): page.mjs
cp "$HERE/fm1_device.c" "$FW/tests/fm1_device.c"
cd "$FW" && cc -O1 -w -Ifirmware/hal -Ifirmware/bsp/chip -Ifirmware/bsp/boards/fm1 -Ibuild/gen -Ifirmware/src -o "$HERE/out/fm1_device" tests/fm1_device.c -lm
rm -f "$FW/tests/fm1_device.c"
node "$HERE/page.mjs" "$ROOT/docs" "$HERE/out"
