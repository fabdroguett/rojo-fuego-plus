# Continuidad de ejecución

Actualización: 23-10-2026. Sesión cerrada a petición del usuario para continuar después.
Al retomar, ejecutar el siguiente paso de este archivo dentro del alcance de
[PLAN_COMPLETO.md](PLAN_COMPLETO.md); el plan completo sigue abierto.

## Punto guardado para retomar

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

1. **ARTE-2: siguiente trabajo al retomar.** Mochila de seis cuadros 64×64 a partir de los
   conceptos de `menu_art_v2`, luego ARTE-3 (Pokédex HGSS) y ARTE-4/5. Para nuevos iconos,
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
