---
name: pokemon-battle-engine
description: "Combate, movimientos, estados, habilidades e IA. Activar para cambiar reglas de batalla."
---

# pokemon-battle-engine

## Descripción y criterios de activación
Combate, movimientos, estados, habilidades e IA. Activar para cambiar reglas de batalla.

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
2. Inspeccionar: src/battle_*.c; src/battle_script_commands.c; src/data/battle_moves.h.
3. Aplicar el cambio acotado teniendo en cuenta: cambiar reglas de batalla.
4. Ejecutar `make BUILD=firered -j2; make check BUILD=firered si disponible` y anotar salida, evidencia y PASS/FAIL/SKIPPED/BLOCKED en `docs/development/TESTING.md`.
5. Revisar diff y criterios de aceptación.

## Archivos y módulos relevantes
src/battle_*.c; src/battle_script_commands.c; src/data/battle_moves.h

## Pruebas obligatorias
`make BUILD=firered -j2; make check BUILD=firered si disponible`. Añadir checks del comportamiento modificado.

## Criterios de aceptación
casos normales y bordes verificados. No afirmar PASS sin evidencia ejecutada.

## Prevención de regresiones
Mantener IDs y datos estables; medir EWRAM/IWRAM/ROM en cambios globales. Para cambios de guardado, consultar también `gba-save-compatibility`. Preservar artefactos y ediciones preexistentes.

## Referencias del proyecto
`docs/development/00-project-audit.md`, `AGENT_WORKFLOW.md`, `ROADMAP.md`, `TESTING.md` y rutas de dominio listadas arriba.
