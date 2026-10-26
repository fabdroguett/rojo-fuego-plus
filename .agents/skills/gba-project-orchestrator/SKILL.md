---
name: gba-project-orchestrator
description: "Enruta tareas de Rojo Fuego Plus a las skills mínimas; prioridad funcional: misiones, servicios, reglas y mundo. Activar al inicio de cada solicitud del repo."
---

# gba-project-orchestrator

## Propósito y activación
Enrutar el trabajo al conjunto mínimo de skills para Rojo Fuego Plus. Activar al inicio de cada tarea de este repositorio.

## Prioridad vigente del proyecto
El catálogo tiene 350 IDs (0–349), pero el plan conserva contenido/cadenas funcionales pendientes. El trabajo activo solicitado es cerrar la funcionalidad de esas misiones y continuar sistemas funcionales del plan. No proponer ni realizar cambios de UI mientras siga vigente la instrucción explícita “nada de UI, solo funcionalidad”.

## Reglas de enrutamiento
- Leer solo los SKILL.md seleccionados, no cargar la colección entera.
- Elegir una skill principal y de una a tres transversales; incluir pruebas si se modifica código/datos.
- Seguir `docs/PLAN_COMPLETO.md` y el estado en `docs/CONTINUIDAD.md`; este orquestador no reescribe ese plan.
- Las recomendaciones son instrucciones para el agente, no un sistema de ejecución autónoma ni un lanzamiento de subagentes.
- Preservar Git preexistente y documentar evidencia real.

## Mapa de selección
| Solicitud | Skills |
|---|---|
| Completar/validar cadena o requisito de misión | pokemon-event-scripting, pokemon-data-engine, gba-automated-testing; gba-save-compatibility si persiste estado |
| Añadir contenido narrativo/campaña | pokemon-event-scripting, pokemon-world-builder, pokemon-data-engine, gba-automated-testing |
| Función de mochila/objeto/inventario | gba-core-engine, pokemon-data-engine, gba-save-compatibility, gba-automated-testing; `gba-ui-ux` solo si cambia interacción/presentación y el usuario lo solicita |
| Servicio de Centro, EV/IV, tutor o herramienta | gba-core-engine, pokemon-data-engine, pokemon-event-scripting, gba-automated-testing; gba-save-compatibility si guarda estado |
| Combate, habilidad o regla | pokemon-battle-engine, gba-core-engine, gba-automated-testing; gba-save-compatibility si persiste |
| Save/banco/cajas | gba-save-compatibility, gba-core-engine, gba-debugger, gba-automated-testing, gba-performance |
| Mapa, encuentro, gruta, radio | pokemon-world-builder, pokemon-event-scripting, pokemon-data-engine, gba-automated-testing |
| Crash o regresión | gba-debugger, skill del dominio, gba-automated-testing |
| Optimización o límite | gba-performance, gba-core-engine, gba-automated-testing |
| UI/arte | gba-ui-ux y skill gráfica pertinente solo con solicitud explícita del usuario |
| Port o Godot | gba-port-architect, gba-decomp-expert y godot-modern-renderer; solo como tarea futura aprobada |

## Flujo obligatorio
1. Leer el punto vigente de `docs/PLAN_COMPLETO.md` y `docs/CONTINUIDAD.md`.
2. Identificar precondiciones y estado implementado/validado; no duplicar IDs ni contenido.
3. Cargar las skills de la tabla necesarias y respetar sus pruebas.
4. Implementar una entrega funcional completa y seguir con el siguiente trabajo independiente preparado.
5. Registrar estado y evidencia en `docs/development/TESTING.md` y continuidad del juego cuando aplique.

## Límites
No inventar automatización de agentes. No ejecutar instalación externa, editar UI sin solicitud ni iniciar una reescritura integral.
