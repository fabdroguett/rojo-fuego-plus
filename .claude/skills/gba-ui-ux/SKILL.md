---
name: gba-ui-ux
description: "Cambios de interacción/presentación GBA únicamente cuando el usuario los solicite expresamente."
---
# gba-ui-ux

## Estado y activación
La UI no pertenece a la prioridad funcional activa del proyecto. Activar solo ante petición explícita del usuario para interfaz, navegación o accesibilidad.

## Responsabilidades
Revisar menús GBA a 240×160, navegación por botones, estados vacío/lleno, textos y compatibilidad con mochila/categorías existentes.

## Restricciones
No iniciar rediseños visuales, iconografía ni cambios cosméticos en tareas funcionales. Preservar estado de la mochila y saves. Confirmar rutas reales antes de editar.

## Herramientas permitidas
Lectura/edición local, scripts existentes, build WSL y harness/capturas mGBA pertinentes.

## Flujo de trabajo
1. Confirmar que la solicitud pide cambio de UI.
2. Leer `DESIGN.md`, `docs/CONTINUIDAD.md` y `docs/RECURSOS_MENU.md`.
3. Implementar solo el flujo pedido; probar mando y casos límite.
4. Registrar capturas/test solo si se ejecutaron.

## Archivos relevantes
`src/*menu*.c`, `src/item_menu.c`, `src/party_menu.c`, `graphics/`, `DESIGN.md`.

## Pruebas obligatorias
`make BUILD=firered -j2` y el harness/captura funcional pertinente. Registrar evidencia real en `docs/development/TESTING.md`.

## Aceptación y regresiones
Flujo navegable y estados vacío/lleno correctos; no modificar progresión, inventario persistente ni IDs. La mochila mantiene capacidad de 80 y siete categorías ya integradas.

## Referencias
`docs/development/AGENT_WORKFLOW.md`, `docs/development/ROADMAP.md`, `docs/CONTINUIDAD.md`, `docs/RECURSOS_MENU.md`.
