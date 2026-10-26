# Registro de validaciones de GameDev Suite

Cada validación registra nombre, objetivo, comando, resultado, evidencia y estado PASS/FAIL/SKIPPED/BLOCKED. La salida completa queda en logs dentro de `build_artifacts/gamedev/`; esta tabla conserva resúmenes legibles. `TESTING.md` informa solo ejecuciones reales.

| Fecha | Nombre / objetivo | Comando | Resultado y evidencia | Estado |
|---|---|---|---|---|
| 2026-10-26 | Rediseño funcional de lista de movimientos | `python tools/rojo_fuego/test/capture_ui.py battle` (fixture temporal) | Capturas de acciones, movimientos y foco en temas oscuro/claro; `build_artifacts/ui/rf-ui-{dark,light}-battle-*.png` | PASS |
| 2026-10-26 | Build ROM de producción con iconos y PP por movimiento | `tools/rojo_fuego/wsl.ps1 -Action build -Jobs 2`; `python tools/rojo_fuego/test/verify_release.py` | ROM BPRE, 32 MiB, `bootPassed=true`, SHA-256 `9cd8bf4e3f1fe71c5791db1c170dbe4e6f3e69e3f43bff9096c54a042cb89051`; fixture retirada antes del build | PASS |
| 2026-10-26 | Build ROM tras pulido visual de mochila | `tools/rojo_fuego/wsl.ps1 -Action build -Jobs 2` | ROM FireRed BPRE, 32 MiB, EWRAM 232.944 B, IWRAM 28.632 B; SHA-256 `14293e808c6eb07549dec837186ef2472e00237ba1b0e1bb16e7289b0753c8a9`; salida `build_artifacts/wsl/pokefirered.gba` | PASS |
| 2026-10-26 | Build UI de batalla unificada (temas claro/oscuro) | `tools/rojo_fuego/wsl.ps1 -Action build -Jobs 2` | ROM FireRed BPRE, 32 MiB, EWRAM 232.944 B, IWRAM 28.632 B; SHA-256 `C280CCF7C15B023594F96EBA51DBDBC15F64A505735B5ED023A3680DE1B3ABD2`; ROM de proyecto y copia de Escritorio con hash coincidente | PASS |
| 2026-10-26 | Build del foco FireRed en menús de batalla | `tools/rojo_fuego/wsl.ps1 -Action build -Jobs 2` | ROM FireRed BPRE, 32 MiB, EWRAM 232.944 B, IWRAM 28.632 B; SHA-256 `A2AEB04C9FAEF32412D78EEC4ACF4E65B4AF667A833F6D23977E18493C7CADD1`; proyecto y Escritorio coinciden; ROM abierta en mGBA | PASS |
| 2026-10-26 | Captura visual de combate y comandos | `python tools/rojo_fuego/test/capture_ui.py battle` con fixture temporal `Wild` | Menú de acciones, selección de movimientos y foco capturados en temas oscuro y claro; evidencia `build_artifacts/ui/rf-ui-{dark,light}-battle-*.png`. La fixture se usa solo para capturar y no se distribuye | PASS |
| 2026-10-26 | Build HUD batalla pulido tras revisión visual | `tools/rojo_fuego/wsl.ps1 -Action build -Jobs 2` | BPRE, ROM 32 MiB; EWRAM 232.944 B, IWRAM 28.632 B; SHA-256 `A855ADF4A964186D9220B792ACECAE55638203D814C8BB07984D13E8FE1D288C`; copia de Escritorio coincide; build de producción sin fixture | PASS |
| 2026-10-26 | Auditoría de solo lectura del repo | PowerShell, `rg`, `git status` | Identificó decomp FireRed/Expansion, WSL, mGBA y 39 entradas Git preexistentes; ver `00-project-audit.md` | PASS |
| 2026-10-26 | Validación de 17 skills Codex/Claude | `pwsh -NoProfile -File scripts/Setup-GameDev.ps1 -Validate` | Hash de fuente/copia y frontmatter coinciden | PASS |
| 2026-10-26 | Instalación local simulada | `Setup-GameDev.ps1 -Install -DryRun` | No había destinos pendientes; no modificó archivos | PASS |
| 2026-10-26 | Restauración simulada | `Restore-GameDev.ps1 -WhatIf` | Verificó hashes y respaldos; no restauró nada | PASS |
| 2026-10-26 | Misiones y suite nativa | `pwsh -NoProfile -File scripts/Test-GameDev.ps1 -RunNativeTests -Jobs 2` | `validate_quests.py`: 350 IDs válidos; unittest: 18/18. Log y JSON en `build_artifacts/gamedev/` | PASS |
| 2026-10-26 | Captura UI mGBA | `python tools/rojo_fuego/test/capture_ui.py`; `python tools/rojo_fuego/test/capture_ui.py party-slow faint voiceover` | 104 capturas oscuro + 104 claro, con rutas de navegación; intercambios de equipo reportaron índice final 1. Capturas/logs en `build_artifacts/ui/` | PASS |
| 2026-10-26 | Build temporal de UI | Make FireRed con `RFUiFixture_*` aisladas en `/work` | EWRAM 232.948 B; IWRAM 28.632 B; ROM enlazada 27.186.308 B. Fixture retirada antes de producción | PASS |
| 2026-10-26 | Build de código sin ROM | `tools/rojo_fuego/wsl.ps1 -Action check -Jobs 2` | ELF enlazado sin crear/cambiar ROM; EWRAM 232.948 B, IWRAM 28.632 B y ROM enlazada 27.184.632 B | PASS |
| 2026-10-26 | Build de producción y arranque | `scripts/Test-GameDev.ps1 -RunBuild -Jobs 2`; `python tools/rojo_fuego/test/verify_release.py` | BPRE, 32 MiB, `bootPassed=true`, SHA-256 `d79b391a34316c7a26e0a8d64728e318a3736294866987d1fe6d017fd099d7c3`; sin símbolos fixture | PASS |
| 2026-10-26 | Sintaxis de scripts/diff | PowerShell Parser; `git diff --check` | 3 scripts PowerShell parsean; diff sin errores de whitespace (Git solo avisó normalización CRLF en archivos preexistentes) | PASS |

## Ejecución registrada por el script

La ejecución combinada más reciente `Test-GameDev.ps1 -RunNativeTests -RunBuild -Jobs 2` está registrada en `build_artifacts/gamedev/test-report-20261026-122953.json`. El registro detallado de salida de WSL se conserva en los logs enlazados arriba; esta tabla evita duplicar las filas de suite/build del apéndice automático.

## Incidencias resueltas durante la validación

- La primera ejecución detectó que `test_quests.py` no recogía `rf_quests_extra.h` ni declaraba tipos/API de Pokédex en su mock. Se actualizó el harness; el test aislado y luego las 18 pruebas completas pasan.
- La primera validación de skills detectó anclas de regex incompatibles con CRLF y copias locales desactualizadas. Se sincronizaron con respaldo y la validación posterior pasa.
- La primera simulación de restore expuso un parámetro PowerShell no válido (`Select-Object -Reverse`). Se cambió por `[array]::Reverse`; el `-WhatIf` ahora valida las cuatro copias respaldadas sin escribirlas.

## Límites

Las capturas no prueban todas las partidas/guardados ni Android. La fixture PC no contiene Pokémon guardados; queda pendiente una prueba de caja poblada. La fixture de UI no forma parte de la ROM de producción.

La captura `battle` ejecuta esta iteración final en mGBA con ambos temas y registra los cuatro focos del selector de movimientos. La fixture es temporal y fue retirada antes del build de producción.


| 2026-10-26 | Corrección de formato de movimientos (glifos 8×8 y PP en línea) | `tools/rojo_fuego/wsl.ps1 -Action build -Jobs 2`; `python tools/rojo_fuego/test/capture_ui.py battle`; `python tools/rojo_fuego/test/verify_release.py` | Build y arranque PASS; captura de acciones/movimientos/foco en claro y oscuro. SHA-256 `4071ab854da91643f2eef11adf06d6e6f3c79dfdbf38a48452fd3421afd41f38`; detalle y capturas en `docs/ui/battle-redesign.md`. | PASS |
| 2026-10-26 | Entrega de ROM FireRed desbloqueada | Copia a `OneDrive/Desktop/pokefirered.gba`; comparación SHA-256; apertura de mGBA | Hash `4071ab854da91643f2eef11adf06d6e6f3c79dfdbf38a48452fd3421afd41f38` coincide; proceso mGBA apunta a la ROM de Escritorio. | PASS |
| 2026-10-26 | Corrección de lectura y cursor de movimientos | `python tools/rojo_fuego/test/capture_ui.py battle`; build de producción; `verify_release.py` | Capturas en temas claro/oscuro verifican foco en primera y segunda fila, nombres sin PP repetidos, PP/tipo del seleccionado en panel lateral. ROM BPRE arranca; Escritorio coincide. SHA-256 `cd110a97a3b9ef85be821963a3a2f8bb7d13a2bc5bdb1b3fcdb39567a3d5a09a`. | PASS |
| 2026-10-26 | Ampliación de celdas del selector de movimientos | `python tools/rojo_fuego/test/capture_ui.py battle`; `wsl.ps1 -Action build`; `verify_release.py` | Nombres completos de la fixture, PP/tipo en lateral y foco correcto en cuatro casillas en claro/oscuro. BPRE arranca; Escritorio con hash coincidente. SHA-256 `3359b5d3343a982e19f2fe988ca4dd316686ab30b8ba46cd96ead164aa020d23`. | PASS |
| 2026-10-26 | Debug y rediseño completo de menús de batalla | `python tools/rojo_fuego/test/capture_ui.py battle` con fixture temporal | mGBA capturó pregunta y comandos nuevos, cuadrícula 2×2 con iconos/PP, ambos temas y navegación a las cuatro casillas; capturas `build_artifacts/ui/rf-ui-{dark,light}-battle-*.png` | PASS |
| 2026-10-26 | Build/arranque de la ROM rediseñada | `tools/rojo_fuego/wsl.ps1 -Action build -Jobs 2`; `python tools/rojo_fuego/test/verify_release.py` | BPRE, 32 MiB, EWRAM 232.940 B, IWRAM 28.632 B, `bootPassed=true`; SHA-256 `362d21aa85ef00c816f1544f8001e150ac65e98e67b25aba6bb7e864b0cf7746`; release `build_artifacts/releases/20261026-172558/pokefirered.gba` | PASS |
| 2026-10-26 | ROM actualizada en Escritorio y abierta | Copia desde release, comparación SHA-256, apertura con mGBA | `OneDrive/Desktop/pokefirered.gba` coincide con el release; mGBA 0.10.5 muestra `POKEMON FIRE`; copia anterior respaldada | PASS |
| 2026-10-26 | Estructura de combate según concepto (pregunta + comandos 2×2, rejilla de movimientos + panel PP/tipo, iconos de tipo a color) | `tools/rojo_fuego/wsl.ps1 -Action build -Jobs 4`; ROM temporal con `-u RFUiFixture_Wild -u RFUiFixture_Day -u RFUiFixture_Tutorial`; `python tools/rojo_fuego/test/capture_ui.py battle` | Build PASS sin avisos que detengan; EWRAM 232.940 B, IWRAM 28.640 B. mGBA capturó acciones, movimientos y foco en las cuatro casillas, temas oscuro y claro: tarjetas con icono, cursor dorado, panel PP/tipo con etiqueta del color del tipo. Evidencia `build_artifacts/ui/rf-ui-{dark,light}-battle-*.png` | PASS |
| 2026-10-26 | Build/arranque de producción tras la nueva estructura | Fixture retirada de `/work`; `tools/rojo_fuego/wsl.ps1 -Action build -Jobs 4`; `python tools/rojo_fuego/test/verify_release.py` | Sin símbolos `RFUiFixture_`; BPRE, 32 MiB, `bootPassed=true`; SHA-256 `99b8d1ecbb0f263637a196b2e0dbf40005c02769ff8122bf617c49b57da2356e`; release `build_artifacts/releases/20261026-175047/pokefirered.gba`. No se copió al Escritorio | PASS |
| 2026-10-26 | Menús de Safari, tutorial de Oak y Wally con la nueva estructura | Sin captura dedicada | Compilan y usan las mismas funciones de dibujo; falta verificación visual en partida | SKIPPED |
