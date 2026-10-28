"""Batch C3, part C: Viridian, Victory Road approach, Seafoam and the first Sevii Islands.

Appended to story_content.CAMPAIGNS; see story_content.RECONVERTED_C3 for slot assignment.
"""
from story_content import CAMPAIGNS, CHOICES, PLACES, RECONVERTED_C3, TRAINERS, actor2, q

PLACES.update({
    'ViridianForest': 'Bosque Verde', 'Route22': 'Ruta 22', 'Route23': 'Ruta 23',
    'ViridianCity_PokemonCenter_1F': 'Centro Pokémon de Verde', 'OneIsland': 'Isla Uno',
    'OneIsland_PokemonCenter_1F': 'Centro Pokémon de Isla Uno', 'TwoIsland': 'Isla Dos',
    'TwoIsland_CapeBrink': 'Cabo Extremo, Isla Dos', 'Route21_North': 'Ruta 21',
    'Route4_PokemonCenter_1F': 'Centro Pokémon de Ruta 4', 'VermilionCity_PokemonCenter_1F': 'Centro Pokémon de Carmín',
})
CHOICES.update({'bosque_con_senderos': 8, 'liga_con_guia': 9, 'cabo_abierto': 10})


def campaign(title, summary, rows):
    CAMPAIGNS.append((title, summary, rows))
    RECONVERTED_C3.append(title)


def trainer(name, level, gender, pic, species):
    TRAINERS.append((name, level, gender, pic, species))
    return len(TRAINERS) - 1


# 17 --------------------------------------------------------------------------------------
actor2('silvia', 'Silvia', 'ViridianForest', 'Marco los senderos del bosque con cintas. Los Caterpie se las comen.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('julio', 'Julio', 'ViridianCity_PokemonCenter_1F', 'Me perdí en el Bosque Verde a los seis años. Todavía sueño con sus árboles.')
campaign('SENDEROS DEL BOSQUE VERDE', 'Silvia quiere que nadie vuelva a perderse en el bosque, sin convertirlo en un parque.', [
    q('Cintas en los árboles', 'silvia', 'silvia', 'Habla con Silvia en el Bosque Verde.', 'Silvia marca senderos con cintas porque cada semana se pierde algún niño en el bosque.', 'Silvia dice que las cintas desaparecen. Sospecha de los Caterpie, pero quiere comprobarlo.', flag='FLAG_BADGE01_GET'),
    q('Quién come cintas', 'silvia', 'silvia', 'Observa un Caterpie y vuelve con Silvia.', 'Silvia quiere confirmar que los Caterpie se comen las cintas. Observa uno.', 'Silvia ríe. Las cintas son de tela vegetal; los Caterpie las confunden con hojas.', kind='field', goal=('QUEST_DEX_SEEN', 'SPECIES_CATERPIE')),
    q('El niño que se perdió', 'silvia', 'julio', 'Habla con Julio en el Centro Pokémon de Verde.', 'Julio se perdió en el bosque de niño. Silvia quiere saber qué le habría ayudado.', 'Julio recuerda que lo que más miedo le dio fue no saber si iba hacia dentro o hacia fuera.'),
    q('Marcas que no se comen', 'julio', 'silvia', 'Entrega dos Repelentes a Silvia.', 'Julio sugiere marcar los árboles con piedras pintadas en vez de cintas. Silvia necesitará Repelentes mientras trabaja.', 'Silvia guarda los Repelentes. Podrá pintar piedras sin que la interrumpan cada dos pasos.', kind='delivery', proof=('ITEM_REPEL', 2)),
    q('Contar los insectos', 'silvia', 'silvia', 'Captura dos Pokémon en el Bosque Verde y vuelve con Silvia.', 'Silvia no quiere colocar piedras donde anidan los Pokémon. Registra dos capturas en el bosque para saber dónde viven.', 'Silvia marca los nidos en su mapa. Las piedras irán lejos de ellos.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_MAP', 'MAP_VIRIDIAN_FOREST', 2)),
    q('Senderos o bosque', 'silvia', 'silvia', 'Ayuda a Silvia a decidir cómo marcar el bosque.', 'Silvia puede abrir senderos anchos y señalizados, o solo poner piedras discretas en los cruces.', 'Silvia abre senderos claros. Nadie volverá a perderse, aunque el bosque parezca menos salvaje.', kind='choice', choice='bosque_con_senderos', question='Silvia pregunta: ¿abrimos senderos anchos?', alt_ending='Silvia solo marca los cruces con piedras pintadas. El bosque sigue igual de denso, pero ahora tiene pistas.'),
    q('Probar el camino', 'silvia', 'julio', 'Cuenta a Julio cómo ha quedado el bosque.', 'Julio quiere saber si un niño podría cruzar el bosque ahora sin perderse.', 'Julio decide cruzar el bosque él mismo, por primera vez desde niño.', choice='bosque_con_senderos', alt_ending='Julio estudia las piedras de los cruces. Dice que las habría seguido sin dudar.'),
    q('Senderos del Bosque Verde', 'julio', 'silvia', 'Lleva a Silvia la noticia de Julio.', 'Julio ha cruzado el bosque sin perderse. Ve a contárselo a Silvia.', 'Silvia guarda la última cinta como recuerdo. Los Caterpie tendrán que buscar otra merienda.', reward='ITEM_SILVER_POWDER', quantity=1, choice='bosque_con_senderos', alt_ending='Silvia pinta una última piedra con el nombre de Julio y la deja en el cruce donde se perdió.', bonus=dict(money=1500)),
])

# 18 --------------------------------------------------------------------------------------
actor2('hugo', 'Hugo', 'Route22', 'Mi primer combate fue aquí. Lo perdí, pero sigo volviendo.')
actor2('elena', 'Elena', 'Route23', 'Reviso medallas a la entrada de la Calle Victoria. Veo nervios cada día.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
T_HUGO = trainer('HUGO', 42, 'Male', 'Camper Frlg', ['Nidoking', 'Fearow', 'Primeape'])
campaign('EL CAMINO A LA LIGA', 'Un aspirante veterano y una revisora de medallas piensan cómo acompañar a quien intenta llegar a la Liga.', [
    q('Medallas y nervios', 'elena', 'elena', 'Habla con Elena en la Ruta 23.', 'Elena revisa medallas en la Ruta 23. Ve a muchos entrenadores llegar agotados y sin consejo.', 'Elena quiere crear una guía para aspirantes. Necesita la voz de alguien que haya fallado y vuelto.', flag='FLAG_BADGE07_GET'),
    q('El que siempre vuelve', 'elena', 'hugo', 'Busca a Hugo en la Ruta 22.', 'Hugo lleva años intentando llegar a la Liga. Elena cree que su experiencia vale más que una victoria.', 'Hugo acepta ayudar, pero antes quiere un combate: lleva mucho sin enfrentarse a alguien fuerte.'),
    q('Un combate honesto', 'hugo', 'hugo', 'Acepta el combate de Hugo.', 'Hugo quiere medir cuánto ha mejorado. Te desafía en la Ruta 22.', 'Hugo pierde, pero sonríe. Dice que ha aguantado más que nunca.', kind='battle', proof=T_HUGO),
    q('Lo que llevaría', 'hugo', 'elena', 'Entrega dos Revivir a Elena para el puesto de control.', 'Hugo dice que lo que más falta en la Calle Victoria son Revivir. Lleva dos a Elena para el puesto.', 'Elena guarda los Revivir en el mostrador, para quien los necesite.', kind='delivery', proof=('ITEM_REVIVE', 2)),
    q('Consejos de aspirante', 'elena', 'elena', 'Responde las preguntas de Elena.', 'Elena quiere comprobar que los consejos de Hugo se entienden bien.', 'Elena anota tus respuestas. La guía empieza a tener forma.', kind='puzzle', questions=(
        ('Elena pregunta: ¿se entra a la Calle Victoria sin Revivir?', False),
        ('Elena sigue: ¿perder un combate te impide volver?', False),
        ('Elena termina: ¿descansar también es entrenar?', True))),
    q('Guía o compañía', 'elena', 'elena', 'Ayuda a Elena a decidir el formato de la guía.', 'Elena puede escribir una guía, o pedir que Hugo acompañe a los aspirantes hasta la entrada.', 'Elena decide que Hugo acompañe a quien lo pida. Una persona vale más que un folleto.', kind='choice', choice='liga_con_guia', question='Elena pregunta: ¿pedimos a Hugo que acompañe a los aspirantes?', alt_ending='Elena escribe la guía con las palabras de Hugo. Cada aspirante recibirá una copia en el puesto.'),
    q('La respuesta de Hugo', 'elena', 'hugo', 'Cuenta a Hugo la propuesta de Elena.', 'Hugo espera en la Ruta 22. Ve a contarle qué ha decidido Elena.', 'Hugo acepta. Dice que acompañar a otros quizá sea su manera de llegar a la Liga.', choice='liga_con_guia', alt_ending='Hugo se emociona al saber que sus palabras irán en la guía. Firma la primera página.'),
    q('El camino a la Liga', 'hugo', 'elena', 'Lleva a Elena el primer informe de Hugo.', 'Hugo ha acompañado a su primer aspirante. Lleva su informe a Elena.', 'Elena cuelga el informe en el puesto. El camino a la Liga ya no se recorre solo.', reward='ITEM_MAX_REVIVE', quantity=1, choice='liga_con_guia', alt_ending='Elena reparte la primera guía. La aspirante que la recibe promete devolverla con una página nueva.', bonus=dict(money=3000)),
])

# 19 --------------------------------------------------------------------------------------
actor2('pau', 'Pau', 'PowerPlant', 'Reviso la Central Energía. Hay Pokémon que viven de las chispas que se escapan.')
actor2('olivia', 'Olivia', 'Route10', 'Vivo junto a la Central. Por las noches se oyen zumbidos.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
T_PAU = trainer('PAU', 38, 'Male', 'Super Nerd Frlg', ['Electrode', 'Magneton', 'Electabuzz'])
campaign('ZUMBIDOS EN LA CENTRAL', 'Una vecina y un técnico descubren por qué la Central abandonada zumba de noche.', [
    q('Zumbidos nocturnos', 'olivia', 'olivia', 'Habla con Olivia en la Ruta 10.', 'Olivia vive junto a la Central Energía. Cada noche oye zumbidos y no sabe de dónde salen.', 'Olivia sospecha que algo vive dentro. Quiere saber si es peligroso.', flag='FLAG_BADGE04_GET'),
    q('El técnico', 'olivia', 'pau', 'Busca a Pau en la Central Energía.', 'Pau revisa la Central de vez en cuando. Olivia quiere saber qué ha visto.', 'Pau dice que los zumbidos son de Magnemite y Voltorb que se alimentan de la corriente que queda.'),
    q('Pruebas de campo', 'pau', 'pau', 'Observa un Magnemite y vuelve con Pau.', 'Pau quiere confirmar que los Magnemite siguen allí. Observa uno.', 'Pau anota la observación. Los Magnemite están bien, aunque más inquietos que antes.', kind='field', goal=('QUEST_DEX_SEEN', 'SPECIES_MAGNEMITE')),
    q('El reto del técnico', 'pau', 'pau', 'Acepta el combate de Pau.', 'Pau quiere saber si podrías ayudarle a revisar las zonas más peligrosas. Te reta a un combate.', 'Pau se coloca las gafas. Puedes acompañarle.', kind='battle', proof=T_PAU),
    q('Una avería', 'pau', 'pau', 'Entrega dos Antiparalizadores a Pau.', 'Una máquina vieja da descargas. Pau necesita dos Antiparalizadores antes de repararla.', 'Pau repara la máquina. Los zumbidos se vuelven más suaves.', kind='delivery', proof=('ITEM_PARALYZE_HEAL', 2)),
    q('Despejar la sala', 'pau', 'pau', 'Gana tres combates contra salvajes y vuelve con Pau.', 'Algunos Pokémon defienden la zona de la máquina. Gana tres combates para que Pau termine.', 'Pau termina la reparación. Los Pokémon vuelven a sus rincones.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_WILD_WIN', '0', 3)),
    q('Explicar el zumbido', 'pau', 'olivia', 'Cuenta a Olivia lo que hay en la Central.', 'Ya sabes de dónde vienen los zumbidos. Ve a explicárselo a Olivia.', 'Olivia se ríe. Lleva meses con miedo de unos Pokémon que solo comen chispas.'),
    q('Zumbidos en la Central', 'olivia', 'pau', 'Lleva a Pau la invitación de Olivia.', 'Olivia invita a Pau a cenar como agradecimiento. Llévale la invitación.', 'Pau acepta. Esa noche, los zumbidos suenan como música de fondo.', reward='ITEM_MAGNET', quantity=1, bonus=dict(money=2500)),
])

# 20 --------------------------------------------------------------------------------------
actor2('celso', 'Celso', 'OneIsland', 'Soy guía en Isla Uno. Recibo a quienes llegan sin saber por qué han venido.', 'OBJ_EVENT_GFX_HIKER')
actor2('greta', 'Greta', 'OneIsland_KindleRoad', 'Me baño en las aguas termales del Camino Candente. Mi Rapidash vigila.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('iker', 'Iker', 'OneIsland_PokemonCenter_1F', 'Llevo un registro de quienes llegan a las islas. Algunos nunca se van.')
campaign('BIENVENIDA A ISLA UNO', 'El guía de Isla Uno prepara un recorrido para quienes llegan por primera vez a las islas.', [
    q('El guía del muelle', 'celso', 'celso', 'Habla con Celso en Isla Uno.', 'Celso recibe a los viajeros que llegan a Isla Uno. Quiere preparar un recorrido de bienvenida.', 'Celso necesita ayuda para elegir las paradas del recorrido.', flag='FLAG_IS_CHAMPION'),
    q('El registro de llegadas', 'celso', 'iker', 'Habla con Iker en el Centro Pokémon de Isla Uno.', 'Iker lleva el registro de viajeros. Sabe qué buscan quienes llegan.', 'Iker dice que casi todos preguntan por las aguas termales del Camino Candente.'),
    q('Aguas termales', 'iker', 'greta', 'Busca a Greta en el Camino Candente.', 'Greta conoce las aguas termales mejor que nadie. Iker quiere que sea una parada del recorrido.', 'Greta acepta, siempre que los visitantes respeten a los Pokémon que viven allí.'),
    q('Vecinos de las termas', 'greta', 'greta', 'Captura dos Pokémon en el Camino Candente y vuelve con Greta.', 'Greta quiere que los visitantes sepan qué Pokémon viven en el Camino Candente. Registra dos capturas allí.', 'Greta dibuja un cartel con las especies. Los visitantes sabrán a quién no molestar.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_MAP', 'MAP_ONE_ISLAND_KINDLE_ROAD', 2)),
    q('Toallas para todos', 'greta', 'greta', 'Entrega dos Aguas Frescas a Greta.', 'Después de las termas, los visitantes necesitan beber. Greta te pide dos Aguas Frescas para el puesto.', 'Greta coloca el agua junto a la entrada de las termas.', kind='delivery', proof=('ITEM_FRESH_WATER', 2)),
    q('Recorrido de prueba', 'celso', 'greta', 'Ve desde Celso hasta Greta en 90 segundos.', 'Celso quiere saber cuánto se tarda del muelle a las termas. Hazlo en menos de 90 segundos.', 'Greta anota el tiempo. El recorrido cabe en una mañana.', kind='race', seconds=90),
    q('Las paradas', 'greta', 'celso', 'Lleva a Celso la lista de paradas.', 'Greta ha completado la lista de paradas. Llévasela a Celso.', 'Celso coloca el mapa del recorrido en el muelle.'),
    q('Bienvenida a Isla Uno', 'celso', 'iker', 'Cuenta a Iker que el recorrido está listo.', 'El recorrido ya está listo. Iker podrá recomendarlo a cada viajero.', 'Iker añade una línea a su registro: “recorrido de bienvenida”. Nadie llegará sin saber a dónde ir.', reward='ITEM_LAVA_COOKIE', quantity=3, bonus=dict(money=2500)),
])

# 21 --------------------------------------------------------------------------------------
actor2('lola', 'Lola', 'TwoIsland', 'Tengo la tienda de Isla Dos. Vendo menos de lo que regalo.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('teodora', 'Teodora', 'TwoIsland_CapeBrink', 'Vivo en el Cabo Extremo. Quienes vienen buscan a la anciana de los movimientos.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
T_TEODORA = trainer('TEODORA', 50, 'Female', 'Lass Frlg', ['Venusaur', 'Charizard', 'Blastoise'])
campaign('EL CABO EXTREMO', 'Una tendera y una vecina del cabo deciden si el camino al Cabo Extremo debe abrirse a todos.', [
    q('La tienda de Isla Dos', 'lola', 'lola', 'Habla con Lola en Isla Dos.', 'Lola tiene la tienda de Isla Dos. Cada día pasan viajeros preguntando por el Cabo Extremo.', 'Lola cuenta que el camino al cabo es estrecho y algunos viajeros se pierden.', flag='FLAG_IS_CHAMPION'),
    q('La vecina del cabo', 'lola', 'teodora', 'Busca a Teodora en el Cabo Extremo.', 'Teodora vive en el cabo. Lola quiere saber qué opina de tanta visita.', 'Teodora quiere ayudar a los viajeros, pero teme que el cabo pierda su calma.'),
    q('Tres iniciales', 'teodora', 'teodora', 'Acepta el combate de Teodora.', 'Teodora entrena a los tres Pokémon iniciales de Kanto. Te reta para ver si estás preparado.', 'Teodora recoge a sus Pokémon. Dice que entiendes por qué la gente viene hasta aquí.', kind='battle', proof=T_TEODORA),
    q('Botiquín del cabo', 'teodora', 'lola', 'Entrega dos Restaurar Todo a Lola.', 'Teodora quiere que la tienda de Lola tenga un botiquín para los viajeros que llegan agotados.', 'Lola guarda las medicinas en el mostrador.', kind='delivery', proof=('ITEM_FULL_RESTORE', 2)),
    q('Preguntas de Lola', 'lola', 'lola', 'Responde las preguntas de Lola.', 'Lola quiere preparar un cartel con consejos para los viajeros.', 'Lola escribe el cartel con tus respuestas.', kind='puzzle', questions=(
        ('Lola pregunta: ¿se puede ir al cabo sin agua?', False),
        ('Lola sigue: ¿conviene avisar en la tienda antes de ir?', True),
        ('Lola termina: ¿el cabo es un lugar para hacer ruido?', False))),
    q('Abrir el camino', 'lola', 'teodora', 'Ayuda a Teodora a decidir sobre el camino.', 'Teodora puede pedir que se ensanche el camino al cabo, o dejarlo como está y señalizarlo.', 'Teodora decide ensanchar el camino. Los viajeros llegarán con más facilidad.', kind='choice', choice='cabo_abierto', question='Teodora pregunta: ¿ensanchamos el camino al cabo?', alt_ending='Teodora decide dejar el camino estrecho, con carteles. El cabo conservará su calma.'),
    q('Avisar en la tienda', 'teodora', 'lola', 'Cuenta a Lola la decisión de Teodora.', 'Lola quiere saber qué pasará con el camino.', 'Lola se alegra. Los viajeros podrán llegar sin miedo a perderse.', choice='cabo_abierto', alt_ending='Lola pone su cartel junto al de Teodora. El camino seguirá siendo estrecho, pero claro.'),
    q('El Cabo Extremo', 'lola', 'teodora', 'Lleva a Teodora un regalo de Lola.', 'Lola te da un tarro de mermelada para Teodora como agradecimiento.', 'Teodora prueba la mermelada mirando al mar. Dice que el cabo nunca ha estado mejor.', reward='ITEM_PP_UP', quantity=1, choice='cabo_abierto', alt_ending='Teodora guarda la mermelada para los viajeros que lleguen cansados. El cabo sigue en calma.', bonus=dict(money=3000)),
])

# 22 --------------------------------------------------------------------------------------
actor2('anton', 'Antón', 'Route21_North', 'Remo hasta Canela cada semana. El mar entre islas tiene memoria.', 'OBJ_EVENT_GFX_HIKER')
actor2('sol', 'Sol', 'PalletTown', 'Mi abuelo era pescador en la Ruta 21. Quiero aprender lo que él sabía.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
campaign('EL REMERO DE LA RUTA 21', 'Una nieta de pescador y un remero veterano reconstruyen lo que se sabía del mar entre Paleta y Canela.', [
    q('Lo que sabía el abuelo', 'sol', 'sol', 'Habla con Sol en Pueblo Paleta.', 'Sol quiere aprender lo que sabía su abuelo, pescador de la Ruta 21.', 'Sol tiene un cuaderno del abuelo, pero está lleno de palabras que no entiende.', flag='FLAG_BADGE07_GET'),
    q('El remero', 'sol', 'anton', 'Busca a Antón en la Ruta 21.', 'Antón rema por la Ruta 21 desde hace décadas. Quizá entienda el cuaderno.', 'Antón reconoce la letra del abuelo de Sol. Eran amigos.'),
    q('Palabras del mar', 'anton', 'anton', 'Responde las preguntas de Antón.', 'Antón explica algunas palabras del cuaderno. Quiere saber si las entiendes.', 'Antón asiente. Ya puedes explicárselo a Sol.', kind='puzzle', questions=(
        ('Antón pregunta: ¿“mar de leche” significa calma?', True),
        ('Antón sigue: ¿se rema con viento del sur hacia Canela?', True),
        ('Antón termina: ¿los Tentacool salen con el agua fría?', False))),
    q('Lo que nada aquí', 'anton', 'anton', 'Captura dos Pokémon de tipo Agua y vuelve con Antón.', 'El cuaderno nombra especies que Antón ya no ve. Registra dos capturas de tipo Agua para comparar.', 'Antón compara tu lista con el cuaderno. Algunas especies han vuelto.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_TYPE', 'TYPE_WATER', 2)),
    q('El cuaderno anotado', 'anton', 'sol', 'Lleva el cuaderno anotado a Sol.', 'Antón ha escrito notas al margen. Lleva el cuaderno a Sol.', 'Sol lee las notas emocionada. Por primera vez entiende a su abuelo.'),
    q('Un remo para Sol', 'sol', 'anton', 'Entrega una Caña Buena a Antón.', 'Sol quiere aprender a pescar como su abuelo. Antón le enseñará si le llevas una Caña Buena.', 'Antón revisa la caña. Es buena; servirá para empezar.', kind='delivery', proof=('ITEM_GOOD_ROD', 1)),
    q('Primera salida', 'anton', 'sol', 'Avisa a Sol de que Antón la espera.', 'Antón está listo para la primera clase. Ve a avisar a Sol.', 'Sol corre hacia la playa con el cuaderno bajo el brazo.'),
    q('El remero de la Ruta 21', 'sol', 'anton', 'Lleva a Antón el agradecimiento de Sol.', 'Sol ha pescado su primer Pokémon. Te pide que le des las gracias a Antón.', 'Antón sonríe. Dice que el abuelo de Sol estaría orgulloso, y que el mar también.', reward='ITEM_NET_BALL', quantity=5, bonus=dict(money=2500)),
])

# 23 --------------------------------------------------------------------------------------
actor2('nerea_esp', 'Noelia', 'SeafoamIslands_1F', 'Estudio el hielo de las Islas Espuma. Cada año hay menos.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('lucio', 'Lucio', 'Route20', 'Llevo provisiones a la investigadora de las Islas Espuma. Nadie más se atreve.')
campaign('EL HIELO DE ESPUMA', 'Una investigadora y su proveedor miden cuánto hielo queda en las Islas Espuma.', [
    q('El proveedor', 'lucio', 'lucio', 'Habla con Lucio en la Ruta 20.', 'Lucio lleva provisiones a una investigadora de las Islas Espuma. Necesita ayuda con el último envío.', 'Lucio te da una caja para Noelia. Dice que te esperará aquí.', flag='FLAG_BADGE05_GET'),
    q('La investigadora del hielo', 'lucio', 'nerea_esp', 'Lleva la caja a Noelia en las Islas Espuma.', 'Noelia estudia el hielo de las Islas Espuma. Llévale la caja de Lucio.', 'Noelia abre la caja: instrumentos de medida. Te pide ayuda con las mediciones.'),
    q('Quién vive en el hielo', 'nerea_esp', 'nerea_esp', 'Observa un Seel y vuelve con Noelia.', 'Los Seel necesitan hielo. Noelia quiere saber si siguen en las islas.', 'Noelia respira aliviada. Siguen allí, aunque más al fondo.', kind='field', goal=('QUEST_DEX_SEEN', 'SPECIES_SEEL')),
    q('Hielo y agua', 'nerea_esp', 'nerea_esp', 'Captura dos Pokémon de tipo Hielo y vuelve con Noelia.', 'Noelia quiere comparar cuántos Pokémon de tipo Hielo hay hoy con los de hace diez años.', 'Noelia compara las listas. Hay menos, pero no han desaparecido.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_TYPE', 'TYPE_ICE', 2)),
    q('Abrigo para la investigadora', 'nerea_esp', 'nerea_esp', 'Entrega dos Antihielo a Noelia.', 'Noelia pasa mucho frío. Te pide dos Antihielo para el botiquín.', 'Noelia guarda los Antihielo. Podrá trabajar más horas.', kind='delivery', proof=('ITEM_ICE_HEAL', 2)),
    q('Las mediciones', 'nerea_esp', 'nerea_esp', 'Responde las preguntas de Noelia.', 'Noelia quiere comprobar que sabrás contar a Lucio lo que han medido.', 'Noelia te da sus notas. Lucio las llevará a Carmín.', kind='puzzle', questions=(
        ('Noelia pregunta: ¿hay menos hielo que hace diez años?', True),
        ('Noelia sigue: ¿han desaparecido los Pokémon de tipo Hielo?', False),
        ('Noelia termina: ¿medir cada año ayuda a entender el cambio?', True))),
    q('Notas para el continente', 'nerea_esp', 'lucio', 'Lleva las notas de Noelia a Lucio.', 'Lucio llevará las notas al continente para que más gente las conozca.', 'Lucio guarda las notas en una caja impermeable.'),
    q('El hielo de Espuma', 'lucio', 'nerea_esp', 'Lleva a Noelia la respuesta de Carmín.', 'El laboratorio de Carmín ha respondido: financiará la investigación de Noelia diez años más.', 'Noelia lee la carta dos veces. El hielo seguirá teniendo quien lo cuide.', reward='ITEM_NEVER_MELT_ICE', quantity=1, bonus=dict(money=3000)),
])

# 24 --------------------------------------------------------------------------------------
actor2('otto', 'Otto', 'Route4_PokemonCenter_1F', 'Vendía Magikarp en este Centro. Ahora los devuelvo al agua.', 'OBJ_EVENT_GFX_HIKER')
actor2('candela', 'Candela', 'VermilionCity_PokemonCenter_1F', 'Mi Gyarados empezó como un Magikarp que nadie quería.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
T_CANDELA = trainer('CANDELA', 30, 'Female', 'Lass Frlg', ['Gyarados', 'Goldeen', 'Psyduck'])
campaign('EL VENDEDOR DE MAGIKARP', 'Un antiguo vendedor de Magikarp quiere arreglar lo que hizo y aprende de quien crió uno sin prisa.', [
    q('Un pasado que pesa', 'otto', 'otto', 'Habla con Otto en el Centro Pokémon de la Ruta 4.', 'Otto vendía Magikarp a precios abusivos. Ahora quiere saber qué fue de ellos.', 'Otto guarda una lista de compradores. Quiere encontrar al menos uno.', flag='FLAG_BADGE02_GET'),
    q('Ver un Magikarp', 'otto', 'otto', 'Observa un Magikarp y vuelve con Otto.', 'Otto no ha mirado un Magikarp de verdad en años. Quiere que le cuentes cómo es uno libre.', 'Otto escucha en silencio. Nunca se había fijado en cómo nadan.', kind='field', goal=('QUEST_DEX_SEEN', 'SPECIES_MAGIKARP')),
    q('Una compradora', 'otto', 'candela', 'Busca a Candela en el Centro Pokémon de Carmín.', 'Candela compró un Magikarp a Otto hace años. Él quiere disculparse.', 'Candela escucha la disculpa. Dice que aquel Magikarp fue lo mejor que le pasó.'),
    q('Lo que creció', 'candela', 'candela', 'Acepta el combate de Candela.', 'Candela quiere enseñar a Otto en qué se convirtió su Magikarp.', 'El Gyarados de Candela ruge al terminar. Otto lo mira con la boca abierta.', kind='battle', proof=T_CANDELA),
    q('Paciencia', 'candela', 'otto', 'Lleva el consejo de Candela a Otto.', 'Candela dice que su secreto fue la paciencia. Lleva su consejo a Otto.', 'Otto anota la palabra en su lista, junto al nombre de Candela.'),
    q('Devolver al agua', 'otto', 'otto', 'Captura dos Pokémon de tipo Agua y vuelve con Otto.', 'Otto quiere aprender a cuidar Pokémon de agua antes de ayudar a otros. Registra dos capturas para enseñarle.', 'Otto practica con tus consejos. Sus manos ya no tiemblan al tocar el agua.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_TYPE', 'TYPE_WATER', 2)),
    q('Cebo para el estanque', 'otto', 'candela', 'Entrega tres Bayas Aranja a Candela.', 'Candela cuida un estanque para Magikarp abandonados. Otto quiere colaborar enviándole bayas.', 'Candela echa las bayas al estanque. Los Magikarp saltan.', kind='delivery', proof=('ITEM_ORAN_BERRY', 3)),
    q('El vendedor de Magikarp', 'candela', 'otto', 'Lleva a Otto la invitación de Candela.', 'Candela invita a Otto a ayudar en el estanque cada semana.', 'Otto acepta. Ya no vende Magikarp: los cuida, sin prisa.', reward='ITEM_RARE_CANDY', quantity=2, bonus=dict(money=2000)),
])
