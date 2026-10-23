# Selector de iniciales

Pedido del 23-10-2026: pantalla propia, generaciones 1–9 disponibles desde el inicio,
más alternativas elegidas por criterio y opción de habilidad oculta.

Se abre al interactuar con cualquiera de las tres Poké Balls durante la elección en el
laboratorio de Oak. Son **54 opciones, nivel 5**, en 18 grupos de tres, sin desbloqueos.

| Grupo | Opciones |
|---|---|
| Kanto | Bulbasaur, Charmander, Squirtle |
| Johto | Chikorita, Cyndaquil, Totodile |
| Hoenn | Treecko, Torchic, Mudkip |
| Sinnoh | Turtwig, Chimchar, Piplup |
| Teselia | Snivy, Tepig, Oshawott |
| Kalos | Chespin, Fennekin, Froakie |
| Alola | Rowlet, Litten, Popplio |
| Galar | Grookey, Scorbunny, Sobble |
| Paldea | Sprigatito, Fuecoco, Quaxly |
| Extras 1 | Beldum, Riolu, Gible |
| Extras 2 | Axew, Dratini, Larvitar |
| Extras 3 | Bagon, Deino, Goomy |
| Extras 4 | Jangmo-o, Dreepy, Frigibax |
| Extras 5 | Eevee, Ralts, Shinx |
| Extras 6 | Rockruff, Zorua, Absol |
| Extras 7 | Toxel, Charcadet, Pawniard |
| Extras 8 | Mareep, Trapinch, Noibat |
| Extras 9 | Togepi, Mimikyu, Duraludon |

- L/R cambia de grupo, con recorrido circular. Izquierda/derecha elige entre los tres.
- SELECT alterna habilidad normal y oculta; muestra el nombre de la habilidad real.
  La selección comienza en normal y se reinicia al cambiar de Pokémon o grupo.
- Deino, Zorua y Mimikyu conservan su habilidad propia: no se inventa una oculta.
- A abre confirmación, A confirma; B vuelve a elegir. B fuera de confirmación cierra
  sin recibir Pokémon y permite volver a abrir cualquiera de las Poké Balls.

Diseño nativo 240×160 con tema claro/oscuro. Revisión del 23-10-2026: cabecera con región
y generación, paginador de 18 grupos (el actual dorado y más alto), iconos de tipo por
tarjeta, doble anillo dorado y salto del Pokémon enfocado; los no elegidos se atenúan.
Fila de habilidad con selector NORMAL/OCULTA (desactivado si no existe), descripción real
y pie con glifos de botones. A abre una confirmación modal con especie, habilidad y nivel.
Cada repintado tarda ~3 fotogramas (antes ~12, perdía pulsaciones rápidas).
No introduce nuevos gráficos externos ni campos de guardado. Conserva el nombre opcional,
las banderas y las escenas posteriores. El rival conserva sus equipos de Kanto: las tres
columnas usan las ramas de historia hierba/fuego/agua (0/2/1); no se cambian sus equipos
a la generación del inicial elegido. Las especies alternativas pueden tener distinta
fuerza inicial; es una selección libre, no un nuevo modo de dificultad.

Implementación: `src/rf_starter_menu.c` y script del laboratorio. La habilidad se aplica
inmediatamente después de `givemon`, antes de apodos y diálogo del rival, validando la
especie recibida; no modifica el equipo al navegar o cancelar.

Validación reproducible: `tools/rojo_fuego/test/capture_starters.py` con el fixture
`starter_fixture.c` y mGBA. Recorre las 54 entregas desde el script de Oak con guardado
sintético, comprueba especie, habilidad, escena 3 y las tres ramas del rival; también
vuelve desde confirmación, cancela sin recibir Pokémon y captura claro/oscuro. El fixture
solo se copia a `src/` dentro del contenedor para probar y se retira antes de entregar.
