# Plan completo de Rojo Fuego Plus

Actualizado: 23-10-2026. El usuario pide completar el plan e incorporar **también para
implementación todas las ideas recogidas en IDEAS_OTROS_HACKS.md**. Este documento es la
lista de ejecución vigente; ROJO_FUEGO_PLAN.md conserva el historial y las decisiones técnicas,
e IDEAS_OTROS_HACKS.md conserva las referencias y el razonamiento de las adaptaciones.
Las recomendaciones antiguas de «evaluar» o «dejar para después» pasan a ser fases posteriores,
no exclusiones del alcance. Se mantienen las preferencias explícitas del usuario, incluyendo
no añadir dificultad a las selecciones de Imperium, Radical Red e Hyper Emerald.

Una casilla pendiente puede requerir implementación, completar una integración parcial o
validar código existente. No cuenta como terminada solo porque compile. Las funciones
repetidas entre hacks se implementan una vez y se comparten entre sistemas.

## Ejecución por dependencias — reestructurada el 23-10-2026

Las secciones 1–8 son el **catálogo de alcance**, no una secuencia que deba completarse
capítulo por capítulo. El orden operativo es la tabla siguiente. Cada paquete entrega una
función utilizable y comprobada; no se cierran sesiones por alcanzar una tanda pequeña.
Se continúa con el siguiente paso preparado mientras no exista un bloqueo real.

### Paquetes de trabajo

| Orden / ID | Entrega y alcance del catálogo | Depende de | Condición para pasar al siguiente |
|---|---|---|---|
| 0 / BASE | Estado real de funciones, inventario de recursos, reproducción de fallos de Noa y pruebas de referencia (§1–2) | Nada | Cada fallo tiene reproducción o registro de límites; rutas que se van a modificar tienen una prueba base |
| 1 / EVENTOS | Objetivos de misión por eventos, recompensas seguras y vínculo con logros (§3) | BASE | Una cadena combina diálogo, recogida/entrega y combate; guardar/cargar y premio pendiente funcionan |
| 2 / SERVICIOS | Preparación, reaprendizaje, tutores, formas, EV/IV, herramientas y consumibles (§4) | BASE; EVENTOS para premios de misión | Servicios accesibles y compatibles con progresión; cambios de Pokémon y cancelación comprobados |
| 3 / REGLAS | Perfiles y restauración de reglas/equipos temporales; ampliar Noa/defensa donde corresponda (§2, §5) | BASE | Entrar/salir, perder y guardar/cargar conservan equipo y reglas de historia |
| 4 / COMBATES | Alquiler, espejo, dobles, equipos variables, incursiones y jefes por fases (§5) | REGLAS; EVENTOS para objetivos/premios | Cada modalidad completa un ciclo jugable; equipos y premios definidos por datos |
| 5 / MUNDO | Registro compartido de encuentros/legendarios, radio, mapas contextuales, grutas y reapariciones (§2, §6) | BASE; EVENTOS | DexNav, radio y mapa consultan datos coherentes; renovación diaria y estados persistentes probados |
| 6 / EXPLORACIÓN | Cámaras/puzles, minería, acciones Omni Ring, obstáculos persistentes, Safari, VS Seeker y conexiones (§6) | EVENTOS + MUNDO | Primera zona completa con entrada, objetivos, salida y recuperación; después ampliar usando sus sistemas |
| 7 / CAMPAÑAS | Artefactos, personajes, rescates, tesoros, viajes, Sete, buceo, Porygon y expediciones (§3, §6) | EVENTOS + EXPLORACIÓN; COMBATES donde se use | Cadenas con desenlace y cambios de mundo; aumentar el catálogo hasta 350 misiones distintas |
| 8 / PRESENTACIÓN | Seguidor, vivienda, cosméticos, iniciales, estaciones, fondos, minijuegos, música y señalización (§7) | BASE; MUNDO para estaciones; EVENTOS para desbloqueos | Recursos completos para cada acción y estados claro/oscuro; recompensas conectadas |
| 9 / AMPLIACIONES | Nueva Partida+, innatas, megas propias, mundo abierto, otra región, gimnasio y viajes temporales (§8) | Sistemas que reutiliza cada ampliación | Progresión y compatibilidad definidas antes de añadir datos/mapas; recorrido propio validado |
| 10 / ENTREGA | Regresión integrada, traducción restante, ROM local y prueba Android (§9) | Todos los paquetes y almacenamiento | Funciones y cadenas completas verificadas; entrega sin fixtures y código sin binarios en Git |

La dependencia indica qué se necesita, no obliga a terminar todas las modalidades de un paquete
antes de usar una base ya comprobada. Ejemplo: una misión de combate puede avanzar cuando REGLAS
y su modalidad estén listas, sin esperar a todos los jefes legendarios.

### Almacenamiento: resolver pronto sin frenar el resto

- [ ] **SAVE-1, después de BASE:** calcular capacidad real, opciones de ampliación de cajas,
  memoria de trabajo y migración; dejar una decisión técnica reproducible.
- [ ] **SAVE-2:** implementar la opción viable y probar carga antigua/nueva, copia de Pokémon,
  cifrado/integridad, cajas vacías/llenas y operaciones masivas.
- [ ] **SAVE-3:** validar integración con PC portátil, favoritos, ordenación y recompensas.

SAVE-2 solo bloquea funciones que necesiten más capacidad o su nuevo formato; los servicios,
diálogos y cadenas compatibles con el guardado vigente pueden seguir. Todo campo persistente
nuevo se coordina con SAVE-1, evitando que cada sistema reserve memoria por su cuenta.

### Cómo reducir trabajo repetido

1. Auditar **antes de tocar cada sistema**. BASE no es una auditoría exhaustiva de todo el juego
   que impida empezar a implementar; prioriza código existente, fallos y límites compartidos.
2. Unificar requisitos, eventos y premios cuando se reutilicen. No crear un motor genérico
   enorme de antemano: extraer lo compartido al implementar el primer uso real.
3. Agrupar cambios que comparten archivos y pruebas. Preparación/tutores/formas usan servicios
   comunes; alquiler/espejo/dobles usan reglas temporales; radio/DexNav/mapa comparten encuentros.
4. Separar motores de contenido: primero una cadena variada completa; después ampliar por
   zonas con catálogos de datos y validadores. Objetivo orientativo: lotes de 15–25 misiones
   cuando sus sistemas estén disponibles; nunca rellenar cifras con visitas repetidas.
5. Escribir texto español y requisitos de arte al crear contenido. La fase final revisa y
   completa; no concentra toda la traducción o los sprites faltantes al final.
6. Mantener una sola integración activa. Registrar tareas independientes preparadas para
   continuar, sin abrir cambios incompatibles simultáneos ni duplicar trabajo entre sesiones.
7. No repetir compilaciones completas o recorridos ajenos al cambio sin motivo: compilar y
   probar el sistema afectado por cada cambio coherente; ampliar regresión al cerrar paquete,
   cambiar guardado/motor compartido o aparecer un fallo nuevo. Mantener controles obligatorios.
8. Usar guardados sintéticos y recorridos reproducibles para entrar directamente a cada función.
   Recalcular símbolos tras enlaces y recrear estados del emulador cuando corresponda.

### Seguimiento y siguiente trabajo

Cada paquete registra cuatro estados: **pendiente → en curso → implementado → validado**.
Además anota dependencia, archivos, prueba realizada y siguiente acción concreta. El código
existente empieza como implementado cuando hay evidencia, sin marcarlo validado por defecto.

| Trabajo preparado | Estado inicial | Siguiente acción |
|---|---|---|
| BASE | Pendiente de consolidación | Reproducir victoria/premio de Noa con equipo sintético obediente; aislar el caso de desobediencia y guardar la prueba |
| SAVE-1 | Pendiente | Medir distribución de guardado y memoria usada por cajas; comparar alternativas antes de cambiar estructuras |
| EVENTOS | Parcial: existen visitas/equipo/objetos y premios únicos | Añadir objetivos de diálogo/entrega/combate con una cadena de prueba; conservar IDs 0–34 |
| Validadores de contenido | Pendiente | Comprobar IDs, referencias, ciclos de requisitos, objetivos, textos y recompensas del catálogo |

Tras BASE se ejecutan SAVE-1 y EVENTOS en ese orden; después SERVICIOS. Si una tarea queda
bloqueada, se registra la causa y se continúa con la siguiente que no dependa de ella. Las
pruebas de Noa no justifican rehacer toda la interfaz ni posponer sistemas independientes.

El progreso se informa por entregas **validadas**, número de misiones únicas y pendientes
concretos. No se promete un porcentaje de aceleración ni una fecha sin mediciones de trabajo.

## 1. Consolidación y compatibilidad

- [ ] Auditar el código frente al historial y registrar por función: implementada, parcial,
  pendiente o validada. Sustituir los pendientes históricos que ya estén resueltos.
- [ ] Mantener un inventario único de flags, variables, campos persistentes y eventos.
- [ ] Diseñar migraciones de guardado antes de ampliar cajas y añadir sistemas persistentes;
  probar partidas sintéticas antiguas y nuevas, sin modificar partidas reales.
- [ ] Medir ROM, RAM, VRAM y tamaño de guardado en cada ampliación importante.
- [ ] Corregir fallos observados y conservar las funciones actuales durante las integraciones.

## 2. Funciones existentes que deben conservarse y validarse

Ya hay código para EXP al capturar, Exp. All, claves, Omni Ring, mecánicas modernas,
Pokémon de generaciones 1–9, seguidores, encuentros visibles, brotes, ciclo horario,
DexNav ampliado, PC portátil, botiquín, registro múltiple de objetos, entrenamiento,
revanchas y 325 logros. No se contabilizan como nuevas implementaciones pendientes.

- [ ] Completar pruebas funcionales y de guardado de reserva de EXP, botín, colección del
  DexNav, mochila ampliada, protección/ordenación del PC y sus opciones de desactivación.
- [ ] Validar las grutas diarias existentes y su renovación, recompensas y cambio de día.
- [ ] Validar defensa del título existente: requisitos, aspirantes, victoria, derrota y registro.
- [ ] Completar la validación de Noa: victoria, premio diario, cambio de ciudad y limpieza
  de reglas; investigar el bloqueo observado durante la prueba con desobediencia.
- [ ] Auditar el rastreador de legendarios existente antes de ampliarlo a un archivo completo.
- [ ] Revisar cobertura de la Pokédex HGSS: evoluciones, movimientos, formas y hábitats.
- [ ] Completar la revisión del tema oscuro y de iluminación en todas las pantallas y estados.
- [ ] Validar logros, títulos, recompensas y regeneración del archivo local RetroAchievements.

## 3. Misiones, personajes y colecciones

- [ ] Completar **350 misiones únicas**: 35 implementadas, 315 restantes. Mantener IDs estables.
  Distribución objetivo: personajes 70, exploración 60, Oak/hábitats 60, rescates 45,
  combate 50, legendarios/posjuego 40 y finales con objetivos propios 25.
- [ ] Añadir objetivos por diálogo, captura, combate, recogida, entrega y uso de objetos;
  cadenas con cambios visibles, seguimiento y pistas para retomar partidas.
- [ ] Completar diario por capítulos, filtros de zona/dificultad/disponibilidad, progreso,
  coleccionables individuales y ocultación del contenido aún no descubierto.
- [ ] Crear campaña de artefactos entre museo de Plateada y Sete: pistas, fragmentos,
  investigaciones, expediciones y desenlace; no contar cada fragmento como misión completa.
- [ ] Añadir historias de Silph, Bill, Oak y visitantes de otras regiones con combates opcionales.
- [ ] Añadir guías progresivas mediante Bill y ayudantes, y diálogos que reconozcan resultados
  anteriores, incluyendo continuidad de personajes como Cubone.
- [ ] Crear escondites, rescates, carreras y minijuegos con personajes/Pokémon.
- [ ] Integrar eventos de misión y logros sin duplicar progreso ni premios.
- [ ] Añadir encargos repetibles por separado; la capacidad de 500 no sustituye la meta de 350.

## 4. Preparación, herramientas y almacenamiento

- [ ] Ampliar las cajas de Pokémon con migración segura: actualmente 14 cajas / 420 plazas.
- [ ] Añadir editor opcional de EV/IV compatible con vitaminas, chapas y entrenamiento extremo.
- [ ] Auditar y completar reaprendizaje desde el equipo: nivel, huevo y preevoluciones.
- [ ] Añadir servicios claros de preparación y crianza en Centros Pokémon: naturaleza,
  habilidades, entrenamiento y acceso a herramientas existentes.
- [ ] Crear zonas de entrenamiento por estadística y ajuste de EV accesible.
- [ ] Unificar reloj, repelente, botiquín y PC en un menú de herramientas.
- [ ] Auditar restauración de consumibles equipados tras entrenadores; completar su clave
  opcional y verificar bayas y objetos consumidos o retirados.
- [ ] Centralizar tutores y terminales por categorías con compatibilidad y costes visibles.
- [ ] Añadir recompensa temática de tutor permanente después de cada gimnasio.
- [ ] Crear laboratorio de formas/fusiones con NPC, requisitos y explicaciones en la Pokédex.
- [ ] Mejorar el selector de cuatro objetos registrados con una vista gráfica.

## 5. Reglas y desafíos de combate

- [ ] Crear perfiles Casual, Clásico, Reto y Personalizado, con reglas de objetos, EV e IV.
- [ ] Diseñar equipos alternativos de líderes y Liga por rol y dificultad, incluyendo objetos,
  movimientos y estrategia; integrar equipos variables sin limitarse a cambiar niveles.
- [ ] Crear instalación de alquiler en Azulona tras tres medallas: tres Pokémon prestados,
  rachas, puntos y devolución segura del equipo propio tras salir, perder o guardar/cargar.
- [ ] Crear arena espejo aislada que intercambie temporalmente los equipos de los combatientes.
- [ ] Crear circuito de dobles y revanchas con equipos diseñados para ese formato.
- [ ] Añadir incursiones diarias de Sete, jefes y premios limitados por día.
- [ ] Crear desafíos legendarios por fases previos a capturas normales.
- [ ] Revisar balance de especies poco utilizadas mediante movimientos y habilidades como
  modo/proyecto separado; preservar las reglas de historia por defecto.

## 6. Exploración y mundo

- [ ] Crear Radio de Kanto: brotes, pistas de errantes y canal de encuentros limitado.
- [ ] Completar archivo de legendarios: siluetas, pistas y estados descubierto/desbloqueado/
  capturado; presentación de errantes y reaparición tras derrota/huida sin duplicar únicos.
- [ ] Crear cámaras opcionales y puzles en Monte Moon, Túnel Roca, Islas Espuma, Mansión y Sete,
  con pistas, reinicio y recuperación segura.
- [ ] Añadir minería y excavación de fósiles, piedras y tesoros, con encuentros y vínculo al museo.
- [ ] Persistir árboles cortados, rocas rotas y soluciones de puzles cuando corresponda.
- [ ] Añadir búsqueda del tesoro con desenlace legendario.
- [ ] Mejorar VS Seeker y revanchas del posjuego más allá de los líderes.
- [ ] Reducir tutoriales repetidos y revisar conexiones, tiendas y acceso a servicios.
- [ ] Añadir acciones contextuales del Omni Ring: cabezazos a árboles, escalada, perforación
  y crecimiento, con requisitos de medallas/misiones y sin ocupar movimientos.
- [ ] Ampliar Safari con biomas y modo de exploración sin límite de pasos configurable.
- [ ] Integrar carteles de encuentros, hábitats y conexiones en DexNav/mapa sin duplicar datos.
- [ ] Crear pequeñas islas, puertos y rutas nuevas de Sete; mapa contextual con destinos desbloqueados.
- [ ] Crear buceo, cuevas y ruinas submarinas de Sete.
- [ ] Crear cadena de Porygon/Silph/Bill y agujeros de gusano hacia destinos opcionales.
- [ ] Crear expediciones de 5–10 encuentros, rutas elegibles, recursos limitados y salida con premios.
- [ ] Conectar posjuego y viajes mediante objetivos y misiones, no solo encuentros aislados.

## 7. Presentación, personalización y actividades

- [ ] Ampliar interacción del seguidor: comentarios por zona/estado/eventos y acciones de campo.
- [ ] Crear vivienda y decoración en Paleta: recuerdos de logros, plantas y muñecos.
- [ ] Añadir protagonistas, ropa, cabello y colores con sprites completos para todas las acciones.
- [ ] Añadir selector de iniciales por región, compatible con historia, rival y primeras rutas.
- [ ] Añadir estaciones seleccionables o del reloj simulado: paletas, encuentros y misiones
  recuperables; evitar contenido perdido por calendario.
- [ ] Crear fondos propios para encuentros legendarios importantes.
- [ ] Añadir minijuegos de habilidad en Azulona, incluyendo ruleta y Snake, con premios cosméticos.
- [ ] Crear selector musical retro con arreglos y presupuesto de ROM comprobados.
- [ ] Añadir señalización shiny persistente además de sonido/color sin alterar probabilidades.
- [ ] Completar traducción al español, textos de ayuda y revisión de legibilidad a 240×160.

## 8. Ampliaciones mayores, también incluidas

Estos puntos pasan a fases tardías. Requieren diseño previo y se desarrollan sobre una base
estable; no deben hacer incompatible la partida principal de forma silenciosa.

- [ ] Nueva Partida+: definir qué se conserva y qué se reinicia, evitando premios duplicados.
- [ ] Habilidades innatas simultáneas como modo opcional, con IA e información de combate adaptadas.
- [ ] Megaevoluciones propias con arte, datos, balance e integración en herramientas.
- [ ] Exploración abierta: comenzar por posjuego y ampliar revisando barreras, scripts e historia.
- [ ] Otra región/campaña conectada por viajes, reutilizando protagonista y equipo de forma segura.
- [ ] Actividad de dirigir un gimnasio: aspirantes, progresión y decoración, como diseño propio.
- [ ] Expediciones temporales a escenarios antiguos como ampliación de la campaña de artefactos.

Tomar diseños de hacks de DS/Game Boy implica reimplementarlos en C para esta base. No es una
función adicional ni autorización para copiar binarios, historia, código o recursos sin revisar
licencias y créditos. Se conservan las advertencias de atribución del documento de investigación.

## 9. Orden de ejecución y cierre

El orden operativo y las dependencias están en **Ejecución por dependencias**, al principio
de este documento. No ejecutar las secciones del catálogo como fases rígidas. La entrega
PC/Android utiliza la ruta de emulador acordada y exige cerrar los siguientes controles.

- [ ] Por cada sistema: compilar sin errores/advertencias; pruebas apropiadas de lógica y
  recorrido en emulador; guardar/cargar, cancelar, perder y recuperar premios cuando corresponda.
- [ ] Probar integración entre funciones, partidas antiguas/nuevas y recorrido de historia/posjuego.
- [ ] Generar ROM local de entrega con respaldo, sin fixtures, y verificar arranque y cabecera.
- [ ] Probar controles, guardado y funciones de reloj en Android con el emulador elegido.
- [ ] Publicar únicamente código y documentación; mantener ROMs y guardados fuera de Git.

El port Android nativo sigue siendo una ruta técnica separada, pendiente de decisión según
lo acordado en el plan original. Completar este plan no depende de convertir el motor a SDL2.
No se fija una fecha ni un porcentaje global hasta auditar cada integración y sus dependencias.
