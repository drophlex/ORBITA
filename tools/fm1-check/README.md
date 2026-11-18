# FM-1 installer check

`run.sh <firmware-src>` tests ÓRBITA's FM-1 installer (`docs/fm1/`) against the FM-1 firmware's own C code, on a computer. It works with any Felucca-family tree that has `build/gen`, for example ANDROMEDA's (drophlex/ANDROMEDAFM1).

- `orbita_install_test`: the exact SysEx requests the page sends go to the firmware's editor handler (`editor_fm6.c`) over simulated flash. It checks every reply, the bank LIST (name and 32 voice names), and that SLOT B1–B32 load the records byte for byte. It plays every voice through FM6 (peak, clipping, DC, tail → `out/orbita_voices.wav`), then checks *Remove*.
- `page.mjs`: the page itself in Chrome, with its Web MIDI wired to the firmware (`fm1_device`). It presses the button, waits for the check mark in English and Spanish, presses *Remove*, and saves screenshots to `out/`.
