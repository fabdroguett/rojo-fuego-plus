# Native UI validation

Current default: Ubuntu WSL, without Docker. Run `./tools/rojo_fuego/wsl.ps1 sync`
from PowerShell, then `./tools/rojo_fuego/wsl.ps1 shell` to prepare fixtures in `/work`.
The Python verifiers run from Windows and use `Ubuntu-24.04` automatically. Build commands
below still apply inside `/work`; replace container execution with the Ubuntu shell.
See [ENTORNO_WSL.md](../../../docs/ENTORNO_WSL.md). The historical Docker workflow remains
available by setting `RF_TEST_BACKEND=docker` before running the verifiers.

`ui_fixture.c` is an emulator-only entry-point fixture, outside `src/` deliberately.
It creates synthetic Pokémon and starts native menus. It must never enter a delivered ROM.
The headless mGBA harness uses an in-memory save; no player save is opened.

For a disposable build in the running Ubuntu WSL distro:

1. Copy `ui_fixture.c` into `/work/src/rf_ui_fixture.c`.
2. Link `pokefirered.gba` with normal `LDFLAGS` plus `-u RFUiFixture_<name>` for
   `Wild Faint Tutorial Voiceover DexNav DexNavLight DexNavEmpty DexNavUnknown Summary Bag BagFemale Pokedex Quests Keys PC Trainer Mart FieldParty CheckOrder Day Slow`.
   Keep the normal map, memory-usage and garbage-collection flags.
3. Build `harness.c` against libmgba as `/tmp/rf-mgba-harness`.
4. Run `python tools/rojo_fuego/test/capture_ui.py`, then explicitly
   `python tools/rojo_fuego/test/capture_ui.py party-slow faint`.
   `python tools/rojo_fuego/test/capture_ui.py voiceover` exercises the same Oak tutorial frame
   and text path as the user's white-border screenshot, with synthetic battle data.

`python tools/rojo_fuego/test/capture_ui.py pokedex pokedex-search pokedex-forms` captures
the list, search menu and information tabs (habitat, stats, evolutions, forms, cry, size).
`python tools/rojo_fuego/test/capture_ui.py bag bag-female start quests keys pc mart field-party trainer`
reviews the shared icon/menu family in light and dark themes.

Captures, scripts and memory-read logs go into ignored `build_artifacts/ui/`. The Bag
scenario checks all seven category filters in both themes, plus the Balls/TM pockets; `bag-female`
exercises the alternate sprite sheet with the same art and animation contract.
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

Starter selection regression:

Copy `starter_fixture.c` into `/work/src/rf_starter_fixture.c` and link with the normal
flags plus `-u RFUiFixture_StarterLab -u RFUiFixture_StarterScript
-u RFUiFixture_StarterRead -u RFUiFixture_StarterDay`. Run
`python tools/rojo_fuego/test/capture_starters.py` from Windows. It warps a synthetic
new game into Oak's actual lab, runs its ball script for all 54 species, checks species,
ability, rival branch and final scene, then tests cancellation and captures the light theme and the confirmation modal (`confirm.ppm`).
Results are in ignored `build_artifacts/ui/starters/`. It waits three frames after debug
callback injection so the field callback handoff has finished before reading results.
Remove `/work/src/rf_starter_fixture.c` and the ELF, then perform the production release
steps above. A passing fixture ROM must never be copied to the user's Desktop.
