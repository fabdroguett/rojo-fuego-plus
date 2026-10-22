# ROM de avance — FireRed UI Remastered

Generada el 22 de octubre de 2026 a petición del usuario. Incluye los cambios del
checkpoint `507eaacb`: menú de iconos, equipo de campo con vista previa, mochila,
DexNav y adaptación del tema de Pokédex, ajustes y ficha de entrenador.

Archivo para abrir en mGBA: `pokefirered.gba`, en la raíz del repositorio.
Es una versión de avance: la revisión visual completa del rediseño sigue pendiente.
Los componentes de prueba fueron retirados antes de enlazar esta ROM.

Validación: once pruebas automáticas correctas, arranque, entrada al mapa y apertura
del menú comprobados en mGBA. Tamaño 32 MiB, código BPRE y checksum de cabecera
correctos. Ningún símbolo RFUiFixture en el ELF de producción. Guardado existente
sin cambios. La ROM anterior está respaldada junto con capturas y validación en
`build_artifacts/remastered-preview-20261022-155556`.

SHA256: `e9406a940265d2dd6a2a9c05576180022777e2fad783955e41234affeefaabed`.

Cerrar/reabrir la ROM en mGBA para cargar el archivo actualizado. Guardar la partida
antes de cerrar el juego. No reutilizar estados rápidos creados con otra compilación.
Este documento actualiza la referencia de ROM del checkpoint anterior.
