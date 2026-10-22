# Native UI validation

`ui_fixture.c` is an emulator-only entry-point fixture, outside `src/` deliberately.
It creates synthetic Pokémon and starts native menus. It must never enter a delivered ROM.
The headless mGBA harness uses an in-memory save; no player save is opened.

For a disposable build in the running `rojo-fuego-check` Docker container:

1. Copy `ui_fixture.c` into `/work/src/rf_ui_fixture.c`.
2. Link `pokefirered.gba` with normal `LDFLAGS` plus `-u RFUiFixture_<name>` for
   `Wild Faint Tutorial Voiceover DexNav DexNavLight DexNavEmpty DexNavUnknown Summary Bag Quests Keys PC Day Slow`.
   Keep the normal map, memory-usage and garbage-collection flags.
3. Build `harness.c` against libmgba as `/tmp/rf-mgba-harness`.
4. Run `python tools/rojo_fuego/test/capture_ui.py`, then explicitly
   `python tools/rojo_fuego/test/capture_ui.py party-slow faint`.
   `python tools/rojo_fuego/test/capture_ui.py voiceover` exercises the same Oak tutorial frame
   and text path as the user's white-border screenshot, with synthetic battle data.

Captures, scripts and memory-read logs go into ignored `build_artifacts/ui/`.
The scripts resolve addresses from the current ELF. Never reuse savestates after rebuilding.
Switch scenarios assert the active player's party index becomes 1. Slow mode deliberately waits
longer for animations; failure to wait for the action menu is not evidence of a game freeze.

Release steps are mandatory:

1. Remove `/work/src/rf_ui_fixture.c` and `/work/pokefirered.elf` (individual files only).
2. Rebuild with default linker flags and explicit `pokefirered.gba` target.
3. Confirm `arm-none-eabi-nm pokefirered.elf` has **no `RFUiFixture_` symbols**.
4. Validate 32 MiB size, BPRE game code, GBA header checksum and SHA-256.
5. Back up the previous local ROM before replacing it and smoke-test the production ROM.

Production code regression tests:
`python3 -m unittest discover -s tools/rojo_fuego/tests -v` inside a Linux C toolchain environment.
The battle-handoff test extracts the real callback and simulates menu ownership changes during
both task and controller passes, palette fades, link/recorded modes and disabled acceleration.
Screenshots demonstrate the exercised flows; they do not prove the original reported freeze's cause.
