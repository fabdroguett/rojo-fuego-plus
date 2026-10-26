---
name: gba-port-architect
description: "Separar lógica y subsistemas dependientes de GBA. Activar para analizar port o abstracción."
---

# gba-port-architect

## Descripción y criterios de activación
Separar lógica y subsistemas dependientes de GBA. Activar para analizar port o abstracción.

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
2. Inspeccionar: docs/development/PORT_ARCHITECTURE.md; src/; include/; libagbsyscall/.
3. Aplicar el cambio acotado teniendo en cuenta: analizar port o abstracción.
4. Ejecutar `build FireRed más pruebas deterministas del módulo` y anotar salida, evidencia y PASS/FAIL/SKIPPED/BLOCKED en `docs/development/TESTING.md`.
5. Revisar diff y criterios de aceptación.

## Archivos y módulos relevantes
docs/development/PORT_ARCHITECTURE.md; src/; include/; libagbsyscall/

## Pruebas obligatorias
`build FireRed más pruebas deterministas del módulo`. Añadir checks del comportamiento modificado.

## Criterios de aceptación
equivalencia demostrada en prototipo acotado. No afirmar PASS sin evidencia ejecutada.

## Prevención de regresiones
Mantener IDs y datos estables; medir EWRAM/IWRAM/ROM en cambios globales. Para cambios de guardado, consultar también `gba-save-compatibility`. Preservar artefactos y ediciones preexistentes.

## Referencias del proyecto
`docs/development/00-project-audit.md`, `AGENT_WORKFLOW.md`, `ROADMAP.md`, `TESTING.md` y rutas de dominio listadas arriba.
