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

## Reference-based bag

`verify_bag_reference.py` runs the real menus and asserts item consumption, HP recovery,
registration, source-catalog sorting and return callbacks. It exercises empty bags,
999/99/9 quantities, long names, missing/extended descriptions, repeated navigation,
field medicine, battle medicine and battle cancellation in both themes. Reports, scripts,
logs and native 2× PNGs go to `build_artifacts/ui/bag-reference/`.

Prepare only in the disposable Linux build snapshot:

```bash
cp tools/rojo_fuego/test/ui_fixture.c src/rf_ui_fixture.c
cp tools/rojo_fuego/test/bag_reference_fixture.c src/rf_bag_fixture.c
fixture_flags="$(sed -n 's/^void \(RFBagFixture_[^(]*\).*/-u \1/p' tools/rojo_fuego/test/bag_reference_fixture.c)"
make BUILD=firered -j2 pokefirered.gba \
  LDFLAGS="-Map ../../pokefirered.map --print-memory-usage --gc-sections $fixture_flags -u RFUiFixture_Bag -u RFUiFixture_BagFemale -u RFUiFixture_Day -u RFUiFixture_CheckOrder -u RFUiFixture_Voiceover"
```

From Windows, run `python tools/rojo_fuego/test/verify_bag_reference.py`.
`--workdir` selects a snapshot other than `/work`; `--case` can select individual cases.
The 26-10-2026 session used `RF_WSL_DISTRO=Ubuntu` and
`--workdir /home/fabian/rf-ui-20261026` because only `Ubuntu` 26.04.1 LTS (WSL 1) is installed here,
not `Ubuntu-24.04`. This is an execution override, not a global configuration change.

Category/pocket/swap captures additionally use the existing `capture_ui.py` sequences.
The session's adapted copy is in the same artifact directory. User-provided UI sprites are generated
by `python tools/rojo_fuego/import_bag_user_sprites.py`; the renderer reuses source item
sprites at half scale and caches their IDs. The synthetic resource probe checks matrices,
palettes, OAM sprites and OBJ tile usage after repeated scrolling/filter changes.
`--output build_artifacts/ui/bag-user-sprites` keeps this replacement's evidence separate.

After validation, remove the two fixture copies and the ELF from the snapshot, rebuild
with default flags and run `verify_release.py`. Production validation rejects both
`RFUiFixture` and `RFBagFixture` symbols. Never distribute the fixture build.
# Arte Kanto nativo y menú 3×3, 27-10-2026

`make_kanto_ui_art.py` es el generador vigente; los wrappers de importación
antiguos apuntan a esos maestros. La mochila usa cuatro filas de 24 px y
sprites de objetos a escala nativa, sin matrices afines de reducción.
`verify_kanto_ui.py` comprueba el menú real; `build_kanto_fixture.py` prepara
o retira fixtures únicamente de un snapshot Linux con `RF_DISPOSABLE_BUILD=1`.
Los callbacks periódicos del mundo se aíslan en esas fixtures para probar UI;
no se alteran los callbacks de producción. No correr `verify_code.sh` en paralelo
con mGBA: reenlaza el ELF y deja sus símbolos desincronizados de la ROM fixture.
Producción rechaza también símbolos `RFMainMenuFixture`.
Informe y comandos completos: `docs/ui/main-menu-redesign.md`.
Evidencia final: `build_artifacts/ui/kanto-art/final-menu/` y `bag/`.
