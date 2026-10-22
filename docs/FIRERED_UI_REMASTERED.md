# FireRed UI Remastered — análisis y contrato

## Base verificada antes de modificar código

Punto de partida: commit `314e1806`, árbol limpio. Hay fuentes C/Thumb, mapas, scripts,
PNG indexados, paletas y conversores; no es una ROM binaria sin fuentes.
README identifica RHH pokeemerald-expansion 1.17.1 sobre pret/pokeemerald. Makefile selecciona
FireRed con `BUILD=firered`, título POKEMON FIRE y código BPRE. No es pret/pokefirered puro.
No se debe sustituir esta base por otra ni importar estructuras de un proyecto diferente.

Windows dispone de Python y Docker CLI; no aparecen make ni ARM GCC en PATH. La cadena
existente es Docker `rojo-fuego-toolchain:local`, contenedor `rojo-fuego-check`, GCC ARM y
Make en `/work`, fuentes del host en `/source`. El harness existente prueba con mGBA y un
guardado temporal. No se deben usar guardados reales para automatización.

## Mapa de implementación comprobado

| Superficie | Código | Recursos / dependencias |
|---|---|---|
| Menú principal | src/start_menu.c, src/menu.c | ventanas BG0, acciones y callbacks existentes |
| Equipo | src/party_menu.c, src/data/party_menu.h | graphics/party_menu, paletas de selección/PS/género |
| Mochila | src/item_menu.c | graphics/bag, listas, pilas y bolsillos ampliados |
| DexNav | src/dexnav.c | include/dexnav.h, cursor 4bpp propio, sprites y colección |
| Pokédex | src/pokedex.c, src/pokedex_plus_hgss.c | graphics/pokedex/hgss; HGSS habilitado, estadísticas/evoluciones/formas |
| Ficha | src/pokemon_summary_screen.c | graphics/summary_screen, IV/EV, movimientos |
| Combate | src/battle_bg.c, src/battle_message.c | graphics/battle_interface, ventanas y controladores |
| PC | src/pokemon_storage_system.c | graphics/pokemon_storage, wallpapers y protección de Pokémon |
| Misiones | src/rf_quest_menu.c | catálogo RFQuests, capítulos, progreso, recompensas |
| Claves | src/key_system.c | ventanas, listas y preferencias persistentes |
| Tienda / opciones | src/shop.c, src/option_menu.c | graphics/shop, paletas y marcos |
| Ventanas comunes | src/text_window.c, src/rf_ui_theme.c | graphics/text_window, índices de texto y borde |

## Restricciones y riesgos

240×160 píxeles, tiles 8×8, BG y OBJ con paletas indexadas; 16 bancos BG y 16 OBJ de 16 colores
en 4bpp, 128 entradas OAM, VRAM compartida y copias DMA/VBlank. El último enlace ocupa
230220 bytes EWRAM, 28648 IWRAM y unos 27.13 MB ROM, con archivo GBA rellenado a 32 MiB.
Un lienzo de ventana completo consume 600 tiles / 19200 bytes de píxeles; hay que comprobar
su base de tiles y las ventanas coexistentes. Los textos requieren medir el ancho real.

Los callbacks de combate y equipo tienen una protección de traspaso reciente que se conserva.
No modificar SaveBlock ni posiciones de datos por una tarea visual. No recolorear sprites,
PS, estados, tipos o wallpapers usando una transformación global de paleta.
No reutilizar estados de emulador entre enlaces: cambian direcciones.

El menú actual tiene nueve acciones como máximo y misiones en R. Safari, enlace y otras
instalaciones tienen acciones especiales que deben mantenerse. La imagen no incluye todas
las funciones del proyecto y muestra botones X/Y inexistentes en GBA: usar A/B/L/R/START/SELECT.

## Dirección autorizada

Referencia: imagen del usuario del 22-10-2026; nombre FireRed UI Remastered. Verde esmeralda,
azul petróleo y turquesa; selección dorada, bordes de dos o tres tonos con esquinas de píxeles.
Se conserva la preferencia por marcos oscuros sin líneas blancas dominantes. La referencia guía
la composición, no promete colocar todas sus columnas a la misma escala dentro de 240×160.

Renovar estructura y jerarquía, no solo paletas. Mantener misiones, DexNav, Key System,
almacenamiento, objetos, reserva EXP y demás mejoras existentes; adaptar sus interfaces.
Los controles y callbacks del juego siguen siendo la autoridad funcional.

La inspección adicional confirma `POKEDEX_PLUS_HGSS=TRUE` y `HGSS_DARK_MODE=FALSE` en
`include/config/pokedex_plus_hgss.h`: el diseño visible lo implementa la extensión HGSS,
no solo pokedex.c. Hay que conectar su tema al Key System conservando búsqueda, hábitat,
estadísticas, evoluciones, formas, grito y tamaño; no reemplazarlo por una Pokédex simplificada.

## Verificación y entrega

Comparar capturas reales a resolución nativa y 2×, estados vacíos, texto largo, selección y
retornos. Probar equipo en combate normal/rápido y reemplazo tras debilitarse. Las pruebas de
almacenamiento, misiones y guardado deben seguir pasando. Las ROM de fixtures son desechables:
retirar sus símbolos antes de la ROM de entrega. Respaldar la anterior, no subir ROMs a GitHub,
y apagar Docker al finalizar.
