"""Authored campaign masters. Stable slots 350–413; shipped slots 0–349 stay intact.

Dialogue is deliberately separate from engine code and from the short journal objective.
No legendary, fossil, party member or main-story NPC is removed by these stories.
"""
ACTORS = {}
PLACES = {
 'PewterCity_Museum_1F':'Museo de Plateada, planta 1', 'Route4':'Ruta 4',
 'MtMoon_B2F':'Monte Moon, sótano 2', 'OneIsland_PokemonCenter_1F':'Centro Pokémon de Isla Uno',
 'SixIsland_RuinValley':'Valle Ruinas, Isla Seis', 'SevenIsland_TanobyRuins_MoneanChamber':'Cámara Monan, Tanoby',
 'CeruleanCity':'Ciudad Celeste', 'Route25_SeaCottage':'Casa de Bill, Ruta 25',
 'SilphCo_1F':'Silph, planta 1', 'CeladonCity':'Ciudad Azulona', 'PowerPlant':'Central de Energía',
 'FiveIsland_RocketWarehouse':'Almacén Rocket, Isla Cinco', 'LavenderTown':'Pueblo Lavanda',
 'LavenderTown_VolunteerPokemonHouse':'Casa de Fuji, Lavanda', 'Route12':'Ruta 12',
 'FuchsiaCity_SafariZone_Entrance':'Entrada del Safari, Fucsia',
 'PalletTown_ProfessorOaksLab':'Laboratorio de Oak, Paleta', 'ViridianForest':'Bosque Verde',
 'RockTunnel_B1F':'Túnel Roca, sótano 1', 'RockTunnel_1F':'Túnel Roca, planta 1',
 'SeafoamIslands_1F':'Islas Espuma, planta 1', 'FuchsiaCity_SafariZone_Office':'Oficina del Safari, Fucsia',
 'OneIsland_KindleRoad':'Camino Candente, Isla Uno', 'ViridianCity':'Ciudad Verde', 'Route3':'Ruta 3',
 'FuchsiaCity':'Ciudad Fucsia', 'ThreeIsland_BerryForest':'Bosque Baya, Isla Tres',
 'VermilionCity':'Ciudad Carmín', 'SaffronCity':'Ciudad Azafrán', 'SevenIsland':'Isla Siete',
 'FiveIsland_MemorialPillar':'Pilar Recuerdo, Isla Cinco', 'TwoIsland_CapeBrink':'Cabo Extremo, Isla Dos',
 'SevenIsland_SevaultCanyon':'Cañón Sétano, Isla Siete', 'SixIsland_GreenPath':'Camino Verde, Isla Seis',
 'Route22':'Ruta 22', 'CinnabarIsland_PokemonLab_ExperimentRoom':'Laboratorio de Canela',
 'PokemonTower_5F':'Torre Pokémon, planta 5', 'PokemonTower_6F':'Torre Pokémon, planta 6',
 'PokemonTower_7F':'Torre Pokémon, planta 7',
}


def actor(key, name, place, greeting, gfx='OBJ_EVENT_GFX_SCIENTIST'):
    ACTORS[key] = dict(name=name, place=place, greeting=greeting, gfx=gfx)
    return key


actor('alma','Alma','PewterCity_Museum_1F','Un museo conserva preguntas, además de respuestas. Soy Alma, la conservadora.')
actor('iria','Iria','Route4','Dibujo las marcas de las rocas sin extraerlas. Así otro investigador podrá encontrarlas igual.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('edda','Edda','MtMoon_B2F','Los fósiles no están aquí para que llenemos nuestras mochilas. Anotamos su posición y los protegemos.')
actor('nilo','Nilo','OneIsland_PokemonCenter_1F','Trabajo con el archivo de Celio. Una conexión sirve de poco si nadie comparte lo que descubre.')
actor('tala','Tala','SixIsland_RuinValley','Las ruinas de Sete tienen autores distintos. Parecerse no significa contar la misma historia.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('monan','Atril de Monan','SevenIsland_TanobyRuins_MoneanChamber','Las marcas del atril siguen el orden roca, agua y vida.', 'OBJ_EVENT_GFX_ITEM_BALL')
actor('leila','Leila','CeruleanCity','Soy mensajera. Antes de confiar en una dirección, compruebo quién la escribió.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('tomas','Tomás','Route25_SeaCottage','Ayudo a Bill con sus copias de seguridad. Él investiga; yo insisto en etiquetarlas.')
actor('ivo','Ivo','SilphCo_1F','Revisamos la red después de lo ocurrido en Silph. No queremos volver a confundir una orden con una amenaza.')
actor('lio','Lio','CeladonCity','Me pagaron por mover paquetes. Ahora quiero saber a quién perjudicaban.', 'OBJ_EVENT_GFX_HIKER')
actor('vera','Vera','PowerPlant','La Central puede albergar Pokémon sin convertirse otra vez en una trampa.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('terminal','Terminal aislada','FiveIsland_RocketWarehouse','La terminal está desconectada de la antigua red Rocket.', 'OBJ_EVENT_GFX_ITEM_BALL')
actor('rosa','Rosa','LavenderTown','Colaboro con el señor Fuji. Acompañar también significa saber esperar.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('ines','Inés','LavenderTown_VolunteerPokemonHouse','Este refugio recibe Pokémon sin separar a los entrenadores de sus compañeros.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('dario','Darío','Route12','Mantenemos los caminos tranquilos para quienes están aprendiendo a salir otra vez.', 'OBJ_EVENT_GFX_HIKER')
actor('zaira','Zaira','FuchsiaCity_SafariZone_Office','Observa primero y acércate después. El miedo no se cura persiguiendo a nadie.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('ada','Ada','PalletTown_ProfessorOaksLab','Oak reúne los registros; yo reviso si nuestras conclusiones respetan lo que observamos.')
actor('silva','Silva','ViridianForest','Si callamos un momento, el bosque deja de parecer vacío.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('bruno','Bruno, el geólogo','RockTunnel_B1F','No soy el miembro del Alto Mando. Mis combates son con las libretas mojadas.', 'OBJ_EVENT_GFX_HIKER')
actor('mar','Mar','SeafoamIslands_1F','Las aves buscan corrientes, no fronteras. Anotamos sus rutas sin bloquear sus nidos.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('salo','Salo','FuchsiaCity_SafariZone_Office','Ningún informe vale más que la seguridad de los Pokémon que estudiamos.')
actor('nora','Nora','OneIsland_KindleRoad','El calor cambia el camino, pero también dónde descansan los Pokémon.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('lina','Lina','ViridianCity','Coordinamos auxilio, no hazañas. Avisar a tiempo suele ser lo más importante.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('paz','Paz','Route3','Estoy organizando las provisiones de los excursionistas.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('teo','Teo','RockTunnel_1F','No moveré al herido sin comprobar primero que el camino está despejado.', 'OBJ_EVENT_GFX_HIKER')
actor('ruben','Rubén','FuchsiaCity','Aviso a las familias cuando una patrulla vuelve. Una espera sin noticias también pesa.', 'OBJ_EVENT_GFX_HIKER')
actor('yara','Yara','ThreeIsland_BerryForest','La patrulla conoce los senderos; aun así dejamos señales para quien venga detrás.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('hana','Hana','VermilionCity','Vengo de Johto para intercambiar técnicas, no para demostrar que una región es mejor.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('ken','Ken','CeladonCity','En Hoenn aprendí a mirar el clima antes de elegir un movimiento.', 'OBJ_EVENT_GFX_HIKER')
actor('mael','Mael','SaffronCity','Traje apuntes de Sinnoh. Los mejores huecos del cuaderno son los que deja otro entrenador.', 'OBJ_EVENT_GFX_HIKER')
actor('sara','Sara','SevenIsland','En Teselia hablamos mucho de ideales. Aquí estoy aprendiendo a escuchar antes de combatir.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('mara','Mara','VermilionCity','Hago inventario de los muelles. Un tesoro sin procedencia puede ser la pérdida de otra persona.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('baliza','Baliza antigua','FiveIsland_MemorialPillar','La inscripción pide recordar a quienes no regresaron.', 'OBJ_EVENT_GFX_ITEM_BALL')
actor('faro','Faro de los recuerdos','TwoIsland_CapeBrink','La placa del camino tiene marcas de antiguas mareas.', 'OBJ_EVENT_GFX_ITEM_BALL')
actor('gael','Gael','SevenIsland_SevaultCanyon','Preparamos expediciones cortas y una vuelta segura. Llegar no basta.', 'OBJ_EVENT_GFX_HIKER')
actor('belen','Belén','SixIsland_GreenPath','Una expedición termina cuando todas las personas han vuelto.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor('riel','Riel','Route22','Entrenamos orientación entre dos puestos. Si pierdes el ritmo, vuelve a empezar.', 'OBJ_EVENT_GFX_HIKER')


def q(name, giver, target, objective, offer, ending, *, kind='talk', proof=None, flag='0', reward='ITEM_GREAT_BALL', quantity=2,
      questions=(), seconds=0):
    """questions: (yes/no question, expected answer) for puzzles; seconds: race time limit."""
    assert (kind=='puzzle')==bool(questions) and (kind=='race')==bool(seconds),name
    return dict(name=name,giver=giver,target=target,objective=objective,offer=offer,ending=ending,
                kind=kind,proof=proof,flag=flag,reward=reward,quantity=quantity,questions=list(questions),seconds=seconds)


CAMPAIGNS = [
('MEMORIA DE KANTO','Alma no busca un arma antigua: quiere devolver a cada hallazgo su contexto.',[
q('Un museo con preguntas','alma','iria','Habla con Iria en Ruta 4.','Una placa llegó sin procedencia. Iria copia marcas en Ruta 4; pregúntale si reconoce su dibujo. No retires nada de la montaña.','Iria reconoce el trazo de una antigua cantera. La investigación empieza con una procedencia, no con una leyenda.'),
q('La cantera callada','iria','edda','Muestra una Piedra Lunar a Edda, en Monte Moon B2.','La placa salió de una escombrera. Edda compara rocas en Monte Moon. Una Piedra Lunar ayudará a separar las marcas naturales de las talladas.','Edda distingue tres técnicas. Una sola pieza no demuestra que toda Kanto perteneciera a un mismo pueblo.',proof=('QUEST_COLLECT_ITEM','ITEM_MOON_STONE','0')),
q('Tres huellas, un informe','edda','edda','Revisa los tres atriles del cuaderno.','Compara los atriles de Monte Moon B2, Túnel Roca B1 y el laboratorio de Canela. Anota los tres antes de presentar una conclusión.','Los tres calcos quedan archivados juntos. Las marcas viajaron con sus artesanos; no eran un mapa de conquista.',kind='clues',flag='FLAG_BADGE07_GET',reward='ITEM_RARE_CANDY',quantity=2),
q('El ámbar no cuenta todo','edda','alma','Lleva Aerodactyl al museo de Plateada.','El laboratorio puede recuperar un Aerodactyl del Ámbar Viejo. Muéstralo a Alma: estudiar un ser vivo exige cuidado, no solo entusiasmo.','Alma separa hechos y conjeturas en la exposición. Tu Aerodactyl permanece contigo; el museo conserva únicamente sus observaciones.',proof=('QUEST_PARTY_EITHER','SPECIES_AERODACTYL','SPECIES_NONE')),
q('Un puente hacia Sete','alma','nilo','Entrega el informe a Nilo en Isla Uno.','Las copias se compartirán con el archivo de Celio. Cuando seas Campeón, busca a Nilo en el Centro Pokémon de Isla Uno.','Nilo recibe el informe y permite contrastarlo con los archivos de Sete. No necesita extraer una sola piedra de las ruinas.',flag='FLAG_IS_CHAMPION'),
q('Un alfabeto, muchas voces','nilo','tala','Muestra Unown a Tala, en Valle Ruinas.','Tala estudia cómo cambian los signos entre islas. Lleva un Unown para comparar sus formas; no lo entregarás ni lo dejarás en las ruinas.','Tala encuentra diferencias entre los signos. Los Unown ayudan a leerlos, pero no convierten una interpretación en verdad absoluta.',proof=('QUEST_PARTY_EITHER','SPECIES_UNOWN','SPECIES_NONE')),
q('El atril de Monan','tala','monan','Resuelve el orden del atril de Monan.','El atril ordena tres etapas: primero roca, después agua y al final vida. Lee sus preguntas en la Cámara Monan. Puedes volver a intentarlo.','La secuencia abre el índice de la cámara, no una puerta sellada. El índice describe intercambios entre comunidades, sin un reino perdido.',kind='puzzle',questions=(
    ('Recuerda: roca, agua y vida. ¿La roca va antes que el agua?',True),
    ('Recuerda: roca, agua y vida. ¿La vida va antes que la roca?',False),
    ('Recuerda: roca, agua y vida. ¿La vida ocupa el último lugar?',True))),
q('Una memoria compartida','tala','alma','Presenta las conclusiones en el museo.','Lleva las conclusiones a Alma. La exposición debe explicar qué sabemos, qué ignoramos y por qué las piezas siguen en sus lugares.','La sala dedica su nueva cartela a las comunidades de Kanto y Sete. El expediente queda abierto a futuras preguntas, pero esta investigación concluye.',reward='ITEM_BOTTLE_CAP',quantity=1),
]),
('LA RED Y SUS VOCES','Una fuga de datos obliga a Bill y Silph a reconstruir confianza, no solo cables.',[
q('Una dirección prestada','leila','tomas','Consulta a Tomás en la casa de Bill.','Una remesa usa la dirección de Bill sin permiso. Leila teme que alguien esté suplantando a sus clientes. Conserva los datos y consulta a Tomás.','La dirección fue copiada de un archivo viejo. Tomás advierte a Bill y separa esa copia de la red actual.',flag='FLAG_BADGE02_GET'),
q('Copias con dueño','tomas','ivo','Habla con Ivo en Silph, tras seis medallas.','Cuando Silph vuelva a trabajar con normalidad, Ivo podrá revisar el envío. No entres en zonas cerradas para adelantar la investigación.','Ivo conserva el registro y confirma que un intermediario de Azulona usó el archivo. Las comunicaciones personales quedan fuera del análisis.',flag='FLAG_BADGE06_GET'),
q('El último paquete','ivo','lio','Vence a Lio en su práctica de Azulona.','Lio aceptó transportar paquetes sin conocer su contenido. Quiere comprobar si puedes acompañarlo sin depender de quienes le pagaban. Su combate es voluntario.','Lio rompe su contrato y entrega las etiquetas. No revela direcciones privadas: solo la ruta de los paquetes.',kind='battle',proof=0),
q('Nadie es una contraseña','lio','tomas','Devuelve las etiquetas a Tomás.','Las etiquetas parecen señalar a un Pokémon digital. Lleva una copia a Tomás; perseguir a un Porygon no es una forma de pedir ayuda.','Bill y Tomás sustituyen las claves expuestas. El problema era el permiso de acceso, no la existencia de Porygon.'),
q('Energía sin jaulas','tomas','vera','Muestra Magnemite o Magneton en la Central.','Vera necesita comprobar una instalación sin capturar a los Pokémon que viven allí. Acércale un compañero eléctrico dispuesto a colaborar.','Vera prueba una alimentación aislada. La Central conserva su hábitat y deja de ser una excusa para encerrar Pokémon.',proof=('QUEST_PARTY_EITHER','SPECIES_MAGNEMITE','SPECIES_MAGNETON')),
q('Tres nodos, una copia','vera','ivo','Revisa los nodos de Silph, Bill y Canela.','Antes de conectar nada, revisa los nodos de Silph 1F, la casa de Bill y el laboratorio de Canela. Las tres comprobaciones forman un único informe.','Ivo recibe las tres verificaciones. La nueva copia tiene procedencia y permisos claros; una avería no borrará el trabajo de los demás.',kind='clues',reward='ITEM_ULTRA_BALL',quantity=3),
q('Un colaborador digital','ivo','tomas','Registra Porygon y habla con Tomás.','Registra a Porygon en tu Pokédex y consulta a Tomás. El estudio utiliza tus observaciones; tu Pokémon no se transfiere a ningún ordenador.','Tomás documenta cómo colaborar con Porygon sin confundirlo con una herramienta desechable.',proof=('QUEST_DEX_CAUGHT','SPECIES_PORYGON','0')),
q('Cerrar la puerta de atrás','tomas','terminal','Aísla la terminal del almacén de Isla Cinco.','Ya como Campeón, revisa la terminal del antiguo almacén Rocket de Isla Cinco. Debes llegar por los accesos normales de tu aventura.','La terminal queda aislada y el archivo limpio vuelve a Silph. Bill conserva una copia de emergencia. La red ya no depende de una clave robada.',flag='FLAG_IS_CHAMPION',reward='ITEM_BOTTLE_CAP',quantity=1),
]),
('UNA PUERTA ABIERTA','Cubone recupera decisiones y compañía; el duelo nunca se convierte en un objeto que entregar.',[
q('Después de la torre','rosa','ines','Visita a Inés en el refugio de Fuji.','Cuando el señor Fuji esté a salvo, visita a Inés en su casa de Pueblo Lavanda. Un Cubone del refugio evita los ruidos del exterior.','Inés te presenta al pequeño sin forzarlo a acercarse. Ayudar comienza por respetar la distancia que pide.',flag='FLAG_RESCUED_MR_FUJI'),
q('A la misma altura','ines','rosa','Lleva Cubone o Marowak junto a Rosa.','Un compañero de su especie puede hacer la visita más tranquila. Lleva Cubone o Marowak a Rosa, sin entregarle tu Pokémon.','Rosa observa un primer acercamiento. Tu compañero vuelve contigo; el pequeño del refugio empieza a reconocer voces amigas.',proof=('QUEST_PARTY_EITHER','SPECIES_CUBONE','SPECIES_MAROWAK')),
q('Un camino tranquilo','rosa','dario','Consulta el paseo de Ruta 12 con Darío.','Darío conoce un tramo tranquilo de Ruta 12. Pídele que revise el camino antes de organizar un paseo con el refugio.','Darío retira su equipo de pesca del paso. Inés podrá acompañar al grupo sin obligarlo a atravesar una multitud.'),
q('Algo que pueda elegir','dario','zaira','Entrega una Pokémuñeca a Zaira en Fucsia.','Zaira preparará objetos para que el pequeño elija cómo jugar. Lleva una Pokémuñeca a la oficina del Safari; se consume solo al confirmar.','Zaira incorpora la muñeca al material del refugio. Nadie exige que Cubone juegue con ella ni que deje su hueso.',kind='delivery',proof=('ITEM_POKE_DOLL',1)),
q('Cuidar a quienes cuidan','zaira','ines','Lleva un Antídoto al refugio de Lavanda.','La salida requiere un botiquín para todo el grupo. Inés necesita un Antídoto; comprarlo o encontrarlo sirve, y la entrega queda registrada.','El botiquín queda preparado. El cuidado cotidiano permite que el refugio organice más paseos sin asumir riesgos innecesarios.',kind='delivery',proof=('ITEM_ANTIDOTE',1)),
q('Tres nombres que recordar','ines','rosa','Lee los tres memoriales de la Torre.','Lee los memoriales de las plantas 5, 6 y 7 de la Torre Pokémon. Rosa reunirá las tres notas en un solo recuerdo, sin llevarse sus ofrendas.','Rosa prepara un recuerdo con permiso del refugio. La historia no borra la pérdida; deja espacio para hablar de ella.',kind='clues'),
q('La vuelta del paseo','rosa','dario','Vuelve al puesto de Darío en Ruta 12.','El grupo ha realizado su primer paseo acompañado. Pregunta a Darío cómo fue y lleva noticias a quienes esperan, sin buscar al pequeño por tu cuenta.','Darío cuenta que Cubone quiso regresar antes que los demás. El grupo lo respetó; elegir volver también es ganar confianza.'),
q('Una puerta abierta','dario','ines','Habla con Inés en la casa de Fuji.','El refugio tiene una nueva rutina. Visita a Inés para cerrar el informe y saber qué ha elegido hacer Cubone.','Cubone decide quedarse con Inés y salir con el grupo cuando le apetezca. Puedes visitarlo aquí. Tu Pokémon nunca fue entregado al refugio.',reward='ITEM_HEAL_BALL',quantity=5),
]),
('CUADERNOS DE CAMPO','Oak y Ada enseñan a observar sin convertir los hábitats en listas de trofeos.',[
q('Una ficha necesita contexto','ada','silva','Registra Caterpie y consulta a Silva.','La Pokédex de Oak necesita observaciones de campo. Registra Caterpie y visita a Silva en Bosque Verde para aprender cómo anotar su hábitat.','Silva registra luz, refugio y alimento. La ficha de Caterpie ya no es solo un número.',proof=('QUEST_DEX_CAUGHT','SPECIES_CATERPIE','0')),
q('La vida bajo nuestros pasos','silva','bruno','Registra Geodude y visita Túnel Roca B1.','Compara la vida del bosque con la de una cueva. El geólogo Bruno te espera en Túnel Roca B1 cuando tengas el registro de Geodude.',flag='FLAG_BADGE03_GET',proof=('QUEST_DEX_CAUGHT','SPECIES_GEODUDE','0'),ending='Bruno distingue huellas recientes de marcas antiguas. El grupo evita usar un nido como zona de descanso.'),
q('La orilla también es hogar','bruno','mar','Registra Seel y visita Islas Espuma 1F.','Mar estudia los lugares donde descansan los Pokémon de la costa. Registra Seel antes de consultar sus notas en Islas Espuma.',flag='FLAG_BADGE05_GET',proof=('QUEST_DEX_CAUGHT','SPECIES_SEEL','0'),ending='Mar añade zonas de descanso al informe. Los investigadores cambian su recorrido para no ocuparlas.'),
q('Hábitats sin encierro','mar','salo','Consulta a Salo con el registro de Nidoran hembra.','El Safari necesita comparar sus cuidados con los hábitats silvestres. Registra Nidoran hembra y habla con Salo en la oficina de Fucsia.',proof=('QUEST_DEX_CAUGHT','SPECIES_NIDORAN_F','0'),ending='Salo corrige el plano de refugios. La conclusión distingue lo observado de lo que aún debe comprobarse.'),
q('Lo que atrae la corriente','salo','vera','Registra Voltorb y visita a Vera.','El comportamiento eléctrico también depende del entorno. Registra Voltorb y contrasta sus lugares de descanso con Vera en la Central.',proof=('QUEST_DEX_CAUGHT','SPECIES_VOLTORB','0'),ending='Vera anota distancias seguras para sus pruebas. La investigación de Oak y la reparación de la red comparten observaciones, no Pokémon.'),
q('La sombra junto al calor','vera','nora','Registra Ponyta y visita Camino Candente.','Nora compara los refugios cálidos de Isla Uno. Registra Ponyta y lleva la ficha a Camino Candente cuando puedas viajar a Sete.',flag='FLAG_BADGE07_GET',proof=('QUEST_DEX_CAUGHT','SPECIES_PONYTA','0'),ending='Nora encuentra que descanso y temperatura deben anotarse juntos. Un dato aislado no explica todo el comportamiento.'),
q('Un cuaderno sin atajos','nora','ada','Devuelve los cuadernos al laboratorio de Oak.','Ada reunirá las notas de bosque, cueva, costa, Safari, Central e islas. Vuelve al laboratorio de Paleta para revisar sus diferencias.','Ada conserva las discrepancias en vez de borrarlas. El archivo de Oak explica cómo se obtuvo cada dato.'),
q('La siguiente pregunta','ada','ada','Revisa con Ada el archivo de Paleta.','Revisa las conclusiones con Ada. Los demás registros de Pokédex seguirán ampliando el archivo; no tienes que completarlos todos para cerrar esta campaña.','Oak recibe el estudio y Ada firma también tu trabajo. La investigación concluye con una metodología que otros podrán repetir.',reward='ITEM_RARE_CANDY',quantity=2),
]),
('NADIE SE QUEDA ATRÁS','Una red de auxilio aprende a prevenir extravíos y a dar noticias a las familias.',[
q('Un aviso a tiempo','lina','paz','Lleva una Poción a Paz en Ruta 3.','Un excursionista pidió ayuda antes de entrar en Monte Moon. Paz organiza el puesto de Ruta 3 y necesita una Poción para su botiquín.','Paz prepara el puesto sin enviar a nadie solo a la cueva. Avisar a tiempo ha evitado una búsqueda nocturna.',kind='delivery',proof=('ITEM_POTION',1),flag='FLAG_BADGE01_GET'),
q('La señal de regreso','paz','iria','Consulta las señales de Ruta 4 con Iria.','Paz ha recibido dos indicaciones distintas sobre una salida. Iria está comprobando las señales de Ruta 4; habla con ella antes de mover al grupo.','Iria corrige una copia del plano. La patrulla acuerda un punto de regreso que todos pueden reconocer.'),
q('La voz al otro lado','iria','teo','Habla con Teo en Túnel Roca 1F.','Teo mantiene a salvo a un excursionista desorientado en Túnel Roca. No hace falta enfrentarse a él: necesita noticias claras de la salida.','Teo puede acompañar al excursionista hasta un punto seguro. El rescate queda anotado y la familia recibe el aviso.',flag='FLAG_BADGE03_GET'),
q('Luz para el relevo','teo','teo','Entrega un Repelente al puesto de Teo.','El siguiente turno necesita un Repelente para reducir encuentros durante el traslado. Entrégalo al puesto; no se usa sobre tu equipo.','El relevo recibe el material y confirma la salida. Teo conserva su puesto para orientar a otros viajeros.',kind='delivery',proof=('ITEM_REPEL',1)),
q('Quienes esperan noticias','teo','ruben','Informa a Rubén en Ciudad Fucsia.','Rubén reúne los avisos para las familias de los excursionistas. Lleva la confirmación a Ciudad Fucsia, sin dar por desaparecido a nadie más.','Rubén cierra la búsqueda y conserva los teléfonos de contacto. La familia ya sabe que el grupo está de regreso.',flag='FLAG_BADGE04_GET'),
q('Un sendero para volver','ruben','yara','Visita la patrulla del Bosque Baya.','Yara pide comparar señales en Bosque Baya, en Isla Tres. La patrulla trabaja después del rescate de Pedrita y no sustituye aquella historia.','Yara confirma una ruta de retorno y mantiene una patrulla visible. Los nuevos visitantes tendrán un punto al que pedir ayuda.',flag='FLAG_RESCUED_LOSTELLE'),
q('La distancia justa','yara','zaira','Consulta el relevo con Zaira en Fucsia.','La patrulla necesita pautas para ayudar a Pokémon asustados sin acorralarlos. Zaira revisará las notas en la oficina del Safari.','Zaira incorpora una distancia de seguridad a las instrucciones. La red aprende de cada rescate, en vez de limitarse a contarlos.'),
q('Una red de manos','zaira','lina','Cierra el informe con Lina en Ciudad Verde.','Lina espera los informes de las patrullas. Vuelve a Ciudad Verde para dejar acordados los puestos de auxilio y los avisos a las familias.','Los puestos quedan coordinados. Lina abre encargos de provisiones repetibles, separados de esta campaña y con un límite diario.',reward='ITEM_REVIVE',quantity=3),
]),
('CAMINOS QUE SE CRUZAN','Cuatro visitantes intercambian estrategias sin imponerlas a la aventura principal.',[
q('Una carta desde Johto','leila','hana','Entrega la invitación a Hana en Carmín.','Leila trae una invitación de intercambio para una entrenadora de Johto. Hana espera en Ciudad Carmín y decidirá si quiere participar.','Hana acepta intercambiar apuntes. Nadie apuesta Pokémon ni condiciona tu paso por la ciudad.',flag='FLAG_BADGE03_GET'),
q('El ritmo de Hana','hana','hana','Acepta y gana la práctica de Hana.','Hana quiere estudiar cómo respondes a un equipo de apoyo. Puedes rechazar el combate y retomarlo cuando tu equipo esté preparado.','Hana anota tus decisiones y te entrega una copia de su estrategia. El intercambio continúa con Ken, en Azulona.',kind='battle',proof=1),
q('El cielo de Hoenn','hana','ken','Consulta las notas de Ken en Azulona.','Ken trae observaciones de Hoenn. Compara con él cómo cambia un equipo cuando no puede depender de un único movimiento.','Ken comparte sus notas y propone una práctica. Su objetivo es mostrar alternativas, no modificar las reglas de tu partida.',flag='FLAG_BADGE04_GET'),
q('Otra forma de responder','ken','ken','Gana la práctica de Ken en Azulona.','Ken ha preparado un equipo con respuestas distintas a un mismo rival. El combate es opcional y podrás volver si pierdes.','Ken encuentra una debilidad en sus propios apuntes. Ambos conservan sus equipos y sus reglas habituales.',kind='battle',proof=2),
q('Los márgenes de Sinnoh','ken','mael','Visita a Mael en Ciudad Azafrán.','Mael quiere contrastar sus anotaciones de Sinnoh con las tuyas. Te espera en Azafrán después de obtener seis medallas.','Mael reserva espacio para los errores de su hipótesis. Propone comprobarla contigo en un combate de práctica.',flag='FLAG_BADGE06_GET'),
q('La hipótesis de Mael','mael','mael','Gana la práctica de Mael en Azafrán.','Una hipótesis también debe sobrevivir a una derrota. Mael ha preparado un equipo equilibrado; vuelve cuando quieras intentar su práctica.','Mael corrige su propuesta sin atribuir el resultado a una región. Sus notas pasan a formar parte del intercambio.',kind='battle',proof=3),
q('La invitación de Isla Siete','mael','sara','Encuentra a Sara en Isla Siete.','Sara reúne el intercambio final en Isla Siete. Cuando seas Campeón, llévale los cuadernos de Hana, Ken y Mael.','Sara descubre que los cuadernos coinciden en escuchar al compañero. Propone un último combate sin apuestas ni cambios permanentes.',flag='FLAG_IS_CHAMPION'),
q('Aprender de la respuesta','sara','sara','Gana la práctica final de Sara.','Sara quiere cerrar el intercambio en combate. Puedes intentarlo varias veces; cada derrota conserva el progreso de los demás encuentros.','Los cuatro visitantes firman el cuaderno compartido. No hay una escuela vencedora: quedan estrategias que otros entrenadores podrán discutir.',kind='battle',proof=4,reward='ITEM_BOTTLE_CAP',quantity=2),
]),
('LO QUE DEVUELVE EL MAR','La búsqueda de un tesoro acaba devolviendo memoria, no apropiándose de ella.',[
q('Un inventario incompleto','mara','faro','Lee la placa de Cabo Extremo.','Mara ha encontrado una lista de objetos sin propietarios. La primera anotación apunta a una placa de Cabo Extremo, en Isla Dos.','La placa vincula el inventario con antiguas rutas de ayuda. No señala un cofre: enumera lo que debía devolverse.',flag='FLAG_IS_CHAMPION'),
q('La marea guarda nombres','faro','baliza','Lee la baliza del Pilar Recuerdo.','La lista menciona un pilar levantado en memoria de quienes faltan. Lee su baliza en Isla Cinco sin tocar las ofrendas.','La baliza conserva los nombres de una tripulación. Mara podrá buscar a sus familias, en lugar de vender lo hallado.'),
q('Un puerto, tres testimonios','baliza','mara','Reúne los testimonios de Carmín, Ruta 12 y Cabo Extremo.','Busca los tres testimonios señalados en Carmín, Ruta 12 y Cabo Extremo. Juntos permitirán reconstruir el inventario, sin convertir cada nota en un encargo.','Las tres notas confirman la ruta y sus destinatarios. Mara guarda una copia pública y protege las direcciones privadas.',kind='clues'),
q('El valor de una devolución','mara','nilo','Entrega una Perla al archivo de Isla Uno.','Una Perla común permitirá reconstruir una pieza del inventario sin usar objetos únicos. Nilo preparará una réplica documentada para las familias.','Nilo identifica la réplica como tal. Las familias reciben su historia sin que nadie les venda una antigüedad falsa.',kind='delivery',proof=('ITEM_PEARL',1)),
q('El nombre que faltaba','nilo','tala','Contrasta el último nombre con Tala.','El último nombre aparece también en Valle Ruinas. Tala comprobará la lectura sin relacionar cualquier ruina con un mismo autor.','Tala distingue dos personas con nombres parecidos. Evita que el archivo atribuya la historia a la familia equivocada.'),
q('Un guardián no es un premio','tala','gael','Completa Espíritus de Alola y habla con Gael en Cañón Sétano.','El diario de Leyendas tiene el encargo Espíritus de Alola. Reúne el registro de sus cuatro guardianes y vuelve a Gael. Si ya lo completaste, tu archivo sirve: nadie debe volver a capturarlos.','El archivo de los guardianes explica la ruta del sur. Gael separa su cuidado del valor del inventario; no aparece ningún ejemplar duplicado.',proof=('QUEST_LEGENDS','2','0')),
q('El envío de regreso','gael','mara','Devuelve el inventario a Mara en Carmín.','El inventario está documentado. Devuélvelo a Mara en Carmín para que pueda organizar las entregas a las familias.','Mara prepara las devoluciones. El valor de lo encontrado está en que vuelva a tener dueño.'),
q('El tesoro que permanece','mara','alma','Visita la nueva cartela del museo.','Alma expondrá una copia del inventario y explicará qué se devolvió. Visítala en el museo para cerrar la búsqueda.','La cartela reconoce a las familias y a quienes mantuvieron las rutas de ayuda. El tesoro queda compartido como historia, no repartido como botín.',reward='ITEM_BIG_PEARL',quantity=2),
]),
('EL CAMINO DE VUELTA','Orientación, juego y expediciones cortas preparan un regreso con todos a salvo.',[
q('Antes de dar el primer paso','lina','riel','Consulta a Riel en Ruta 22.','Riel organiza prácticas de orientación entre Ruta 22 y Ciudad Verde. Habla con él: no necesitarás abandonar a ningún compañero.','Riel explica el trayecto y sus puestos. Si una práctica se interrumpe, se vuelve a empezar sin perder el encargo.',flag='FLAG_BADGE01_GET'),
q('Un mensaje en el camino','riel','lina','Habla con Riel y llega a Lina en 45 segundos.','Inicia la práctica hablando otra vez con Riel en Ruta 22. Lleva su mensaje a Lina en Ciudad Verde antes de 45 segundos. Puedes reintentarlo.','Lina recibe el mensaje a tiempo. Una llegada segura vale más que el récord; Riel conserva el puesto para nuevas prácticas.',kind='race',seconds=45),
q('La memoria del sendero','lina','silva','Resuelve las preguntas de Silva.','Silva propone recordar el orden roca, agua y vida con un juego de preguntas. Un error reinicia el juego, no tu aventura.','Silva entrega la secuencia correcta al grupo. El juego enseña a confirmar señales antes de seguir una ruta.',kind='puzzle',questions=(
    ('Silva señala el sendero. ¿El agua viene justo tras la roca?',True),
    ('Silva repasa la pista. ¿La roca cierra la secuencia?',False),
    ('Silva cambia el orden. ¿El agua va antes que la roca?',False),
    ('Silva mira el mapa. ¿La vida sigue al agua?',True))),
q('Preparar la expedición','silva','gael','Entrega dos Superrepelentes a Gael.','Ya como Campeón, Gael prepara una expedición de cinco puestos en Cañón Sétano. Entrega dos Superrepelentes para la reserva común.','Gael guarda las provisiones de la expedición. No se retira tu equipo ni se modifica lo que llevas equipado.',kind='delivery',proof=('ITEM_SUPER_REPEL',2),flag='FLAG_IS_CHAMPION'),
q('Cinco puestos, una ruta','gael','gael','Recorre los cinco puestos de Cañón Sétano.','Pasa por los cinco puestos señalados de Cañón Sétano y regresa a Gael. El cuaderno conserva cada puesto visitado aunque guardes y salgas.','Gael comprueba los cinco sellos. Todos los puestos conservan una indicación de regreso; la expedición no termina en el lugar más lejano.',kind='clues'),
q('Un relevo en Camino Verde','gael','belen','Lleva el informe a Belén en Camino Verde.','Belén comparará la expedición con su patrulla de Isla Seis. Llévale el informe para que otros puedan escoger una ruta adecuada.','Belén incorpora caminos de retorno más cortos para quienes los necesiten. La ruta deja de depender de un único guía.'),
q('Saber cuándo regresar','belen','gael','Gana la práctica de regreso con Gael.','Gael simula el último encuentro de una expedición. Es un combate voluntario con tu equipo habitual; puedes volver a prepararte si pierdes.','Gael da por finalizada la práctica. La prueba no exigía agotar recursos, sino reconocer cuándo prepararse mejor.',kind='battle',proof=5),
q('Volver con los demás','gael','lina','Cierra el cuaderno con Lina en Ciudad Verde.','Entrega a Lina el cuaderno de orientación y expediciones. Cada participante ha confirmado su regreso; falta cerrar el registro común.','Lina archiva la ruta, los cinco sellos y el regreso del grupo. La campaña termina con todas las personas localizadas y encargos diarios opcionales.',reward='ITEM_RARE_CANDY',quantity=3),
]),
]

# An investigation is one mission with several individually persistent observations.
CLUES = {
2: [('Atril de Moon','MtMoon_B2F','Las marcas fueron hechas después de que la roca se enfriara.'),('Atril de Roca','RockTunnel_B1F','El trazo sigue una grieta natural; no representa una frontera.'),('Atril de Canela','CinnabarIsland_PokemonLab_ExperimentRoom','El registro del laboratorio conserva la procedencia de la muestra.')],
13: [('Nodo de Silph','SilphCo_1F','La copia de Silph tiene permisos de lectura, no acceso a los equipos.'),('Nodo de Bill','Route25_SeaCottage','Bill conserva una copia aislada y una fecha de revisión.'),('Nodo de Canela','CinnabarIsland_PokemonLab_ExperimentRoom','El laboratorio solo comparte observaciones autorizadas.')],
21: [('Memorial del cuidado','PokemonTower_5F','Aquí se recuerda a quienes cuidaron de otros sin pedir recompensa.'),('Memorial de la espera','PokemonTower_6F','La inscripción deja espacio para nombres que aún se están buscando.'),('Memorial del regreso','PokemonTower_7F','Las ofrendas permanecen aquí; el refugio recibirá solo una copia del texto.')],
60: [('Puesto de salida','SevenIsland_SevaultCanyon','El primer puesto registra la salida del grupo.'),('Puesto de sombra','SevenIsland_SevaultCanyon','El segundo puesto señala un descanso protegido.'),('Puesto del agua','SevenIsland_SevaultCanyon','El tercer puesto comprueba las provisiones de agua.'),('Puesto de retorno','SevenIsland_SevaultCanyon','El cuarto puesto ofrece una vuelta más corta.'),('Puesto de llegada','SevenIsland_SevaultCanyon','El quinto puesto confirma que nadie ha quedado atrás.')],
50: [('Testigo del muelle','VermilionCity','La lista original distinguía objetos prestados de donaciones.'),('Testigo del puente','Route12','Las entregas se anotaban al regreso, no al salir del puerto.'),('Testigo del cabo','TwoIsland_CapeBrink','Los nombres del cabo corresponden a familias, no a propietarios de tesoros.')],
}

TRAINERS = [
('LIO',34,'Male','Hiker Frlg',['Raticate','Koffing','Machoke']),
('HANA',27,'Female','Lass Frlg',['Flaaffy','Skiploom','Quagsire']),
('KEN',36,'Male','Cooltrainer M Frlg',['Pelipper','Breloom','Lairon']),
('MAEL',45,'Male','Cooltrainer M Frlg',['Luxray','Bronzong','Gastrodon']),
('SARA',62,'Female','Cooltrainer F Frlg',['Mienshao','Chandelure','Braviary','Jellicent']),
('GAEL',60,'Male','Hiker Frlg',['Donphan','Crobat','Arcanine','Slowbro']),
]

CONTRACTS = [
('lina','Poción para la patrulla','ITEM_POTION',1,'ITEM_ORAN_BERRY',3,39),
('mara','Réplica para el archivo','ITEM_PEARL',1,'ITEM_NET_BALL',2,55),
('gael','Reserva de expedición','ITEM_SUPER_REPEL',1,'ITEM_GREAT_BALL',2,63),
]
