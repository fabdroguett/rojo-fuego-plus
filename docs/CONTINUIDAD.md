# Continuidad de ejecución

Actualización: 23-10-2026. Sesión cerrada a petición del usuario para continuar después.
Al retomar, ejecutar el siguiente paso de este archivo dentro del alcance de
[PLAN_COMPLETO.md](PLAN_COMPLETO.md); el plan completo sigue abierto.

## Punto guardado para retomar

**Pokédex DEX-1 y DEX-2 (23-10-2026 23:16).** Pestañas en español dibujadas sobre la barra
HGSS (`RFDex_DrawTabs`, también en ÁREA con ventana propia en 0x260 de BG1), rótulos de sprites y
de la lista regenerados por `tools/rojo_fuego/make_dex_labels.py` (MENÚ/BUSCAR, VIST./CAPT.,
NACIONAL, PS AT DF AE DE VE), logo en el índice 12 (claro en ambos temas), estadísticas de la
lista con texto del tema de noche, búsqueda con el fondo del tema (índice 3) y textos de la Pokédex
en español; `UNITS` pasa a métrico. ROM `build_artifacts/releases/20261023-231623/` SHA-256
`9c7c56b7…a12c`, copiada al Escritorio (respaldo `Rojo Fuego Plus (respaldo 20261023-2230).gba`).
Botones de la búsqueda en español (commit ec167a83).

**DEX-3 Área real (24-10-2026 02:10).** `pokedex_area_screen.c`: `RFArea_ExplainNoWildArea` salta
a la franja horaria con la especie al abrir ÁREA; si no hay zona salvaje, rótulo LEYENDA (mapa de
la saga resaltado vía `RFLegends_FindSpecies`), EVOLUCIÓN o DESCONOCIDA. Fixture
`RFUiFixture_PokedexArea` (Ivysaur, Pidgey, Tandemaus, Uxie en la Nacional). ROM
`build_artifacts/releases/20261024-021033/` SHA-256 `a0a3f549…3ae0`, en el Escritorio (respaldo
`Rojo Fuego Plus (respaldo 20261024-0000).gba`).

**DEX-4 Debilidades (24-10-2026 09:30).** `pokedex_plus_hgss.c`: `sRFInfoShowMatchups` (A en INFO),
`RFDex_LayoutMatchups` con `GetTypeModifier` para los dos tipos, aviso "A DEBILIDADES/DESCRIPCIÓN"
bajo el sprite (`RF_INFO_HINT_TOP`). Fixture `RFUiFixture_PokedexArea` suma Skarmory. ROM
`build_artifacts/releases/20261024-092730/` SHA-256 `94f0ddf1…1df3`, en el Escritorio (respaldo
`Rojo Fuego Plus (respaldo 20261024-0210).gba`).

**DEX-5 Crianza y EV (24-10-2026 11:05).** `pokedex_plus_hgss.c`, vista A de DATOS: EV "+N", amistad
numérica, pasos de eclosión (`eggCycles` × 128 con `P_EGG_CYCLE_LENGTH` Gen 8+), título GRUPOS HUEVO
(WIN_STATS_LEFT pasa a 12 tiles de alto y los `baseBlock` siguientes suben 48), sexos en porcentaje
y colores invertidos de la barra de navegación en modo oscuro. ROM
`build_artifacts/releases/20261024-110309/` SHA-256 `76d8b03a…09ec`, en el Escritorio (respaldo
`Rojo Fuego Plus (respaldo 20261024-0927).gba`).

**DEX-6 Movimientos (24-10-2026 12:30).** `pokedex_plus_hgss.c`: `RFDex_PrintMoveMethod` (método a la
derecha: NV., EVOL. para nivel 0, MT/MO, TUTOR, HUEVO); vista A con categoría/contacto y PP/PRIOR. en
lugar de APPEAL/JAM; icono de tipo en ambas vistas; descripciones largas en `FONT_SMALL_NARROWER`.
ROM `build_artifacts/releases/20261024-122902/` SHA-256 `0d76f257…b44b`, en el Escritorio (respaldo
`Rojo Fuego Plus (respaldo 20261024-1103).gba`). Siguiente: DEX-7 Variocolor/formas.

**Pantalla de datos del Pokémon rehecha desde cero (23-10-2026 22:30).** Pedido: reestructurar
estado, características y movimientos "desde 0" con la misma información o más, con margen y
relleno en todos los textos. `src/rf_summary_screen.c` implementa la API pública
(`ShowPokemonSummaryScreen`, `ShowSelectMovePokemonSummaryScreen`, `GetMoveSlotToReplace`,
`SummaryScreen_SetAnimDelayTaskId`); la pantalla antigua sigue compilada como `Legacy_*`
(`RF_SUMMARY_SCREEN` en `config/summary_screen.h`). Un único lienzo de 30×20 teselas con paneles
RF del tema activo; sprites para Pokémon, Poké Ball, marcas, tipos y categoría.
- Páginas: DATOS (especie, tipos, objeto, naturaleza con menta y efecto, habilidad), VALORES
  (Valor/Base/IV/EV a la vez, totales, EXP y lo que falta), MOVIMIENTOS (tipo, PP con aviso,
  detalle con categoría/potencia/precisión/descripción; A para detalle y reordenar, START para el
  recordador), MEMORIA (EO, ID, cintas, Poké Ball, encuentro, amistad, carácter por IV, Poder Oculto,
  teratipo). Columna fija: nº nacional, variocolor, nombre/sexo, nivel, estado, PS con barra.
- Conserva: intercambio de movimientos con PP extra, olvidar movimiento (fila NUEVO, aviso de MO),
  apodo, recordador, cajas, combate multi, socios, Huevos (solo DATOS) y modos bloqueados.
- Fallo encontrado y corregido durante la prueba: el icono de categoría se creaba con la animación 0
  (`DAMAGE_CATEGORY_NONE`, entrada NULL) y en combate terminaba en salto a basura y reinicio.
- Endurecimiento: `ResetSpriteData` cancela `Task_AnimateAfterDelay` y `Task_HandleMonAnimation`
  pendientes (guardan punteros a sprites que se liberan ahí), descontando el contador de KO.
- Validado en mGBA: 4 páginas en claro/oscuro, cursor, intercambio, aprender 5.º movimiento (devuelve
  la ranura elegida), Pokémon con estado y objeto, Huevo, apertura desde el equipo en el mapa con
  reordenamiento posterior correcto, y 16 combinaciones de apertura en combate sin reinicios ni fallos
  de memoria propios (`battle_stress.py` en el scratchpad; harness de depuración que imprime PC/LR).
  No probado: apertura desde el PC, recordador y apodo de punta a punta.
- **Fallos previos detectados, sin resolver:** accesos inválidos durante la intro de partida nueva al
  pulsar A seguido (fotograma ~1901 en `Task_HandleMonAnimation`, ~2810 en sprites de la pantalla de
  nombre) y al entrar en combate (`VBlankCB_Slice`). Candidatos al reinicio aleatorio del usuario.
- ROM `build_artifacts/releases/20261023-223009/` SHA-256 `2844a02a…c351`, copiada al Escritorio;
  respaldo de la anterior como `Rojo Fuego Plus (respaldo 20261023-1919).gba`.

**Tema de color configurable (23-10-2026 20:23).** Pedido: Pokédex "con esteroides", otra paleta
(carmesí o azur), otra fuente general y colores configurables en todos los menús RF.
- `rf_ui_theme.c`: tabla `sThemeColors[tema][día/noche][rol]` con seis temas (MENTA, CARMESÍ,
  AZUR, VIOLETA, ÁMBAR, GRAFITO) y 13 roles (`enum RFUIRole`). `RF_UI_NIGHT_*`/`RF_UI_DAY_*`
  leen el tema activo vía `RFUI_Color`, por lo que ya no son constantes: las tablas de recoloreo
  nocturno guardan roles y se cargan con `RFUI_LoadNightRole`. Menta reproduce los colores previos.
- Claves: `KEY_COLOR_THEME` al final del enum (no mueve bits guardados); `sKeyMenuOrder` lo
  muestra tras Modo oscuro. Guardado: `SaveBlock3.rfUiThemeMagic` ("THM1") + `rfUiTheme`;
  sin magic se usa Menta. Cambiarlo repinta el menú al momento.
- Validado en mGBA: 6 temas × claro/oscuro en Pokédex (lista, ficha, estadísticas), mochila y
  menú principal; opción en Claves con ciclo y cierre al mapa. Fixture `RFUiFixture_NextTheme`.
  ROM de producción sin fixtures, cabecera válida, arranque a `CB2_Overworld`, 18 pruebas nativas.
  Entrega `build_artifacts/releases/20261023-202304/` SHA-256 `b1532bfd…24f0`; **no se copió
  al Escritorio** porque mGBA estaba abierto con la ROM.
- **Fuente seleccionable** (Claves → Fuente, tras Tema de color): M5X7 (por defecto), M6X11,
  MONOGRAM y CLÁSICA. Guardada en `SaveBlock3.rfUiFont` bajo el mismo magic que el tema; el
  primer cambio escribe ambos campos. `text.c` pide glifos y anchos a `RFFont_Glyphs/Widths`
  (`rf_fonts.c`) para las hojas normal, estrecha, corta y pequeña (y sus estrechas); las
  "más estrechas" y el japonés siguen clásicos. `make_rf_fonts.py` (biblioteca estándar) genera
  `graphics/rojo_fuego/fonts/<estilo>/latin_*.png` y `src/data/rf_font_widths.h`;
  `rasterize_font.py` (Pillow + fontTools) pasa los TTF a JSON de mapa de bits.
  m6x11plus nativa no cabe en las cajas (+20 % de ancho, 17 filas): se condensa y sus hojas
  corta/pequeña usan m5x7. Fuentes: Monogram de la copia del paquete oficial en GitHub
  `flygohr/NuradanRPG` (tamaños iguales a itch.io); m5x7 (34300 B, CC0) y m6x11plus (18192 B)
  de copias en GitHub, porque la descarga de itch.io por script rechaza la clave. Créditos en CREDITS.md.
  Validado en mGBA: combate, mochila, Pokédex, resumen y Claves con las cuatro fuentes.
  Conocido: "VERY HARD" de la ficha de estadísticas se recorta con M5X7/M6X11 (se acorta en DEX-2).
  Entrega vigente `build_artifacts/releases/20261023-204409/` SHA-256 `693d0761…b879` (temas +
  fuentes; sin fixtures, cabecera válida, arranque a `CB2_Overworld`, 18 pruebas nativas). ROM
  enlazada 27754192 B. **Pendiente copiarla al Escritorio** cuando mGBA esté cerrado, respaldando
  la ROM actual (la de ARTE-2, `defe9d69…`).
- Siguiente: DEX-1 (rótulos en español, logo/pestañas legibles en oscuro) → DEX-2…11.

**ARTE-2: mochila nueva (23-10-2026 19:19).** `tools/rojo_fuego/make_bag_frames.py` (solo
biblioteca estándar) reduce `menu_art_v2/concepts/backpack-v1.png` a 62 px, lo pasa a una paleta
fija de 15 colores por materiales (contorno, rojo ×5, petróleo ×5, dorado ×2, crema, blanco),
limpia píxeles aislados y genera los seis cuadros en el orden de `item_menu_icons.c`: cerrado,
Objetos (bolsillo frontal izq., poción), Objetos clave (compartimento principal, llave),
Poké Balls (lateral izq., Poké Ball), MT/MO (lateral der., disco) y Bayas (frontal der., baya).
`bag.pal` es compartida: la mochila femenina invierte las rampas rojo/petróleo. No editar los PNG
a mano; ajustar el script (`POCKETS`, figuras de contenido) y regenerar.
Validado en mGBA con el fixture `RFUiFixture_Bag` y el nuevo `RFUiFixture_BagFemale`
(caso `bag-female` de `capture_ui.py`): cinco bolsillos de cada género en claro y oscuro y
cierre al mapa. Capturas en `build_artifacts/ui/bag/`. ROM de producción sin símbolos de
fixtures, cabecera BPRE y checksum válidos, arranque hasta `CB2_Overworld`. Escritorio SHA-256
`defe9d69f83fb82843ba8349000eeaed8df966c7a4012c4b1134a36c3a2b7750`, entrega
`build_artifacts/releases/20261023-191907/`; la ROM previa del Escritorio (22-10) quedó como
`Rojo Fuego Plus (respaldo 20261022-2143).gba`. Memoria: EWRAM 230628 B, IWRAM 28648 B,
ROM 27152624 B. Siguiente: **ARTE-3** (Pokédex HGSS).

**Entorno en el PC `Fabian-Pc` (sin Docker).** Se compila en WSL Ubuntu (`arm-none-eabi-gcc`,
`libmgba-dev` instalados): copia en `~/work` sincronizada con
`rsync -a --exclude .git --exclude build_artifacts /mnt/c/Users/Fabian-Pc/rojo-fuego-plus/ ~/work/`,
`touch .histignore` (la copia no tiene historial git) y
`make SHELL='bash -o pipefail' BUILD=firered -j8 -O pokefirered.gba`. El harness se compila con
`gcc -O2 -o /tmp/rf-mgba-harness tools/rojo_fuego/test/harness.c -lmgba`. Los scripts
`capture_*.py` siguen esperando Docker; en este PC se ejecutan sus guiones del harness desde WSL.
Las 209 advertencias `libpng warning: bKGD` son de recursos preexistentes, no del código.

**Las 1025 especies obtenibles (23-10-2026 14:07).** Pedido: todos los Pokémon en el juego,
salvajes repartidos a criterio y legendarios mediante misiones y ubicaciones especiales.
- Salvajes: `tools/rojo_fuego/encounters.py` ahora incluye formas por defecto con sufijo
  (Burmy, Shellos, Deerling, Gimmighoul…), la pareja Karrablast/Shelmet, especies definidas por
  macros (Scatterbug, Flabébé, Furfrou, Minior) y evoluciones imposibles sin cable o
  monedas (Escavalier, Accelgor, Gholdengo, Basculegion). 404 especies en tablas de
  tarde/noche; el día conserva las tablas originales de Kanto.
- Legendarios: `tools/rojo_fuego/legends.py` → 117 legendarios, singulares, Ultraentes y
  paradójicos en 28 sagas (misiones 43–70, capítulos LEYENDAS DE KANTO / DEL MUNDO /
  ULTRAENTES Y PARADOJAS) y 64 mapas. Al aceptar la misión aparecen como Pokémon visibles en
  casillas abiertas y alcanzables calculadas desde los datos del mapa; capturarlos marca un bit
  en `SaveBlock3.rfLegendCaught` (magic propio, +20 bytes) y los retira; la misión queda lista
  con toda la saga. Ubicaciones y requisitos: [LEYENDAS.md](LEYENDAS.md).
- Cobertura calculada: 1025/1025 (Floette y Alcremie por evolución; Unown en Tanoby).
Validado en mGBA: Arceus visible solo con su misión activa, combate Nv. 65, captura con Master
Ball, bit guardado y objeto retirado; Regigigas (saga de uno) deja la misión LISTA; capítulos
nuevos en el menú. 54 iniciales y 18 pruebas nativas (incluida la nueva de sagas) pasan.
ROM en el Escritorio SHA-256 `7e82a3fd6f7a5de8327b542811a8d3261509e895bbded9ce83cc393fe7abee7a`,
entrega `build_artifacts/releases/20261023-140757/`. Pendiente: recorrer a pie cada ubicación
(se validaron 2 de 64), equilibrar niveles y fuente Monogram (descarga manual en itch.io).

**Salvajes visibles ampliados (23-10-2026 13:29).** Pedido: más Pokémon visibles a la vez.
`OWE_SPAWNS_MAX` 4 → 6 (IDs locales 247–252), espera por salvaje activo 30 → 15 fotogramas y
`WE_OWE_SPAWN_REPLACEMENT` activo (el más antiguo se renueva a los 240 fotogramas; el contador
es u8). Se conservan los encuentros aleatorios. Comprobado en Ruta 1 y Bosque Verde (hasta 6
visibles) sin errores de mGBA. ROM en el Escritorio SHA-256
`082982ab33b508fb1e0de5cab0ff036a7c30f4176e36a2f1561324afe96fe339`, entrega
`build_artifacts/releases/20261023-132941/`. Pendiente: fuente global Monogram (CC0) elegida
por el usuario; la descarga de itch.io requiere hacerla a mano.

**Corrección 23-10-2026 13:19 — cuelgue con SELECT en combate.** Las ROM entregadas se
compilan sin `RELEASE`, así que `DISABLED_ON_RELEASE` dejaba activos el menú de depuración de
combate (SELECT), el de mapa (R+START) y el visor de sprites del resumen. Al salir del menú de
combate quedan sprites/estado corruptos (reproducido en mGBA) y el usuario obtuvo
"Jumped to invalid address" atacando a un Rattata salvaje. Los tres quedan en `FALSE` en
`include/config/debug.h`. Verificado: SELECT no hace nada y el combate sigue; ROM de producción
sin fixtures, arranque correcto. Escritorio SHA-256
`958adc5d6c4be1097f2507b4dcd608d2d96690c2ca6815fb2832bab7fb1f44bf`, entrega
`build_artifacts/releases/20261023-131914/` (anterior `5bd46e5b…` respaldada).
Salvajes visibles (OWE) comprobados en Ruta 1, Bosque Verde y Mt. Moon. Nuevos puntos del
fixture de iniciales: `RFUiFixture_StarterRival` (combate rival real) y
`RFUiFixture_StarterWarp` + `gRFUiFixture_Warp` (warp a mapa/coordenadas).

Última implementación (23-10-2026, tarde): **ARTE-1 integrado y selector de iniciales mejorado**.
- Iconos: nueve del Kenney 1-Bit Pack (CC0) + Poké Ball propia, como máscaras de 1 bit
  generadas por `tools/rojo_fuego/make_menu_icons.py` desde
  `graphics/rojo_fuego/icons/kenney_1bit_menu.png`; se pintan con `RFUI_DrawIcon` según tema
  y foco. Usados en el menú principal (todas las acciones) y la cabecera de Misiones.
- `RFUI_FillRect`/`RFUI_DrawRing`: `RFUI_DrawPanel` ya no rellena píxel a píxel. El selector
  de iniciales tardaba ~12 fotogramas por repintado y perdía pulsaciones; ahora ~3. Beneficia
  a todos los menús RF que usan paneles (inicio, DexNav, mochila, equipo, PC, mapa).
- Selector de iniciales: región/generación, paginador, tipos, foco animado y atenuación,
  chips NORMAL/OCULTA, descripción de habilidad y confirmación modal ([INICIALES.md](INICIALES.md)).
- Glifos de botones en pies de menú principal, mochila y Misiones.
Validado: 54 elecciones + cancelación + vuelta desde confirmación en mGBA (`capture_starters.py`),
capturas claro/oscuro de inicio, Misiones, Claves, mochila, DexNav, ficha, PC y equipo en combate
(`capture_ui.py`; el cambio de equipo sigue pasando). Build de producción sin fixtures
(0 símbolos `RFUiFixture_`/`RFProgress_`), cabecera BPRE válida y arranque hasta el mapa.
ROM en el Escritorio, SHA-256 `5bd46e5b8ffbe5c9cdd5e5d7e681915bb7c057e15af64f155b9d3019333e7003`;
entrega `build_artifacts/releases/20261023-121954/` con la anterior (`f79fc39f…`) respaldada.
Memoria: EWRAM 230384 B, IWRAM 28648 B, ROM enlazada 27154640 B.
No se probaron Android ni el recorrido completo de partida nueva con apodo/guardado.

Selector de iniciales original (antes de esta revisión): 54 opciones/18 grupos con habilidad
oculta, integrado con Oak conservando el rival de Kanto. Investigación de 28 fuentes:
[ASSETS_INTERNET.md](ASSETS_INTERNET.md).
ROM entregada: `C:/Users/fabia/OneDrive/Desktop/Rojo Fuego Plus.gba`.

Pedido anterior: actualizar el plan y buscar/crear mejores sprites de mochila, Pokédex
y otros menús. Prioridad actual **ARTE-1–5** de PLAN_COMPLETO.md. Dos conceptos propios están
en `graphics/rojo_fuego/menu_art_v2/concepts/`; fuentes y límites en RECURSOS_MENU.md.
Los conceptos todavía no son recursos indexados nativos y no están integrados en la ROM.

- BASE: corregido en código el premio perdido de Noa si la mochila está llena. Ahora se
  conserva el nivel del premio, se reclama al hablar con ella y no se duplica al cambiar de día.
  El conjunto actualizado pasa 18 pruebas automáticas (C con ASan/UBSan y Python).
- Noa: mGBA confirmó victoria real con Mewtwo propio de nivel 30, dos medallas, premio de
  cinco Super Balls, reglas desactivadas y ausencia de premio pendiente. La prueba anterior
  falló solo en la comparación de hexadecimal mayúscula/minúscula del verificador; se corrigió.
  El bloqueo original con Pokémon desobediente sigue sin diagnóstico confirmado.
- EVENTOS: nuevos objetivos por eventos y entregas explícitas, adaptador de victorias contra
  entrenadores y diálogo del ayudante del diario de Oak. Ocho misiones nuevas, total 43.
  Pruebas nativas de progresión/entregas pasan. Recorrido integrado en mGBA completado, incluida
  una victoria de revancha contra Brock que marca READY la misión correcta.
- SAVE-1: mediciones y estrategia documentadas en [ALMACENAMIENTO_SAVE1.md](ALMACENAMIENTO_SAVE1.md).
  Las cajas siguen en 14; el prototipo de banco adicional aún no está implementado.
- Validador de catálogo: `python tools/rojo_fuego/validate_quests.py`, 43 misiones válidas.
- SERVICIOS: habilitados movimientos de preevoluciones. Listas del recordador deduplicadas
  y acotadas a 254 movimientos + Cancelar; aprendizaje real probado con Vileplume en mGBA.
- Clave Reponer bayas: repone la baya original usada por el jugador tras entrenadores,
  sin devolver robos ni sustituir otro objeto. Verificada en mGBA activada/desactivada.
- Editor EV/IV gratuito junto al entrenador de chapas de Azulona, con previsualización,
  confirmación y límites. Flujo de cancelación, EV y cambio de IV probado en mGBA.
  Detalles en [SERVICIOS_ENTRENAMIENTO.md](SERVICIOS_ENTRENAMIENTO.md).
- Tutores temáticos permanentes en las ocho estatuas de gimnasio tras su medalla, gratis
  y con selección de equipo/PC. Ocho enseñanzas y un bloqueo sin medalla pasan en mGBA.
  `make_tutors.py` reconoce la macro compartida para generar compatibilidades correctas.
- Entrega anterior a iniciales: mediciones y hash históricos en
  [ENTREGA_2026_10_23.md](ENTREGA_2026_10_23.md). La ROM vigente y sus mediciones son
  las indicadas al inicio de este archivo.

## Entorno actual

Fuentes del host en `/source` (solo lectura), copia compilable en `/work`, contenedor
`rojo-fuego-check`. Las fuentes temporales de fixtures se retiraron de `/work/src` antes
de reenlazar y generar la ROM de producción. Nunca entregar las ROM de fixtures.
El harness mGBA está en `/tmp/rf-mgba-harness`. Artefactos ignorados en
`build_artifacts/progression/` y `build_artifacts/training/`; no se abren guardados del jugador.
Los nueve casos de gimnasio están en `build_artifacts/gym-tutors/`. Las 54 elecciones y
las capturas del selector están en `build_artifacts/ui/starters/` (`report.json`); verificador
reproducible: `tools/rojo_fuego/test/capture_starters.py`, con fixture temporal según
`tools/rojo_fuego/test/UI_VALIDATION.md`. La entrega vigente es
`build_artifacts/releases/20261023-114921/`; la de `20261023-111358/` es anterior.
Docker Desktop y el contenedor quedaron apagados. No arrancarlos para una actualización
solo documental; al modificar código, iniciarlos y sincronizar archivos del host a `/work`.

Para reproducir: copiar las fuentes/herramientas actualizadas a `/work`, ejecutar allí
`bash tools/rojo_fuego/test/build_progression_fixture.sh`, y ejecutar desde Windows
`python tools/rojo_fuego/test/verify_progression.py` (también `--berries-off`). Regenerar siempre
símbolos/estados al cambiar el enlace. Ver también `test/UI_VALIDATION.md`.

## Siguiente acción concreta

1. **ARTE-3: siguiente trabajo al retomar.** ARTE-2 (mochila) está integrado; seguir con la
   Pokédex HGSS a partir de `menu_art_v2/concepts/pokedex-v1.png` y luego ARTE-4/5. Para nuevos iconos,
   añadir el índice de la hoja Kenney al script y regenerar; no editar `rf_menu_icons.h`.
   Candidatos pendientes: estados de misiones (bloqueada/lista/completada), Claves y DexNav,
   cuyo pie con glifos no cabe aún en 232 px.
2. Seguir **ARTE-3 → ARTE-4/5** con verificación de cada integración.
3. Después continuar SERVICIOS con preparación visible en Centros Pokémon (naturaleza/habilidades/
   crianza) y herramientas unificadas, reutilizando mentas y parches existentes.
4. Auditar laboratorio de formas/fusiones y zonas de entrenamiento; SAVE-2 sigue aislado hasta cerrar sectores
   y migración. No rehacer sistemas existentes.
5. Mantener validador, pruebas nativas y recorrido de emulador apropiados a cada cambio.
6. Actualizar este archivo y subir código, documentación y recursos fuente propios en cada
   punto estable; mantener ROMs y guardados fuera de Git.

## Límites y pendientes que no deben perderse

- Iniciales: las 54 entregas y escenas de elección están comprobadas; el rival conserva
  equipos de Kanto. Quedan para regresión integral el recorrido de partida nueva hasta
  la primera batalla, apodo, guardado/carga y Android. No afirmar que ya se probaron.
- Assets: 28 fuentes investigadas; menús externos y conceptos propios todavía sin
  importar a sus atlas nativos. No marcar ARTE-1–5 como terminados por tener referencias.
- Misiones: 43/350; faltan 307. Cajas: 14/420 plazas; SAVE-2/3 pendientes.
- Noa: el caso original de desobediencia continúa sin diagnóstico confirmado.

La fixture de revancha usa un NPC generado y estadísticas sintéticas altas para garantizar
una victoria: valida entrega del evento, no equilibrio del jefe. Los intentos iniciales
detectaron una invocación sin NPC y una derrota; no se atribuyen a fallos del juego.

Antes de una ROM de entrega retirar individualmente las fixtures de `/work/src`, reenlazar
con flags normales, comprobar ausencia de `RFProgress_`/`RFUiFixture_`, respaldar la ROM local
anterior y verificar cabecera/arranque. Nunca subir ROMs, `.sav`, `build/` o ramas de respaldo
que contengan binarios. Apagar Docker al cerrar el trabajo.
