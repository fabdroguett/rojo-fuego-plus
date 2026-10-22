# Ideas de otros hacks para Rojo Fuego Plus

Evaluación: 2026-10-22. Las propuestas se implementan por tandas autorizadas por el usuario; consultar el avance siguiente. Esfuerzo relativo estimado; falta auditar cada integración y el espacio real de ROM y guardado.

## Avance de implementación

La instrucción del usuario de implementar el plan y estas ideas autoriza las tandas descritas
abajo. Las propuestas restantes siguen pendientes; esta lista no significa que estén todas hechas.

- Primera tanda: diario de 15 misiones, favoritos/protecciones y ordenación del PC de Pokémon,
  mochila de 60 espacios y objetos de comodidad. Castform de arena como forma de especie existente.
- Segunda tanda: **30 misiones en total**, tres capítulos nuevos de refugios, hábitats y evolución;
  consulta anticipada de premios con START; ordenación de objetos del PC con R; aceleración
  de escritura manteniendo R inspirada en la comodidad propuesta para Imperium, preservando
  la confirmación de páginas y decisiones con A/B.
- Tercera tanda: botín configurable de salvajes, desactivado inicialmente; usa objetos equipados
  restantes, probabilidad del 25 %, aviso en combate y reserva persistente de ocho unidades
  cuando no cabe en la mochila. Recuperación al abrirla desde START o en una victoria posterior.
- Pendientes prioritarios: reserva de EXP, progreso del DexNav, grutas y
  retos diarios. No se modifican equipos ni dificultad como parte de estas tandas.


## Fuentes y límites

- https://pokemonheartsoul.com/features/ — reglas, radio, personalización. La propia página declara ser una web de fans; confirmar detalles en el código del autor antes de portar.
- https://pokemonsoulgold.com/features/ — exploración, progresión y posjuego.
- https://www.hackdex.app/ — devuelve HTTP 403; no se pudo revisar su catálogo ni atribuirle hallazgos.
- https://github.com/ShockSlayer/ccdocs/blob/master/docs/Documentation.md — documentación del autor de Crystal Clear (Gen 2).
- https://www.reddit.com/r/PokemonROMhacks/comments/1ogzapo/introducing_the_pool_system_pok%C3%A9mon_blaze_black_2/ — anuncio del creador de BB2/VW2 Redux (Gen 5); sistema anunciado en desarrollo, no asumir que está publicado.
- https://www.pokecommunity.com/threads/pokemon-emerald-rogue.479406/ — presentación de Emerald Rogue.
- https://www.pokecommunity.com/threads/pok%C3%A9mon-inclement-emerald-a-decomp-difficulty-hack-version-1-13.457039/ — presentación del autor de Inclement Emerald.

## Punto de partida local

El plan y el código incluyen DexNav ampliado, seguidores, encuentros visibles, ciclo horario, brotes, mecánicas modernas, Omni Ring, claves, botiquín, PC portátil, caramelos EXP, mentas y chapas, revanchas y logros. No se consideran novedades. La dificultad documentada cambia niveles +/-10%; diseñar equipos distintos sí sería una ampliación. La presencia de una opción en expansion no garantiza que esté integrada en FRLG.

## Propuestas prioritarias

| Idea | Referencia | Adaptación concreta | Esfuerzo |
|---|---|---|---|
| Reserva de experiencia | SoulGold, Candy Jar | Convertir EXP excedente al llegar al límite en caramelos; permitir incorporar nuevos miembros sin repetir combates | Medio |
| Grutas diarias | SoulGold, Hidden Grottos | Pequeños escondites en Kanto y Sete con encuentros y objetos renovables por día de juego | Medio |
| Defensa del título | SoulGold | Tras ser Campeón, enfrentar aspirantes diferentes en la Liga y registrar defensas | Medio-alto |
| Instalación de alquiler temprana | SoulGold, Battle Factory | Abrir en Azulona tras 3 medallas; elegir 3 Pokémon prestados y ganar puntos por rachas | Alto |
| Botín de salvajes | Heart & Soul | Opción que permite obtener objetos equipados al derrotar salvajes, con probabilidad y control de economía | Bajo-medio |
| Perfiles de reglas | Heart & Soul | Guardar configuraciones Casual, Clásico, Reto y Personalizado; ampliar las claves con reglas de objetos, EV e IV | Medio |
| Modo espejo | Heart & Soul | Arena opcional donde ambos jugadores usan el equipo del rival; no sustituir el equipo guardado de la historia | Medio-alto |
| Radio de Kanto | Heart & Soul | Boletín de brotes, pistas de errantes y canal que altera una selección limitada de encuentros | Medio |
| Equipos de jefes variables | BB2/VW2 Redux, Pool System anunciado | Selecciones diseñadas por rol para líderes y Liga; conservar identidad y documentar alternativas | Medio-alto |
| Retos diarios y entrenadores itinerantes | Crystal Clear | NPC que cambia de ciudad por día y propone combates con reglas especiales | Medio |
| Vivienda y decoración | Crystal Clear | Mejorar la habitación de Pueblo Paleta y colocar recuerdos de logros, plantas y muñecos | Alto |
| Dificultad por composición | Inclement Emerald como referencia de diseño | Cambiar equipos, objetos, movimientos y estrategia según modo; evitar depender solo de subir niveles | Alto |
| Expediciones cortas | Emerald Rogue como inspiración | Modo aparte de 5-10 encuentros con rutas elegibles, recursos limitados y recompensas al salir | Muy alto |

## Ideas adicionales

- Progreso de colección por ruta: contador por método y horario, conectado al DexNav; auditar primero cuánto cubre ya su interfaz.
- Jefes legendarios por fases: desafío previo a una captura normal; no limitarse a multiplicar PS. Coste alto y revisión del motor de combate.
- Aspectos y colores del protagonista: premio cosmético para logros, con sprites completos de caminar, correr, bici, surf y escenas.
- Selector de iniciales por región: dar más opciones en partida nueva sin cambiar la historia; equilibrar encuentros iniciales y rival.
- Tema oscuro y controles de iluminación nocturna: revisar todos los menús y paletas para mantener contraste.
- Favoritos y protección en el PC: preparar equipos con menos pasos; comprobar soporte existente antes de programar.
- Mejor interacción con el seguidor: comentarios por zona, estado y eventos; ampliar el sistema existente.
- Pokédex como guía interna de evoluciones, movimientos y hábitats: auditar primero las funciones incluidas en expansion.

## Qué dejaría para después

- Habilidades innatas simultáneas y megas propias: requieren rediseñar equilibrio, IA e interfaz, además de arte en el segundo caso.
- Mundo abierto total como Crystal Clear: obliga a revisar barreras, scripts y orden narrativo de Kanto. Primero probarlo en posjuego.
- Otra región completa: mucho trabajo de mapas, historia, encuentros y pruebas; es mejor ampliar Sete por etapas.
- Migrar código de hacks de DS o Game Boy: tomar el diseño y reimplementarlo en C de GBA; no son módulos compatibles.

## Orden recomendado

1. Reserva de EXP, botín configurable y progreso del DexNav por ruta.
2. Grutas y retos diarios, usando el reloj y los logros existentes.
3. Equipos de dificultad diseñados y defensa del título.
4. Instalación de alquiler y nuevas zonas de Sete.
5. Expediciones, cosméticos y mecánicas experimentales como modos separados.

Antes de implementar: verificar solapes de flags/vars de FRLG, guardado, presupuesto de ROM, recompensas y compatibilidad de partidas. Reutilizar ideas no implica que el código o los gráficos puedan trasladarse sin revisar su licencia y créditos.

## Sistema extenso de misiones (consulta del usuario)

Alcance solicitado por el usuario: ampliar a 240-350 o 500. Objetivo de diseño adoptado: **350 misiones**, ampliables a 500, además de encargos repetibles separados. Cantidad objetivo solicitada por el usuario; actualmente hay 30 implementadas.

| Tipo | Cantidad |
|---|---:|
| Historias secundarias de personajes | 70 |
| Exploración y misterios de Kanto/Sete | 60 |
| Investigaciones de Oak y hábitats | 60 |
| Rescates y ayuda a habitantes | 45 |
| Desafíos de combate | 50 |
| Cadenas de legendarios y posjuego | 40 |
| Finales de grandes cadenas | 25 |
| Total | 350 |

Las finales deben ser misiones distintas con objetivos propios, no contar dos veces el cierre de otra misión. Mezclar encargos breves y cadenas largas. La ampliación a 500 dependerá de añadir contenido con suficiente variedad.

- Historias de personajes, investigaciones de Oak, rescates, misterios, exploración y retos de combate.
- Cadenas con inicio, avances, desenlace y cambios visibles en el mundo; evitar convertir cada misión en una cifra de capturas.
- Diario: activas/completadas, requisitos, zona, pista, objetivos, progreso, premios y seguimiento de una misión.
- Recompensas: objetos útiles y cosméticos, encuentros o pequeñas zonas; reservar funciones esenciales para la progresión principal.
- Primer lote: unas 15 misiones completas para validar variedad, interfaz y guardado antes de ampliar.
- Integración con logros: compartir eventos cuando convenga, manteniendo el estado de misión independiente. No consumir automáticamente flags libres sin diseñar el almacenamiento.
- Eventos y enganches a considerar: conversar con NPC, entrar en mapa, capturar, vencer, recoger, entregar y usar objetos. Preferir actualización por evento y objetivos concretos.
- Prevenir bloqueos: entregas sin eliminar ejemplares favoritos por accidente, recuperación tras derrota, premios reclamables con mochila llena, pistas tras pausas largas y persistencia al guardar/cargar.
- Coste estimado: alto por el motor, interfaz y guardado; muy alto por escribir y probar 350 misiones. La cantidad final debe depender de contenido variado, no de repetir encargos.

## Unbound: evaluación específica

Fuente principal consultada: presentación de Skeli en https://www.pokecommunity.com/threads/pok%C3%A9mon-unbound-completed.382178/ . Algunos apartados antiguos de su FAQ contienen datos desactualizados; no tomar sus listas de contenido faltante como estado actual.

Ideas seleccionadas y adaptación propuesta (no implementadas):

1. Diario con objetivos y registro de coleccionables: base útil para nuestras 350 misiones; filtros por ciudad, dificultad y disponibilidad serían decisiones nuestras.
2. Misiones encadenadas y mejoras como recompensa: relacionar investigaciones de Oak y encargos de Silph con progresión opcional del equipo, sin retirar funciones ya disponibles.
3. Incursiones diarias: guaridas con jefes y premios; integrarlas como contenido de Sete y limitar recompensas por día de juego. Esfuerzo alto.
4. Minería: minijuego de fósiles, piedras y tesoros en Monte Moon/Túnel Roca; herramientas y tablas por zona serían diseño propio. Esfuerzo alto.
5. Mazmorras con puzles: nuevas cámaras opcionales de Islas Espuma, Mansión y Ruinas Sete; pistas y reinicio seguro. Esfuerzo medio-alto por zona.
6. Dificultad real: equipos, estrategia y reglas por modo, ampliando el ajuste actual de niveles. Esfuerzo alto por balance y pruebas.
7. Cosméticos: ropa, cabello y paletas como premios de cadenas, con recursos completos para cada acción. Esfuerzo alto de arte.
8. Posjuego con rutas de objetivos: conectar nuevas zonas, desafíos y legendarios con misiones en vez de encuentros aislados. Esfuerzo alto.
9. Nueva Partida+: idea secundaria; definir con precisión qué conservar, reiniciar progreso narrativo y evitar duplicación de premios. Requiere verificar comportamiento de Unbound en documentación actual antes de describirlo como equivalente.

Prioridad recomendada: diario/misiones → puzles y exploración → minería → incursiones → cosméticos y Nueva Partida+. El motor CFRU que usa Unbound y la base expansion de este proyecto requieren adaptar o reimplementar sistemas; no asumir compatibilidad directa.

## Crystal Advance Redux: evaluación específica

Fuente del autor Arnie, consultada 2026-10-22: https://www.pokecommunity.com/threads/pok%C3%A9mon-crystal-advance-redux.527988/ . No confundir Redux (hack binario) con repositorios de proyectos llamados PokemonCrystalAdvance.

Ideas y adaptación propuesta, no implementadas:

- **Estaciones y luz nocturna**: su entorno y fondos cambian por época y hora. Propuesta propia: estaciones seleccionables o ligadas al reloj simulado, con paletas, encuentros y misiones estacionales recuperables. Coste alto por arte y revisión de mapas.
- **Fondos de legendarios**: escenarios específicos para encuentros importantes. Propuesta: fondos propios para Mewtwo, aves y eventos de Sete; coste medio-alto de arte.
- **Circuito de combates dobles**: el autor indica líderes en dobles. Propuesta: torneo o revanchas con equipos diseñados para dobles, conservando el formato de historia por defecto. Coste medio-alto por equipos y balance.
- **Tutor tras cada gimnasio**: el hack ofrece un movimiento de tutor del Champ Guy al superar cada gimnasio. Propuesta: recompensa temática permanente por medalla. Coste bajo-medio y revisión de tutores ya disponibles.
- **Buceo y zonas submarinas**: propuesta de nuevas cuevas y ruinas bajo Sete, enlazadas a misiones y herramientas de exploración. Coste alto de mapas, navegación y contenido.
- **Casino con minijuegos**: el autor enumera ruleta y Snake. Propuesta: ampliar Azulona con juegos de habilidad y premios cosméticos. Coste medio-alto.
- **Selector musical retro**: inspirado en su GB Player; exige arreglos musicales y presupuesto de ROM, no solo una opción de menú.
- **Selección de protagonista**: ampliar personajes exige sprites de todas las acciones y escenas. Coste alto de arte.

No priorizar como novedades: seguidores, PC portátil, mentas, EXP All y grutas (estas últimas ya propuestas). Tampoco adoptar por defecto sus IV/EV y shiny odds: el sistema de Stat EXP que permite maximizar todas las estadísticas cambia el equilibrio de combate; la probabilidad shiny de 1/256 cambia el valor de la cacería. Podrían estudiarse como modos opcionales independientes.

Selección recomendada: tutores por gimnasio, fondos de legendarios y circuito de dobles primero; estaciones y buceo después. Tomar el diseño como referencia y crear una implementación para nuestra base.

## Emerald Imperium y Radical Red: comodidad y contenido

Pedido del usuario: evaluar excluyendo dificultad. No proponer cambios de IA, restricciones de mochila o equipos competitivos como parte de esta selección.

Fuentes primarias:
- Imperium, anuncio de su creador: https://www.reddit.com/r/PokemonROMhacks/comments/1i9202n/new_release_pokemon_emerald_imperium/
- Radical Red, hilo de soupercell: https://www.pokecommunity.com/threads/pok%C3%A9mon-radical-red-version-4-1-released-gen-9-dlc-pokemon-character-customization-now-available.437688/

Propuestas basadas en Imperium:
- Editor de EV/IV con interfaz clara, separado de la dificultad y opcional. Definir cómo convive con chapas y vitaminas existentes.
- Reaprender movimientos de nivel y huevo desde el equipo; auditar primero la configuración existente del recordador en nuestro proyecto.
- Acelerar texto manteniendo R; preservar decisiones y opciones para evitar aceptar cosas involuntariamente.
- Servicio de preparación en Centros Pokémon: naturaleza y acceso claro a herramientas, reutilizando mentas y parches existentes.
- Tutores accesibles mediante MT o un menú central; revisar economía y progresión.
- Encuentros con personajes visitantes de otras regiones como eventos de historia, con combates opcionales normales.

Propuestas basadas en Radical Red:
- Menú de herramientas: reloj, repelente y botiquín; el PC, botiquín y registro múltiple ya existen, así que la novedad sería un acceso unificado.
- Restaurar consumibles equipados después de combates contra entrenadores como clave opcional. El proyecto ya declara B_RESTORE_HELD_BATTLE_ITEMS GEN_LATEST; auditar alcance exacto antes de cambiar, especialmente bayas.
- Facilitar crianza y entrenamiento mediante servicios visibles; no asumir que los efectos modernos de habilidades sean nuevos.
- Zonas de entrenamiento por estadística y ajuste accesible de EV, sin subir la dificultad de la historia.
- Mejorar oportunidades para especies poco usadas mediante movimientos/habilidades como proyecto de balance separado, no trasladar todos los cambios de tipos de RR automáticamente.

MGM: puede ofrecerse como modo de preparación opcional, pero IV perfectos y EV eliminados cambian las reglas estadísticas. Para este usuario priorizar herramientas de edición y preparación sobre obligar a jugar sin EV.

## PokéClassic (fork de DaniRainbow)

Fuentes: https://github.com/DaniRainbow/pokeclassic y https://github.com/DaniRainbow/pokeclassic/releases . Se consultaron README, notas de versiones y data/scripts/follower.inc del código. El fork recrea Yellow sobre Emerald; no asumir compatibilidad directa con expansion.

Ideas con encaje local:
- Reacciones del seguidor y acciones de campo desde la interacción; ampliar nuestro seguidor existente.
- Minijuegos de personajes: las notas mencionan jugar a perseguirse con Tangela. Propuesta propia: escondite, rescates y carreras en parques de Kanto como misiones breves.
- Bill y ayudantes como guías: explicaciones progresivas y pistas para localizar NPC, mejoras y contenido de posjuego.
- Aprendizaje de movimientos de preevoluciones: evitar castigar al jugador por evolucionar pronto; auditar lo que ya trae expansion.
- Reaparición de legendarios: propuesta propia de revancha tras derrota/huida sin permitir duplicados únicos automáticamente. Las notas de Classic describen un sistema más permisivo y cotidiano.
- Microconsecuencias narrativas: visitas a Cubone y diálogos que reconocen progresos, como referencia para personajes que continúan su historia después de resolver su misión.

## Emerald Extended Cut

Fuentes secundarias: https://romhackguides.com/hacks/emerald-extended-cut/ y https://pokemonemeraldextendedcut.com/ . Esta última declara ser web de fans. Hackdex no fue accesible; no se verificó código ni publicación primaria del autor. No tomar cifras de mapas ni la función de dirigir un gimnasio como confirmadas por el creador.

- Referencia de diseño: conectar regiones mediante puertos y permitir exploración más libre. Propuesta propia: primero conectar pequeñas islas y nuevas zonas de Sete mediante expediciones.
- Distribuir Pokémon por hábitats adecuados, con rutas de investigación y un mapa que informe de conexiones.
- Arcos de viaje que reutilicen al protagonista y su equipo en otra campaña; incorporar una región completa sería un proyecto grande independiente.
- Idea a confirmar de la web de fans: dirigir un gimnasio. Como diseño propio puede convertirse en actividad de posjuego con aspirantes y decoración, pero no atribuir su implementación a Extended Cut sin más verificación.

## FireRed Reignited / LeafGreen Regrown

Fuentes del autor: https://github.com/Bruhcation/pokefireredlegacy y https://github.com/Bruhcation/pokefireredlegacy/releases . No confundir base y fork Jvtrovao/plus.

- Árboles cortados y rocas rotas permanentes; conservar soluciones de puzles cuando proceda. Revisión de flags y persistencia necesaria.
- Áreas opcionales de Monte Moon, Túnel Roca y Sete: crear cámaras y rutas propias conectadas a misiones.
- Búsqueda del tesoro que desemboca en un evento legendario, inspirada en su búsqueda de Celebi.
- Mejora del VS Seeker en posjuego y revisión de revanchas de entrenadores; nuestra revancha de líderes no cubre todo esto.
- Presentación de especies errantes para registrarlas y buscarlas, evitando que la búsqueda dependa de verlas al azar.
- Reducir tutoriales repetidos y mejorar conexiones, tiendas y acceso a servicios como una revisión de recorrido.

## Fire Red Extended (DjTarma)

Fuente primaria: https://www.pokecommunity.com/threads/pok%C3%A9mon-fire-red-extended-version.466535/ . Se tomó la versión de DjTarma, no otros hacks homónimos.

- Exploración mediante acciones: Headbutt en árboles, Rock Climb, Drill Run, Growth y otras. Propuesta: incorporar algunas como herramientas contextuales del Omni Ring, sujetas a medallas y misiones, sin ocupar movimientos.
- Excavación de fósiles con encuentros posibles: conectar paleontología, excavaciones y museo de Plateada.
- Agujeros de gusano ligados a investigación con Porygon: propuesta de cadena de Silph/Bill que abre destinos opcionales y encuentros especiales.
- Safari ampliado con más biomas y sin límite de pasos: modo exploración configurable con reglas y premios claros.
- Máquinas que enseñan grupos de movimientos en tiendas: propuesta de terminal de entrenamiento con categorías y explicación de compatibilidad.
- Carteles de encuentros por ruta: integrarlos en DexNav y mapas para evitar información duplicada.

Prioridad combinada para el proyecto: comodidad de preparación y navegación → misiones y NPC con continuidad → fósiles y nuevas cámaras de Sete → acciones de exploración → viajes y contenido de otras regiones. Todo sigue como evaluación; no se han implementado estas propuestas.

## Hyper Emerald: Lost Artifacts 5.7

Fuentes consultadas:
- https://nn-exe.github.io/Pokemon-Hyper-Emerald-5.7-QoL/ — guía comunitaria basada en datos extraídos del juego; no es documentación del equipo original.
- https://github.com/Nn-Exe/Pokemon-Hyper-Emerald-5.7-QoL — fuente primaria del parche comunitario English + QoL, no del hack original.

Distinguir el hack 5.7 de las extensiones del parche Nn-Exe. El diario, Quest Log y mejoras de interfaz descritas en ese repositorio corresponden al parche; Gen 9 y Area Zero se presentan como incorporaciones de su v1.9, no como funciones del 5.7 original. Algunos movimientos y habilidades nuevos usan aproximaciones según su README.

Ideas para nuestro juego, excluyendo dificultad:

1. **Campaña de artefactos**: cadena larga con reliquias, pistas y desenlace. Propuesta propia: ruinas de Sete y museo de Plateada como eje, con fragmentos e investigaciones que desbloqueen expediciones.
2. **Diario por capítulos** (referencia del parche QoL): integrar nuestras 350 misiones en capítulos con progreso, objetivos actuales y contenido futuro oculto.
3. **Archivo de legendarios** (parche QoL): siluetas, pistas y estados descubierto/desbloqueado/capturado. No mostrar automáticamente la solución completa.
4. **Colecciones con registro individual**: guardar cuáles artefactos se encontraron y pistas de los restantes, en lugar de un contador genérico. Evitar contar las piezas como misiones completas si solo se recogen.
5. **Viajes y mapa contextual** (guía/parche): mapa de la región o archipiélago actual con destinos previamente desbloqueados; desarrollar primero Sete, no otra región entera.
6. **Laboratorio de formas**: dar acceso claro a cambios de forma y fusiones existentes mediante NPC y explicación en la Pokédex; auditar primero qué trae nuestra base.
7. **Señalización shiny accesible** (parche QoL): indicador persistente visible además del color y sonido, manteniendo contraste de PS y textos. No exige cambiar probabilidades.
8. **Selector gráfico de objetos registrados** (parche QoL): posible mejora visual de nuestras cuatro ranuras, que ya funcionan mediante lista. Prioridad baja frente al contenido.

Recomendación: campaña de reliquias + diario por capítulos + archivo de legendarios. Los saltos temporales y expediciones a espacios antiguos serían diseño propio, con coste alto de mapas, escenas y pruebas. No implica copiar la historia de Volo/Arceus ni portar sus parches binarios a expansion.
