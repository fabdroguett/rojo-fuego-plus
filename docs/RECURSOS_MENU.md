# Recursos visuales de menús — investigación y propuestas

Ampliación del 23-10-2026: [28 fuentes de recursos de Internet](ASSETS_INTERNET.md), con
iconos, controles, paneles, fuentes y menús comunitarios; usos, licencias y adaptación GBA.

Pedido del usuario, 23-10-2026: buscar mejores sprites para mochila, Pokédex y otros menús,
o crearlos. Resultado: referencias verificadas y dos conceptos originales generados con
la herramienta integrada imagegen, guardados como fuentes visuales en el proyecto.

## Recursos encontrados

| Recurso | Contenido comprobado | Encaje en este proyecto |
|---|---|---|
| [Black & White Patch Collection for Fire Red](https://www.pokecommunity.com/threads/black-white-patch-collection-for-fire-red-hp-bars-all-menus-ev-iv-screen-and-more.542015/) | Mochila B2W2, Pokédex BW, equipo, ficha, PC, tienda y otros menús; créditos por recurso en la descarga | Referencia visual útil. El hilo describe parches y offsets para FireRed; nuestra base es expansion en modo FRLG, por lo que no se considera un parche directamente compatible |
| [Inventory filter icons — twiswist](https://opengameart.org/content/inventory-filter-icons) | Iconos 16×16, bolsa, monedas, candados, cuadrícula y filtros; licencia CC0; paleta Arne16 | Alternativa para iconos secundarios de inventario. No cubre por sí sola una Pokédex ni una familia completa de pantallas Pokémon |

El hilo de la colección enlaza los recursos originales de mochila y Pokédex; esas páginas
individuales no pudieron recuperarse en esta consulta. No se descargaron ni aplicaron parches.
Los créditos y condiciones de cualquier recurso externo se revisan antes de incorporarlo.

## Propuestas propias creadas

- [Mochila, concepto v1](../graphics/rojo_fuego/menu_art_v2/concepts/backpack-v1.png): cuerpo rojo,
  correas verde petróleo, detalles turquesa y herrajes dorados.
- [Pokédex, concepto v1](../graphics/rojo_fuego/menu_art_v2/concepts/pokedex-v1.png): dispositivo
  abierto rojo con pantalla turquesa, marco petróleo y controles dorados.
- [Prompts y procedencia](../graphics/rojo_fuego/menu_art_v2/PROMPTS.md).

Ambos archivos son PNG RGBA de **1312×1199**, con transparencia comprobada en las esquinas.
Son conceptos ampliados: **no son sprites de 48×48/64×64 ya indexados**, aunque esas escalas
lógicas se pidieron como dirección artística. Incluyen más detalle, colores y alfa parcial
que los formatos nativos. Requieren simplificación de píxeles, ajuste de paleta y alfa binario
antes de entrar al motor. No se han reemplazado los gráficos actuales ni modificado la ROM.

## Formato real que hay que respetar

Inspección local de los PNG y del código de la base actual:

| Uso | Recurso actual | Formato / contrato |
|---|---|---|
| Iconos del menú principal | `src/data/rf_menu_icons.h` (generado) | 16×16, máscaras de 1 bit; `RFUI_DrawIcon` las pinta con roles del tema. Fuente Kenney 1-Bit (CC0) y script `make_menu_icons.py` |
| Mochila | `graphics/bag/bag_male.png`, `bag_female.png` | 64×384, PNG indexado de 4 bits; seis cuadros de 64×64, offsets de animación en item_menu_icons.c |
| Tiles de fondo de mochila | `graphics/bag/menu.png` | Atlas 128×32 indexado; depende de tilemap y paletas propios |
| Pokédex HGSS | `graphics/pokedex/hgss/tileset_interface.png` | Atlas 32×512 indexado, más atlas de listas/búsqueda, tilemaps y paletas claros/oscuros |

La pantalla mide 240×160. Un PNG grande del dispositivo no sustituye un atlas ni una pantalla
funcional. Primero se preparan variantes nativas y después se integran con las funciones
existentes de mochila/Pokédex, sin reducir información o alterar navegación.

## Decisión y ejecución

Mantener una familia propia coherente con FireRed UI Remastered, usando los conceptos como
dirección artística y los recursos encontrados como comparación. El contraste del contenido
y la legibilidad a tamaño real prevalecen sobre el detalle de las imágenes ampliadas.

Trabajo pendiente ARTE-1–5 en [PLAN_COMPLETO.md](PLAN_COMPLETO.md): iconos nativos, cuadros de
mochila, adaptación de atlas HGSS, familia de menús relacionados y pruebas reales de integración.
Revisar claro/oscuro, selección, bolsillos, listas vacías, texto largo y retorno al mapa.
Las funciones existentes del DexNav/Pokédex y las protecciones de callbacks se conservan.

Actualización 26-10-2026: la paleta del sprite de mochila se adaptó a rojo, oro y sombra
petróleo en `graphics/bag/bag.pal`. Conserva ambas hojas indexadas de 64×384 y sus seis
cuadros; se verificaron con filtros, bolsillos y temas claro/oscuro en mGBA. El atlas HGSS
ya integrado se revisó en lista, búsqueda, hábitat, estadísticas, evoluciones, formas, grito
y tamaño. Pendiente de ARTE-2: redibujar las seis poses con la silueta del concepto, en
formato de 4 bits y manteniendo la animación existente.
