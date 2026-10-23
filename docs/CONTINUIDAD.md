# Continuidad de ejecución

Actualización: 23-10-2026. Sesión cerrada a petición del usuario para continuar después.
Al retomar, ejecutar el siguiente paso de este archivo dentro del alcance de
[PLAN_COMPLETO.md](PLAN_COMPLETO.md); el plan completo sigue abierto.

## Punto guardado para retomar

Última implementación completada: selector de iniciales de generaciones 1–9, extras como
Beldum/Riolu/Gible/Axew y más a criterio, con habilidad oculta. Implementación en
`src/rf_starter_menu.c`: 54 opciones/18 grupos; integración con Oak conservando rival Kanto.
Catálogo y controles: [INICIALES.md](INICIALES.md). Investigación ampliada de 28 fuentes:
[ASSETS_INTERNET.md](ASSETS_INTERNET.md). Validado: 54 elecciones en mGBA, cancelación, 18 regresiones y arranque de producción.
ROM actualizada en el Escritorio, SHA-256
`f79fc39f164350b6167dcae79d1dcd8676a9ce5876470e15b4ec16f3f267f82f`.
Entrega local: `build_artifacts/releases/20261023-114921/`, sin fixtures.
Memoria: EWRAM 230376 B, IWRAM 28648 B, ROM enlazada 27154056 B.
Código y documentación sincronizados con `origin/rojo-fuego`, commit `7a96f850`.
La actualización posterior de estos documentos solo cambia el plan, no la ROM.
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

1. **ARTE-1: siguiente trabajo al retomar.** Partir del catálogo de Internet
   [ASSETS_INTERNET.md](ASSETS_INTERNET.md): seleccionar una familia coherente de iconos
   16×16 para mochila, Pokédex, equipo, misiones, logros, claves, guardar, mapa y objetos
   registrados. Revisar el archivo y licencia concretos, guardar procedencia/créditos y
   adaptar al formato nativo de `src/data/rf_menu_icons.h`. Los conceptos propios son
   referencias opcionales; no confundirlos con sprites integrados. Validar a 1×/2×,
   claro/oscuro, navegación y retorno al mapa antes de actualizar la ROM.
2. Seguir **ARTE-2 → ARTE-3 → ARTE-4/5**: mochila de seis cuadros 64×64, Pokédex HGSS
   y extensión visual a las demás superficies, con verificación de cada integración.
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
