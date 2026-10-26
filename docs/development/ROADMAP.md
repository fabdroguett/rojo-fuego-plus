# Hoja de ruta alineada con Rojo Fuego Plus

Este documento acompaña `docs/PLAN_COMPLETO.md` y `docs/CONTINUIDAD.md`; no crea una hoja paralela. El pedido más reciente del usuario prioriza un barrido integral de UI nativa y nueva, adaptando las mejoras ya existentes (mochila de 80 espacios y siete categorías). Al terminar ese paquete se retoma el cierre funcional de las 350 misiones: los IDs 0–349 ya existen, pero el contenido/cadenas pendientes del plan deben completarse.

## Cola operativa vigente

| Orden | Paquete del plan | Estado según documentación del proyecto | Acción alineada | Criterio de salida |
|---:|---|---|---|---|
| 1 | Barrido de UI nativa/nueva | Barrido automatizado ejecutado: 208 capturas; se corrigieron encabezado y pie contextual de mochila en `src/item_menu.c`. Build de producción posterior pasó sin fixtures | Cerrar cobertura de caja PC y recorrido humano; pasada visual entre familias para alinear paleta, jerarquía, márgenes, títulos y leyendas de botones según `UI_REVIEW.md` | Flujo comprensible y potente, identidad visual consistente en claro/oscuro, estados vacíos/llenos comprobados, capturas nativas y build sin fixtures |
| 2 | BASE / EVENTOS / CAMPAÑAS de misiones | 350 IDs y objetivos funcionales parciales implementados; distribución narrativa completa pendiente | Cruzar las 350 entradas con objetivos, prerrequisitos, mapas/eventos y campañas de `PLAN_COMPLETO.md` e `IDEAS_OTROS_HACKS.md`; marcar vacíos funcionales y completar cadenas únicas sin inflar conteo | catálogo conserva IDs 0–349; validador cubre reglas; cada cadena tiene inicio, progreso, desenlace/recompensa y prueba; no quedan requisitos del plan omitidos sin estado |
| 3 | SERVICIOS | editor EV/IV, recordador, bayas y ocho tutores por medalla ya existen; expansión pendiente | completar preparación en Centros y herramientas unificadas, reutilizando servicios existentes | uso/cancelación/progresión y persistencia comprobados; pruebas de dominio y build |
| 4 | REGLAS / Noa | casos normales y premio pendiente validados; caso original de desobediencia/investigaciones de calendario y save pendientes | investigar de forma aislada y reproducible, sin detener paquetes independientes | causa registrada o bloqueo con evidencia; reglas/equipos restaurados al salir/perder/guardar/cargar |
| 5 | COMBATES | modalidades del catálogo pendientes | perfiles temporales, alquiler/espejo/dobles y ciclos de jefes según dependencias | cada modalidad completa entrada, batalla, salida, recompensa y restauración |
| 6 | MUNDO / EXPLORACIÓN | expansiones funcionales pendientes; depender de EVENTOS | registro compartido, radio/mapa/grutas/reaparición; después una zona completa con entrada, objetivos, salida/recuperación | estado persistente coherente y pruebas de renovación/recorrido |
| 7 | CAMPAÑAS | el conteo de misiones está, varias campañas/contenidos del plan siguen pendientes | ampliar personajes, rescates, artefactos, tesoros, Sete, buceo y expediciones con cadenas enlazadas a sistemas jugables | historias con efectos de mundo y misiones distintas; aceptar datos solo cuando estén integrados y probados |
| 8 | SAVE-2 / SAVE-3 | SAVE-1 medido; 14 cajas/420 Pokémon; banco/migración no habilitados | investigar sectores 30/31 y prototipo aislado; no aumentar cajas hasta resolver integridad/compatibilidad | copia antigua/nueva, checksum, rollback, cifrado e interrupción probados |
| 9 | ENTREGA / regresión | build/harness existen; recorridos completos y Android pendientes | cerrar regresión por paquetes; pruebas de nueva partida/apodo/primera batalla/save y captura mGBA donde corresponda | build sin fixtures, límites de memoria registrados, evidencia local y artefactos fuera de Git |

## Alcance estacionado

- El redibujo de arte queda fuera del barrido salvo que una falla impida legibilidad o consistencia de la UI. Prioridad en esta revisión: adaptación y claridad nativas de las pantallas existentes.
- Port y renderer moderno quedan como diseño/investigación de futuro. No crear runtime Godot/SDL/raylib en esta cola.
- La versión moderna y decompilación desacoplada de la matriz no han comenzado.

## Método por entrega

1. Consultar primero la tarea correspondiente en `PLAN_COMPLETO.md` y el punto concreto en `CONTINUIDAD.md`.
2. Mantener una integración activa y auditar el sistema local a cambiar; no rehacer el inventario completo por cada paquete.
3. Implementar primero sistemas reutilizables y luego contenido; no duplicar objetivos o cadenas para cumplir una cuota.
4. Agrupar lotes de 15–25 misiones cuando sus sistemas estén listos, pero no mezclar IDs públicos ni reordenarlos.
5. Al cambiar guardado, seguir SAVE-1 y no guardar progreso intermedio innecesario en misiones compuestas.
6. Ejecutar `validate_quests.py`, unittest relevante, `verify_code.sh` o prueba mGBA proporcional al cambio. Registrar comando, salida y evidencia en `TESTING.md`.
7. Continuar con el siguiente trabajo independiente que el plan permita; no terminar una sesión dejando un cambio parcialmente integrado.

## Estado de esta suite

| Componente | Estado | Aceptación |
|---|---|---|
| Auditoría | Implementado | arquitectura real, cambios Git preexistentes y riesgos anotados |
| 17 skills locales | Implementado | instrucciones repo-específicas en fuentes Codex/Claude, cargables selectivamente |
| Orquestador y reglas | Implementado | enrutamiento manual/documentado; sin ejecución autónoma inventada |
| Scripts de setup/test/restore | En desarrollo | validación/dry-run, suite Python, check de ELF y build de ROM registrados; falta probar restauración efectiva en una instalación desechable |
| Barrido de UI nativa | En desarrollo | 104 capturas mGBA claro/oscuro; franja de mochila y pie contextual corregidos; falta recorrido de caja PC con datos |
| Cierre funcional de misiones | Pendiente | después del barrido, cruzar objetivos/cadenas con el plan completo |
| Port/renderer | No iniciado | documentos preliminares; sin código ni instalación externa |
