# Barrido de interfaz GBA

## Alcance y evidencia

La UI corre sobre framebuffer GBA fijo de 240×160. La revisión usa el harness `tools/rojo_fuego/test/capture_ui.py`, su fixture aislada `ui_fixture.c` y mGBA. Se capturaron los escenarios predeterminados y los escenarios adicionales `party-slow`, `faint` y `voiceover` en temas oscuro y claro: 104 capturas por tema, 208 en total. No reemplaza un recorrido manual con partidas reales ni prueba Android. Evidencia de esta revisión: capturas/logs en `build_artifacts/ui/`; la evidencia anterior se respaldó bajo `build_artifacts/ui/pre-ui-sweep-20261026-120523/` antes de volver a capturar.

Escenarios cubiertos por el barrido: Opciones, Trainer Card, Pokédex (lista/detalle/búsqueda/formas), equipo de campo (foco/acción/resumen/cambio/orden), menú START (foco/filas/salida), DexNav (vacío/no visto/encuentros/colección/registro/filtro), resumen (perfil/stats/movimientos), mochila masculina y femenina (siete categorías, Balls/TM, acciones), Misiones, Claves, PC, Naming Screen, mapa y Fly Map, logros, objetos registrados, tienda (menú/compra/cantidad), recordador, evolución, popups de batalla, combate (acciones/movimientos), intercambio de Pokémon, derrota/revive/cambio y tutorial de Oak.

## Hallazgos y cambios

1. **Nombre del bolsillo y categorías de mochila se pintaban en la misma franja.** El `WIN_POCKET_NAME` empezaba en la segunda fila del encabezado, donde se muestran las siete categorías. Se movió al primer renglón del encabezado, junto al título Mochila.
2. **Indicadores heredados de bolsillo se superponían a las tabs nuevas.** Con la nueva banda de categorías ya no aportan información; se ocultan solo cuando está activo el chrome renovado. El comportamiento heredado permanece como fallback.
3. **Pie de controles incorrecto según el bolsillo.** Se mostraba “L/R Tipos” en cualquier bolsillo aunque L/R cambian de bolsillo fuera de Objetos. Ahora el pie indica “Filtrar” en Objetos, “Bolsillo” en los demás y solo A/B en la bolsa tutorial.
4. **Los filtros afectan contenido y navegación.** Las capturas confirman listas específicas para combate, evolución, equipo, tesoros y otras; una sonda desechable reportó conteos de 7 en Todo, 3 en Cura, 2 en Combate/Evolución/Equipo/Tesoros y 1 en Otros (incluye “Cerrar mochila”).
5. **La selección de categoría y los controles tenían poco contraste.** Las pestañas ahora usan tinta secundaria legible y una selección rellena distinta en ambos temas; la leyenda tiene una franja propia con borde y texto ajustado al ancho. Los nombres de bolsillos visibles ahora están en español.

## Notas de consistencia

- La UI nueva (inicio, DexNav, Misiones, Claves y mochila) usa el tema compartido; las pantallas nativas conservan paletas semánticas como HP, tipos, Pokémon y wallpapers de cajas. No aplicar un reemplazo global que vuelva ilegibles esos colores.
- Los contenidos base de FireRed incluyen nombres/descripciones en inglés (por ejemplo, objetos y Trainer Card); esta revisión detecta el contraste entre esos textos y los rótulos personalizados en español. La localización completa es una tarea separada de contenido y requiere inventario de strings, no reemplazos visuales improvisados.
- El PC de la fixture aparece sin Pokémon porque la fixture de UI no siembra cajas. No cuenta como prueba de una caja guardada real; añadir fixtures de PC con especies/objetos antes de declarar validada la gestión de cajas.
- Las capturas muestran la presentación de los flujos, pero cada escenario solo tiene aserciones funcionales cuando el script/harness las especifica. Los intercambios de equipo sí afirman el índice final; otros escenarios necesitan aserciones de estado para elevar la cobertura.

## Criterio visual para la siguiente pasada

La referencia de `.taste-skill/direction.json` se adapta al lienzo GBA, no se copia como layout web. La identidad compartida es el “instrumento de campo de Kanto” descrito en `.impeccable/design.json`: composición compacta, superficies planas, tipografía bitmap, acento FireRed medido y sprites originales intactos.

- Unificar entre pantallas el sistema de color (fondo, panel, borde, texto, secundario y selección), el ritmo de márgenes, la alineación de títulos y la ubicación de instrucciones de botones. Los componentes pueden tener formas distintas si la tarea lo exige, pero deben compartir los mismos roles visuales.
- Mantener jerarquía clara a resolución nativa: título, selección/estado, contenido y controles. Usar el acento para selección o acción principal; no añadir brillos, gradientes ni adornos que compitan con información de juego.
- Mejorar composición por pantalla con grupos y alineación consistentes, evitando amontonar texto o reducirlo para que quepa. Para cadenas largas, ajustar/truncar de forma visible y conservar una ruta para consultar el dato completo cuando sea necesario.
- Revisar oscuro y claro como pareja: mismo orden de lectura y estados; contraste suficiente en ambas paletas. Los colores semánticos de Pokémon, tipos, HP y estados quedan bajo control de sus sistemas existentes.
- La aceptación visual requiere comparar capturas nativas lado a lado y revisar al tamaño 240×160 (además de ampliación entera para inspección), no aprobar por un mockup o solo por compilación.

## Pendiente para cierre de cobertura

- Añadir datos de cajas al escenario PC y aserciones de categorías/estados a los tests de UI.
- Recorrido humano de nueva partida, apodo, primera batalla, save/load y Android sigue pendiente, según `docs/CONTINUIDAD.md`.

El build de producción sin `RFUiFixture_` ya se repitió y pasó; su resultado y SHA están en `TESTING.md`. Estos pendientes son límites de cobertura de la revisión, no fallos de compilación.

## Renovación de interfaz de batalla

- La ventana de acciones y la de movimientos ahora comparten roles de superficie, borde, tinta y sombra con el tema general. Se aplican tanto al modo claro como al oscuro; el marco FireRed añade un acento cálido coherente en ambos.
- El texto del flujo principal de batalla se localizó al español: acciones, Safari, selección de movimiento, confirmación de captura/cambio y etiqueta de tipo. Los nombres de tipos visibles también usan español; `TYPE_NAME_LENGTH` pasa a 9 para contener «Eléctrico» y «Siniestro».
- La primera iteración con bordes coral gruesos y selección roja se descartó tras la revisión visual del usuario. La versión actual usa paneles verde petróleo/crema, selección menta de alto contraste, cursor dorado e iconos compactos para Luchar, Bolsa, Equipo y Huir. El prompt, menús de acciones y movimientos y cajas de estado comparten el mismo lenguaje cromático en claro y oscuro.
- Los textos de acción y flujo se localizan al español: Luchar, Bolsa, Equipo, Huir, Safari, selección/confirmación y TIPO. Los nombres visibles de tipos también están en español; `TYPE_NAME_LENGTH` pasa a 9 para «Eléctrico» y «Siniestro». Nombres de movimientos y varios mensajes narrativos heredados siguen en inglés.
- Se preservan las paletas semánticas de Pokémon, tipos, HP, EXP y estados. Los marcos de estado usan el tema, pero se eliminó el acento coral del bisel/sombra que producía grandes bloques rojos. Los iconos se dibujan en el buffer nativo y no agregan recursos externos.
- Se capturó el combate de prueba en ambos temas con `python tools/rojo_fuego/test/capture_ui.py battle`; imágenes de comandos, movimientos y foco en `build_artifacts/ui/`. Las capturas de la fixture solo validan presentación/navegación sintética; no reemplazan un recorrido manual con un guardado real.
- La composición de campo sigue limitada al framebuffer 240×160 y conserva las posiciones base para no tapar los Pokémon. La renovación rehace el sistema visual del HUD y sus comandos dentro de esa geometría; un cambio de composición de campo completo todavía requeriría un estudio de espacio y cobertura adicional.
- Build de producción sin símbolos fixture y copia al Escritorio: ver hash final en `TESTING.md`.
