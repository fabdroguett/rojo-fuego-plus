# Interfaz de combate: estado y dirección

## Dirección acordada

Los menús pueden rehacerse por completo. La distribución original no es un límite visual; la implementación conserva las reglas del combate y los controles. La pantalla sigue sujeta al espacio de GBA (240×160, VRAM, tiles, paletas y OAM).

## Diagnóstico del fallo visual

La captura que reportó el usuario era de la ROM actual, no una ROM antigua. El código había conservado la composición nativa: pregunta a la izquierda, comandos en una ventana estrecha y cuatro ventanas de movimientos de 10 tiles más paneles laterales. El cursor reservaba una columna que también invadía la ventana contigua. Los PP y los badges OBJ se pintaban encima de esa cuadrícula. Por eso los cambios de color no podían corregir el formato: faltaba rehacer la composición.

La impresión directa de PP añadía rectángulos claros porque los colores del `TextPrinterTemplate` estaban en orden fondo/texto/sombra incorrecto.

## Estructura actual (concepto «Interfaz de combate con iconos de tipo»)

La franja inferior (tiles 1–28 del textbox) se reparte así en ambos temas:

| Pantalla | Zona izquierda | Zona derecha |
|---|---|---|
| Menú principal | Tarjeta de pregunta en dos líneas, 12 × 4 tiles (`B_WIN_ACTION_PROMPT`) | Comandos 2 × 2, tarjetas de 8 × 2 tiles con icono (`B_WIN_ACTION_MENU`, 16 × 4) |
| Movimientos | Rejilla 2 × 2, tarjetas de 10 × 2 tiles con icono de tipo y nombre (`B_WIN_MOVE_NAME_1..4`) | Panel de 8 × 4 tiles con PP del movimiento seleccionado y etiqueta de tipo en su color (`B_WIN_PP`) |

- Cada tarjeta es redondeada y deja 1 px del color del marco a la derecha y abajo, así las vecinas no forman doble línea. El marco del textbox (índice 9 de la paleta 0) usa el verde azulado oscuro del tema.
- Iconos: arte original de 12 × 12 en cinco tonos (contorno, sombra, base, luz y brillo) para los 18 tipos y para Bolsa, Poké Ball y Huir. El puño de Lucha sirve para «Luchar». Los genera `python tools/rojo_fuego/make_battle_icons.py` en `src/data/rf_battle_icons.h`. La vista previa queda en `graphics/rojo_fuego/icons/battle_icons_preview.png`.
- Paletas: cada tarjeta tiene su banco BG (12–15) con la rampa de su icono; la pregunta y el panel de información usan el 10. Seleccionar solo reescribe la paleta de la tarjeta, sin tocar tiles: fondo menta, texto oscuro y flecha dorada. La flecha es invisible en reposo porque usa el color del relleno. Un movimiento tomado para reordenar recibe borde dorado.
- El marcador de efectividad aparece en el borde derecho de la tarjeta seleccionada. Los nombres reservan ese espacio, pasan a `FONT_NARROWER` si no caben y, como último recurso, terminan en «…».
- PP: conserva los colores de aviso de `SetPPNumbersPaletteInMoveSelection` (mitad, cuarto, vacío).
- Safari (Bola/Cebo/Roca/Huir), el tutorial de Oak y Wally usan las mismas funciones (`DrawBattleActionMenu`, `DrawBattleActionPrompt`).

### Límites conocidos

- Al entrar en Luchar, los iconos de acciones pueden cambiar de color durante un fotograma, mientras el scroll de BG0 todavía muestra las acciones. Comparten bancos 12–15 con los movimientos.
- Los movimientos Z reutilizan las ventanas de nombre, que tienen 10 tiles; un nombre Z largo se reduce de fuente. Los Z no forman parte del juego base.
- No se capturaron Safari, el tutorial de Oak, Wally ni el combate doble en esta iteración.

## Capturas mGBA verificadas

- `build_artifacts/ui/rf-ui-dark-battle-actions.png`
- `build_artifacts/ui/rf-ui-light-battle-actions.png`
- `build_artifacts/ui/rf-ui-dark-battle-moves.png`
- `build_artifacts/ui/rf-ui-light-battle-moves.png`
- `build_artifacts/ui/rf-ui-dark-battle-moves-down.png`
- `build_artifacts/ui/rf-ui-light-battle-moves-down.png`
- `build_artifacts/ui/rf-ui-dark-battle-moves-down-right.png`
- `build_artifacts/ui/rf-ui-light-battle-moves-down-right.png`
- `build_artifacts/ui/rf-ui-dark-battle-moves-focus.png`
- `build_artifacts/ui/rf-ui-light-battle-moves-focus.png`

## Bloqueo de compilación resuelto

El workspace `/work` tenía grupos/eventos Emerald y `.map_version` marcado como FireRed. Make reutilizaba esa mezcla, causando flags cero y layouts ausentes. Se eliminó el marcador obsoleto solo en WSL y se regeneró con `MAP_VERSION=firered`. No se modificaron datos fuente ni macros de mapas.

## Validación

| Prueba | Resultado |
|---|---|
| Captura mGBA `python tools/rojo_fuego/test/capture_ui.py battle` | PASS; acciones, cuatro movimientos y cursores en ambos temas. Evidencia en `build_artifacts/ui/rf-ui-{dark,light}-battle-*.png`. |
| Build de producción `tools/rojo_fuego/wsl.ps1 -Action build -Jobs 2` | PASS; BPRE, 32 MiB; EWRAM 232.940 B, IWRAM 28.632 B; fixture retirada. SHA-256 `362d21aa85ef00c816f1544f8001e150ac65e98e67b25aba6bb7e864b0cf7746`. |
| Arranque de producción `python tools/rojo_fuego/test/verify_release.py` | PASS; `bootPassed=true`; ROM en `build_artifacts/releases/20261026-172558/pokefirered.gba`. |
| Copia local | PASS; Escritorio y release tienen el mismo SHA-256. mGBA 0.10.5 abrió la ROM del Escritorio. Copia anterior respaldada como `.pre-debug-20261026-172616.bak`. |
| Estructura del concepto: build, captura mGBA y arranque | PASS; capturas `battle` en claro/oscuro; ROM de producción SHA-256 `99b8d1ecbb0f263637a196b2e0dbf40005c02769ff8122bf617c49b57da2356e`, `bootPassed=true` (`build_artifacts/releases/20261026-175047/`). |

## Siguiente etapa

Revisar el resultado en una partida real y pulir tamaños de texto/nombres largos y contextos especiales (batalla doble, Safari y movimientos Z), manteniendo la geometría nueva.
