# Arte nativo «instrumento de campo de Kanto»

Maestros editables: dibujos por filas `ART`, flechas y retorno en
`tools/rojo_fuego/make_kanto_ui_art.py`. Cada símbolo representa un píxel final.
No se reducen ilustraciones ni se aplican filtros. Contorno de un píxel,
sombras interiores, reflejos discretos y transparencia binaria.

`python tools/rojo_fuego/make_kanto_ui_art.py` regenera PNG RGBA, PNG indexados,
tiles `.4bpp`, paletas BGR555 `.gbapal`, máscaras compartidas y cabeceras C.
Los nueve iconos del menú y las categorías/flechas de mochila son 16×16.
El retorno grande tiene su propio dibujo de 32×32. Los botones A/B/L/R y D-pad
se dibujan por separado a 8×8 para caber en la barra inferior.

Los originales del paquete `rojo_fuego_plus_menu_9` se conservan en
`reference_source/`. Los de mochila permanecen en `../bag_user/source/`.
`python tools/rojo_fuego/audit_kanto_ui_art.py` revisa los 100 glifos originales,
comprueba FNT contra JSON y produce comparaciones nativas y ampliadas en
`build_artifacts/ui/kanto-art/`.

Kanto Compact Pro usa máscaras de 5 píxeles de ancho en celdas de 6×8,
avance proporcional y 104 glifos revisados. `font-audit.json` contiene cada
fila y código de carácter del motor. Los caracteres sin código en el charmap
permanecen en el atlas Unicode; el renderer usa `?` como sustitución segura.
No se cambia ningún ID ni fuente global del motor.

`kanto.gbapal` es la paleta OBJ compartida de mochila. `menu_day.gbapal` y
`menu_night.gbapal` documentan los colores del menú BG; la autoridad de los
temas sigue siendo `src/rf_ui_theme.c`. Los índices 0 y 1–8 se reservan para
transparencia y superficies/texto; 6/7 y 9–15 dan contornos y tonos estables
a las ilustraciones. Las conversiones son exactas por índice, sin median cut.

Consumo fijo de mochila: 1792 B para 14 frames pequeños, 512 B para retorno,
32 B para una sola paleta OBJ. El menú dibuja en su ventana existente y no
crea sprites OBJ ni ventanas adicionales. Véase `docs/ui/main-menu-redesign.md`.
