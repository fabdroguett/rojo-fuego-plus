# Arquitectura incremental para un port

## Situación

La base es una decompilación C ARM7TDMI orientada a GBA, no una biblioteca portable. Macros, registros, VRAM, DMA, callbacks, audio y temporización aparecen entrelazados con gameplay. Una migración debe empezar con medición y extracción progresiva, sin reescritura integral.

## Límites propuestos

```text
Datos y reglas de juego
        │ API de comandos/consultas determinista
        ├── Backend GBA: tareas, memoria, entrada, render, audio, save
        └── Backend moderno: reloj, input, renderer, audio, archivos/save
```

1. Congelar fixtures de datos y casos de combate/progresión reproducibles.
2. Identificar dependencias por módulo con referencias a `REG_DISPCNT`, `REG_VCOUNT`, DMA, `gMain`, callbacks, `PlaySE`/mixer y funciones de save.
3. Elegir un módulo de bajo riesgo (por ejemplo, validación de datos o lógica pura) para una biblioteca de compatibilidad C, sin cambiar el binario GBA.
4. Definir reloj/input/audio/render/save como interfaces explícitas cuando exista un consumidor real; el backend GBA conserva semántica actual.
5. Comparar resultados de la implementación original con la extracción usando fixtures y serialización canónica.
6. Portar una pantalla/escena vertical slice y comparar datos de guardado antes de ampliar cobertura.

## Clasificación

| Sistema | Dependencia | Diseño para futura extracción |
|---|---|---|
| Reglas de combate/progresión | Mixta | separar cálculos deterministas de animación/callbacks; fijar RNG y estado |
| Datos de especies/movimientos/objetos | Baja/media | exportación versionada, IDs estables y validador de referencias |
| Mapas/eventos | Media | conservar formato lógico; resolver coordenadas, scripts y referencias con loader propio |
| Presentación | Muy alta | backend de tiles GBA frente a renderer moderno con escala pixel-perfect |
| Audio | Alta | API de eventos/streams y backend que reproduce secuencias existentes |
| Entrada | Media/alta | traducir botones a acciones; preservar repetición/orden de frame |
| Temporización | Muy alta | reloj/tick explícito y simulación con paso fijo |
| Memoria/VRAM/DMA | Muy alta | aislar recursos y operaciones en adaptador GBA; no emular registros en lógica portable |
| Guardado | Alta | especificar versión, endianess, checksums, claves y migraciones con fixtures |

## Riesgos y puerta de decisión

Compatibilidad legal/de recursos, desincronización de RNG/frame, diferencias de scripts, formato de save y coste de mantener dos runtimes. Antes de implementar un port, demostrar un prototipo de una escena y un caso de combate/guardado; medir velocidad, memoria, legibilidad y fidelidad. No se inicia hasta que el análisis de viabilidad y alcance esté aceptado.
