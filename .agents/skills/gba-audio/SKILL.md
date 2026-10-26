---
name: gba-audio
description: "Música, efectos y secuencias GBA. Activar para cambiar sonido o assets."
---

# gba-audio

## Descripción y criterios de activación
Música, efectos y secuencias GBA. Activar para cambiar sonido o assets.

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
2. Inspeccionar: sound/; audio_rules.mk; tools/mid2agb/; tools/wav2agb/.
3. Aplicar el cambio acotado teniendo en cuenta: cambiar sonido o assets.
4. Ejecutar `make BUILD=firered -j2; escuchar escena en mGBA` y anotar salida, evidencia y PASS/FAIL/SKIPPED/BLOCKED en `docs/development/TESTING.md`.
5. Revisar diff y criterios de aceptación.

## Archivos y módulos relevantes
sound/; audio_rules.mk; tools/mid2agb/; tools/wav2agb/

## Pruebas obligatorias
`make BUILD=firered -j2; escuchar escena en mGBA`. Añadir checks del comportamiento modificado.

## Criterios de aceptación
conversión, loop y mezcla comprobados. No afirmar PASS sin evidencia ejecutada.

## Prevención de regresiones
Mantener IDs y datos estables; medir EWRAM/IWRAM/ROM en cambios globales. Para cambios de guardado, consultar también `gba-save-compatibility`. Preservar artefactos y ediciones preexistentes.

## Referencias del proyecto
`docs/development/00-project-audit.md`, `AGENT_WORKFLOW.md`, `ROADMAP.md`, `TESTING.md` y rutas de dominio listadas arriba.
