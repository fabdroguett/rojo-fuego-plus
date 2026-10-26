---
name: gba-automated-testing
description: "Tests Python/C, validadores, build y emulador. Activar para agregar código o validar regresiones."
---

# gba-automated-testing

## Descripción y criterios de activación
Tests Python/C, validadores, build y emulador. Activar para agregar código o validar regresiones.

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
2. Inspeccionar: tools/rojo_fuego/tests/; tools/rojo_fuego/test/; Makefile.
3. Aplicar el cambio acotado teniendo en cuenta: agregar código o validar regresiones.
4. Ejecutar `python3 tools/rojo_fuego/validate_quests.py; python3 -m unittest discover -s tools/rojo_fuego/tests -v; bash tools/rojo_fuego/verify_code.sh` y anotar salida, evidencia y PASS/FAIL/SKIPPED/BLOCKED en `docs/development/TESTING.md`.
5. Revisar diff y criterios de aceptación.

## Archivos y módulos relevantes
tools/rojo_fuego/tests/; tools/rojo_fuego/test/; Makefile

## Pruebas obligatorias
`python3 tools/rojo_fuego/validate_quests.py; python3 -m unittest discover -s tools/rojo_fuego/tests -v; bash tools/rojo_fuego/verify_code.sh`. Añadir checks del comportamiento modificado.

## Criterios de aceptación
estados y evidencia reales registrados. No afirmar PASS sin evidencia ejecutada.

## Prevención de regresiones
Mantener IDs y datos estables; medir EWRAM/IWRAM/ROM en cambios globales. Para cambios de guardado, consultar también `gba-save-compatibility`. Preservar artefactos y ediciones preexistentes.

## Referencias del proyecto
`docs/development/00-project-audit.md`, `AGENT_WORKFLOW.md`, `ROADMAP.md`, `TESTING.md` y rutas de dominio listadas arriba.
