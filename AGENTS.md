# Instrucciones para agentes en este repositorio

Lee [el flujo de trabajo compartido](docs/development/AGENT_WORKFLOW.md) y enruta la tarea con [gba-project-orchestrator](skills/gba-project-orchestrator/SKILL.md). Carga solo las skills del dominio requerido y sus transversales necesarias.

Preserva cambios locales preexistentes, no instales dependencias externas ni cambies configuración global. Antes de editar confirma rutas/APIs en el código. Ejecuta las pruebas relevantes y registra comando, evidencia y estado en `docs/development/TESTING.md`. Consulta `docs/development/00-project-audit.md`, `ROADMAP.md` y documentos del dominio.
