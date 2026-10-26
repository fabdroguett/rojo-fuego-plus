---
name: pokemon-data-engine
description: "Especies, stats, tipos, evoluciones, movimientos y objetos. Activar para editar o validar catálogos."
---

# pokemon-data-engine

## Descripción y criterios de activación
Especies, stats, tipos, evoluciones, movimientos y objetos. Activar para editar o validar catálogos.

## Responsabilidades
- Inspeccionar la implementación y datos existentes antes de editar.
- Mantener compatibilidad con la decompilación en modo FireRed.
- Actualizar documentación, generadores y tests que dependan del cambio.

## Restricciones
- Preservar cambios preexistentes; nunca `git reset`, `clean` o checkout destructivo.
- No instalar paquetes ni cambiar configuración global.
- Confirmar símbolos y APIs en el árbol; no inferirlos.
- No reportar pruebas que no se ejecutaron.

## Herramientas permitidas
`rg`, Git de lectura, scripts locales, Python, PowerShell/WSL, Make y mGBA disponible. Ejecutar solo herramientas necesarias.

## Flujo de trabajo
1. Revisar `git status --short` y `docs/development/00-project-audit.md`.
2. Inspeccionar: src/data/; include/constants/.
3. Aplicar el cambio acotado teniendo en cuenta: editar o validar catálogos.
4. Ejecutar `make BUILD=firered -j2; validador pertinente` y anotar salida, evidencia y PASS/FAIL/SKIPPED/BLOCKED en `docs/development/TESTING.md`.
5. Revisar diff y criterios de aceptación.

## Archivos y módulos relevantes
src/data/; include/constants/

## Pruebas obligatorias
`make BUILD=firered -j2; validador pertinente`. Añadir checks del comportamiento modificado.

## Criterios de aceptación
IDs y referencias coherentes. No afirmar PASS sin evidencia ejecutada.

## Prevención de regresiones
Mantener IDs y datos estables; medir EWRAM/IWRAM/ROM en cambios globales. Para cambios de guardado, consultar también `gba-save-compatibility`. Preservar artefactos y ediciones preexistentes.

## Referencias del proyecto
`docs/development/00-project-audit.md`, `AGENT_WORKFLOW.md`, `ROADMAP.md`, `TESTING.md` y rutas de dominio listadas arriba.
