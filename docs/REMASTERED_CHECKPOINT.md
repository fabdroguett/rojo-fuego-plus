# Punto de pausa — FireRed UI Remastered

Trabajo detenido a petición del usuario el 22 de octubre de 2026. Este commit
conserva el estado actual completo del workspace, incluidos archivos normalmente
ignorados: ROM existente, mapas, salidas de compilación, capturas, registros,
respaldos y el guardado local. No se han continuado las mejoras tras la petición.

## Estado real

- Implementados: menú de iconos, equipo de campo vertical con vista previa,
  mochila con lista y vista lateral, adaptación de DexNav/misiones y tema HGSS.
- Últimos cambios: contraste de textos OBJ de Pokédex, ajustes y ficha de entrenador.
  La compilación ARM de comprobación terminó correctamente; estos últimos cambios
  todavía no tienen revisión visual completa.
- Once pruebas automáticas habían pasado antes de esos últimos ajustes.
  Las capturas previas comprobaron resumen, retorno y reordenación del equipo.
- Pendientes: verificación final de todas las pantallas, pruebas de regresión,
  revisión visual independiente y actualización del sistema de diseño documentado.
  DESIGN.md y .impeccable/design.json describen el estado anterior y no certifican
  este rediseño en curso. Las capturas pueden corresponder a etapas diferentes.

## ROMs

`pokefirered.gba` en la raíz sigue siendo la entrega anterior, SHA256
`c3d72ec789eeb9af89141cf0e7494a093393de497d07fe15db8e7df878bb2419`.
No contiene el nuevo rediseño en curso. Su guardado no fue modificado.

`build_artifacts/remastered-checkpoint/TEST-ONLY-pokefirered.gba` y su ELF son la
compilación de comprobación del código actual. Incluyen fixtures retenidos para
el harness; NO son una entrega de producción ni sustituyen la ROM de la raíz.
No se ha generado una nueva ROM de producción al detener el trabajo.

`build_artifacts/latest-ui.json` corresponde a la entrega anterior.

## Continuación en otro PC

El objetivo sigue siendo completar el proyecto existente, preservando sus mejoras
funcionales y adaptándolas a FireRed UI Remastered. Consultar en este orden:

1. `docs/ROJO_FUEGO_PLAN.md`: plan general y prioridades.
2. `docs/IDEAS_OTROS_HACKS.md`: ideas de Unbound, Crystal Advance Redux, Emerald
   Imperium, Radical Red sin aumentar la dificultad, Extended Cut, pokeclassic,
   FireRed Reignited/Extended e Hyper Emerald Lost Artifacts.
3. `docs/IMPLEMENTACION_MISIONES_ALMACENAMIENTO.md`: misiones, Pokémon e ítems.
4. `docs/FIRERED_UI_REMASTERED.md`: análisis de la base real, distribución y riesgos
   del Prompt Maestro de interfaz. Mantener las mejoras ya implementadas.
5. `docs/DEXNAV_REDESIGN.md` y `tools/rojo_fuego/test/UI_VALIDATION.md`.

Las preferencias siguen activas por defecto y deben poder desactivarse desde
Key System. La interfaz objetivo usa paneles pixelados esmeralda/petróleo/turquesa,
selección dorada, bordes discretos sin franjas blancas dominantes, iconos legibles
 y controles reales de GBA a 240×160.

Para otro equipo: clonar la rama `rojo-fuego`, leer `INSTALL.md` y las instrucciones
de compilación del repositorio. Los archivos `build/` y los ejecutables de herramientas
son una instantánea de este equipo; regenerarlos para el entorno de destino cuando
corresponda. El contenedor Docker y el emulador instalado no se transportan con Git.
