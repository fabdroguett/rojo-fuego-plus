# Misiones, almacenamiento y formas alternativas

Estado de esta tanda: código implementado; verificación por pruebas nativas y enlace ELF.
No se genera ni publica una ROM. La prueba de menús, gráficos y partidas en emulador queda
pendiente de la autorización para generar el `.gba`.

## Misiones

- Diario accesible con **START → R**, después de recibir la Pokédex, o mediante el objeto
  **Diario**. Un ayudante del laboratorio de Oak entrega el objeto, también en el postgame.
- **15 misiones** implementadas en tres capítulos: museo y fósiles, reliquias de Sete e
  investigación de Oak. El plan de contenido sigue siendo 350 misiones; no están todas hechas.
- **A** acepta o reclama; **SELECT** sigue una misión; **L/R** cambia de capítulo; **B** sale.
  El seguimiento abre el capítulo y la fila correspondientes al regresar al diario.
- Estados persistentes: disponible, aceptada, objetivo cumplido y premio reclamado.
  Se reservan 500 identificadores estables en 125 bytes; los ID publicados no deben reordenarse.
- Las visitas cuentan después de aceptar el encargo. Las investigaciones de especies revisan
  el equipo, excluyen huevos y reconocen formas como Unown. Los fósiles evolucionados también
  sirven para la investigación correspondiente. Ningún ejemplar se consume.
- Una mochila llena conserva el premio pendiente; reclamar dos veces no entrega dos premios.

## Almacenamiento

La mochila de objetos pasa de **30 a 60 espacios**, con el límite existente de 999 unidades
por pila. Los otros bolsillos conservan sus capacidades. Se mantiene el formato y ubicación
de los primeros 30 espacios; los adicionales se guardan al final de SaveBlock3, con inicialización
para partidas anteriores. La rotación de la clave de cifrado no atraviesa los límites del bolsillo.

Se corrigen la retirada excesiva de objetos repartidos en varias pilas, los accesos fuera de
capacidad y los fallos de asignación en las operaciones modificadas. La retirada de la última
unidad del PC compacta sus huecos; las cantidades inválidas no desbordan el contador.
El depósito de objetos del PC conserva sus **50 espacios**.

El PC de Pokémon incorpora **ordenar caja por Pokédex**. La ordenación es estable, deja huecos
al final y mueve registros completos, conservando datos, marcas y objetos. Exige soltar primero
el Pokémon u objeto que se esté moviendo.

La liberación masiva conserva Pokémon marcados, shiny, huevos, huevos malos, portadores de
objetos, legendarios, singulares, Ultraentes y usuarios de movimientos de campo. Cualquier marca
sirve para proteger un favorito. La liberación individual exige quitar marcas y objeto primero.
La confirmación masiva mantiene **No** como selección inicial. Una caja totalmente protegida
produce un aviso en lugar de informar una liberación inexistente.

La capacidad de Pokémon sigue en **14 cajas / 420 Pokémon**. Ampliarla requiere diseñar y probar
una migración de guardado; aumentar el tamaño permitido de ROM no amplía por sí solo la memoria
de guardado del cartucho.

## Objetos nuevos

| Objeto | Utilidad | Obtención |
| --- | --- | --- |
| Diario | Abre las misiones; se puede registrar | Ayudante de Oak |
| Recarga médica | Restablece los tres usos del Botiquín | Centro comercial de Azulona, 2.º piso; 3000 |
| Batería Omni | Recarga el Omni Ring sin cambiar sus módulos | Mismo mostrador; 5000 |
| Emisor Repel | Repelente reutilizable que se enciende y apaga | Mismo mostrador; 15000 |

Las recargas requieren su objeto correspondiente y no se consumen si ya está lleno.
El emisor utiliza la regla habitual del repelente: evita salvajes de nivel inferior al Pokémon
de referencia; no cancela todos los encuentros. Requiere tener el objeto y activar su opción.
El mismo mostrador también ofrece Éter y Elixir.

## Forma alternativa

**Castform de arena**, tipo Roca y estadísticas base sin aumento, cambia mediante Predicción
durante una tormenta de arena. Revierte siguiendo las reglas de cambio de clima, retirada,
debilitamiento y fin del combate. Conserva la identidad de Castform en la Pokédex y sus movimientos.
Se utiliza el icono y el aspecto exterior de Castform normal; no se agregó una especie independiente.

Arte de Mikitari del [Free Fakemon Pack](https://eeveeexpo.com/resources/1517/), publicado con
permiso de uso en proyectos. Créditos en `CREDITS.md`, hashes de los originales en
`graphics/pokemon/castform/sandy/source.json` y conversor reproducible en
`tools/rojo_fuego/import_castform_sand.py`. Solo se importó esta variante relacionada con una
especie existente.

## Verificación sin ROM

En un entorno Linux con las herramientas del proyecto:

```sh
python3 -m unittest discover -s tools/rojo_fuego/tests -v
bash tools/rojo_fuego/verify_code.sh
```

Las pruebas compilan el motor de misiones y funciones de inventario/protección reales con
ASan y UBSan. Cubren progresión, premios, migración, IDs inválidos, formas, huevos, mochila llena,
rotación de cifrado, límites de espacios, fallos de memoria y retiradas de varias pilas.
No reemplazan pruebas de ejecución en GBA/emulador ni certifican partidas antiguas reales.

El script solicita exclusivamente `pokefirered.elf`, comprueba el hash de una ROM si existe y
falla si aparece o cambia una. No ejecutar `make`, `make firered` ni el destino `.gba` para
esta verificación. Docker sirve para disponer del compilador Linux desde Windows.


Resultado del enlace de esta tanda: EWRAM 230036 B (87,75 %), IWRAM 28648 B (87,43 %),
región ROM del ELF aproximadamente 27,12 MB (80,82 % de 32 MiB). El símbolo de SaveBlock3
mide 1492 bytes de los 1624 disponibles. Estos tamaños corresponden al ELF; no se extrajo
un archivo `.gba`.
