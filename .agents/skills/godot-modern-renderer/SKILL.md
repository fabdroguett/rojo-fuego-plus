---
name: godot-modern-renderer
description: "Candidato Godot 4, HD-2D y render moderno. Activar para solicitud explícita de prototipo moderno."
---

# godot-modern-renderer

## Descripción y criterios de activación
Candidato Godot 4, HD-2D y render moderno. Activar para solicitud explícita de prototipo moderno.

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
2. Inspeccionar: docs/development/MODERN_RENDERER_DESIGN.md; graphics/.
3. Aplicar el cambio acotado teniendo en cuenta: solicitud explícita de prototipo moderno.
4. Ejecutar `validar proyecto Godot cuando exista; si no, SKIPPED` y anotar salida, evidencia y PASS/FAIL/SKIPPED/BLOCKED en `docs/development/TESTING.md`.
5. Revisar diff y criterios de aceptación.

## Archivos y módulos relevantes
docs/development/MODERN_RENDERER_DESIGN.md; graphics/

## Pruebas obligatorias
`validar proyecto Godot cuando exista; si no, SKIPPED`. Añadir checks del comportamiento modificado.

## Criterios de aceptación
vertical slice pixel-perfect medible. No afirmar PASS sin evidencia ejecutada.

## Prevención de regresiones
Mantener IDs y datos estables; medir EWRAM/IWRAM/ROM en cambios globales. Para cambios de guardado, consultar también `gba-save-compatibility`. Preservar artefactos y ediciones preexistentes.

## Referencias del proyecto
`docs/development/00-project-audit.md`, `AGENT_WORKFLOW.md`, `ROADMAP.md`, `TESTING.md` y rutas de dominio listadas arriba.
