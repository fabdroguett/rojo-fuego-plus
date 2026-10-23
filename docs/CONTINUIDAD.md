# Continuidad de ejecución

Actualización: 23-10-2026. Instrucción vigente: continuar el plan completo sin detenerse por
tandas pequeñas. Alcance y dependencias: [PLAN_COMPLETO.md](PLAN_COMPLETO.md).

## Trabajo activo

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
- Enlace de producción limpio: EWRAM 230360 B, IWRAM 28648 B, ROM 27151776 B. Sin fixtures
  ni ROM generada; verificar de nuevo después de cambios posteriores.

## Entorno actual

Fuentes del host en `/source` (solo lectura), copia compilable en `/work`, contenedor
`rojo-fuego-check`. La última ROM **de prueba** y su fuente temporal fueron retiradas
de `/work`; solo quedó un ELF de producción. Nunca entregar las ROM de fixtures.
El harness mGBA está en `/tmp/rf-mgba-harness`. Artefactos ignorados en
`build_artifacts/progression/` y `build_artifacts/training/`; no se abren guardados del jugador.

Para reproducir: copiar las fuentes/herramientas actualizadas a `/work`, ejecutar allí
`bash tools/rojo_fuego/test/build_progression_fixture.sh`, y ejecutar desde Windows
`python tools/rojo_fuego/test/verify_progression.py` (también `--berries-off`). Regenerar siempre
símbolos/estados al cambiar el enlace. Ver también `test/UI_VALIDATION.md`.

## Siguiente acción concreta

1. Continuar SERVICIOS con preparación visible en Centros Pokémon (naturaleza/habilidades/
   crianza) y herramientas unificadas, reutilizando mentas y parches existentes.
2. Auditar laboratorio de formas/fusiones y zonas de entrenamiento; SAVE-2 sigue aislado hasta cerrar sectores
   y migración. No rehacer sistemas existentes.
3. Mantener validador, pruebas nativas y recorrido de emulador apropiados a cada cambio.
4. Actualizar este archivo y subir exclusivamente código/documentación en cada punto estable.

La fixture de revancha usa un NPC generado y estadísticas sintéticas altas para garantizar
una victoria: valida entrega del evento, no equilibrio del jefe. Los intentos iniciales
detectaron una invocación sin NPC y una derrota; no se atribuyen a fallos del juego.

Antes de una ROM de entrega retirar individualmente las fixtures de `/work/src`, reenlazar
con flags normales, comprobar ausencia de `RFProgress_`/`RFUiFixture_`, respaldar la ROM local
anterior y verificar cabecera/arranque. Nunca subir ROMs, `.sav`, `build/` o ramas de respaldo
que contengan binarios. Apagar Docker al cerrar el trabajo.
