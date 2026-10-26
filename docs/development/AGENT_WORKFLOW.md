# Flujo de trabajo para agentes

1. Lee `docs/development/00-project-audit.md`, `ROADMAP.md` y el documento específico del sistema antes de editar.
2. Comprueba `git status --short` y preserva los cambios locales. No uses `reset`, `clean` ni checkout destructivo.
3. Elige solo las skills necesarias con `skills/gba-project-orchestrator/SKILL.md`; no cargues las 16 por defecto. La tarea activa es el barrido de UI nativa/nueva solicitado más recientemente; después se retoma el cierre de contenido/cadenas de misiones 0–349 y SERVICIOS según el plan.
4. Confirma rutas/símbolos en el código antes de asumir interfaces. `BUILD=firered`, `Makefile`, `config.mk` y scripts WSL son autoridad para los comandos actuales.
5. Haz cambios pequeños y mantén la compatibilidad de guardados, mapas, eventos y formatos. Mide RAM/ROM al tocar recursos o sistemas globales.
6. Ejecuta las pruebas adecuadas. Registra comando, objetivo, resultado, evidencia y estado PASS/FAIL/SKIPPED/BLOCKED en `docs/development/TESTING.md`.
7. Revisa `git diff` para separar claramente el trabajo nuevo de los cambios previos.

No instales paquetes externos ni modifiques configuración global. No comiences una reescritura/port sin un plan aprobado. Para una tarea de UI, selecciona UI/graphics y las skills de dominio necesarias; no se cargan por rutina en tareas funcionales.
