# Misiones, almacenamiento y formas alternativas

Estado de esta tanda: código implementado; verificación por pruebas nativas y enlace ELF.
El usuario autorizó generar la ROM tras completar la quinta tanda. El archivo queda local,
con respaldo de la versión anterior; las pruebas completas de menús y partidas siguen pendientes.

## Misiones

- Diario accesible con **START → R**, después de recibir la Pokédex, o mediante el objeto
  **Diario**. Un ayudante del laboratorio de Oak entrega el objeto, también en el postgame.
- **35 misiones** implementadas en seis capítulos: museo y fósiles, reliquias de Sete,
  investigación de Oak, refugios de Kanto, vida y hábitats, cuidado y evolución. El plan de contenido sigue siendo 350 misiones; no están todas hechas.
- **A** acepta o reclama; **SELECT** sigue una misión; **L/R** cambia de capítulo; **START** muestra el premio; **B** sale.
  La cabecera muestra las misiones reclamadas y el total del capítulo.
  El seguimiento abre el capítulo y la fila correspondientes al regresar al diario.
- Estados persistentes: disponible, aceptada, objetivo cumplido y premio reclamado.
  Se reservan 500 identificadores estables en 125 bytes; los ID publicados no deben reordenarse.
- Las visitas cuentan después de aceptar el encargo. Las investigaciones de especies revisan
  el equipo, excluyen huevos y reconocen formas como Unown. Los fósiles evolucionados también
  sirven para la investigación correspondiente. Ningún ejemplar se consume.
- Una mochila llena conserva el premio pendiente; reclamar dos veces no entrega dos premios.
- Continuación del 23-10-2026: cinco encargos de exploración después de **Archivo de Sete**,
  en las cámaras Liptoo, Weepth, Dilford, Scufib y Rixy. Completan el archivo de las siete
  cámaras. Recompensas en ese orden: 5 Ocaso Balls, 3 Repelentes Máximos, 1 Trozo Estrella,
  5 Acopio Balls y 1 Chapa Plateada. IDs 30–34; se conserva el progreso de las 30 anteriores.

## Almacenamiento

La mochila de objetos pasa de **30 a 60 espacios**, con el límite existente de 999 unidades
por pila. Los otros bolsillos conservan sus capacidades. Se mantiene el formato y ubicación
de los primeros 30 espacios; los adicionales se guardan al final de SaveBlock3, con inicialización
para partidas anteriores. La rotación de la clave de cifrado no atraviesa los límites del bolsillo.

Se corrigen la retirada excesiva de objetos repartidos en varias pilas, los accesos fuera de
capacidad y los fallos de asignación en las operaciones modificadas. La retirada de la última
unidad del PC compacta sus huecos; las cantidades inválidas no desbordan el contador.
El depósito de objetos del PC conserva sus **50 espacios**. **R** ordena por identificador de
objeto, dejando huecos al final y conservando cantidades y pilas; **SELECT** permite moverlos
manualmente. El control R se aplica en la lista normal, sin interferir con el movimiento manual.

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


Resultado del enlace de esta tanda: EWRAM 230220 B (87,82 %), IWRAM 28648 B (87,43 %),
región ROM del ELF aproximadamente 27,12 MB (80,84 % de 32 MiB). El símbolo de SaveBlock3
mide 1524 bytes de los 1624 disponibles. Estos tamaños corresponden al ELF. Tras la autorización más reciente se extrajo la ROM;
ver el registro de entrega en `ROM_2026_10_22.md`.


Segunda tanda: mantener **R** acelera la escritura de diálogos donde se permite acelerar
con A/B. No confirma páginas, no elige respuestas y no cambia la velocidad guardada en opciones.
La aceleración con R también puede apagarse con su clave. Se amplió la prueba del catálogo para detectar ciclos de requisitos y capítulos inválidos,
y se verifican cantidades y estabilidad de la ordenación de objetos del PC.


## Tercera tanda: botín de salvajes

La clave **Botín de salvajes** empieza activa y puede apagarse desde el Key System.
La cuarta tanda migra una vez los valores anteriores y conserva las desactivaciones posteriores. Solo tras ganar un combate salvaje normal,
cada rival debilitado que participó tiene un 25 % de probabilidad de entregar el objeto
que todavía conserva, si coincide con su objeto original. No se inventa un objeto cuando
el salvaje no llevaba ninguno. Se excluyen objetos importantes, robados, pendientes de
restauración por otra mecánica y objetos usados durante el combate.

No se aplica a capturas, huidas, derrotas, entrenadores, Safari, tutoriales, instalaciones,
combates grabados, enlaces ni encuentros con compañero. Una victoria con dos salvajes
puede entregar un objeto por cada rival elegible. No modifica niveles, IA ni equipos.

El combate avisa cuando se recoge botín. Si falta espacio, guarda cada unidad en una
**reserva persistente de ocho unidades**, añadida al final del guardado sin mover campos
anteriores. Se reclama automáticamente al abrir la mochila desde START o al ganar otro
combate salvaje normal, incluso si se apagó la clave después de obtenerlo. Cada unidad
se elimina de la reserva solo después de que la mochila acepte el objeto.

Con la reserva llena no se realizan nuevas tiradas hasta recuperar espacio. No es un
almacén ilimitado. Las pruebas cubren migración, filtros, dobles salvajes, probabilidad
determinista, propiedad de objetos, restauración, reclamaciones repetidas y falta de espacio.
La comprobación en emulador de mensajes y combate completo sigue pendiente.


## Cuarta tanda: valores por defecto y reserva de EXP

Por instrucción del usuario, todos los **interruptores booleanos** del Key System comienzan
activos. Incluye botín, combates rápidos, modo oscuro, emisor repelente, reserva de EXP y
aceleración de texto con R. Las funciones que requieren un objeto, una medalla o un módulo
siguen requiriéndolos; activar la clave no los entrega automáticamente. Dificultad y probabilidad
shiny son selectores de valores y mantienen su selección normal.

La migración, marcada por una versión propia en SaveBlock3, cambia una sola vez las cuatro
claves que antes empezaban apagadas. No se repite al cargar y no borra las personalizaciones
de las otras claves. EXP All mantiene su flag existente, activado en partidas nuevas.
Cada interruptor puede apagarse en su menú y la elección queda guardada.

El límite por medallas estaba configurado como `EXP_CAP_NONE` pese a existir la clave. En
FRLG ahora utiliza el límite duro por medallas. Desactivar **Límite de nivel** devuelve el tope
normal de 100; desactivar **Reserva de EXP** deja de guardar nueva EXP bloqueada. Los caramelos
respetan el mismo límite para no saltárselo desde la mochila. El saldo ya acumulado se conserva
si se apaga la reserva y puede seguir convirtiéndose.

La reserva guarda exclusivamente la diferencia entre la recompensa calculada y la EXP que
cabe hasta el límite, después de los multiplicadores. También recoge la recompensa elegible
que perdería un Pokémon al nivel 100. Excluye miembros que no tienen derecho a recibir EXP,
huevos y miembros no aptos, siguiendo la lógica de reparto del combate. No acredita otra vez
la EXP que sí recibe el Pokémon.

Al abrir la mochila desde START se convierte el saldo en caramelos existentes: XL (30000),
L (10000), M (3000), S (800) y XS (100), intentando primero los grandes. Cada lote se descuenta
solo después de que el inventario lo acepte. El resto inferior a 100 se mantiene; la mochila
llena conserva el saldo. Hay cinco intentos acotados por apertura, sin bucles por cada punto
de EXP. El máximo de reserva es **9 999 999 EXP**; al llenarse no acumula más hasta liberar saldo.
La descripción de la clave muestra el saldo actual.

Las pruebas adicionales ejecutan el código real de migración y de reserva con ASan/UBSan:
valores iniciales, interruptores persistentes, claves inválidas, tope por medallas, desactivación
del límite, conservación de EXP, saturación y mochila llena. Sigue pendiente la prueba completa
de combate, barras de experiencia, menús y partidas en emulador. La entrega actual sí incluye
una ROM autorizada con prueba básica de arranque.


## Quinta tanda: colección del DexNav

**START** en la pantalla del DexNav muestra el resumen en su panel derecho. **L/R** cambia
entre mañana, día, tarde, noche y la unión del día completo. **START** o **B** regresa a la
información del Pokémon; mientras se ve el resumen no inicia búsquedas accidentalmente.
La clave **Colección por zona** empieza activa y permite desactivar esta vista.

Incluye hierba, Surf, Golpe Roca, pesca con las tres cañas y encuentros ocultos. Cuenta especies
únicas según su número de Pokédex Nacional, sin duplicar slots, formas ni métodos en el total.
El numerador refleja las capturas ya registradas en la Pokédex, aunque se obtuvieran en otra
zona. No exige recapturar cada especie en cada lugar. Se utiliza la tabla de encuentros del
mapa y el fallback horario configurado; una tabla ausente o sin encuentros no inventa especies.
No pretende cubrir regalos, encuentros de scripts, errantes ni otros eventos ajenos a esas tablas.

El resumen se calcula al abrirlo/cambiar horario. No agrega campos al guardado ni altera las
especies, tasas o niveles de los encuentros. Las pruebas cubren formas, duplicados, cinco métodos,
unión de horarios, fallback, horarios desactivados, mapas vacíos y números de especie inválidos.
Esta tanda pasa nueve pruebas nativas y el enlace de todo el juego.
