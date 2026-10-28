# Continuidad de ejecución

## Legendarios recolocados (28-10-2026)

- `legends.py`: los atributos de tiles FRLG son de 32 bits pero su comportamiento ya está en la
  numeración de Emerald (0x4F escaleras de roca, 0x20 hielo); la traducción con nombres FRLG era
  errónea y se retiró. Escaleras, puentes y pendientes de la Ruta Ciclista cuentan como transitables.
- Colocación: se conserva cada legendario ya situado en suelo alcanzable; el resto se recoloca a pie,
  en modo ajustado para cuevas estrechas (sin cortar el paso), en tierra alcanzable surfeando o, como
  último recurso, en agua. Resultado: 109 a pie, 6 surfeando, 2 en agua (Thundurus e Iron Jugulis,
  Ruta 17). 23 movidos; `docs/LEYENDAS.md` regenerado. 4 actores de campaña se recolocaron con la
  clasificación corregida (baliza y tres puestos del Cañón Sétano).
- `src/wild_encounter_ow.c`: falta de paleta o tiles para un salvaje visible ya no muestra la
  pantalla de aserción (las ROM sin `RELEASE` se detenían con «could not load sprite tiles»); se
  omite esa aparición.
- Verificador: nuevo caso `legends` (117 legendarios con su saga activa, dos lecturas por la carga de
  mapas con presentación); lecturas puestas a cero antes de cada consulta en `actors` y `legends`,
  porque una consulta que no se ejecutaba repetía el valor anterior y daba falsos PASS.

## Misiones: Fase C, lote 1 (28-10-2026)

430 misiones; campañas «Censo de la hierba alta» (414–421) y «La escuela del camino» (422–429).
- `story_content.py`: `actor2`, `CHOICES` (bit 0 = `escuela_exigencia`), escenas `field`/`choice`,
  `alt_offer`/`alt_ending`, `bonus`, `visible_after`. Entrenador nuevo `TRAINER_RF_STORY_6` (Quim).
- `rf_story.c`: progreso en pistas, mensaje de contadores llenos, `Script_RFStoryChoice`.
  `rf_quest_menu.c`: nombres de capítulo generados (`src/data/rf_story_chapters.h`).
- Generador de actores: entradas por conexión solo si ambos lados son transitables; `MapGrid`
  lee atributos de 32 bits en layouts `frlg`; respaldo con Surf (solo la baliza del Pilar Recuerdo).
  20 actores ya enviados cambiaron de casilla; se revisaron los 63 en capturas mGBA.
- Pendiente detectado: 23 objetos de legendarios en casillas no transitables según la clasificación
  corregida: FiveIsland_Meadow (070), FiveIsland_MemorialPillar (044), FiveIsland_ResortGorgeous (055, 073), FourIsland_IcefallCave_B1F (023, 108: botón de Fuerza), OneIsland_KindleRoad (032), OneIsland_TreasureBeach (027, 074), Route13 (008), Route17 (012, 110), Route18 (013), Route21_North (054), Route21_South (087), Route23 (080), SevenIsland_TanobyRuins (049, 067), SixIsland_GreenPath (041, 104), SixIsland_OutcastIsland (035, 106), SixIsland_WaterPath (033). Regenerar con `legends.py` cambiará sus posiciones; validar a pie.
- mGBA: 56/56 (casos c1-seen, c1-wild-win, c1-evolve, c1-hatch, c1-cora-hidden, c1-census-final,
  c1-quim, c1-choice, c1-alt-offer). La captura en combate no tiene recorrido real (prueba C).

## Misiones: Fase B (28-10-2026)

Rama `fase-a-misiones`: Fase A en `7240f629` y Fase B en el commit que añade este apartado. Motor descrito en
[MOTOR_MISIONES.md](MOTOR_MISIONES.md).
- Medición real con el compilador ARM: SaveBlock1 15840/15872, SaveBlock2 3884/3968 (no se amplía:
  su tamaño entra en el intercambio por cable, `src/data/trade.h`), SaveBlock3 1564/1624 → ahora
  1620/1624 con la extensión de misiones, PokemonStorage 34144/35712.
- `src/achievements.c`: cola compartida con avisos de misión (prioridad, reintento si un cartel
  interrumpe el aviso al instante, descarte de avisos obsoletos).
- Pruebas: C con catálogo real mutado en copia temporal (contadores, filtros, huecos llenos,
  decisiones, IDs 500+, condiciones, requisitos por grupo, premios compuestos atómicos, objetos y
  casillas, avisos). mGBA: guardado de la extensión y partida antigua; avisos LISTA y AQUÍ.

Siguiente: Fase C (contenido), empezando por un lote que use contadores y tablas para validar
en mGBA los caminos que aún no tienen misión real.

## Misiones: Fase A (28-10-2026)

Catálogo real de 414 misiones (64 escenas de campaña en 350–413); meta acordada de 650 distintas.
- `extra_quests.py`: los registros 71–349 forman 27 cadenas de hasta 10 (la cabeza de cada cadena
  abre la siguiente); los 10 legendarios dependen solo de `RF_QUEST_REMATCH_REPORT`. Los títulos
  usan `speciesName` de la expansión. Los estados guardados no cambian de casilla.
- `rf_quest_menu.c`: una escena de campaña disponible muestra «Pídelo en persona» y quién la ofrece;
  A ya no la acepta a distancia (`RFStory_GiverHint`).
- `rf_story.c`: puzles (`RFStoryQuestion`) y carrera (`RFStoryRace`, segundos y textos) llegan desde
  `story_content.py`. Silva usa cuatro preguntas propias; Monan conserva las suyas. Corregido al
  integrar: el tiempo de llegada se evalúa antes de limpiar la carrera.
- Pruebas: 25/25 C/Python, validador de 414, ELF enlazado y verificador mGBA de campañas con casos
  nuevos `puzzle-silva`, `race`, `race-late` y `journal-locked` (ver TESTING.md). Corregido solo en
  pruebas: `RFStoryFixture_FaceActor` dejaba `callback1` en NULL (sin entrada en mapas exteriores)
  y su giro se perdía al terminar la carga; ahora conserva `CB1_Overworld` y el verificador pulsa
  la cruceta hacia el actor antes de A. Fallaban `talk` y los casos al aire libre.
- `/work` de Ubuntu-24.04 quedó sin fixture (ROM de producción reconstruida, sin símbolos de prueba).
- Nora (Camino Candente) no era un fallo del juego: el fixture usaba `SetWarpDestination`
  (coordenadas `s8`) y su fila 133 se desbordaba, dejando al jugador en el centro del mapa.
  `RFStoryFixture_WarpActor` aplica ahora la casilla completa; nuevo caso `nora` y 55/55 actores.
- Cierre: el harness usa un archivo de guardado privado (el `VFileMemChunk` vacío hacía fallar
  mGBA al guardar) y el caso `save` pasa. La «A perdida» era real: `Achievements_CheckAll`
  congelaba el mapa ~10 fotogramas cada 2 s; ahora el barrido se reparte (3 logros/fotograma) y el
  máximo bloqueo medido es 2 fotogramas. Verificador de campañas: 34/34 sin parches de tiempo.
  ROM local `build_artifacts/releases/20261028-103145/` (SHA-256 `2a92ce30…a327`), arranque al mapa.

**Fase A cerrada.** Siguiente: medir SaveBlock3 y diseñar la extensión para 650; después Fase B.

## Arte nativo y menú 3×3 (27-10-2026)

Menú principal integrado con nueve iconos redibujados a 16×16 y fuente local
Kanto Compact Pro (104 glifos, acentos españoles, avance proporcional).
Mochila sustituye el muestreo de ilustraciones grandes por maestros nativos;
comparte una paleta OBJ y muestra cuatro filas de 24 píxeles para conservar
los sprites del catálogo a tamaño real. Inventario, guardados, funciones y
condiciones de acceso se mantienen. Los originales y la importación histórica
se preservan. Generador: `tools/rojo_fuego/make_kanto_ui_art.py`.
Informe: `docs/ui/main-menu-redesign.md`; evidencia:
`build_artifacts/ui/kanto-art/`. Pasaron 21 pruebas C/Python, 350 misiones,
18 recorridos de Mochila y 23 escenarios de menú con aserciones, además de
seis escenarios de capturas revisadas. Todas las acciones se probaron en ambos
temas. Estrés: 1 matriz, 8 paletas, 28 sprites, 17408 B OBJ.


## Sprites aportados por el usuario (26-10-2026)

Se reemplazan los dibujos de la mochila anterior por los PNG de
`Downloads/mochila_sprites_separados`: siete categorías, mochila de encabezado,
cursor, flechas de bolsillo, scroll y retorno. Los originales se conservan en
`graphics/rojo_fuego/bag_user/source/`; el manifiesto guarda hashes y conversión.
Atlas 4bpp de 16×16 y retorno 32×32, con tres paletas compartidas para mantener
el morado, verde, oro y grises. `src/rf_bag_art.c` administra estos sprites.
No cambian las siete categorías, la capacidad de 80 espacios ni los controles.
Evidencia de esta sustitución en `build_artifacts/ui/bag-user-sprites/`.
Validación: 19 pruebas, 18 recorridos mGBA y 106 capturas. ROM de producción en
`build_artifacts/releases/20261026-233217/pokefirered.gba`, SHA-256
`c24eaf41d306c78c5b1f8ba403debce8c8a326b0c6a8fb94003249a1450c3563`.
Arranque al mapa comprobado, sin fixtures. Recursos observados: 30 sprites,
12 paletas OBJ, 7 matrices y 18 KiB de tiles OBJ.

## Mochila basada en referencia pixel art (26-10-2026)

La nueva petición adopta la referencia adjunta: cabecera de 40 px con siete iconos
originales de 8×8, selección menta, cursor dorado, papel crema en claro y superficies
turquesa en oscuro. Lista de seis filas con iconos reales a media escala, cantidades
alineadas y barra de desplazamiento. Se conservan los IDs y las reglas de las siete
categorías; la capacidad sigue en 80 espacios. La vista previa muestra nombre y cantidad.

`RFUI_LoadBagPalette` integra los dos temas; `RFUI_PrintParagraph` ajusta descripciones
por palabras, con fallback español para texto ausente y elipsis cuando no cabe completo.
`src/data/rf_bag_text.h` localiza textos habituales para esta pantalla sin alterar el
catálogo global; los textos aún no traducidos mantienen su original. El orden por nombre
sigue el catálogo original. No se modificaron formatos de guardado ni efectos de objetos.

Verificador reproducible: `tools/rojo_fuego/test/verify_bag_reference.py`, con fixtures
temporales de medicina en campo/combate, cancelación, registro, ordenamiento, casos de
texto y presupuesto de recursos. Evidencia en `build_artifacts/ui/bag-reference/`;
comandos y resultados finales en `docs/development/TESTING.md`. En esta máquina solo está
instalada `Ubuntu` 26.04.1 LTS (WSL 1), por lo que se usa el snapshot `/home/fabian/rf-ui-20261026`.
Entrega final: `build_artifacts/releases/20261026-220850/pokefirered.gba`, SHA-256
`3e7d5d10a390e407c7839c507d8240507ddfde6263b6b17c19ece6e4244482bf`.
19 pruebas, 18 recorridos mGBA y 106 capturas; arranque de producción al mapa correcto.

## Mochila recompuesta desde cero (26-10-2026)

Pedido más reciente: cambiarla y hacerla desde cero. Nueva composición con lista a la
derecha y vista del objeto, nombre y descripción a la izquierda. Se oculta el gran
sprite de mochila, se añaden acentos coral y se recolocan flechas, línea de intercambio
y datos de MT/MO. Sus cuatro filas compactas ya no invaden el pie. Las acciones
principales están en español. Mantiene siete categorías, siete filas visibles,
capacidad de 80 espacios y el mismo guardado. Evidencia en `build_artifacts/bag-redesign/`.
Las pasadas anteriores se conservan debajo como historial.

## Mejora de legibilidad de mochila (26-10-2026)

Segunda pasada: las siete pestañas ocupan todo el ancho; el nombre completo del filtro
se muestra en el encabezado. Una banda de fondo y un indicador lateral siguen la fila
seleccionada; se tradujo la salida a «Cerrar mochila». 48 capturas adicionales de ambos
temas/sprites incluyen movimiento del foco, última fila y regreso al mapa; evidencia en
`build_artifacts/ui-improvements-v2/`. Se conservan categorías, capacidad y guardados.

Por petición de interfaz y gráficos: encabezado con icono nativo, nombre completo del
filtro activo, marca superior de selección y tinta oscura sobre la pestaña dorada en
tema claro. El pie muestra también la cruceta para cambiar de bolsillo. Conserva las
siete categorías y la capacidad de 80 espacios. Capturas mGBA de ambos sprites y temas
en `build_artifacts/ui-improvements/`; 18 pruebas y las 350 misiones validadas.
Esta entrega mejora la presentación existente; ARTE-2 sigue pendiente de redibujo.

Actualización: 26-10-2026. Continuar el trabajo abierto de [PLAN_COMPLETO.md](PLAN_COMPLETO.md);
Ubuntu 24.04 en WSL es el entorno de compilación activo y Docker permanece apagado.

Actualización 26-10-2026: el catálogo ya tiene **350 misiones** (IDs 0–349); el validador
confirma referencias, textos y prerrequisitos, y la ROM compila en Ubuntu. El motor ya incluye
captura persistente, recogida de objetos, uso exitoso y objetivos de dos requisitos, sin cambios
de interfaz.

## Mochila ampliada y ROM validada (26-10-2026)

- El bolsillo de objetos pasa de 60 a **80 espacios**: 30 del guardado original, 30 de la
  extensión existente y 20 nuevos. Los 84 bytes nuevos se anexan al final de SaveBlock1,
  con magic independiente; no se desplazan offsets de datos existentes. La compilación
  impone los límites de tamaño serializado de SaveBlock1 y SaveBlock3.
- Se añadieron siete filtros navegables con L/R: TODO, CURA, COMB, EVO, EQU, TES y OTR.
  La clasificación usa `sortType`; cambiar categorías conserva los espacios físicos y
  ordenar se desactiva bajo filtros. El control L/R funciona también con la configuración
  normal de botones. Las capturas mGBA confirman las siete listas en tema claro/oscuro y
  la navegación posterior a Balls y MT; material en `build_artifacts/ui/`.
- `tools/rojo_fuego/tests/test_bag.py` cubre slots 60 y 79, inicialización de los dos bloques,
  límites, cifrado y remoción; las 18 pruebas nativas y los 71 datos de misiones pasan.
- `wsl.ps1 check` enlaza el ELF sin generar ROM. `wsl.ps1 build` genera la ROM limpia y
  `verify_release.py` la inicia hasta el mapa con el harness mGBA. Entrega local:
  `build_artifacts/releases/20261026-111139/pokefirered.gba`, 32 MiB, código BPRE,
  SHA-256 `92d69f4ed913dbdda32b518a2b5ca5aba7e34af5788b0cb42981afb57668b4b1`.
- La ROM no lleva fixtures. No se copió al Escritorio ni se modificó ninguna partida real.
- ARTE-2 parcial: la paleta común de las hojas masculina/femenina ahora usa rojos, oro y
  sombra petróleo. Se mantienen tamaño 64×384, los seis cuadros y la animación. Capturas
  mGBA verifican ambos sprites, categorías y bolsillos en claro/oscuro. Falta redibujar la
  silueta con el concepto original, conservando los seis cuadros.
- ARTE-3 validado: mGBA muestra lista, búsqueda, hábitat, estadísticas, evoluciones, formas,
  grito y tamaño del atlas HGSS. ARTE-4: inicio, equipo, PC, Misiones, Claves y tienda se
  revisaron en ambos temas; faltan variantes iconográficas propias para algunas pantallas.
  Capturas en `build_artifacts/ui/`.
- Pruebas finales: `wsl.ps1 test` → 18/18 y 71 misiones válidas; `verify_release.py` confirma
  arranque hasta el mapa. WSL queda sin el fixture de interfaz en `src/`.
- Siguiente paso visual: redibujar las seis poses del sprite de mochila en formato nativo
  (ARTE-2); luego continuar la implementación de menú y presentación del plan completo.

## Punto guardado para retomar

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

**Cambio solicitado el 26-10-2026: trabajar directamente en Ubuntu WSL, con Docker apagado.**
Distribución `Ubuntu-24.04`, fuentes en la carpeta Windows habitual y copia compilable en
`/work`. Comandos, dependencias y verificadores: [ENTORNO_WSL.md](ENTORNO_WSL.md).
Usar `./tools/rojo_fuego/wsl.ps1 check`, `test` y `build` desde PowerShell (dos trabajos por
defecto). Los verificadores Python usan Ubuntu; `RF_TEST_BACKEND=docker` conserva el entorno
anterior. No arrancar Docker para continuar el plan. Las instrucciones de contenedor que
siguen son históricas; las fixtures se preparan ahora en `/work` de Ubuntu.
Validado: enlace ELF, 18 pruebas nativas, catálogo de 71 misiones, sincronización incremental,
ROM sin fixtures y arranque hasta el mapa en mGBA. ROM de comprobación en
`build_artifacts/wsl/pokefirered.gba`; informe `build_artifacts/releases/20261026-093902/`.
La ROM del Escritorio no se reemplazó. Los avisos de metadatos PNG se detallan en la guía.
Ubuntu queda encendido porque hay procesos de otro proyecto en la misma distribución.

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
Docker Desktop y el contenedor permanecen apagados. Ubuntu WSL es el entorno activo;
no detener la distro porque allí hay procesos de otro proyecto.

Para reproducir: copiar las fuentes/herramientas actualizadas a `/work`, ejecutar allí
`bash tools/rojo_fuego/test/build_progression_fixture.sh`, y ejecutar desde Windows
`python tools/rojo_fuego/test/verify_progression.py` (también `--berries-off`). Regenerar siempre
símbolos/estados al cambiar el enlace. Ver también `test/UI_VALIDATION.md`.

## Siguiente acción concreta

1. Completar ARTE-2: redibujar las seis poses de mochila para que coincidan con el concepto,
   conservando ambas variantes, la animación, las categorías y el contraste claro/oscuro.
2. Extender ARTE-4 con iconografía propia para equipo, PC, Claves y tienda, validando tema,
   selección y desactivación.
3. Cerrar ARTE-5 con captura mGBA tras integrar el sprite; validar ROM limpia y memoria/VRAM.
4. Continuar SERVICIOS con preparación en Centros Pokémon y herramientas unificadas, reutilizando
   mentas, parches y tutores existentes.
5. Mantener SAVE-2 separado hasta cerrar sectores y migración; completar regresión integral de
   apodo, primera batalla, guardado/carga y Android.
6. Registrar cada paquete validado aquí y en PLAN_COMPLETO.md; conservar ROMs y guardados fuera de Git.

## Límites y pendientes que no deben perderse

- Iniciales: las 54 entregas y escenas de elección están comprobadas; el rival conserva
  equipos de Kanto. Quedan para regresión integral el recorrido de partida nueva hasta
  la primera batalla, apodo, guardado/carga y Android. No afirmar que ya se probaron.
- Assets: ARTE-1 y 3 validados; ARTE-2 parcial (paleta integrada, silueta por redibujar);
  ARTE-4 tiene revisión visual pero falta integrar variantes; ARTE-5 se repite tras esos cambios.
- Misiones: 414 (meta 650); el catálogo extendido combina Pokédex, recogida, uso exitoso y requisitos
  dobles. La distribución narrativa completa sigue pendiente. Cajas: 14/420 plazas; SAVE-2/3
  pendientes.
- Noa: el caso original de desobediencia continúa sin diagnóstico confirmado.

La fixture de revancha usa un NPC generado y estadísticas sintéticas altas para garantizar
una victoria: valida entrega del evento, no equilibrio del jefe. Los intentos iniciales
detectaron una invocación sin NPC y una derrota; no se atribuyen a fallos del juego.

Antes de una ROM de entrega retirar individualmente las fixtures de `/work/src`, reenlazar
con flags normales, comprobar ausencia de `RFProgress_`/`RFUiFixture_`, respaldar la ROM local
anterior y verificar cabecera/arranque. Nunca subir ROMs, `.sav`, `build/` o ramas de respaldo
que contengan binarios. Mantener Docker apagado y Ubuntu WSL disponible para continuar.
