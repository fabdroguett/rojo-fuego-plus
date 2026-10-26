# Instalación local de skills

Las skills de esta suite son instrucciones en Markdown, sin dependencias de runtime. La fuente está en `skills/`; las copias locales están en `.agents/skills/` y `.claude/skills/`. No modifica configuración global de Codex, Claude Code, Git, Python, Node ni Windows.

## Uso

Desde PowerShell en la raíz del repositorio:

```powershell
pwsh -NoProfile -File scripts/Setup-GameDev.ps1 -Diagnose
pwsh -NoProfile -File scripts/Setup-GameDev.ps1 -Validate
pwsh -NoProfile -File scripts/Setup-GameDev.ps1 -Install -DryRun
pwsh -NoProfile -File scripts/Setup-GameDev.ps1 -Install
```

La instalación hace copia local, crea un respaldo con manifiesto y registra log en `build_artifacts/gamedev/`. Si una copia existente difiere de la fuente, se detiene para conservarla; `-Force` permite reemplazarla después de respaldarla. `-IncludeExternal` solo agrega el inventario/diagnóstico de colecciones; no descarga ni instala software.

Restauración:

```powershell
pwsh -NoProfile -File scripts/Restore-GameDev.ps1 -WhatIf
pwsh -NoProfile -File scripts/Restore-GameDev.ps1
```

El restaurador solo toca destinos registrados por Setup y comprueba su hash. Si alguien editó el destino después de instalar, se detiene para evitar pérdida de datos. No usar `-Force` como sustituto de una copia de seguridad revisada.

## Activación

Codex descubre skills del proyecto bajo `.agents/skills/`; Claude Code usa `.claude/skills/`. `AGENTS.md` y `CLAUDE.md` dirigen a la guía compartida `docs/development/AGENT_WORKFLOW.md`. Los archivos se pueden leer manualmente si el agente no carga skills automáticamente. El orquestador es una guía de enrutamiento; no ejecuta agentes ni promete selección autónoma.

## Externas

No se descargaron colecciones externas. Consultar `01-skills-inventory.md`; activar evaluación de una colección requiere revisar de nuevo el commit, licencia, scripts y hooks, porque sus repositorios cambian.
