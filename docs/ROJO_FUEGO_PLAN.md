# PLAN — Pokémon Rojo Fuego: modificaciones + ruta a PC/Android

## Actualización de trabajo: misiones y almacenamiento (22-10-2026)

La instrucción vigente es **verificar código con Docker, incluidos objetos y enlace ELF, sin
crear ni publicar un `.gba`**. Sustituye las indicaciones anteriores de compilar una ROM tras
cada cambio. Al terminar se apaga Docker Desktop y se sube únicamente el código a GitHub.

Las tandas implementadas suman 30 misiones reales con espacio persistente para 500 (meta de contenido:
350), mochila de objetos de 60 espacios, ordenación y protección del PC, cuatro objetos y
Castform de arena. Alcance, controles, restricciones de guardado y validación en
[IMPLEMENTACION_MISIONES_ALMACENAMIENTO.md](IMPLEMENTACION_MISIONES_ALMACENAMIENTO.md).
También se implementó botín de salvajes opcional con reserva persistente y aviso en combate.
La investigación y el contenido pendiente están en [IDEAS_OTROS_HACKS.md](IDEAS_OTROS_HACKS.md).
No se considera completado el plan de 350 misiones ni la ampliación de cajas de Pokémon.


> Documento de continuidad. Si se agotan los tokens de la sesión, leer este archivo y seguir
> desde la sección "ESTADO ACTUAL" (cerca del final). Fecha de última actualización: 2026-10-22.

## ⚠ INSTRUCCIONES DE TRABAJO ACTUALES (usuario, 2026-10-22)
- **Trabajar sin detenerse** en el orden del plan hasta que quede ~5 % del presupuesto de tokens.
- **Antes de terminar**: (1) dejar todo documentado en este PLAN; (2) subir el **código** a un repo
  **privado** de GitHub del usuario (con `gh`; el MCP de GitHub no conecta) — **la ROM NO se sube**
  (copyright de Nintendo / ToS de GitHub; avisado al usuario); (3) copiar la ROM generada al
  **Escritorio** (`C:\Users\Fabian-Pc\Desktop\`).
- Orden de trabajo: DexNav restante → Omni Ring → resto del paso 1 (megapiedras con Pokémon a
  derrotar, desafíos Z, dificultad, apariciones masivas, RTC) → mejoras "hay que programarlas" →
  logros (325) + RetroAchievements. Compilar tras cada cambio (0 errores, 0 warnings).
  **Ese orden está COMPLETADO** (ver secciones de implementación); quedan los pendientes opcionales.

## Objetivo final

Modificar Pokémon Rojo Fuego (agregar y quitar funcionalidades; ejemplos dados por el usuario:
**capture EXP**, **Exp. All**, etc.) y llegar a ejecutarlo en **PC** y luego en **Android (APK)**.

## Decisiones tomadas (confirmadas por el usuario)

1. **Ruta técnica: decompilación legible `pret/pokefirered`** (C editable, recompilable).
   - El usuario inicialmente eligió "Recomp" (`mstan/FireRedLeafGreenRecomp`), pero luego aclaró
     que quiere **agregar y quitar funcionalidades**: la recompilación estática genera C ilegible
     a partir de código máquina → **descartada para edición**. El decomp es el camino correcto.
   - `morluto/rea` (Ghidra/Hopper MCP) queda como herramienta de análisis puntual para rutinas que
     el decomp no cubra (p. ej. código del BIOS o hardware), no como camino principal.
2. **Entorno de build: WSL** (recomendado por INSTALL.md de pret). **Montado y operativo**
   (WSL1 + Ubuntu; ver "Estado actual"). WSL2 descartado (VT-x desactivado en BIOS, no hace falta).
3. **APK Android: dos rutas, empezar por la rápida**
   - Rápida (días): ROM modificada + **mGBA APK** en el móvil (emulador, uso personal).
   - Nativa (meses): port a SDL2 de FR/LG → target Android de SDL2 → APK. **No existe port nativo
     de FR/LG hecho**; precedente: `gradenGnostic/pokeemerald-multiplatform` (Emerald, con
     Makefile_pc para Windows y soporte Android). Solo si se quiere el esfuerzo.
4. **Legal**: ingeniería inversa de ROM propia para uso privado/interoperabilidad OK (Directiva UE
   2009/24/EC). **No distribuir** binarios/APK/ROMs modificadas con assets de Nintendo. Uso personal.

## Estado actual (verificado en la máquina)

- SO: Windows 11, PowerShell 5.1 (sin `&&`; usar `;`).
- `git` 2.55.0 OK; `winget` OK. `make` NO instalado (no hace falta fuera de WSL).
- **WSL1 + Ubuntu: LISTO (desde 2026-10-21 → usar en próxima sesión)**:
  - Tras un 2.º reinicio, `wsl --set-default-version 1` funcionó (WSL1 no necesita VT-x).
  - **Ubuntu 26.04.1 LTS instalado en WSL1**. Usuario por defecto: **`fabian`** (sudo sin
    contraseña, creado vía `/etc/wsl.conf`). Invocar desde PowerShell con:
    `cmd /c "wsl -d Ubuntu -- bash -lc ""<comando>"""` (con `$env:WSL_UTF8=1` para salida limpia).
  - Paquetes ya instalados dentro de Ubuntu: `build-essential`, `binutils-arm-none-eabi`,
    `git`, `libpng-dev` (apt update OK; quedan 131 paquetes por upgrade — **no** son necesarios).
  - Script helper usado: `C:\Users\Fabian-Pc\decomps\wsl-setup.sh`.
  - Nota: `VirtualMachinePlatform` quedó habilitado pero VT-x está `False` en BIOS → WSL2 no
    funciona; **no hace falta**: WSL1 es el camino elegido y ya opera.
- **ROM del usuario** (NO necesita el build; solo sirve como referencia de textos/guardados):
  - Ruta: `C:\Users\Fabian-Pc\Downloads\Pokemon - Edicion Rojo Fuego (Spain)\Pokemon - Edicion Rojo Fuego (Spain).gba`
  - SHA1: `AB8F6BFE0CCDAF41188CD015C8C74C314D02296A` (edición española, 16 MB).
  - El decomp es de la **versión inglesa**. `make compare` espera SHA1
    `41cb23d8dccc8ebd7c649cd8fbb58eeace6e2fdc` (FR USA v1.0) → **no se podrá verificar
    `compare`** sin esa ROM inglesa. El build funciona igual (todos los assets están en el repo).
- Directorio de trabajo del plan: `C:\Users\Fabian-Pc\decomps\` (futuros clones aquí).

## Fase 1 — Entorno WSL

### ✅ Hecho (no repetir)
1. WSL1 habilitado + reinicio OK; `wsl --set-default-version 1` OK.
2. Ubuntu 26.04.1 LTS instalado en WSL1 (`wsl --install -d Ubuntu --no-launch`).
3. Usuario `fabian` (sudo NOPASSWD) creado y como default (`/etc/wsl.conf`).
4. Paquetes instalados: `build-essential binutils-arm-none-eabi git libpng-dev`.

### ⬜ Pendiente: compilar la ROM base (continuar AQUÍ en la próxima sesión)

Decidir primero la **base**: vanilla `pret/pokefirered` o hack (ver sección "Base alternativa").
Luego, todos los comandos se ejecutan **dentro de WSL**:
`cmd /c "wsl -d Ubuntu -- bash -lc ""<comando>"""` desde PowerShell.

5. Directorio de trabajo: `cd /mnt/c/Users/Fabian-Pc/decomps`
6. Clonar + toolchain agbcc:
   ```bash
   git clone https://github.com/pret/pokefirered   # o el repo del hack elegido
   git clone https://github.com/pret/agbcc
   cd agbcc && ./build.sh && ./install.sh ../pokefirered && cd ..
   ```
   (Si la base es un hack, su INSTALL.md manda; el agbcc es el mismo.)
7. Build base:
   ```bash
   cd pokefirered
   make -j$(nproc)
   ```
   Salida esperada: `pokefirered.gba`. **No confiar a ciegas en `make`: verificar que exista
   el archivo y que no haya errores de compilación en la salida.**
8. **Verificación Fase 1**: `make compare` NO aplica (ROM española / hack). Verificar instalando
   **mGBA en Windows** (p. ej. `winget install mGBA.mGBA` o release de
   https://mgba.io) y abriendo `pokefirered.gba`: debe llegar a la pantalla de título.

## Base alternativa: hack existente en vez de vanilla (consultado 2026-10-20)

El usuario considera usar un hack decomp como base (nivel medio, hereda sus mejoras).
Verificado que **ambos publican fuente** (forks de `pret/pokefirered`, mismo flujo de build):

- **FireRed+/LeafGreen+**: https://github.com/Deokishisu/FRLG-Plus (QoL maduro, FAQ/features).
- **FireRed Reignited**: https://github.com/Bruhcation/pokefireredlegacy (estilo Legacy SmithPlays).

Diferencias vs vanilla: `make compare` no aplica (no hay ROM original que igualar; verificar
solo que compile y arranque), docs de pret aplican ~90% (confirmar funciones con `grep`),
no conviene mergear upstream, en inglés. Decisión de cuál elegir: el que más guste jugando.
Si se elige hack como base, cambiar la Fase 1 para clonar ese repo en vez de `pret/pokefirered`
(el `agbcc` es el mismo).

## Fase 2 — Modificaciones (features de ejemplo)

- Ubicar en el repo clonado (NO inventar rutas; verificar con grep antes de editar):
  - Lógica de EXP en batalla: `src/battle_*.c` (buscar `gBattleMovements`/`AwardExp`/`exp` con grep).
  - Tabla de ítems y efectos de uso: buscar `gItems` / `ItemUseFunc` en `src/`.
  - "Capture EXP": ganar EXP al capturar (hook tras la captura → `src/battle_main.c` o
    `src/pokemon.c` según corresponda).
  - "Exp. All": ítem nuevo que reparte EXP a todo el equipo (efecto similar al EXP Share
    existente; reusar su lógica).
- Flujo por feature: escribir → `make -j$(nproc)` → probar en mGBA → iterar.
- Textos en español: los strings están como texto plano en C (charmap del juego). Traducir con
  la ROM española del usuario como referencia (extraer textos con un tool si hace falta).
- Ayudas: `porymap` (mapas), `poryscript` (scripts), `Tilemap Studio` — según la feature.

## Fase 3 — Android / APK

- **Corta**: la ROM modificada `.gba` + mGBA APK en el móvil (sin código). Uso personal.
- **Larga**: port nativo SDL2 (reutilizar la capa SDL de los ports de Emerald:
  `gradenGnostic/pokeemerald-multiplatform`, base `Kurausukun`/NT_x86/Normmatt/camthesaxman) →
  `SDL2` target Android → APK con NDK/Gradle. Proyecto grande; solo si el usuario lo pide.

## Puntos pendientes / riesgos

- [x] WSL1 + Ubuntu + paquetes instalados (ver sección Estado actual).
- [x] **Base decidida (2026-10-21): FRLG-Plus** (`Deokishisu/FRLG-Plus`, commit `f198fb944`).
      Clonado en `C:\Users\Fabian-Pc\decomps\FRLG-Plus` (+ `agbcc` al lado). Su INSTALL.md
      es el de pret: mismo flujo (`./install.sh ../FRLG-Plus`, `make`).
- [x] Clonar FRLG-Plus + agbcc; agbcc compilado e instalado en `FRLG-Plus/tools/agbcc` (2026-10-21).
- [x] **Primer build OK (2026-10-21)**: `FRLG-Plus\pokefirered.gba` (16 MB, `EXIT=0`).
- [x] Usuario confirmó: en mGBA llega a la pantalla de título → **Fase 1 cerrada**.
- **Pedidos del usuario para la Fase 2 (2026-10-21)**, todo configurable como claves del
  Key System de FRLG-Plus (`src/key_system_menu.c`, flags en `struct KeySystemFlags`,
  `include/global.h`):
  1. **Capture EXP**: EXP completa, como si lo hubiera derrotado.
  2. **Exp. All**: todos los Pokémon del equipo reciben la EXP completa (sin repartir).
  3. **Muchas mejoras de generaciones posteriores** (alcance por definir con el usuario).
  4. **Ítem tipo DexNav mejorado** (diseño por definir con el usuario).
  Orden propuesto: 1+2 primero (están claros), luego 3, luego 4.
- **Catálogo de mejoras para elegir (punto 3)**: https://claude.ai/artifact/M8jQMfoB1bHNNt82gDWdcM
  (fuente: scratchpad `mejoras-rojo-fuego.html`). La selección del usuario se guarda en su db,
  documento `picks/main` (`selected`: ids, `notes`); leerlo con ArtifactData `get`.
- **Selección del usuario (2026-10-22)**: las 42 mejoras del catálogo + nota "agregar todos
  los Pokémon hasta la Gen 9".
- **⚠ Replanteo de base pendiente**: ese alcance es justo lo que ya trae
  `rh-hideout/pokeemerald-expansion`, que en `master` tiene build de FRLG (`make firered`,
  verificado en su Makefile el 2026-10-22). `cawtds/pokefirered-expansion` está abandonado y
  remite a ese build. Hacerlo a mano sobre FRLG-Plus = meses/años. Pregunta abierta al
  usuario: ¿cambiar a expansion (perdiendo el Key System y QoL de FRLG-Plus) o seguir con
  FRLG-Plus con un alcance menor?
  → **Decidido (2026-10-22): cambiar a pokeemerald-expansion en modo FireRed.** Clonar en
  `decomps\pokeemerald-expansion`, compilar con `make firered`, probar en mGBA. FRLG-Plus
  queda como referencia (su `src/key_system_menu.c` sirve de modelo para el Key System nuevo).
  - [x] Clonado `decomps\pokeemerald-expansion` (commit `f4dc700a`, 2026-10-20) desde Windows con
        `git -c core.autocrlf=false clone --depth 1 ...` (WSL tuvo un fallo de DNS puntual).
  - [x] Expansion usa GCC moderno, no agbcc: instalados en WSL `gcc-arm-none-eabi` (14.2.1),
        `libnewlib-arm-none-eabi`, `python3` (3.14).
  - Guía FRLG del repo: `docs/tutorials/how_to_frlg.md` (`make firered`; `make clean` si se
    alterna con emerald; mapas nuevos necesitan `"region": "REGION_KANTO"`).
  - Build: `wsl -d Ubuntu -- bash /mnt/c/Users/Fabian-Pc/decomps/wsl-build.sh pokeemerald-expansion firered`
    (el script ahora recibe carpeta + args de make).
  - [x] **Primer build `make firered` OK (2026-10-22)**: `pokeemerald-expansion\pokefirered.gba`
        (32 MB, `EXIT=0`, 0 errores). Abierto en mGBA.
  - [ ] Usuario confirma que arranca y se juega bien en mGBA (no lo confirmó; pidió seguir).
  - **Key System sobre expansion (diseño propuesto 2026-10-22, pendiente de aprobación)**:
    las opciones de combate pasan por `GetConfig(NOMBRE)` (`include/config_changes.h`; 246
    entradas en `include/constants/config_changes.h`, incl. `B_EXP_CATCH`, `B_SPLIT_EXP`,
    `B_SCALED_EXP`, `B_PHYSICAL_SPECIAL_SPLIT`, `B_CRIT_MULTIPLIER`…). Fuera de tests,
    `GET_CONFIG_VALUE` devuelve el valor de compilación; la idea es que consulte un bloque de
    claves guardado en la partida → claves cambiables en juego sin tocar cada uso.
    Otras claves usan flags que expansion ya ofrece (`I_EXP_SHARE_FLAG`, `B_FLAG_INVERSE_BATTLE`,
    `WE_FLAG_FORCE_DOUBLE_WILD`, `OW_FLAG_NO_TRAINER_SEE`, `B_FLAG_FOLLOWERS_DISABLED`,
    `P_FLAG_SUMMARY_SCREEN_IV_EV_INFO`…). Menú: modelo `FRLG-Plus/src/key_system_menu.c`;
    sitio en expansion: `src/main_menu.c` (FRLG usa `IS_FRLG`).
  - **DexNav**: `include/config/dexnav.h` (`DEXNAV_ENABLED FALSE`, necesita 3 flags + 2 vars
    libres), `src/dexnav.c` (2672 líneas). Diseño del "DexNav mejorado" pendiente con el usuario.
  - **Catálogo v2 publicado** (misma URL): +28 mejoras en dos secciones nuevas ("ya en
    expansion, apagadas" y "hay que programarlas"). Releer `picks/main` cuando el usuario elija.
  - **Selección v2 (2026-10-22, `picks/main` versión 65)**: 64 mejoras. De las nuevas NO eligió:
    combates inversos, dobles salvajes, evitar entrenadores, Nuzlocke, randomizer, bici/surf.
  - **Nuevo pedido: sistema de logros "enorme"** (proyecto propio). El usuario delegó el diseño
    ("como los de Steam con recompensas"). Propuesta publicada: https://claude.ai/artifact/Mzqdgn5o47nQYBMZuJfCBQ
    (fuente: scratchpad `logros-kanto.html`). 157 logros, 12 categorías, rangos Bronce/Plata/Oro/
    Platino (10/25/50/100 pts), ocultos, títulos en la Ficha de Entrenador, aviso tipo toast,
    menú LOGROS en Start con reclamo de recompensas. Lo que quite el usuario y sus notas se guardan
    en db `feedback/main` (`removed`, `notes`).
    Orden acordado: activar mejoras → Key System → DexNav → logros (al final, para cubrir todo).
    - **Ampliado a 325 logros** a pedido del usuario ("500 o 300"; elegí ~300 para evitar relleno).
      Incluye 18 "Especialista <tipo>" generados en JS.
    - **RetroAchievements** (pedido del usuario): espejo de los bits de logros en una dirección
      FIJA de EWRAM (las saveblocks de FRLG se desplazan), y script que genera desde la misma tabla
      un archivo de logros locales para RALibretro. Publicarlo en la web de RA requiere aprobación de
      su equipo. Verificar antes de implementar: formato del archivo local y si RetroArch (Android)
      admite sets locales.
  - **DexNav mejorado (respuesta del usuario 2026-10-22)**: "más información de los Pokémon y si el
    próximo Pokémon es shiny". Idea: pregenerar el Pokémon oculto (incl. personalidad → variocolor)
    al iniciar la búsqueda y mostrar antes del combate: variocolor sí/no, naturaleza, habilidad
    (marcando la oculta), objeto, movimiento huevo, IVs/estrellas, sexo, nivel; más datos de especie
    (tipos, stats base, EVs que da, ratio de captura, % de aparición en la zona, capturado o no).
    **Extras aprobados (usuario: "todos")**: 1) cadena shiny visible tipo Poké Radar con prob.
    actual; 2) modo cazador con filtros (shiny/HA/naturaleza/3★, más pasos); 3) alerta de shiny
    cercano sin buscar; 4) lista de la zona por método (hierba/surf/pesca/rompe rocas) con %, día/
    noche, siluetas y faltantes; 5) aparición masiva activa / legendario errante en la zona;
    6) calculadora de captura por cada ball de la mochila; 7) comparar IVs/stats con tu Pokémon de
    la misma especie; 8) ficha completa: 4 movimientos + tipo de Poder Oculto; 9) flecha de
    dirección hacia el oculto; 10) atajo con Select para repetir búsqueda; 11) historial por especie
    (nivel de búsqueda + mejor encontrado; ojo con el espacio de guardado).

### Omni Ring y objetos de mecánicas (decidido por el usuario 2026-10-22)
- **Solo el Omni Ring** (objeto clave nuevo): sustituye a Mega Aro, Pulsera Z, Maximuñequera y
  Teraorbe. Se entrega **junto con la Pokédex** (Oak). Comprobaciones a cambiar:
  `battle_util.c` (~8564 mega, ~8605 Z), `battle_z_move.c` (121, 187), `battle_dynamax.c` (86),
  `battle_terastal.c` (84), `script_pokemon_util.c` (47, recarga Tera).
- **Es limitado** y tiene **expansiones repartidas por el mapa**. Confirmado por el usuario:
  - Expansiones = **las dos cosas**: unas **desbloquean mecánicas** (el anillo empieza sin poderes)
    y otras **suman cargas**.
  - Límite = **cargas que se recargan en el Centro Pokémon** (como el Teraorbe), aplicado a Tera y
    Dinamax; Mega y Z siguen siendo una vez por combate como en los juegos oficiales.
  - Propuesta mía (por confirmar al implementar): orden de desbloqueo Mega → Z → Dinamax → Tera
    repartido por la historia, y expansiones de carga escondidas/como recompensa (también logros).
  - **El usuario delegó las decisiones** (2026-10-22): qué Pokémon salvajes/jefes y dónde, qué
    objetos y dónde (megapiedras, cristales Z, expansiones del anillo). Decidir y documentar aquí.
- **IMPLEMENTADO (compila, 0 warnings; commit tras `36e09745`)**:
  - Objetos nuevos (`include/constants/items.h` 874-879, datos en `src/data/items.h`): `ITEM_OMNI_RING`,
    `ITEM_OMNI_MODULE_MEGA/_Z/_DYNAMAX/_TERA`, `ITEM_OMNI_CELL`. Lógica en `src/omni_ring.c`
    (`include/omni_ring.h`); estado en SaveBlock3 (`omniRingMechanics`, `omniRingMaxCharges`,
    `omniRingCharges`, máx. 9). Usar el anillo desde la mochila muestra módulos y cargas; usar un
    módulo/célula lo instala y se gasta (`ItemUseOutOfBattle_OmniRing/OmniUpgrade` en `item_use.c`).
    El primer módulo Dinamax o Tera da la primera carga.
  - Combate: Mega/Z aceptan anillo con módulo (`battle_util.c`, `battle_z_move.c`); Dinamax con módulo
    funciona en cualquier combate si hay carga y gasta una (`battle_dynamax.c`); Tera igual
    (`battle_terastal.c`); curar (`HealPlayerParty`, `script_pokemon_util.c`) recarga el anillo.
  - Oak entrega el anillo tras las 5 Poké Balls (`PalletTown_ProfessorOaksLab_Frlg/scripts.inc`).
  - **Generador `tools/rojo_fuego/content.py`** (ejecutar en WSL desde la raíz; idempotente, pero si se
    cambian etiquetas restaurar antes los map.json con `git ls-files -m data/maps | grep map.json$ |
    xargs git checkout --`). Reutiliza objetos del suelo originales (posición accesible + flag propio):
    - Módulos: Mega=Monte Moon B2F (era Revivir), Z=S.S. Anne B1F hab.3 (Éter), Dinamax=Silph 4F (Cuerda
      Huida), Tera=Mansión 1F (Cuerda Huida). Células (8): Túnel Roca 1F, Escondite Rocket B2F, Torre
      Pokémon 3F, Safari Este, Central de Energía, Calle Victoria 2F, Ruta Kindle, Cañón Sevault.
    - 24 guardianes de megapiedra (sprite del Pokémon en el sitio del objeto; combate salvaje con
      +1 Def/DefEsp por `settotemboost`; ganar o capturar → megapiedra): Bosque Verde Beedrill 12,
      Monte Moon Clefairy 14 (Clefablita), Ruta 2 Pidgeotto 18, Ruta 11 Weepinbell 24 (Victreebelita),
      Túnel Roca Onix 26 (Steelixita), Torre Pokémon Haunter 30, Safari Kangaskhan/Pinsir/Scyther 30
      (Scizorita), Silph 6F Kadabra 36, Islas Espuma Gyarados/Slowbro 40, Mansión Aerodactyl 42, Ruta
      Kindle Houndoom 44, Isla Cuatro Starmie 45 (Starminita), Bosque Baya Heracross 45 y Venusaur 55,
      Ruta Acuática Dragonair 50 (Dragoninita), Cañón Sevault Pupitar 52, Monte Ascuas 2× Charizard 55
      (X e Y), Cueva Helada Blastoise 55, Cueva Celeste Machamp 62 (Mewtwonita X) y Alakazam 62 (Y).
    - 18 desafíos Z (vecino fijo de cada pueblo; requisito de medalla/Campeón; ayudante + dominante con
      +1 a todo; flags `FLAG_RF_Z_TRIAL_00..17` = `FLAG_0x0B0..0x0C1`): Normal Pueblo Paleta, Roca
      Plateada, Tierra Verde, Agua Celeste, Eléctrico Carmín, Fantasma Lavanda, Planta y Psíquico Azulona,
      Veneno Fucsia, Fuego Canela, Lucha/Hada/Siniestro Islas 1-3, Hielo/Volador/Bicho/Dragón Islas 4-7,
      Acero Meseta Añil. Al terminar, el vecino vuelve a su diálogo original.
  - Scripts generados: `data/scripts/rojo_fuego_guardians.inc`, `rojo_fuego_ztrials.inc` (incluidos solo
    `#if IS_FRLG` en `data/event_scripts.s`).
  - **Flags libres para contenido nuevo: `FLAG_0x0C2`–`FLAG_0x153`** (resto del bloque reservado).
  - No hecho: cristales Z de especie (Pikanium, etc.) y megapiedras de especies no presentes en Kanto.
- **Cristales Z**: se obtienen superando **desafíos** (estilo pruebas de Alola).
- **Megapiedras**: se obtienen **derrotando a un Pokémon concreto en un punto concreto** del mapa.

### Reloj, dificultad y apariciones masivas (HECHO, compila; commit "day/night clock…")
- **RTC simulado** (`OW_USE_FAKE_RTC TRUE`; 20 s de juego por 1 s real → día = 72 min; partida nueva
  a las 10:00 en `src/fake_rtc.c`). Funciona en cualquier emulador.
- **⚠ Trampa descubierta**: en la build FRLG los IDs de vars de Esmeralda (`include/constants/vars.h`)
  **pisan vars reales de FRLG** (`VAR_DAYS`=0x4040=récord Magikarp, `VAR_BIRCH_STATE`=0x4049=HOF brag,
  `VAR_MIRAGE_RND_H`=0x4024=Cueva Cambiante…). Por eso expansion no ejecutaba eventos diarios en FRLG.
  `src/clock.c`: en FRLG se inicializa solo (`InitTimeBasedEvents` si falta `FLAG_SYS_CLOCK_SET`),
  cuenta días en `VAR_DAYS_FRLG` y `DoDailyEvents` solo hace: flags diarios, semilla, aparición masiva,
  Pokérus y formas. **No usar vars/flags con nombre de Esmeralda en código que corra en FRLG.**
- **Dificultad** como clave de 3 valores en el Key System (`KEY_DIFFICULTY`, `keySystemDifficulty` en
  SaveBlock3): Fácil −10 % (mín. 1) / Normal / Reto +10 % (mín. 2) al nivel de los Pokémon de
  entrenadores (`src/trainer_util.c` → `KeySystem_ScaleTrainerLevel`).
- **Apariciones masivas diarias** (`src/mass_outbreak.c`, `TryStartKantoOutbreak`): 31 zonas, cada una
  con un Pokémon de otra generación (Skwovet R1, Bidoof R22, Lechonk R2, Blipbug Bosque Verde, Starly
  R3, Bronzor Monte Moon, Rookidee R4, Flabébé R24, Fletchling R25, Yamper R5, Shinx R6, Nickit R9,
  Pawmi R10, Rolycoly Túnel Roca, Purrloin R8, Fidough R7, Tandemaus R11, Smoliv R12, Buneary R13,
  Deerling Safari, Nymble R14, Tinkatink R15, Noctowl R16, Wingull R18, Sandygast R21, Litleo Ruta
  Kindle, Petilil Bosque Baya, Riolu R23, Electrike Pradera, Zorua Ruta Acuática, Axew Cañón Sevault),
  solo zonas desbloqueadas por medalla; 50 % de los encuentros de ese mapa; dura 1 día. Sin movimientos
  fijos (usa los de nivel).

### Pokémon de todas las generaciones (HECHO, compila; commit "every Pokémon catchable…")
- `OW_TIME_OF_DAY_ENCOUNTERS TRUE`. Las tablas originales de FRLG siguen siendo las de **mañana y día**
  (son el fallback `OW_TIME_OF_DAY_FALLBACK`); el generador **`tools/rojo_fuego/encounters.py`** añade
  tablas `_Evening` y `_Night` (sufijo del `base_label`) a cada tabla de tierra y agua FRLG (FireRed y
  copia idéntica para LeafGreen). Ejecutar: `wsl -d Ubuntu -- bash /mnt/c/Users/Fabian-Pc/decomps/wsl-encounters.sh`
  (regenera desde cero; semilla fija 1996).
- 377 especies colocadas (todas al menos una vez): formas base Gen 2-9, formas regionales base, y las
  de Gen 1 que no eran salvajes en FireRed (iniciales, Eevee, fósiles, Lapras, Porygon…). Excluidos
  legendarios/singulares/ultraentes/paradoja/megas/gigamax. Hábitat por tipo (cueva, bosque, hielo,
  torre fantasma, central eléctrica, mansión, volcán, safari, rocoso, agua); de noche tipos Siniestro/
  Fantasma/Psíquico/Hada/Veneno, de tarde el resto. Nivel mínimo de zona: pseudolegendarios y Slakoth
  40, BST base ≥450 → 30, iniciales/líneas fuertes de 3 etapas → 20, BST ≥380 → 18. Niveles de cada
  slot copiados de la tabla original. Tanoby y Cueva Cambiante no se tocan.
- Pesca y golpe roca siguen con tablas originales a toda hora.
- ROM al 80,6 % (≈6,3 MB libres), EWRAM 87 %, IWRAM 87 %.

### Tiendas, Entrenamiento extremo, MOs y correr (HECHO, compila)
- Centro Comercial Azulona 4F: todos los objetos evolutivos (piedras de todas las gen, Cordón Unión,
  objetos de intercambio, manzanas, teteras, Galanuez, Armaduras, etc.). 5F (vitaminas): Caramelos EXP
  XS-XL, Cápsula y Parche Habilidad, Chapa Plateada/Dorada, las 21 mentas.
- Marinero de 5F = **Entrenamiento extremo** (`RF_EventScript_HyperTrainer` en
  `data/scripts/rojo_fuego_npcs.inc`; reasignación en `content.py` `NPC_SCRIPTS`; C:
  `Script_HyperTrainChosenMon` en `script_pokemon_util.c`): Chapa Plateada = 1 IV, Dorada = todos.
- Claves nuevas: **MOs sin enseñar** (`KEY_HM_FREE`: medalla + MO en mochila; `ScrCmd_checkfieldmove`
  en `scrcmd.c` y `PartyHasMonWithSurf` en `field_player_avatar.c`) y **Correr siempre**
  (`KEY_AUTORUN`, B camina; durante búsqueda DexNav se camina).
- **Bug mío arreglado**: `FLAG_DOWSING` significa "Zahorí en uso" y bloquea correr; ya no se activa en
  partida nueva. El Buscaobjetos de FRLG activa el Zahorí. **Partidas creadas antes no pueden correr:
  empezar partida nueva.**
- Ya existían en expansion (no requieren trabajo): correr en interiores (`OW_RUNNING_INDOORS`), mover
  varios Pokémon en el PC (modo Mover + SELECT, PC de Esmeralda), captura crítica, Lazo Destino/
  herencias modernas, afecto, cambio al capturar, repetir última ball, descripción de movimientos.

### Botiquín y Enlace PC (HECHO, compila)
- `ITEM_POKE_VIAL` (880, "Botiquín"): cura al equipo, 3 usos (`POKE_VIAL_MAX_CHARGES`,
  `pokeVialCharges` en SaveBlock3), se recarga en `HealPlayerParty`. Uso en `item_use.c`.
- Oak entrega Omni Ring + Botiquín + Enlace PC (`ITEM_POKEMON_BOX_LINK`, ya existía) con la Pokédex.

### Tras reanudar (2026-10-22, HECHO, compila)
- Claves nuevas: **Autoguardado** (`KEY_AUTOSAVE`; `Script_TryAutosave` tras curar, en
  `data/scripts/pkmn_center_nurse.inc`) y **Combates rápidos** (`KEY_BATTLE_SPEED`, **apagada por
  defecto**; `BattleMainCB2` ejecuta un segundo paso de lógica/animación por frame sin leer botones;
  nunca en combates link/grabados). Soporte de claves apagadas por defecto:
  `KeySystem_IsOnByDefault` (el bit de `keySystemOff` = "cambiada respecto a su defecto").
- **Premios de la Pokédex**: Ayudante 3 del laboratorio de Oak (`RF_EventScript_DexRewards`,
  `Script_GetNextDexReward`): 50 Caramelo raro×3, 100 Amuleto Oval, 150 Chapa Dorada, 200 Amuleto
  Captura, 250 Parche Habilidad, 300 Amuleto EXP, 400 Master Ball, 500 Amuleto Iris (capturados en la
  Nacional). Flags `FLAG_RF_DEX_REWARD_0..7` = `FLAG_0x0C2..0x0C9`.
- **Intercambio Prodigio**: Pokéfan de Isla Dos (`RF_EventScript_WonderTrade`, `Script_DoWonderTrade`):
  1/día (`FLAG_RF_DAILY_WONDER_TRADE` = `FLAG_0x390`, primer flag diario FRLG), especie aleatoria no
  legendaria (forma base o regional), mismo nivel, OT = jugador.
- **Revanchas de líderes** (excepto Giovanni, que deja el gimnasio): tras ser Campeón el líder ofrece
  revancha (`rf_leader_rematch` en `rojo_fuego_npcs.inc`; entrenadores `TRAINER_RF_REMATCH_*` 624-630
  en `opponents_frlg.h`, `TRAINERS_COUNT_FRLG` 631; equipos en `src/data/trainers_frlg.party`, nivel
  70-77, IV 31, IA `Smart Trainer`). Se puede repetir (`cleartrainerflag`).
- Nota técnica: en macros de ensamblador no usar `etiqueta::` (el preprocesador lo traduce antes);
  usar `.global X` + `X\():`.
- Flags de contenido libres ahora: **`FLAG_0x0CA`–`FLAG_0x153`**.

### Sistema de logros (HECHO, compila; commits `d576c9b0`, `e8c718cc`)
- **Lista**: `tools/rojo_fuego/achievements.py` (318 logros, 12 categorías, rangos B/S/G/P = 10/25/
  50/100 pts). Ejecutarlo (WSL, raíz del repo) regenera `include/constants/rf_achievements.h`,
  `src/data/rf_achievements.h`, `tools/rojo_fuego/achievements.json` (para la página) y
  `tools/rojo_fuego/retroachievements_local.txt` (necesita haber compilado: lee `pokefirered.map`).
  **Al añadir contadores o cambiar la lista: regenerar y recompilar.**
- Tipos de condición: flag, especies capturadas/vistas, conjunto de especies, estadística de juego
  (`GAME_STAT_*`), contador propio (`ACH_COUNTER_*`, 43), dinero, nivel de equipo, objetos (mochila o PC),
  especies capturadas por tipo, especiales (21, `GetSpecialValue` en `src/achievements.c`), evento
  (solo por enganche), % de logros, todos de un rango, todos los ocultos.
- Motor `src/achievements.c`: revisión cada 120 frames en el mapa (`Achievements_OverworldUpdate`
  desde `CB1_Overworld`), aviso arriba 180 frames (ventana BG0 baseBlock 0x100, se oculta si hay menú o
  script; `Achievements_HideNotification` también desde el Key System), menú con el objeto clave
  **`ITEM_ACHIEVEMENT_LOG` (881, "Logros")** que da Oak (lista de categorías → logros con estado
  HECHO/¡PREMIO!/progreso; A reclama; títulos solo como texto). Guardado en SaveBlock3
  (`achievementsUnlocked/Claimed`, `achievementCounters`, `achievementBallKinds`, `achievementGimmicks`).
- Enganches: fin de combate (`battle_setup.c` 3 CB2 → `Achievements_OnBattleEnd`: victorias, rachas,
  huidas, capturas shiny/primera ball/noche/legendario/safari/pesca/aparición/DexNav, tipos de ball,
  debilitados, victoria al límite, debilitados en la Liga desde Lorelei), Salón de la Fama
  (`save.c` antes de guardar → `Achievements_OnHallOfFame`: retos R01-R12 y B34), superefectivos y
  críticos (`Cmd_resultmessage`/`Cmd_critmessage`), estados (`SetNonVolatileStatus`), captura crítica
  (`FinalizeCapture`), Mega/Z/Tera/Dinamax (`Achievements_OnGimmickUsed`), Caramelos raros/EXP,
  vitaminas, mentas, cápsulas (`party_menu.c`), liberar (`ReleaseMon`), chocar con paredes, Botiquín,
  Entrenamiento extremo, Intercambio Prodigio (+shiny, +legendario: 1 % de los cambios), claves.
- Rendimiento: el recuento por tipo recorre la tabla de especies una vez (no usar
  `NationalPokedexNumToSpecies` en bucles: es búsqueda lineal).
- **RetroAchievements**: copia de los bits en `gRfAchievementMirror` (EWRAM fija, último byte = 0xA5
  cuando está sincronizada). rcheevos mapea EWRAM 0x02000000 → 0x008000. Cada logro: el byte de sincronía
  ya era 0xA5 el frame anterior y su bit pasa de 0 a 1. Archivo local para RALibretro/RAIntegration
  (copiarlo como `<id del juego>-User.txt`); publicarlo en la web requiere aprobación de RA.
- Página de logros actualizada con la lista real: https://claude.ai/artifact/Mzqdgn5o47nQYBMZuJfCBQ

### Más mejoras tras reanudar (HECHO, compila)
- **Registrar hasta 4 objetos en Select** (`src/registered_items.c`, `include/registered_items.h`): ranura 0
  = `registeredItem` original, 3 extra en SaveBlock3 (`extraRegisteredItems`). Con varios, Select abre
  una lista para elegir. Bolsa: `IsItemRegistered`/`ToggleRegisteredItem` en `item_menu.c`.
- **Objetos ocultos brillan** (`KEY_HIDDEN_SPARKLE`, `KeySystem_UpdateHiddenItemSparkle` en
  `key_system.c`, llamado desde `CB1_Overworld`): cada 100 frames uno al azar dentro de 7×5 casillas
  con `FLDEFF_SPARKLE`.
- Verificado que ya venían: Sincronía 100 % (`OW_SYNCHRONIZE_NATURE`), evolución tras el combate,
  icono de capturado en la barra de PS (vanilla).
- **Cambios de estadísticas en combate**: con la descripción de un movimiento abierta (L), SELECT alterna
  a "TÚ: At+2 … / RIVAL: …" (`MoveSelectionDisplayStatStages` en `battle_controller_player.c`).
- **Títulos en la Ficha de Entrenador**: reclamar un título lo equipa (A sobre uno ya reclamado lo vuelve
  a equipar); se muestra en el anverso de la ficha propia, en el hueco de la frase de perfil
  (`achievementTitle` en SaveBlock3, `Achievements_GetEquippedTitle`, `trainer_card.c`).
- **Revisión de solapes de VRAM (BG0)**: aviso de logros movido a baseBlock 0x180 y oculto mientras se ve el
  cartel del mapa (0x107) o hay búsqueda DexNav; el panel de detalles del DexNav (100-380) se cierra al
  abrir el menú Start (0x139+). Mapa de baseBlocks BG0 en el campo: búsqueda DexNav 8-92, panel DexNav
  100-380, cartel de mapa 0x107-0x124, menú Start 0x139+, aviso 0x180-0x1B7, marcos 0x200+; Key System
  1-449, menú de logros 1-448 y lista de Select 1-88 solo con controles bloqueados.
- **Arreglo (reportado por el usuario: "se ganan pero no se ve el texto")**: `DrawStdWindowFrame` y
  `DrawStdFrameWithCustomTileAndPalette` rellenan la ventana, así que llamarlas después de imprimir borraba
  el texto. Afectaba al aviso de logros y al panel de detalles del DexNav. Ahora el marco va antes del
  texto. El aviso pasó a baseBlock 0x2A0 (la caja de mensajes FRLG usa 0x194-0x1FF y el preview de mapa
  0x259-0x284). Verificado con capturas en el emulador.
- **Pruebas automáticas sin ventana**: `tools/rojo_fuego/test/harness.c` (libmgba, `apt install
  libmgba-dev` en WSL; `gcc -O2 -o harness harness.c -lmgba`). Script con `run/hold/mash/shot/w8/r8/dump/
  save/load`; `ppm2png.py` convierte las capturas. Para direcciones de variables estáticas:
  `arm-none-eabi-nm pokefirered.elf`. Los savestates solo sirven con la misma ROM.
- **Pantallas al estilo Pokémon SoulGold** (pedido del usuario, referencia
  https://pokemonheartsoul.com/pokemon-soulgold/; SoulGold es un hack GBA sobre pokeemerald-expansion sin
  código público, así que se copió el diseño de su pantalla de Trofeos a partir de capturas):
  - **Logros a pantalla completa** (`src/achievements.c`, sección "Achievements screen"): cabecera con
    categoría (L/R o ←/→) y conteo, 5 filas con trofeo del color del nivel (bronce/plata/oro/platino,
    silueta apagada si está bloqueado), nombre y estado ("Bloq.", progreso x/y o %, "¡Premio!", "Hecho"),
    flechas de scroll y caja inferior con descripción y premio. A reclama/equipa título, B sale.
    Gráficos generados por `tools/rojo_fuego/achievements_gfx.py` → `src/data/rf_achievements_gfx.h`
    (fondo BG1 + paleta de texto + íconos 16x16). Se abre desde la mochila (CB2 directo) o con Select
    (fundido desde el campo); vuelve con `FieldCB_ReturnToFieldNoScript`.
  - **DexNav en español**: la pantalla ya existía (menú Start → DEXNAV, solo en mapas con salvajes). Los
    títulos dibujados se repintaron con `tools/rojo_fuego/dexnav_gui.py build` (reconstruye la pantalla de
    tiles+tilemap, borra los títulos, dibuja AGUA/TIERRA/OCULTOS/ESPECIE/TIPO/NV. BÚSQUEDA/HAB. OCULTA/
    CADENA ACTUAL/BUSCAR con una fuente de píxeles y vuelve a cortar en tiles). Originales en
    `gui_tiles_en.png`/`gui_tilemap_en.bin`. Textos y mensajes de búsqueda traducidos.
  - Elegir DEXNAV en un mapa sin salvajes muestra "No hay Pokémon aquí." (antes no hacía nada).
  - **Contador de capturados por zona** (como SoulGold): "capturados/total" en las pestañas AGUA,
    TIERRA y OCULTOS. Se dibuja directo en tiles de BG1 que `dexnav_gui.py` deja únicos (cabecera
    generada `src/data/dexnav_counter_tiles.h`), con la fuente de píxeles de los títulos
    (`DrawAreaCounters` en `dexnav.c`). Sin especies en la zona no se dibuja nada.
- **No hecho**: traducción del juego al español (no pedida; proyecto grande).

### GitHub (HECHO 2026-10-22)
- Repo **privado**: https://github.com/fabdroguett/rojo-fuego-plus (rama por defecto `rojo-fuego`,
  remoto local `rojofuego`). El clon era `--depth 1` y GitHub rechaza historiales truncados: los commits
  se pusieron sobre una raíz nueva con el árbol de expansion `f4dc700a` (`git commit-tree` +
  `git rebase --onto`). Subir cambios: `git push rojofuego rojo-fuego`.
- La ROM no está en el repo (`.gba` ignorado). Los únicos `.gba` versionados son las imágenes multiboot
  de upstream (`data/mb_*.gba`), que ya son públicas en expansion.
- ROM actual copiada a `C:\Users\Fabian-Pc\Desktop\Rojo Fuego Plus.gba`.

### ESTADO ACTUAL (2026-10-22)
- Código en `decomps\pokeemerald-expansion`, rama `rojo-fuego`, subido al repo privado (ver GitHub).
  ROM en `C:\Users\Fabian-Pc\Desktop\Rojo Fuego Plus.gba`. Compila con 0 errores y 0 avisos.
- **Nada probado dentro del juego todavía**: lo siguiente es que el usuario juegue y reporte fallos
  (partida NUEVA obligatoria: el formato de SaveBlock3 cambió varias veces).
- Pendiente opcional: ver cambios de estadísticas en combate, títulos en la Ficha de Entrenador,
  traducción al español del resto del juego, ROM final para Android (mGBA APK).

### Implementación — Paso 1: activar mejoras (empezado 2026-10-22)
- **Tanda A HECHA, compila (`EXIT=0`, 0 warnings)**. Cambios (ver `git status` en el repo; sin commits):
  - Flags con nombre en `include/constants/flags.h` (slots libres en FRLG y Emerald):
    `FLAG_EXP_ALL`=0x4A7, `FLAG_DEXNAV_GET`=0x4A8, `FLAG_DEXNAV_SEARCHING`=0x4A9,
    `FLAG_DEXNAV_DETECTOR_MODE`=0x4AA, `FLAG_POKE_RIDER`=0x4AB, `FLAG_DOWSING`=0x4AC.
    **Quedan libres 0x4AD-0x4AF (3 flags)**: el Key System debe guardar sus claves en un bloque
    propio, no en flags.
  - Vars en `include/constants/vars.h`: `VAR_DEXNAV_SPECIES` / `VAR_DEXNAV_STEP_COUNTER`
    (FRLG: 0x4029/0x402A; Emerald: 0x4083/0x408B). Quedan libres en FRLG: 0x402B, 0x402D-F,
    0x4034, 0x403F, 0x4041.
  - Config: Exp. All por flag, MTs reutilizables, Zahorí, DexNav (+modo detector), followers,
    volar desde el mapa, olvidar MOs, IV/EV en el resumen, recordadores de movimientos, Pokédex
    HGSS, encuentros visibles (+brillo shiny), tipos en combate (vistos), límite de nivel por
    medallas.
  - `src/caps.c`: niveles de límite de Kanto (14/21/24/29/43/43/47/50/63) bajo `IS_FRLG`.
  - `src/new_game.c`: partida nueva activa Exp. All, DexNav, detector, volar desde mapa, Zahorí.
  - **El límite de nivel queda siempre activo hasta que exista el Key System** (avisado al usuario).
- **Pendiente del paso 1 (requiere contenido o código, no solo config)**: repartir megapiedras,
  Mega Aro, Tera Orbe, Pulsera Z/cristales, Dinamax; dificultad (equipos por nivel); apariciones
  masivas en FRLG; día/noche con RTC en FRLG (verificar); y las mejoras "hay que programarlas"
  (registrar varios objetos, mover varios en cajas, PC en cualquier lugar, objetos ocultos
  visibles, MOs sin enseñar, correr con R, autoguardado, curación portátil, revanchas, premios
  Oak, combates acelerados, tutor en el equipo, tienda evolutiva, Intercambio Prodigio).
- [ ] Usuario prueba la tanda A en mGBA (partida NUEVA).

### Implementación — Paso 2: Key System v1 (2026-10-22) — compila, 0 warnings, sin probar en juego
- Menú **CLAVES** en el menú Start, en el lugar de SALIR (con 9 opciones la ventana se sale de la
  pantalla: alto = 2·n+2 desde la fila 1; B/Start siguen cerrando). Lista con SÍ/NO a la derecha y
  ventana de descripción abajo; A cambia la clave, B sale.
- Archivos: `include/constants/key_system.h` (enum de claves), `include/key_system.h`,
  `src/key_system.c` (lógica + menú; ventanas BG0 baseBlock 1 y 337, por debajo de 0x200).
- Guardado: `u32 keySystemOff` en `struct SaveBlock3` (`include/global.h`); bit a 1 = clave apagada,
  así una partida vieja con ceros tiene todo encendido. Exp. All sigue usando `FLAG_EXP_ALL`.
- Enganche de combate: `include/config_changes.h` → `KeySystem_ConfigOverride(tag, valor)` dentro de
  los getters `GetConfig_*` (inline, se pliega en las opciones sin clave). `src/battle_util.c` usaba
  `B_PHYSICAL_SPECIAL_SPLIT` directo → cambiado a `GetConfig`.
- Claves v1 (todas encendidas por defecto) y su valor al apagarlas: EXP al capturar (`B_EXP_CATCH`→
  GEN_5), Exp. All (flag), EXP según nivel (`B_SCALED_EXP`→GEN_6), Límite de nivel (`caps.c` devuelve
  MAX_LEVEL), Físico/especial (→GEN_3), Críticos (`B_CRIT_CHANCE`/`MULTIPLIER`→GEN_5), Estados
  (`B_PARALYSIS_SPEED`/`B_BURN_DAMAGE`→GEN_6, `B_SLEEP_TURNS`→GEN_3), Clima 5 turnos
  (`B_ABILITY_WEATHER`→GEN_5), Pokémon que te sigue (`event_object_movement.c`), Salvajes visibles
  (`wild_encounter_ow.c`; además `field_control_avatar.c`: con la clave encendida NO hay encuentros
  aleatorios, estilo Let's Go; apagada vuelven).
- Pendiente: más claves (dificultad, combates acelerados, etc. según se implementen) y el texto
  del resto del juego sigue en inglés (las claves están en español).

### Implementación — Paso 3: DexNav mejorado, parte 1 (2026-10-22) — compila, sin probar en juego
- `src/dexnav.c`: `DexNavRollIndividual()` en `SetUpDexNavSearch` decide personalidad (naturaleza/
  sexo vía `GetSynchronizedGender/Nature`), IVs (aleatorios + `min(3, potencial)` a 31) y shiny
  (`ComputePlayerShinyOdds` con `gDexNavSpecies` puesto un momento para que cuente la cadena).
  `CreateDexNavWildMon` crea exactamente ese individuo (`CreateMonWithIVs` con esa personalidad,
  copia IVs, fuerza `MON_DATA_IS_SHINY`).
- Panel de detalles con **SELECT durante la búsqueda** (`DexNav_TryToggleDetails`, enganchado en
  `field_control_avatar.c` antes del objeto registrado): variocolor sí/no + probabilidad ~1/N
  (amuleto, cebo, cadena 50/100), naturaleza, sexo, tipo de Poder Oculto, habilidad (marca oculta),
  IVs (31 en color), 4 movimientos. Ventana BG0 baseBlock 100, 28×10, en la mitad opuesta a la
  ventana de búsqueda; se cierra sola al acabar la búsqueda.
- `include/constants/pokemon.h`: `SHINY_ODDS` 8 → 16 (1/4096, mejora "shiny-4096" elegida).
- Extras del DexNav ya cubiertos por expansion: flecha de dirección (`DexNavUpdateDirectionArrow`),
  R repite la búsqueda de la especie registrada (cubre el "atajo" #10).
- **Parte 2 HECHA (compila, 0 warnings; commit `36e09745` en rama local `rojo-fuego`)**:
  - Panel: SELECT cicla cerrado → pág. 1 (individuo) → pág. 2 (tipos, % de aparición, stats base,
    EVs que da, ratio de captura, captura aprox. Poké/Super/Ultra a PS llenos/bajos con fórmula Gen 3
    a≈rate·ball·(3M−2H)/3M, P≈a/255; tu mejor suma de IV de la especie en equipo+cajas vs este; nivel
    de búsqueda) → cerrado. (#6, #7, #8 hechos)
  - Modo cazador (#2): en la pantalla del DexNav SELECT cambia el filtro (Todo/Shiny/H.Oculta/3 estr.)
    guardado en `VAR_DEXNAV_FILTER` (FRLG 0x402B). Si el individuo no cumple: termina la búsqueda sin
    romper la cadena y muestra `EventScript_DexNavFilterNoMatch` (`data/scripts/dexnav.inc`).
  - % de aparición en la pantalla del DexNav (#4 parcial: hierba/agua; pesca y golpe roca no) vía nueva
    `GetCurrentMapWildMonEncounterPercent` en `src/wild_encounter.c`.
  - Aviso "¡Legendario errante!" en la barra superior del DexNav si el errante está en el mapa (#5;
    apariciones masivas pendientes hasta que existan en FRLG).
  - Alerta shiny (#3): `SE_SHINY` cuando el detector encuentra un oculto variocolor; además
    `WE_OWE_PREVENT_SHINY_DESPAWN TRUE` (los visibles shiny ya brillan y suenan).
  - **Bug de expansion arreglado**: `TryFindHiddenPokemon` salía si `sDexNavSearchDataPtr == NULL`, así
    que el modo detector nunca funcionaba; ahora `!= NULL`.
  - Historial (#11): `USE_DEXNAV_SEARCH_LEVELS TRUE`, array en SaveBlock3 indexado por Nº Nacional
    (`NATIONAL_DEX_COUNT + 1` bytes, formas comparten). "Mejor encontrado" se cubre con la
    comparación de IV de la pág. 2 (no se guarda aparte por espacio).
- **SaveBlock3 ocupado**: ~1025 (niveles DexNav) + 1 (cadena) + 4 (claves) de 1624 bytes. Los logros
  deben ir en SaveBlock1/2 liberando espacio con `include/config/save.h` (`FREE_*`).
  - Pendiente aún: respuesta del usuario sobre qué le falta al DexNav original.
  - **Mapa selección → expansion** (`include/config/*.h`, la mayoría ya en `GEN_LATEST`):
    - Capture EXP = `B_EXP_CATCH` (battle.h). Exp. All = `I_EXP_SHARE_FLAG` / `I_EXP_SHARE_ITEM`
      (item.h, el flag permite encender/apagar en partida → candidato a clave).
    - Ya activos por defecto: físico/especial (`B_PHYSICAL_SPECIAL_SPLIT`), críticos
      (`B_CRIT_MULTIPLIER`), parálisis/sueño, clima 5 turnos (`B_ABILITY_WEATHER`), movimientos y
      stats/tipos modernos (`B_UPDATED_MOVE_DATA`, `P_UPDATED_STATS`, `P_UPDATED_TYPES`), EXP escalada
      (`B_SCALED_EXP`), cambiar al capturar (`B_CATCH_SWAP_INTO_PARTY`), última ball
      (`B_LAST_USED_BALL`), descripción de movimientos (`B_MOVE_DESCRIPTION_BUTTON`), ciclo día/noche
      (`OW_ENABLE_DNS`), Amuleto Iris (`I_SHINY_CHARM_ADDITIONAL_ROLLS`).
    - Desactivados, hay que encenderlos: DexNav (`DEXNAV_ENABLED`, dexnav.h; base para el "DexNav
      mejorado"), followers (`OW_FOLLOWERS_ENABLED`, overworld.h), eficacia/tipos en combate
      (`B_SHOW_EFFECTIVENESS`, `B_SHOW_TYPES`).
    - Especies Gen 1-9: `include/config/species_enabled.h` (`P_FAMILY_*` / `P_GEN_x_POKEMON`).
    - Por revisar uno a uno: captura crítica, fórmula de captura, Sincronía, icono de capturado,
      habilidades ocultas, caramelos EXP, mentas, cápsula, entrenamiento extremo, IV/EV en
      resumen, MOs, PC en cualquier lugar, objetos ocultos visibles, salvajes visibles, crianza.
- FRLG-Plus ya trae (FEATURES.md): **Exp. Modifier** (0×, ½×, 1×, 2×) y animación instantánea
  de la barra de EXP. **No** trae Capture EXP ni Exp. All → hay que implementarlos (Fase 2).
- Construir con:
      `wsl -d Ubuntu -- bash /mnt/c/Users/Fabian-Pc/decomps/wsl-build.sh` (log en
      `decomps/build.log`, última línea `EXIT=<rc>`). No usar `$?` vía `cmd /c`: se escapa mal.
  - Fix aplicado: `tools/mapjson/json11.cpp` le faltaba `#include <cstdint>` (GCC 13+);
    mismo fix que pret upstream.
- [x] mGBA 0.10.5 instalado en Windows (`winget install JeffreyPfau.mGBA`; el id `mGBA.mGBA` no existe).
- [ ] Decidir alcance exacto de cada feature antes de codificar.
- [ ] APK: por ahora solo ruta emulador; port nativo queda pendiente de decisión.
- [ ] Nunca hacer commit/push ni distribuir ROMs; repos locales nada más (si el usuario pide).

## Referencias clave

- https://github.com/pret/pokefirered (INSTALL.md es la fuente de verdad del setup)
- https://github.com/pret/agbcc
- https://github.com/mstan/FireRedLeafGreenRecomp (recomp, NO sirve para editar features)
- https://github.com/gradenGnostic/pokeemerald-multiplatform (precedente de port SDL multiplataforma)
- https://github.com/morluto/rea (análisis puntual con Ghidra si hace falta)
