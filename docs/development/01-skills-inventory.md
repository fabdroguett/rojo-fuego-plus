# Inventario de skills y colecciones

Revisado el 2026-10-26. La compatibilidad indicada es de formato y flujo de trabajo, no una afirmación de que las herramientas externas se hayan instalado o ejecutado.

## Estado del proyecto

Antes de crear esta suite no había skills de proyecto ni `AGENTS.md`/`CLAUDE.md`. `.impeccable/` y `.taste-skill/` contienen dirección visual local, pero no acreditan la instalación de los paquetes oficiales. Esta suite añade 16 skills específicas y un orquestador en `skills/`, con copias locales de Codex en `.agents/skills/` y Claude Code en `.claude/skills/`.

## Colecciones externas

| Colección oficial | Licencia / revisión | Compatibilidad y dependencias | Decisión local |
|---|---|---|---|
| [Awesome GameDev Agent Skills](https://github.com/gamedev-skills/awesome-gamedev-agent-skills) | Apache-2.0; HEAD observado `184b72be02884f782a0058f2eeae2ce4277453e4` | Skills con formato compatible con Codex; CLI Node requiere Node >=22.20 según README. Muchas skills enfocadas en otros motores. | No instalar toda la colección. Evaluar skills puntuales si comienza un prototipo Godot; mantener este inventario como referencia. |
| [Superpowers](https://github.com/obra/superpowers) y [superpowers-skills](https://github.com/obra/superpowers-skills) | MIT; HEADs `2ce8aa15d05bd57213e6f746f953aafb7ded7a8b` y `cdcd624ad3fd8026deb692e565351854569798dd` | Framework/plugin con hooks y comandos; algunas instrucciones de flujo son obligatorias y pueden cambiar el comportamiento global de sesiones. | No instalar framework/hook. Sus prácticas pueden inspirar revisión y pruebas, sin copiar automatizaciones. |
| [Pixel Art Studio](https://github.com/Gamezxz/pixel-art-studio) | MIT; HEAD `f8c246635c4621a6c2b427833149afdc3dc3c719` | Skill simple adaptable; referencias de instalación orientadas a `.claude/skills`; requiere Pillow para scripts. | Adaptar solo si se desea automatizar ilustración; exigir validación de paleta, tiles y límites GBA. No instalar Pillow automáticamente. |
| [PXL / claude-pxl](https://github.com/crsolver/claude-pxl) | MIT; HEAD `21fee41f7ecce00bc868d7567f24ffd0f3acc8fa` | DSL Node para sprites/sheets/GIF, Node >=22.18 según README; pocos commits y sin historial de versiones maduro observado. | No instalar. Probar en sandbox de assets si se aprueba; validar licencia y exportación GBA antes de integrarlo. |
| [Frontend Design](https://github.com/anthropics/claude-plugins-official/tree/main/plugins/frontend-design) | Colección oficial de plugins Anthropic; HEAD observado `58267944f92187b7d6a1c822165c96c81c864806`. El plugin referencia `LICENSE.txt`, pero no se pudo verificar el archivo de licencia en la ruta publicada. | Es un plugin de Claude Code, no asumir que sus comandos se trasladan a Codex. Las instrucciones Markdown pueden adaptarse manualmente, con atribución y revisión de licencia antes de copiar contenido. | No instalar el plugin. No es una dependencia adecuada para la interfaz GBA de resolución fija; conservarlo como referencia y no redistribuir sus archivos. |
| [Impeccable](https://github.com/pbakaus/impeccable) | Apache-2.0; HEAD `6743a347db6229848348024388033e830ae4af42` | Tiene skill Codex, comandos/detectores y hooks locales que requieren confianza; dependencias npm/browser según funciones. | Se detectó carpeta `.impeccable/` preexistente. No instalar hooks ni duplicar configuración; usar únicamente sus datos locales como referencia de diseño. |
| [Godot Expert Skills](https://github.com/edemaistre/godot-expert-skills) | Comunitaria; tag/HEAD v0.1 `3f3d5be77aa8f3a9402b6da813a58c5d4c966a5c` | Diseñada para Godot 4.7.2; requiere Python >=3.9 y toolkit compartido; autor indica pruebas en macOS Apple Silicon. | No instalar mientras no exista proyecto Godot. Revaluar en prototipo controlado y validar las rutas Codex/Claude. |

Los commits son referencias observadas al auditar; repositorios con rama móvil pueden cambiar. Para instalar una dependencia externa en el futuro, fijar un tag/commit, comprobar licencia, scripts, dependencias y cambios de hooks antes de copiar nada.

## Cobertura de skills locales

`gba-core-engine`, `gba-decomp-expert`, `gba-performance`, `gba-debugger`, `pokemon-battle-engine`, `pokemon-data-engine`, `pokemon-world-builder`, `pokemon-event-scripting`, `gba-graphics-master`, `gba-ui-ux`, `gba-animation`, `gba-audio`, `gba-automated-testing`, `gba-save-compatibility`, `gba-port-architect`, `godot-modern-renderer` y `gba-project-orchestrator`.

Fuente canónica: `skills/<nombre>/SKILL.md`. Instalaciones del proyecto: `.agents/skills/` para Codex y `.claude/skills/` para Claude Code. Ejecutar `scripts/Setup-GameDev.ps1 -Validate` para detectar ausencia o divergencia. No se necesita ejecutar todas simultáneamente; el orquestador recomienda un conjunto reducido por tarea.
