# Sprites de mochila aportados por el usuario

Desde el 27-10-2026 la ROM utiliza los redibujos nativos de
`graphics/rojo_fuego/kanto_ui/`. Los originales de `source/` no se alteraron.
`legacy-import-manifest.json` conserva la conversión anterior; `manifest.json`
describe los recursos vigentes. Los comandos compatibles `import_bag_user_sprites.py`
y `make_bag_category_icons.py` regeneran el arte nativo sin reducir imágenes.
`import_bag_user_sprites.py --legacy-reference` exporta solo un diagnóstico a
`build_artifacts/ui/kanto-art/`, sin sobrescribir los recursos de producción.
El atlas vigente suma 2304 B y comparte **una** paleta de 32 B.

Lo siguiente documenta la importación histórica, sustituida por los redibujos:

Originales copiados desde `C:/Users/Fabian-Pc/Downloads/mochila_sprites_separados`.
Los PNG y su README se conservan sin cambios en `source/`.

Conversión reproducible: `python tools/rojo_fuego/import_bag_user_sprites.py`.
`make_bag_category_icons.py` se mantiene como entrada compatible al mismo conversor.
El resultado es `src/data/rf_bag_user_art.h`; `manifest.json` conserva tamaños,
recortes, SHA-256, grupos y paletas RGB555. No hay dependencias externas.

Se usa muestreo de vecino más próximo, transparencia binaria e índices de 4 bits.
Los sprites de 16×16 contienen el dibujo ajustado al espacio disponible; la flecha
grande de retorno usa 32×32. Tres paletas compartidas separan tonos cálidos,
iconos de color y retorno/scroll, evitando gastar una paleta por cada imagen.
Las hojas suman 2304 bytes y las paletas 96 bytes.

| Categoría | PNG |
|---|---|
| TODO y encabezado | `backpack_mochila.png` |
| CURA | `icon_bottle_purple.png` |
| COMB | `icon_pokeball_red.png` |
| EVOL | `icon_green_stone.png` |
| EQUIPO | `icon_key_gold.png` |
| TES. | `icon_chest_redgold.png` |
| OTROS | `icon_blue_disc.png` |

Las flechas de pestaña, cursor, scroll y retorno usan los PNG correspondientes.
`icon_gold_bundle.png` se incluye como variante de tesoro, sin crear otra categoría.
La geometría, los controles, los objetos reales del inventario y su clasificación
mantienen el comportamiento anterior.
