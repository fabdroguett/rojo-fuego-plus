# Menú 3×3, arte nativo y Kanto Compact Pro

Implementación de 26–27 de octubre de 2026. Incluye la ampliación solicitada
para corregir iconos, categorías, cursores, controles y tipografía integrada.

## Auditoría y diagnóstico

El menú real está en `src/start_menu.c`: `BuildNormalStartMenu` construye las
acciones según los flags, `RFStartMoveCursor` navega y la tabla
`sStartMenuItems` conserva todos los callbacks. La presentación remasterizada
usa una ventana de 224×144 situada a (8,8), sobre el mapa de 240×160.
Los contextos Safari, enlace y Frontier conservan sus menús y restricciones.

Se revisaron `rf_ui_theme.h/.c`, `DESIGN.md`, los dos sidecars de diseño,
las capturas anteriores, herramientas de WSL/mGBA y las skills locales de UI,
gráficos, rendimiento y pruebas. Los tokens C vigentes y la solicitud explícita
de Kanto prevalecen sobre los metadatos antiguos. No se cambiaron ajustes globales.

Hallazgos comprobados:

* La importación de Mochila recortaba ilustraciones grandes y las reducía a
  aproximadamente 14×14. El muestreo y la transparencia umbralizada descartaban
  detalles y producían bordes irregulares antes de llegar al motor.
* Los objetos de cada fila usaban una matriz OBJ de 512, es decir, tamaño visual
  de 50 %. Sus ilustraciones originales, ya dibujadas a 24×24 dentro de un canvas
  de 32×32, perdían píxeles en ese segundo paso.
* Los 100 glifos suministrados se inspeccionaron por sus máscaras, atlas y FNT/JSON.
  Las mayúsculas Á/É/Í/Ó/Ú duplicaban la letra sin acento. Había trazos de tres
  píxeles irregulares y números difíciles de distinguir.
* Las capturas del harness son PPM de 240×160 sin filtros. La ampliación PNG
  duplica píxeles enteros. No se encontró escalado bilineal en esa ruta de mGBA.
* 4bpp permite conservar los nuevos dibujos exactamente: las pruebas decodifican
  sus tiles y comparan cada índice con el maestro. El problema no era un fallo
  de empaquetado de nibbles.

`build_artifacts/ui/kanto-art/source-audit.json` contiene hashes de originales,
los 100 pares de máscaras y los tamaños del importador anterior.

## Implementación

Los nueve iconos aportados mantienen su significado: Pokédex, scanner DexNav,
Poké Ball de Equipo, mochila, perfil, guardar, engranaje, llave y pergamino.
Se redibujaron a 16×16 con contornos de un píxel, sombras y reflejos definidos.
Los originales se preservan en `graphics/rojo_fuego/kanto_ui/reference_source/`.
Los maestros nuevos son filas de píxeles legibles en `make_kanto_ui_art.py`.

Se eligieron 16×16 para cuadrícula y categorías. Un icono de 24 o 32 píxeles
reduce demasiado el ancho útil de una tarjeta de 70×27: MISIONES necesita
hasta 48 píxeles con una fuente de seis de avance. El retorno de vista previa
tiene un dibujo independiente a 32×32. Los controles tienen maestros de 8×8,
no versiones reducidas del paquete. No se aplicó blur, interpolación ni dithering.

El menú conserva tarjetas 3×3, título y dinero alineado a la derecha. La selección
usa menta, borde dorado, texto oscuro y flecha direccional: hay una señal de forma
además del color. Las etiquetas completas incluyen POKÉDEX y MISIONES.
La descripción ocupa un panel inferior y los controles muestran D-pad/A/B/R.
La franja exterior del mapa sigue visible. Las ventanas usan sus tiles actuales;
los iconos se componen como BG dentro de la misma ventana.

`RFUI_LoadMainMenuPalette` se integra en el sistema existente de temas.
El día usa papel crema; la noche, turquesa profundo. Los tonos de iconos se
mantienen estables y los de lectura cambian con el tema. No se introdujo otro
sistema de configuración o selección de tema.

Mochila usa el mismo lenguaje de pixel art para sus siete categorías reales,
mochila de encabezado, flechas, cursor, scroll y retorno. Todos los frames OBJ
comparten ahora una paleta de 16 colores en lugar de tres copias.
Las máscaras monocromas compartidas de `RFUI_DrawIcon` se obtienen de los
contornos nativos nuevos sin cambiar sus IDs ni significados.

La lista de Mochila muestra **cuatro filas de 24 píxeles**. Los objetos originales
se presentan 1:1, sin matrices de reducción. La capacidad, cantidades y clasificación
del inventario no cambian. El texto se mantiene separado de los iconos y cantidades;
la vista previa conserva el sprite completo y el nombre ajustado.
El retorno, separadores, fila seleccionada y línea de intercambio siguen la nueva
altura. El título y los controles de Mochila usan también la tipografía local.

Para admitir 24 píxeles sin ampliar estructuras, `ListMenuInitWithRowHeight`
usa el byte anteriormente sin uso `unk_1F` de `struct ListMenu`. La estructura
sigue ocupando 32 B dentro de los datos de una tarea. La inicialización estándar
establece altura cero y conserva el cálculo original de fuente/padding para
los demás menús. No se altera `ListMenuTemplate` ni el formato de una partida.

## Fuente

Kanto Compact Pro tiene 104 glifos dibujados a resolución final, máscaras 5×7
en celdas 6×8 y avance proporcional. Los acentos tienen píxeles propios;
Ñ/ñ tienen tilde; 0/O, 1/I y 6/8 son distintos. La tabla de equivalencias usa
los códigos existentes de `charmap.txt`, no Unicode directamente en la ROM.
El atlas y `font-audit.json` conservan todos los glifos y sus métricas.

`RFKanto_Print` escribe en los buffers tiled 4bpp de las ventanas del motor,
con límites físicos y de ancho comprobados mediante ASan/UBSan. Evita llamar
a un rectángulo por cada trazo para reducir el trabajo durante la navegación.
Es una fuente local del menú y de la cabecera/controles de Mochila. Los textos
de inventario, diálogos y demás pantallas conservan sus fuentes nativas.

La alternativa fija 8×12 requiere 64 píxeles para MISIONES y deja poco espacio
al icono. Las variantes proporcionales nativas existentes se conservan en los
textos largos de Mochila; no se impuso una fuente mayor ni un cambio global.
Caracteres fuera del conjunto local muestran `?`; los códigos de control y
placeholders se resuelven antes de pasar texto a este renderer de una línea.

## Recursos y memoria

| Recurso | Consumo |
|---|---:|
| 14 frames de Mochila, 16×16 4bpp | 1792 B |
| Retorno independiente, 32×32 4bpp | 512 B |
| Paleta OBJ compartida | 32 B; antes 96 B / tres bancos |
| Nueve iconos BG del menú | 1152 B ROM |
| Cinco controles BG de 8×8 | 160 B ROM |
| Cursor BG de 16×16 | 128 B ROM |
| Fuente indexada por código: máscaras + avances | 2304 B ROM |
| Ventana existente del menú, 28×18 tiles | 16128 B gráficos; sin ampliación |
| Sprites OBJ nuevos en menú principal | 0 |
| Estructura ListMenu | 32 B; sin crecimiento |

La prueba de estrés de Mochila, en ambos temas, pasa de 7 a **1 matriz**,
de 12 a **8 paletas**, de 30 a **28 sprites**, y de 18432 a **17408 B OBJ**.
La matriz restante pertenece a recursos nativos ajenos a las filas.
El atlas de Mochila no cambia su tamaño y la lista reduce dos allocations de
sprites de 32×32. No hay nuevos procesos de dibujo por frame ni un renderer externo.

Producción enlazada: EWRAM **232972 B**, IWRAM **28656 B**, ROM **27205072 B**.
Respecto de la entrega anterior: −8 B EWRAM, IWRAM idéntica y +4496 B ROM.
La ROM distribuida se rellena a 32 MiB. Su SHA se registra en el manifiesto de
entrega y `docs/development/TESTING.md`, separado del build con fixtures.

## Reproducción y evidencia

```text
python tools/rojo_fuego/make_kanto_ui_art.py
python tools/rojo_fuego/audit_kanto_ui_art.py
python3 tools/rojo_fuego/validate_quests.py
python3 -m unittest discover -s tools/rojo_fuego/tests -v
bash tools/rojo_fuego/verify_code.sh
make BUILD=firered -j2 pokefirered.gba
```

En un snapshot Linux descartable:

```text
RF_DISPOSABLE_BUILD=1 python3 tools/rojo_fuego/test/build_kanto_fixture.py
```

En Windows, con `RF_WSL_DISTRO=Ubuntu`:

```text
python tools/rojo_fuego/test/verify_kanto_ui.py --workdir /home/fabian/rf-ui-20261026 --output build_artifacts/ui/kanto-art/final-menu
python tools/rojo_fuego/test/verify_bag_reference.py --workdir /home/fabian/rf-ui-20261026 --output build_artifacts/ui/kanto-art/bag
python tools/rojo_fuego/test/export_kanto_evidence.py
```

Antes de entregar, en el snapshot:

```text
RF_DISPOSABLE_BUILD=1 python3 tools/rojo_fuego/test/build_kanto_fixture.py --release
```

No ejecutar compilaciones/`verify_code.sh` a la vez que las pruebas de emulador:
ese script reenlaza el ELF y puede desincronizar sus símbolos de la ROM fixture.
El helper requiere el indicador explícito de snapshot para no copiar fixtures
accidentalmente a `src/` en el workspace de origen.

Se utilizó la distribución WSL `Ubuntu`, Ubuntu 26.04.1 LTS, porque Ubuntu 24.04
no está instalada. No se instalaron dependencias ni se modificó configuración global.

Evidencia en `build_artifacts/ui/kanto-art/`:

* `final-menu/`: menú claro/oscuro, nueve selecciones, dinero 0/3080/999999,
  estados previos a desbloqueos, R, nueve destinos con A y retorno con B,
  DexNav con y sin encuentros y muestras reales de glifos.
* `bag/`: 18 recorridos automatizados en mGBA con ordenamiento, medicina de
  campo, medicina de combate, cancelación de combate, vacíos, cantidades,
  nombres largos, descripciones ausentes/extensas, registro y estrés.
* `.native.png`: exportación 240×160. `.png` de escenas: ampliación entera 2×.
* `icons-before-after-native.png` y `icons-before-after-6x.png`: los nueve
  originales suministrados frente a los redibujos, sin interpolación.
* `bag-before-after-native.png`: capturas reales anteriores y actuales;
  `before-menu-provided.png`: comparación original aportada por el usuario.
* JSON, scripts de entrada, PPM y logs permiten comprobar las afirmaciones.

Las fixtures usan datos sintéticos en memoria y aíslan las comprobaciones de UI
del callback de actualizaciones periódicas del mundo. No se usó una partida personal.
Las capturas se revisan visualmente; un caso que solo captura no se declara
como una aserción automática de comportamiento. Los informes separan ambos.

Resultados ejecutados: **21/21 pruebas C/Python**, 350 misiones válidas,
**18/18 recorridos de Mochila** y **29 escenarios de menú**. De estos últimos,
23 contienen aserciones automáticas (callbacks, retorno, cursor o ventana) y
seis son capturas para revisión visual. Las nueve acciones y B se comprobaron
en ambos temas. DexNav se comprobó con y sin encuentros. Las hojas de destinos,
retornos, glifos y la comparación de Mochila fueron revisadas visualmente.

## Archivos y límites

Código: `start_menu.c`, `rf_kanto_ui.c/.h`, `rf_ui_theme.c/.h`, `rf_bag_art.c`,
`item_menu.c`, `item_menu_icons.c`, `item_menu.h`, `list_menu.c/.h`.
Datos: `rf_kanto_art.h`, `rf_kanto_font.h`, `rf_bag_user_art.h`, `rf_menu_icons.h`.
Arte, paletas y originales: `graphics/rojo_fuego/kanto_ui/`; se conserva la
procedencia histórica en `graphics/rojo_fuego/bag_user/`.
Generadores, auditor, fixture, validadores y prueba: `tools/rojo_fuego/` y
`tools/rojo_fuego/test/`, `tests/test_kanto_art.py`. Los generadores anteriores
siguen siendo entradas compatibles al nuevo maestro.

Se preservaron las ediciones anteriores de Mochila y sus recursos originales.
No cambian callbacks, desbloqueos, atajos, clasificación, uso, ordenamiento,
registro, cantidades, IDs ni formato de guardado.

Pendientes fuera de la validación ejecutada: prueba en una GBA física,
inspección de filtros de la ventana gráfica personal de mGBA y perfilado de CPU
de los sistemas de mundo/misiones. Los miles de iconos de objetos del catálogo
conservan su pixel art original; la mejora en lista consiste en mostrarlo 1:1,
no en atribuirse un redibujo de ese catálogo.
