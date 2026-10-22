# ROM de avance — FireRed UI Remastered

Actualizada el 22 de octubre de 2026. Parte de la ROM de avance anterior (checkpoint
`507eaacb`: menú de iconos, equipo de campo con vista previa, mochila, DexNav y adaptación
del tema de Pokédex, ajustes y ficha de entrenador) y añade estas correcciones:

- **Ficha de entrenador:** voltearla causaba el error `INVALID SPECIES: 32768`. Al crear
  partida, el código heredado de Esmeralda escribía el récord de Seedot y el número de la
  lotería en variables que FireRed usa para la ficha: `VAR_TRAINER_CARD_MON_ICON_5` (0x4047)
  y `VAR_LINK_WIN_BRAG_STATE` (0x404B). En FireRed ya no se ejecutan esas inicializaciones.
  Además, la ficha descarta valores imposibles; así se reparan, sin modificar el archivo,
  las partidas creadas antes de la corrección (el guardado local tenía ambos valores dañados).
- **Unown en el Salón de la Fama:** la ficha también fallaba con un Unown, que se guarda
  como especie de correo (30000 + letra); ahora se dibuja la forma de su letra.
- **Medallas en tema oscuro:** los recuadros numerados eran invisibles porque compartían
  color con la franja; la franja usa ahora el fondo oscuro.

- **Combate en tema oscuro:** las cajas de vida usan panel petróleo, texto claro y borde
  turquesa; barras de PS, género y estados conservan sus colores. El botón «MOVE INFO»
  tiene paleta propia (tecla L dorada) para no alterar los avisos de habilidad.
- **Logros en tema oscuro:** mismos paneles y trofeos sobre superficies nocturnas; el
  trofeo bloqueado se ve como contorno.

- **Combate a fondo (tema oscuro):** la acción o el movimiento seleccionado es una celda
  dorada; el tipo del movimiento se muestra como insignia de color junto al icono de
  categoría; los PP usan colores legibles (claro, amarillo, naranja, rojo); la ventana de
  la última Poké Ball (R) y los avisos de habilidad/objeto siguen el tema, con el nombre
  de la habilidad en dorado.
- **Tienda:** la lista de compra es oscura con franjas sutiles y borde turquesa.
- **Ficha Pokémon:** títulos de sección con contorno dorado, legibles sobre las barras.
- **Misiones y Claves:** páginas completas sobre el suelo del tema; ya no se ve el mapa
  entre los paneles, y el mapa se restaura al cerrar.
- **Mapa de vuelo:** «FLY to where?» en un panel con borde.

Prueba de regresión nueva: `tools/rojo_fuego/tests/test_new_game_vars.py`. Fixtures y
capturas nuevas: nombre, mapa, vuelo, logros, objetos registrados, tienda, recordador,
evolución y menús de combate.

Archivo para abrir en mGBA: `pokefirered.gba`, en la raíz del repositorio.

Validación: doce pruebas automáticas correctas; 156 capturas de todas las pantallas del
harness en tema oscuro y claro, sin pantallas de error, y con cambios de miembro del equipo
correctos en los cuatro escenarios. La ficha se probó con partida nueva, valores dañados y
especies válidas (incluida una Unown en formato de correo). Arranque, entrada al mapa y
apertura del menú comprobados. Tamaño 32 MiB, código BPRE y checksum de cabecera correctos.
Ningún símbolo RFUiFixture en el ELF de entrega. Guardado sin cambios. La ROM anterior
quedó respaldada junto con capturas y validación en
`build_artifacts/ui-battle-op-20261022-175446`.

SHA256: `1396081769f8bc1182bb514b20ca0efb10a686f309df8ef1f78419eeb4f87926`.

Cerrar y reabrir la ROM en mGBA para cargar el archivo actualizado. Guardar la partida
antes de cerrar el juego. No reutilizar estados rápidos creados con otra compilación.
Sigue pendiente actualizar DESIGN.md y `.impeccable/design.json` con el rediseño actual.
