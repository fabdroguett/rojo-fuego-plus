---
name: pokemon-world-builder
description: "Mapas, tilesets, NPC, colisiones y progresión. Activar para editar mundo o conexión."
---

# pokemon-world-builder

## Descripción y criterios de activación
Mapas, tilesets, NPC, colisiones y progresión. Activar para editar mundo o conexión.

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
2. Inspeccionar: data/maps/; data/layouts/; data/tilesets/.
3. Aplicar el cambio acotado teniendo en cuenta: editar mundo o conexión.
4. Ejecutar `make BUILD=firered -j2; revisar mapa en mGBA` y anotar salida, evidencia y PASS/FAIL/SKIPPED/BLOCKED en `docs/development/TESTING.md`.
5. Revisar diff y criterios de aceptación.

## Archivos y módulos relevantes
data/maps/; data/layouts/; data/tilesets/

## Pruebas obligatorias
`make BUILD=firered -j2; revisar mapa en mGBA`. Añadir checks del comportamiento modificado.

## Criterios de aceptación
mapa compila y progresión/colisión comprobadas. No afirmar PASS sin evidencia ejecutada.

## Prevención de regresiones
Mantener IDs y datos estables; medir EWRAM/IWRAM/ROM en cambios globales. Para cambios de guardado, consultar también `gba-save-compatibility`. Preservar artefactos y ediciones preexistentes.

## Referencias del proyecto
`docs/development/00-project-audit.md`, `AGENT_WORKFLOW.md`, `ROADMAP.md`, `TESTING.md` y rutas de dominio listadas arriba.
