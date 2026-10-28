"""Batch C3, part D: the later Sevii Islands and Saffron City.

Appended to story_content.CAMPAIGNS; see story_content.RECONVERTED_C3 for slot assignment.
The last campaign has seven scenes so that every generic slot (71-333) is used exactly once.
"""
from story_content import CAMPAIGNS, CHOICES, PLACES, RECONVERTED_C3, TRAINERS, actor2, q

PLACES.update({
    'FourIsland_PokemonDayCare': 'Guardería de Isla Cuatro', 'FourIsland_IcefallCave_Entrance': 'Cueva Cascada Hielo',
    'FiveIsland': 'Isla Cinco', 'FiveIsland_Meadow': 'Pradera, Isla Cinco', 'FiveIsland_ResortGorgeous': 'Resort Lujoso',
    'SixIsland': 'Isla Seis', 'SixIsland_WaterPath': 'Camino Acuático', 'SixIsland_RuinValley': 'Valle Ruinas',
    'SevenIsland_TrainerTower': 'Torre Entrenador', 'SevenIsland_SevaultCanyon_Entrance': 'Entrada del Cañón Sevault',
    'Route5': 'Ruta 5', 'SaffronCity_PokemonCenter_1F': 'Centro Pokémon de Azafrán',
    'CinnabarIsland_PokemonCenter_1F': 'Centro Pokémon de Canela',
})
CHOICES.update({'guarderia_abierta': 11, 'pradera_sin_cercas': 12, 'valle_excavado': 13, 'torre_con_novatos': 14})


def campaign(title, summary, rows):
    CAMPAIGNS.append((title, summary, rows))
    RECONVERTED_C3.append(title)


def trainer(name, level, gender, pic, species):
    TRAINERS.append((name, level, gender, pic, species))
    return len(TRAINERS) - 1


# 25 --------------------------------------------------------------------------------------
actor2('abuela_tere', 'Tere', 'FourIsland_PokemonDayCare', 'Llevo cuarenta años en la Guardería. Ningún huevo es igual a otro.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('renzo', 'Renzo', 'FourIsland', 'Quiero criar Pokémon, pero no sé si sabré cuidarlos.')
campaign('HUEVOS EN ISLA CUATRO', 'La cuidadora de la Guardería enseña a un joven que criar es sobre todo esperar.', [
    q('Cuarenta años de huevos', 'abuela_tere', 'abuela_tere', 'Habla con Tere en la Guardería de Isla Cuatro.', 'Tere lleva la Guardería desde hace cuarenta años. Busca a alguien que ayude a un aprendiz.', 'Tere habla de Renzo, un joven que quiere criar Pokémon pero tiene miedo de equivocarse.', flag='FLAG_IS_CHAMPION'),
    q('El aprendiz con dudas', 'abuela_tere', 'renzo', 'Busca a Renzo en Isla Cuatro.', 'Renzo vive en Isla Cuatro. Tere quiere que le cuentes cómo trabajas con tus Pokémon.', 'Renzo escucha con atención. Pregunta si alguna vez has visto nacer un Pokémon.'),
    q('Ver nacer', 'renzo', 'renzo', 'Haz eclosionar un huevo y vuelve con Renzo.', 'Renzo quiere oír de primera mano cómo es una eclosión. Haz eclosionar un huevo.', 'Renzo escucha tu relato sin pestañear. Dice que ahora le da menos miedo.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_HATCH', '0', 1)),
    q('Preguntas de Tere', 'renzo', 'abuela_tere', 'Responde las preguntas de Tere.', 'Tere quiere comprobar que Renzo ha entendido lo básico, y te pide que respondas por él.', 'Tere asiente. Renzo está listo para su primer huevo.', kind='puzzle', questions=(
        ('Tere pregunta: ¿se puede apresurar una eclosión?', False),
        ('Tere sigue: ¿caminar con el huevo ayuda?', True),
        ('Tere termina: ¿cada Pokémon recién nacido necesita tiempo?', True))),
    q('Un nido cómodo', 'abuela_tere', 'abuela_tere', 'Entrega dos Pociones a Tere para la Guardería.', 'Tere quiere preparar un botiquín para los recién nacidos. Entrégale dos Pociones.', 'Tere guarda las Pociones junto a las mantas.', kind='delivery', proof=('ITEM_POTION', 2)),
    q('Abrir la Guardería', 'abuela_tere', 'abuela_tere', 'Ayuda a Tere a decidir sobre la Guardería.', 'Tere puede abrir la Guardería a visitas para enseñar a otros jóvenes, o mantenerla tranquila y formar solo a Renzo.', 'Tere decide abrir las puertas los domingos. Renzo será el guía.', kind='choice', choice='guarderia_abierta', question='Tere pregunta: ¿abrimos la Guardería a visitas?', alt_ending='Tere decide mantener la Guardería tranquila. Renzo aprenderá a su lado, sin prisas ni público.'),
    q('La noticia', 'abuela_tere', 'renzo', 'Cuenta a Renzo la decisión de Tere.', 'Renzo espera fuera. Ve a contarle lo que ha decidido Tere.', 'Renzo salta de alegría. Su primer día como guía será el domingo.', choice='guarderia_abierta', alt_ending='Renzo asiente, aliviado. Prefiere aprender sin público.'),
    q('Huevos en Isla Cuatro', 'renzo', 'abuela_tere', 'Lleva a Tere el primer informe de Renzo.', 'Renzo ha cuidado su primer huevo hasta la eclosión. Llévale su informe a Tere.', 'Tere lee el informe con una sonrisa. Ningún huevo es igual a otro; ningún cuidador tampoco.', reward='ITEM_EVERSTONE', quantity=1, choice='guarderia_abierta', alt_ending='Tere guarda el informe en la caja de los recuerdos. Renzo ya cuida solo.', bonus=dict(money=2500)),
])

# 26 --------------------------------------------------------------------------------------
actor2('hielo_gil', 'Gil', 'FourIsland_IcefallCave_Entrance', 'Guío visitas a la Cueva Cascada Hielo. Resbalar es parte de la visita.', 'OBJ_EVENT_GFX_HIKER')
actor2('blanca', 'Blanca', 'FourIsland', 'Esculpo hielo. Mis esculturas se derriten, y así debe ser.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
T_GIL = trainer('GIL', 52, 'Male', 'Hiker Frlg', ['Lapras', 'Dewgong', 'Cloyster'])
campaign('ESCULTURAS DE HIELO', 'Una escultora y un guía de la cueva preparan un festival de hielo que no deja huella.', [
    q('Esculpir lo efímero', 'blanca', 'blanca', 'Habla con Blanca en Isla Cuatro.', 'Blanca esculpe hielo y quiere organizar un festival en Isla Cuatro.', 'Blanca necesita hielo de la cueva, pero no quiere dañarla. Busca al guía.', flag='FLAG_IS_CHAMPION'),
    q('El guía de la cueva', 'blanca', 'hielo_gil', 'Busca a Gil en la Cueva Cascada Hielo.', 'Gil guía visitas a la cueva. Blanca quiere saber de dónde sacar hielo sin dañarla.', 'Gil dice que hay hielo que se desprende solo cada primavera. Se puede recoger.'),
    q('Reto en el hielo', 'hielo_gil', 'hielo_gil', 'Acepta el combate de Gil.', 'Gil solo lleva a la cueva a quien sabe moverse en el hielo. Te reta a un combate.', 'Gil sonríe. Puedes acompañarle a recoger hielo.', kind='battle', proof=T_GIL),
    q('Vecinos helados', 'hielo_gil', 'hielo_gil', 'Captura dos Pokémon de tipo Hielo y vuelve con Gil.', 'Gil quiere saber qué Pokémon de hielo viven cerca para no molestarlos al recoger.', 'Gil marca en su mapa dónde no recoger.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_TYPE', 'TYPE_ICE', 2)),
    q('Hielo para Blanca', 'hielo_gil', 'blanca', 'Lleva el hielo recogido a Blanca.', 'Gil ha recogido el hielo caído. Llévaselo a Blanca.', 'Blanca toca el hielo con cuidado. Es perfecto: transparente y frío.'),
    q('Herramientas cálidas', 'blanca', 'blanca', 'Entrega dos Antihielo a Blanca.', 'Blanca trabaja horas con hielo. Te pide dos Antihielo por si algún Pokémon se congela.', 'Blanca guarda los Antihielo junto a sus cinceles.', kind='delivery', proof=('ITEM_ICE_HEAL', 2)),
    q('El festival', 'blanca', 'hielo_gil', 'Invita a Gil al festival.', 'Las esculturas están listas. Blanca quiere que Gil sea el invitado de honor.', 'Gil acepta. Dice que nunca había visto el hielo de su cueva con forma de Lapras.'),
    q('Esculturas de hielo', 'hielo_gil', 'blanca', 'Cuenta a Blanca cómo fue el festival.', 'El festival ha terminado y las esculturas se derriten al sol. Gil te pide que agradezcas a Blanca.', 'Blanca mira cómo se derrite su Lapras. Dice que así debe ser: el año que viene habrá otro.', reward='ITEM_NEVER_MELT_ICE', quantity=1, bonus=dict(money=3000)),
])

# 27 --------------------------------------------------------------------------------------
actor2('amapola', 'Amapola', 'FiveIsland_Meadow', 'Cuido la Pradera de Isla Cinco. Los Pokémon la cuidan mejor que yo.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('leandro', 'Leandro', 'FiveIsland', 'Quiero poner cercas en la Pradera. Mis Miltank se escapan.', 'OBJ_EVENT_GFX_HIKER')
campaign('LA PRADERA DE ISLA CINCO', 'Una cuidadora y un ganadero discuten si la Pradera necesita cercas.', [
    q('Miltank fugados', 'leandro', 'leandro', 'Habla con Leandro en Isla Cinco.', 'Los Miltank de Leandro se escapan a la Pradera cada día. Quiere poner cercas.', 'Leandro sabe que Amapola no estará de acuerdo. Te pide que hables con ella.', flag='FLAG_IS_CHAMPION'),
    q('La cuidadora de la Pradera', 'leandro', 'amapola', 'Busca a Amapola en la Pradera.', 'Amapola cuida la Pradera. Teme que las cercas corten el paso de los Pokémon salvajes.', 'Amapola propone otra idea: que los Miltank pasten con los salvajes, pero con campanas.'),
    q('Quién pasta aquí', 'amapola', 'amapola', 'Captura dos Pokémon en la Pradera y vuelve con Amapola.', 'Amapola quiere saber qué Pokémon salvajes comparten la Pradera con los Miltank.', 'Amapola apunta las especies. Hay sitio para todos.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_MAP', 'MAP_FIVE_ISLAND_MEADOW', 2)),
    q('Campanas para los Miltank', 'amapola', 'leandro', 'Entrega una Campana Alivio a Leandro.', 'Amapola sugiere poner una campana al Miltank que más se escapa. Lleva una Campana Alivio a Leandro.', 'Leandro cuelga la campana en el cuello de su Miltank más inquieto.', kind='delivery', proof=('ITEM_SOOTHE_BELL', 1)),
    q('Observar a un Miltank', 'leandro', 'leandro', 'Observa un Miltank y vuelve con Leandro.', 'Leandro quiere que alguien de fuera vea cómo se comportan sus Miltank en la Pradera.', 'Leandro escucha tu relato. Sus Miltank parecen más tranquilos de lo que pensaba.', kind='field', goal=('QUEST_DEX_SEEN', 'SPECIES_MILTANK')),
    q('Cercas o campanas', 'leandro', 'amapola', 'Ayuda a Amapola a decidir.', 'Leandro y Amapola deben decidir: dejar la Pradera sin cercas, con campanas, o cercar solo una parte.', 'Amapola decide no poner cercas. Las campanas bastan para saber dónde están los Miltank.', kind='choice', choice='pradera_sin_cercas', question='Amapola pregunta: ¿dejamos la Pradera sin cercas?', alt_ending='Amapola acepta una cerca baja en una esquina. Los Pokémon salvajes pueden saltarla sin problema.'),
    q('Contárselo a Leandro', 'amapola', 'leandro', 'Cuenta a Leandro la decisión.', 'Leandro espera en Isla Cinco. Ve a contarle qué ha decidido Amapola.', 'Leandro acepta. Mientras suene la campana, sabrá dónde están sus Miltank.', choice='pradera_sin_cercas', alt_ending='Leandro construye la cerca baja con madera de la playa. Sus Miltank la ignoran, pero la respetan.'),
    q('La Pradera de Isla Cinco', 'leandro', 'amapola', 'Lleva a Amapola leche de los Miltank.', 'Leandro te da una botella de Leche Mu-mu para Amapola, como agradecimiento.', 'Amapola brinda con la leche. La Pradera sigue abierta para todos.', reward='ITEM_MOOMOO_MILK', quantity=3, choice='pradera_sin_cercas', alt_ending='Amapola brinda con la leche junto a la cerca baja. La Pradera ha encontrado un equilibrio.', bonus=dict(money=2500)),
])

# 28 --------------------------------------------------------------------------------------
actor2('irene', 'Irene', 'FiveIsland_ResortGorgeous', 'Trabajo en el Resort Lujoso. Los huéspedes piden cosas imposibles.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('max', 'Max', 'FiveIsland', 'Hago recados para el Resort. A veces me pagan en propinas, a veces en historias.')
T_MAX = trainer('MAX', 48, 'Male', 'Youngster Frlg', ['Persian', 'Ninetales', 'Dodrio'])
campaign('RECADOS DEL RESORT', 'Una empleada del Resort y un recadero cumplen los encargos más raros de los huéspedes.', [
    q('Un huésped exigente', 'irene', 'irene', 'Habla con Irene en el Resort Lujoso.', 'Irene trabaja en el Resort. Un huésped ha pedido algo muy raro y necesita ayuda.', 'El huésped quiere ver un Pokémon de cada tipo antes de irse. Irene no sabe por dónde empezar.', flag='FLAG_IS_CHAMPION'),
    q('El recadero', 'irene', 'max', 'Busca a Max en Isla Cinco.', 'Max hace recados para el Resort. Irene quiere que te ayude.', 'Max dice que conoce a todos en las islas. Puede ayudar, pero primero quiere ponerte a prueba.'),
    q('El reto del recadero', 'max', 'max', 'Acepta el combate de Max.', 'Max quiere comprobar que eres fiable antes de compartir sus contactos.', 'Max ríe. Eres de fiar.', kind='battle', proof=T_MAX),
    q('Ver de todo', 'max', 'max', 'Captura tres Pokémon de tipo Normal y vuelve con Max.', 'Max propone empezar por lo fácil: tipo Normal. Registra tres capturas.', 'Max apunta las especies. Una parte del encargo está lista.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_TYPE', 'TYPE_NORMAL', 3)),
    q('Un regalo para el huésped', 'max', 'irene', 'Entrega una Perla a Irene.', 'Max sugiere regalar al huésped una Perla para que el encargo no parezca una lista de tareas.', 'Irene envuelve la Perla con un lazo.', kind='delivery', proof=('ITEM_PEARL', 1)),
    q('Preguntas del huésped', 'irene', 'irene', 'Responde las preguntas del huésped.', 'El huésped quiere saber si de verdad conoces a los Pokémon antes de aceptar el regalo.', 'El huésped aplaude. Dice que no había oído respuestas tan sinceras en años.', kind='puzzle', questions=(
        ('El huésped pregunta: ¿fuerte siempre es buen compañero?', False),
        ('El huésped sigue: ¿conocer a un Pokémon lleva tiempo?', True),
        ('El huésped termina: ¿hay que capturarlos para conocerlos?', False))),
    q('Despedir al huésped', 'irene', 'max', 'Avisa a Max de que el encargo está cumplido.', 'El huésped se va contento. Ve a avisar a Max.', 'Max sonríe. Dice que es el encargo más raro del año.'),
    q('Recados del Resort', 'max', 'irene', 'Lleva a Irene la propina de Max.', 'Max reparte la propina contigo e Irene. Lleva su parte al Resort.', 'Irene guarda la propina para el próximo encargo imposible.', reward='ITEM_NUGGET', quantity=1, bonus=dict(money=4000)),
])

# 29 --------------------------------------------------------------------------------------
actor2('agua_paco', 'Paco', 'SixIsland_WaterPath', 'Recojo basura del Camino Acuático. El mar devuelve todo.', 'OBJ_EVENT_GFX_HIKER')
actor2('nadia', 'Nadia', 'SixIsland', 'Dirijo la asociación de vecinos de Isla Seis. Somos pocos pero tercos.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
campaign('LIMPIEZA DEL CAMINO ACUÁTICO', 'Los vecinos de Isla Seis limpian su costa y aprenden quién vive en ella.', [
    q('El mar devuelve', 'nadia', 'nadia', 'Habla con Nadia en Isla Seis.', 'Nadia organiza una limpieza del Camino Acuático. Necesita voluntarios.', 'Nadia te presenta a Paco, que limpia la costa cada día sin que nadie se lo pida.', flag='FLAG_IS_CHAMPION'),
    q('El voluntario de siempre', 'nadia', 'agua_paco', 'Busca a Paco en el Camino Acuático.', 'Paco sabe dónde se acumula la basura. Nadia quiere su ayuda.', 'Paco te enseña las calas donde el mar deja más restos.'),
    q('Vecinos de la costa', 'agua_paco', 'agua_paco', 'Captura dos Pokémon en el Camino Acuático y vuelve con Paco.', 'Paco quiere saber qué Pokémon viven en las calas para no molestarlos al limpiar.', 'Paco marca en su mapa las calas con nidos.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_MAP', 'MAP_SIX_ISLAND_WATER_PATH', 2)),
    q('Guantes para todos', 'agua_paco', 'nadia', 'Entrega dos Antídotos a Nadia.', 'Algunos restos tienen sustancias tóxicas. Nadia quiere Antídotos para los voluntarios.', 'Nadia guarda los Antídotos en la caseta de la asociación.', kind='delivery', proof=('ITEM_ANTIDOTE', 2)),
    q('Limpieza en marcha', 'nadia', 'agua_paco', 'Gana tres combates contra salvajes y vuelve con Paco.', 'Algunos Pokémon defienden las calas. Gana tres combates para que los voluntarios trabajen tranquilos.', 'Paco agradece la ayuda. Los voluntarios ya pueden trabajar.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_WILD_WIN', '0', 3)),
    q('Preguntas de Paco', 'agua_paco', 'agua_paco', 'Responde las preguntas de Paco.', 'Paco quiere comprobar que sabrás explicar las normas a los voluntarios.', 'Paco asiente. Ya puedes dirigir un grupo.', kind='puzzle', questions=(
        ('Paco pregunta: ¿se recoge basura dentro de un nido?', False),
        ('Paco sigue: ¿los restos de red son peligrosos para los Pokémon?', True),
        ('Paco termina: ¿hay que separar la basura antes de tirarla?', True))),
    q('El informe', 'agua_paco', 'nadia', 'Lleva el informe de Paco a Nadia.', 'Paco ha contado todo lo que se ha recogido. Lleva el informe a Nadia.', 'Nadia lee el informe con la boca abierta. Han limpiado tres calas enteras.'),
    q('Limpieza del Camino Acuático', 'nadia', 'agua_paco', 'Lleva a Paco el agradecimiento de la isla.', 'La asociación ha votado dar a Paco una medalla. Llévasela.', 'Paco se ruboriza. Dice que la medalla es para el mar, que devuelve todo.', reward='ITEM_DIVE_BALL', quantity=5, bonus=dict(money=2500)),
])

# 30 --------------------------------------------------------------------------------------
actor2('dora_ruinas', 'Dora', 'SixIsland_RuinValley', 'Soy arqueóloga. Este valle esconde algo, pero no sé si debemos encontrarlo.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('mateo', 'Mateo', 'SixIsland', 'Mi familia vive en Isla Seis desde siempre. El valle es nuestro cementerio.')
T_DORA = trainer('DORA', 54, 'Female', 'Lass Frlg', ['Kabutops', 'Omastar', 'Aerodactyl'])
campaign('EL VALLE RUINAS', 'Una arqueóloga y un vecino discuten si excavar el valle o dejarlo descansar.', [
    q('Algo bajo el valle', 'dora_ruinas', 'dora_ruinas', 'Habla con Dora en el Valle Ruinas.', 'Dora cree que bajo el valle hay una construcción antigua. Quiere excavar, pero tiene dudas.', 'Dora sabe que los vecinos de Isla Seis consideran el valle un lugar sagrado.', flag='FLAG_IS_CHAMPION'),
    q('La voz de la isla', 'dora_ruinas', 'mateo', 'Busca a Mateo en Isla Seis.', 'Mateo es de una familia antigua de la isla. Dora quiere saber qué opina.', 'Mateo cuenta que su abuela hablaba de Pokémon que dormían en el valle.'),
    q('Ver un Unown', 'mateo', 'mateo', 'Observa un Unown y vuelve con Mateo.', 'Mateo cree que los Pokémon de la leyenda son Unown. Observa uno.', 'Mateo escucha tu descripción. Dice que su abuela tenía razón.', kind='field', goal=('QUEST_DEX_SEEN', 'SPECIES_UNOWN')),
    q('Fósiles de la arqueóloga', 'mateo', 'dora_ruinas', 'Acepta el combate de Dora.', 'Dora entrena Pokémon revividos de fósiles. Quiere demostrar a Mateo que respeta el pasado.', 'Dora recoge a sus Pokémon. Mateo la mira de otra manera.', kind='battle', proof=T_DORA),
    q('Un regalo para el valle', 'dora_ruinas', 'mateo', 'Entrega una Esquirla Estrella a Mateo.', 'Dora quiere dejar una ofrenda en el valle, como hacía la abuela de Mateo.', 'Mateo coloca la Esquirla en la entrada del valle.', kind='delivery', proof=('ITEM_STAR_PIECE', 1)),
    q('Excavar o esperar', 'mateo', 'dora_ruinas', 'Ayuda a Dora a decidir.', 'Dora puede excavar con cuidado y con los vecinos, o renunciar y documentar solo lo que se ve.', 'Dora decide excavar con los vecinos. Mateo dirigirá a los voluntarios de la isla.', kind='choice', choice='valle_excavado', question='Dora pregunta: ¿excavamos el valle?', alt_ending='Dora decide no excavar. Documentará el valle tal como está, con dibujos y sin palas.'),
    q('Avisar a Mateo', 'dora_ruinas', 'mateo', 'Cuenta a Mateo la decisión.', 'Mateo espera en Isla Seis. Ve a contarle qué ha decidido Dora.', 'Mateo acepta. Dice que su abuela querría saber qué hay ahí abajo.', choice='valle_excavado', alt_ending='Mateo se emociona. Dice que el valle seguirá descansando, como debe.'),
    q('El Valle Ruinas', 'mateo', 'dora_ruinas', 'Lleva a Dora el dibujo de la abuela de Mateo.', 'Mateo te da un dibujo antiguo de su abuela para Dora. Muestra el valle hace cien años.', 'Dora compara el dibujo con el valle actual. Ha cambiado poco: alguien siempre lo ha cuidado.', reward='ITEM_RARE_CANDY', quantity=2, choice='valle_excavado', alt_ending='Dora coloca el dibujo en la primera página de su cuaderno. El valle tendrá su historia escrita.', bonus=dict(money=3500)),
])

# 31 --------------------------------------------------------------------------------------
actor2('vega', 'Vega', 'SevenIsland_TrainerTower', 'Organizo los retos de la Torre Entrenador. Últimamente solo vienen expertos.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('pipo', 'Pipo', 'SevenIsland', 'Quiero subir a la Torre, pero me da vergüenza perder delante de todos.')
T_VEGA = trainer('VEGA', 58, 'Female', 'Lass Frlg', ['Alakazam', 'Gengar', 'Starmie'])
campaign('LA TORRE PARA TODOS', 'La organizadora de la Torre Entrenador quiere que los novatos también se atrevan a subir.', [
    q('Una torre para expertos', 'vega', 'vega', 'Habla con Vega en la Torre Entrenador.', 'Vega organiza los retos de la Torre. Le preocupa que solo vengan entrenadores expertos.', 'Vega quiere atraer a novatos, pero no sabe qué los frena.', flag='FLAG_IS_CHAMPION'),
    q('El novato tímido', 'vega', 'pipo', 'Busca a Pipo en Isla Siete.', 'Pipo quiere subir a la Torre pero no se atreve. Vega quiere saber por qué.', 'Pipo confiesa que le da miedo perder delante de todos.'),
    q('Una derrota que enseña', 'pipo', 'vega', 'Acepta el combate de Vega.', 'Vega quiere mostrar a Pipo que perder contra alguien fuerte no es una vergüenza. Te reta delante de él.', 'Gane quien gane, Vega se acerca a Pipo y le explica lo que ha aprendido del combate.', kind='battle', proof=T_VEGA),
    q('Entrenar sin público', 'vega', 'pipo', 'Gana tres combates contra salvajes y vuelve con Pipo.', 'Vega sugiere que Pipo entrene contra salvajes antes de la Torre. Acompáñalo ganando tres combates.', 'Pipo te ha observado con atención. Dice que ya tiene un plan.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_WILD_WIN', '0', 3)),
    q('Botiquín de novato', 'pipo', 'pipo', 'Entrega tres Superpociones a Pipo.', 'Pipo quiere ir preparado a la Torre. Entrégale tres Superpociones.', 'Pipo guarda las Superpociones en su mochila nueva.', kind='delivery', proof=('ITEM_SUPER_POTION', 3)),
    q('Retos para novatos', 'pipo', 'vega', 'Ayuda a Vega a decidir.', 'Vega puede crear un reto para novatos con su propia planta, o mantener la Torre igual y entrenar a los novatos fuera.', 'Vega abre una planta para novatos. Pipo será el primero en subir.', kind='choice', choice='torre_con_novatos', question='Vega pregunta: ¿creamos una planta para novatos?', alt_ending='Vega decide entrenar a los novatos en la playa. Cuando estén listos, subirán a la Torre como todos.'),
    q('La primera subida', 'vega', 'pipo', 'Cuenta a Pipo la decisión de Vega.', 'Pipo espera fuera de la Torre. Ve a contarle lo que ha decidido Vega.', 'Pipo respira hondo y entra en la Torre.', choice='torre_con_novatos', alt_ending='Pipo sonríe aliviado. Entrenará en la playa hasta sentirse listo.'),
    q('La Torre para todos', 'pipo', 'vega', 'Lleva a Vega el resultado de Pipo.', 'Pipo ha completado su primer reto. Te pide que se lo cuentes a Vega.', 'Vega apunta el nombre de Pipo en la pizarra de la entrada. Es el primer novato del año.', reward='ITEM_PP_MAX', quantity=1, choice='torre_con_novatos', alt_ending='Vega apunta el nombre de Pipo en la lista de la playa. Pronto estará en la pizarra de la Torre.', bonus=dict(money=4000)),
])

# 32 --------------------------------------------------------------------------------------
actor2('roque', 'Roque', 'SevenIsland_SevaultCanyon_Entrance', 'Escalo el Cañón Sevault. Las rocas tienen nombres que solo yo conozco.', 'OBJ_EVENT_GFX_HIKER')
actor2('tina', 'Tina', 'SevenIsland', 'Mi hermano escala el cañón. Siempre vuelve tarde.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
T_ROQUE = trainer('ROQUE', 56, 'Male', 'Hiker Frlg', ['Golem', 'Onix', 'Rhydon'])
campaign('EL ESCALADOR DEL CAÑÓN', 'Una hermana preocupada y un escalador terco buscan cómo volver a tiempo del Cañón Sevault.', [
    q('Siempre vuelve tarde', 'tina', 'tina', 'Habla con Tina en Isla Siete.', 'Tina se preocupa por su hermano Roque, que escala el Cañón Sevault y siempre vuelve tarde.', 'Tina quiere que alguien hable con él. A ella no la escucha.', flag='FLAG_IS_CHAMPION'),
    q('El escalador', 'tina', 'roque', 'Busca a Roque en la entrada del Cañón Sevault.', 'Roque escala el cañón casi cada día. Tina quiere que vuelva antes del anochecer.', 'Roque dice que no se arriesga. Solo pierde la noción del tiempo.'),
    q('Escalar con cabeza', 'roque', 'roque', 'Acepta el combate de Roque.', 'Roque solo escucha a quien sabe moverse en terreno difícil. Te reta a un combate.', 'Roque asiente. Está dispuesto a escucharte.', kind='battle', proof=T_ROQUE),
    q('Roca viva', 'roque', 'roque', 'Captura dos Pokémon de tipo Roca y vuelve con Roque.', 'Roque conoce las rocas pero no tanto a los Pokémon que viven entre ellas. Registra dos capturas de tipo Roca.', 'Roque apunta las especies. Algunas le explican ruidos que oía al escalar.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_TYPE', 'TYPE_ROCK', 2)),
    q('Una cuerda para volver', 'roque', 'tina', 'Entrega una Cuerda Huida a Tina.', 'Roque propone llevar siempre una Cuerda Huida. Tina se la dará antes de cada salida.', 'Tina guarda la Cuerda Huida en la mochila de su hermano.', kind='delivery', proof=('ITEM_ESCAPE_ROPE', 1)),
    q('La hora de volver', 'tina', 'roque', 'Ve desde Tina hasta Roque en 60 segundos.', 'Tina quiere saber cuánto tarda Roque en volver desde la entrada del cañón. Haz el recorrido inverso en menos de un minuto.', 'Roque mira el reloj. Si sale a la hora, llegará a cenar.', kind='race', seconds=60),
    q('Una promesa', 'roque', 'tina', 'Lleva a Tina la promesa de Roque.', 'Roque promete volver antes del anochecer. Lleva su promesa a Tina.', 'Tina sonríe. Esta vez lo cree.'),
    q('El escalador del cañón', 'tina', 'roque', 'Lleva a Roque la cena de Tina.', 'Roque ha vuelto a tiempo tres días seguidos. Tina te da su cena favorita para él.', 'Roque se come la cena en la entrada del cañón, mirando el atardecer. Esta vez lo ve entero.', reward='ITEM_HARD_STONE', quantity=1, bonus=dict(money=3500)),
])

# 33 (seven scenes) -----------------------------------------------------------------------
actor2('lin', 'Lin', 'SaffronCity_PokemonCenter_1F', 'Llegué a Azafrán sin conocer a nadie. Ahora ayudo a quien llega igual.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('omar', 'Omar', 'Route5', 'Acabo de llegar a Kanto. Todo es grande y nadie me mira.', 'OBJ_EVENT_GFX_HIKER')
campaign('RECIÉN LLEGADOS A AZAFRÁN', 'Lin ayuda a Omar, que acaba de llegar a Kanto, a encontrar su sitio en la ciudad.', [
    q('Alguien que acaba de llegar', 'lin', 'lin', 'Habla con Lin en el Centro Pokémon de Azafrán.', 'Lin ayuda a quienes llegan a Azafrán sin conocer a nadie. Ha visto a un chico perdido en la Ruta 5.', 'Lin te pide que hables con él. A veces una cara amable basta para empezar.', flag='FLAG_BADGE06_GET'),
    q('El chico de la Ruta 5', 'lin', 'omar', 'Busca a Omar en la Ruta 5.', 'Omar acaba de llegar a Kanto. Está solo en la Ruta 5 con su Pokémon.', 'Omar cuenta que viene de lejos y no sabe por dónde empezar.'),
    q('Pokémon de aquí', 'omar', 'omar', 'Captura dos Pokémon en la Ruta 5 y vuelve con Omar.', 'Omar no conoce los Pokémon de Kanto. Enséñale capturando dos en la Ruta 5.', 'Omar observa a tus Pokémon con curiosidad. Algunos se parecen a los de su tierra.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_MAP', 'MAP_ROUTE5', 2)),
    q('Un mapa para Omar', 'omar', 'lin', 'Lleva las preguntas de Omar a Lin.', 'Omar tiene muchas preguntas sobre la ciudad. Lleva su lista a Lin.', 'Lin responde a cada pregunta en un mapa dibujado a mano.'),
    q('Primeras provisiones', 'lin', 'omar', 'Entrega tres Pociones a Omar.', 'Lin sugiere que Omar empiece con un botiquín. Entrégale tres Pociones.', 'Omar guarda las Pociones con cuidado. Es lo primero que tiene de Kanto.', kind='delivery', proof=('ITEM_POTION', 3)),
    q('Las calles de Azafrán', 'omar', 'omar', 'Responde las preguntas de Omar.', 'Omar quiere comprobar que ha entendido el mapa de Lin.', 'Omar sonríe. Ya sabe ir del Centro Pokémon al Gimnasio sin perderse.', kind='puzzle', questions=(
        ('Omar pregunta: ¿el Centro Pokémon cura gratis?', True),
        ('Omar sigue: ¿hay que pagar para entrar en la ciudad?', False),
        ('Omar termina: ¿puedo pedir ayuda a cualquiera en el Centro?', True))),
    q('Recién llegados a Azafrán', 'omar', 'lin', 'Lleva a Lin el agradecimiento de Omar.', 'Omar ha encontrado un sitio donde quedarse. Te pide que agradezcas a Lin.', 'Lin guarda la nota de Omar junto a otras muchas. Cada una es alguien que ya no está perdido.', reward='ITEM_LUXURY_BALL', quantity=3, bonus=dict(money=2500)),
])
