# FM-1 installer check

`run.sh <firmware-src>` tests ÓRBITA's FM-1 installer (`docs/fm1/`) against the FM-1 firmware's own C code, on a computer. It works with any Felucca-family tree that has `build/gen`, for example ANDROMEDA's (drophlex/ANDROMEDAFM1). Every check is run twice: with Dark Funk Durcal's built-in bank (ANDROMEDA, DFD), and without it (`-DDFD_NO_BANK`, Felucca as released).

**`orbita_install_test`** sends the exact requests the page sends to the firmware's editor handler, over simulated NOR flash. It checks:

- Every reply, the bank LIST (name and the 32 voice names), and that SLOT B1–B32 load the records byte for byte.
- Each voice at notes 24–96, as minor chords at velocity 127: no clipping, no DC (`out/orbita_voices.wav`).
- *Remove* brings the built-in voices back.
- A **power cut / cable pulled after every one of ~7,300 programmed bytes** and during every erase, for a first install and for a reinstall. The result is always the old voices or ÓRBITA whole, never a mix.
- An install abandoned half-way leaves the old voices, and the next press installs cleanly.
- Repeated install and Remove cycles.
- Pressing install while the FM-1 plays: the transport stops as the firmware does, then the install goes through.

**`fm1_device` + `page.mjs`** run the page itself in Chrome. Its Web MIDI is wired to the firmware's whole editor: `editor.c` with its real INFO, the USB-MIDI packet path and flash. The page presses the button and waits for the check mark, then presses *Remove*, in English and Spanish. It also runs with injected faults:

- a reply lost at the INFO probe, at the first LIST, mid-transfer and at the final END;
- the transfer forgotten by the device (rc 5).

The page retries and still installs, and screenshots go to `out/`. A Yamaha DX7 bank sent to a Felucca-family FM-1 is ignored safely.

The official M-VAVE firmware's path (a standard DX7 bank, saved on the device) can't be simulated here.
