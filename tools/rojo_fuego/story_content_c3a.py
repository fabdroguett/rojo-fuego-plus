"""Batch C3, part A: early Kanto, from Pallet Town to Lavender Town.

Appended to story_content.CAMPAIGNS; see story_content.RECONVERTED_C3 for slot assignment.
"""
from story_content import (ACTORS_V2, CAMPAIGNS, CHOICES, CLUES, CLUE_BASE, PLACES, RECONVERTED_C3,
                           TRAINERS, actor2, q)

PLACES.update({
    'PalletTown': 'Pueblo Paleta', 'Route1': 'Ruta 1', 'PewterCity_Museum_1F': 'Museo de Plateada',
    'MtMoon_1F': 'Monte Moon, planta 1', 'Route25': 'Ruta 25', 'VermilionCity_PokemonFanClub': 'Club de Fans, Carmín',
    'Route11': 'Ruta 11', 'DiglettsCave_B1F': 'Cueva Diglett', 'Route9': 'Ruta 9',
    'LavenderTown_PokemonCenter_1F': 'Centro Pokémon de Lavanda', 'PokemonTower_1F': 'Torre Pokémon, planta 1',
    'Route6': 'Ruta 6', 'PewterCity_PokemonCenter_1F': 'Centro Pokémon de Plateada',
})
CHOICES.update({'piedra_al_museo': 2, 'musica_en_la_torre': 3})


def campaign(title, summary, rows):
    CAMPAIGNS.append((title, summary, rows))
    RECONVERTED_C3.append(title)


def trainer(name, level, gender, pic, species):
    TRAINERS.append((name, level, gender, pic, species))
    return len(TRAINERS) - 1


def next_scene(row):
    return sum(len(rows) for _, _, rows in CAMPAIGNS) + row


# 1 ---------------------------------------------------------------------------------------
actor2('tobi', 'Tobi', 'PalletTown', 'Reparto el correo de Paleta a pie. Así conozco a todos los vecinos y a sus Pokémon.', 'OBJ_EVENT_GFX_HIKER')
actor2('meli', 'Meli', 'ViridianCity', 'Espero cartas de mi hermano, que viaja por Kanto. Siempre llegan con barro.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('teodoro', 'Teodoro', 'Route1', 'Camino por la Ruta 1 cada mañana. Los Rattata ya me saludan.')
T_TOBI = trainer('TOBI', 9, 'Male', 'Youngster Frlg', ['Pidgey', 'Rattata'])
campaign('EL CARTERO DE PALETA', 'Tobi reparte cartas entre Paleta y Verde, y aprende que el camino también es parte del mensaje.', [
    q('Una saca demasiado llena', 'tobi', 'meli', 'Lleva la primera carta a Meli, en Ciudad Verde.', 'Tobi tiene una saca llena y una pierna cansada. Te pide llevar una carta a Meli, en Ciudad Verde. No tiene prisa: solo quiere que llegue.', 'Meli abre la carta allí mismo. Es de su hermano, que cuenta que ha visto su primer Pidgeotto.', flag='FLAG_BADGE01_GET'),
    q('Una respuesta por escrito', 'meli', 'tobi', 'Devuelve la respuesta de Meli a Tobi.', 'Meli escribe una respuesta corta y te pide que se la des a Tobi para la próxima ruta del correo.', 'Tobi guarda la carta en el bolsillo de las urgentes. Dice que las respuestas pesan menos que las preguntas.'),
    q('El paseo de Teodoro', 'tobi', 'teodoro', 'Busca a Teodoro en la Ruta 1.', 'Teodoro espera un paquete pero nunca está en casa: camina por la Ruta 1 a todas horas. Encuéntralo y pregúntale dónde quiere recibirlo.', 'Teodoro se ríe. Quiere recibir el paquete en la ruta, junto al cartel, porque allí pasa más tiempo que en su casa.'),
    q('Tres sustos en la hierba', 'teodoro', 'teodoro', 'Gana tres combates contra salvajes y vuelve con Teodoro.', 'Teodoro cuenta que los salvajes asustan a los repartidores nuevos. Gana tres combates en la hierba para que Tobi vea que el camino es seguro.', 'Teodoro asiente. Los Pokémon de la Ruta 1 solo defendían su sitio; ahora el repartidor sabrá por dónde pasar.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_WILD_WIN', '0', 3)),
    q('El paquete de Teodoro', 'teodoro', 'tobi', 'Entrega dos Pociones a Tobi para su botiquín.', 'Teodoro sugiere que Tobi lleve un botiquín en la saca. Entrégale dos Pociones de tu parte.', 'Tobi guarda las Pociones junto a las cartas. Ahora sabe que repartir también es cuidar a quien acompaña.', kind='delivery', proof=('ITEM_POTION', 2)),
    q('Un combate entre carteros', 'tobi', 'tobi', 'Acepta el combate de práctica de Tobi.', 'Tobi quiere saber si su Pidgey está listo para acompañarlo por las rutas. Te propone un combate de práctica en Paleta.', 'Tobi recoge a su Pidgey con una sonrisa. No ganó, pero ahora sabe qué entrenar antes de salir.', kind='battle', proof=T_TOBI),
    q('Correo hasta Verde', 'tobi', 'meli', 'Avisa a Meli de que el correo vuelve a ser diario.', 'Con el camino seguro y el botiquín listo, Tobi repartirá cada día. Ve a contárselo a Meli.', 'Meli sonríe. Ahora podrá contestar a su hermano sin esperar una semana.'),
    q('Las cartas siguen llegando', 'meli', 'tobi', 'Cierra la ruta del correo con Tobi en Paleta.', 'Meli te entrega una nota de agradecimiento para Tobi. Llévala a Paleta para cerrar la primera semana del correo diario.', 'Tobi cuelga la nota junto a su puerta. El correo de Paleta ya no depende de un solo par de piernas.', reward='ITEM_SUPER_POTION', quantity=3, bonus=dict(money=1000)),
])

# 2 ---------------------------------------------------------------------------------------
actor2('gema', 'Gema', 'PewterCity_Museum_1F', 'Organizo visitas para quienes nunca han entrado en un museo. Las preguntas son lo mejor.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('lazaro', 'Lázaro', 'Route3', 'Busco piedras con formas curiosas. Casi todas se quedan donde estaban.', 'OBJ_EVENT_GFX_HIKER')
actor2('nita', 'Nita', 'PewterCity', 'Mi abuela trabajó en la cantera. Dice que las rocas guardan nombres de personas.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
campaign('EL MUSEO SIN VITRINAS', 'Gema quiere sacar el museo a la calle sin llevarse nada de su lugar.', [
    q('Un museo a la intemperie', 'gema', 'nita', 'Habla con Nita en Ciudad Plateada.', 'Gema quiere enseñar geología fuera del museo. Nita conoce historias de la vieja cantera; pregúntale si quiere participar.', 'Nita acepta encantada. Su abuela contaba que cada veta de la cantera tenía un nombre.', flag='FLAG_BADGE01_GET'),
    q('Rocas que se mueven', 'nita', 'nita', 'Observa un Geodude y vuelve con Nita.', 'Nita dice que muchos visitantes confunden a los Geodude con piedras. Observa uno en Ruta 3 o en Monte Moon para contarles cómo distinguirlos.', 'Nita apunta tus observaciones. La visita empezará enseñando que no todas las piedras están quietas.', kind='field', goal=('QUEST_DEX_SEEN', 'SPECIES_GEODUDE')),
    q('El buscador de formas', 'nita', 'lazaro', 'Busca a Lázaro en la Ruta 3.', 'Lázaro recorre la Ruta 3 buscando piedras curiosas. Gema quiere que guíe la parte exterior de la visita.', 'Lázaro acepta con una condición: nadie se lleva piedras a casa. Las fotos bastan.'),
    q('Preguntas de Lázaro', 'lazaro', 'lazaro', 'Responde las preguntas de Lázaro.', 'Lázaro quiere comprobar que sabes explicar la regla de la visita. Responde a sus preguntas antes de que la pruebe con los niños.', 'Lázaro aplaude. Has entendido que el mejor recuerdo de una roca es dejarla en su sitio.', kind='puzzle', questions=(
        ('Lázaro pregunta: ¿se puede llevar una piedra de recuerdo?', False),
        ('Lázaro sigue: ¿una foto sirve como recuerdo?', True),
        ('Lázaro termina: ¿se toca un fósil sin guantes?', False))),
    q('Cartelas portátiles', 'lazaro', 'gema', 'Entrega dos Repelentes a Gema para la visita.', 'Los grupos pasarán por zonas con muchos Zubat. Gema necesita dos Repelentes para que los visitantes no se asusten.', 'Gema guarda los Repelentes en la mochila de la visita. Nadie tendrá que correr por la cueva.', kind='delivery', proof=('ITEM_REPEL', 2)),
    q('La primera visita', 'gema', 'nita', 'Avisa a Nita de que la visita está lista.', 'Todo está preparado. Ve a avisar a Nita: ella abrirá la visita con la historia de su abuela.', 'Nita ensaya en voz alta. La historia de la cantera emociona incluso a quien la cuenta.'),
    q('Lo que dijeron los niños', 'nita', 'lazaro', 'Cuenta a Lázaro cómo fue la visita.', 'La primera visita fue un éxito. Lázaro quiere saber qué preguntaron los niños para preparar la siguiente.', 'Lázaro anota las preguntas. La más repetida: si las piedras también tienen familia.'),
    q('El museo sin vitrinas', 'lazaro', 'gema', 'Cierra el proyecto con Gema en el museo.', 'Lázaro te entrega sus notas para Gema. Con ellas el museo podrá repetir la visita cada mes.', 'Gema cuelga un mapa nuevo en el museo: la mitad de las piezas siguen fuera, donde deben estar.', reward='ITEM_RARE_CANDY', quantity=1, bonus=dict(money=1500)),
])

# 3 ---------------------------------------------------------------------------------------
actor2('celia', 'Celia', 'MtMoon_1F', 'Estudio a los Clefairy. Bailan cuando creen que nadie mira.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('rufo', 'Rufo', 'Route4', 'Coleccionaba piedras lunares. Ahora no sé qué hacer con ellas.', 'OBJ_EVENT_GFX_HIKER')
T_RUFO = trainer('RUFO', 14, 'Male', 'Hiker Frlg', ['Geodude', 'Zubat', 'Sandshrew'])
campaign('ECOS DEL MONTE MOON', 'Una investigadora y un antiguo coleccionista discuten qué hacer con lo que el monte guarda.', [
    q('Pasos en la oscuridad', 'celia', 'celia', 'Habla con Celia en Monte Moon.', 'Celia estudia el comportamiento de los Clefairy y necesita ayuda. Encuéntrala en la primera planta de Monte Moon.', 'Celia te cuenta que alguien ha estado recogiendo piedras lunares y los Clefairy están inquietos.', flag='FLAG_BADGE01_GET'),
    q('Un baile en silencio', 'celia', 'celia', 'Observa un Clefairy y vuelve con Celia.', 'Celia quiere saber si los Clefairy siguen saliendo pese a las molestias. Basta con ver uno.', 'Celia respira aliviada. Siguen allí, aunque más escondidos que antes.', kind='field', goal=('QUEST_DEX_SEEN', 'SPECIES_CLEFAIRY')),
    q('El coleccionista', 'celia', 'rufo', 'Busca a Rufo en la Ruta 4.', 'Celia sospecha de un coleccionista que vive junto a la Ruta 4. Habla con él antes de sacar conclusiones.', 'Rufo admite que recogía piedras lunares, pero dice que dejó de hacerlo hace semanas.'),
    q('Pruebas de buena fe', 'rufo', 'rufo', 'Acepta el combate de Rufo.', 'Rufo quiere demostrar que no tiene nada que esconder. Propone un combate: si ganas, te enseñará su colección.', 'Rufo abre una caja con doce piedras lunares. Las guardó porque no sabía a quién devolverlas.', kind='battle', proof=T_RUFO),
    q('Una piedra para decidir', 'rufo', 'celia', 'Lleva una Piedra Lunar a Celia.', 'Rufo quiere devolver las piedras, pero no sabe dónde. Lleva una Piedra Lunar a Celia como gesto de buena voluntad.', 'Celia examina la piedra. Ahora toca decidir qué hacer con las demás.', kind='delivery', proof=('ITEM_MOON_STONE', 1)),
    q('El museo o el monte', 'celia', 'celia', 'Ayuda a Celia a decidir el destino de las piedras.', 'Las piedras podrían ir al museo de Plateada, donde la gente aprendería de ellas, o volver al monte con los Clefairy. Celia quiere tu opinión.', 'Celia decide devolver las piedras al monte. Los Clefairy volverán a bailar alrededor de ellas.', kind='choice', choice='piedra_al_museo', question='Celia pregunta: ¿llevamos las piedras al museo?', alt_ending='Celia decide llevar las piedras al museo, con una cartela que explica de dónde vienen y por qué no deben recogerse más.'),
    q('Avisar a Rufo', 'celia', 'rufo', 'Cuenta a Rufo la decisión.', 'Rufo espera en la Ruta 4. Ve a contarle qué se hará con su colección.', 'Rufo asiente. Le alivia saber que las piedras tendrán un destino pensado.', choice='piedra_al_museo', alt_ending='Rufo se ofrece a guiar visitas en el museo. Dice que conoce cada piedra por su nombre.'),
    q('Ecos del Monte Moon', 'rufo', 'celia', 'Cierra la investigación con Celia.', 'Rufo te pide que lleves sus notas a Celia: dónde encontró cada piedra.', 'Celia completa su mapa. El monte vuelve a estar en calma y Rufo ya no colecciona: observa.', reward='ITEM_MOON_STONE', quantity=1, choice='piedra_al_museo', alt_ending='Celia completa la ficha del museo con las notas de Rufo. Cada piedra expuesta lleva el nombre del lugar donde estuvo.', bonus=dict(money=2000)),
])

# 4 ---------------------------------------------------------------------------------------
actor2('pilar', 'Pilar', 'CeruleanCity', 'Organizo la feria de Celeste. Este año habrá carreras, música y nada de prisas.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('arturo', 'Arturo', 'Route24', 'Cuido la meta de la carrera. Lo importante es llegar, no llegar primero.')
actor2('dana', 'Dana', 'Route25', 'Vendo limonada en la feria. Bueno, cuando no me la bebo yo.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
T_ARTURO = trainer('ARTURO', 18, 'Male', 'Camper Frlg', ['Oddish', 'Psyduck', 'Mankey'])
campaign('LA FERIA DE CELESTE', 'Pilar prepara una feria donde nadie compite contra nadie, salvo contra el reloj.', [
    q('Preparativos', 'pilar', 'arturo', 'Habla con Arturo en la Ruta 24.', 'Pilar organiza la feria de Celeste y necesita a alguien que revise la meta de la carrera en la Ruta 24.', 'Arturo enseña la meta: un banderín junto al puente. La carrera empieza en la plaza de Celeste.', flag='FLAG_BADGE02_GET'),
    q('Contar a los vecinos', 'arturo', 'arturo', 'Captura dos Pokémon en la Ruta 24 y vuelve con Arturo.', 'Arturo quiere colocar carteles sobre los Pokémon de la ruta para que los corredores no los molesten. Registra dos capturas allí.', 'Arturo dibuja los carteles. Los corredores sabrán que la hierba de la ruta tiene vecinos.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_MAP', 'MAP_ROUTE24', 2)),
    q('Limonada para la feria', 'arturo', 'dana', 'Busca a Dana en la Ruta 25.', 'Dana vende bebidas en la feria pero se ha quedado sin provisiones. Búscala en la Ruta 25.', 'Dana cuenta que necesita una Limonada para preparar la receta de la feria.'),
    q('La receta de Dana', 'dana', 'dana', 'Entrega una Limonada a Dana.', 'Dana quiere probar su receta antes de la feria. Entrégale una Limonada.', 'Dana prueba la receta y pone cara seria. Luego sonríe: está perfecta.', kind='delivery', proof=('ITEM_LEMONADE', 1)),
    q('La carrera de prueba', 'pilar', 'arturo', 'Corre desde Pilar hasta Arturo en 40 segundos.', 'Pilar quiere comprobar que la carrera es segura. Sal desde la plaza y llega hasta Arturo, en la Ruta 24, en menos de 40 segundos.', 'Arturo detiene el cronómetro. El recorrido es seguro y divertido; la feria puede empezar.', kind='race', seconds=40),
    q('Un desafío amistoso', 'arturo', 'arturo', 'Acepta el combate amistoso de Arturo.', 'Arturo quiere abrir la feria con un combate de exhibición. Te elige como rival.', 'El público aplaude a los dos. En esta feria nadie pierde del todo.', kind='battle', proof=T_ARTURO),
    q('Abrir la feria', 'arturo', 'pilar', 'Avisa a Pilar de que todo está listo.', 'La meta, los carteles y la limonada están listos. Ve a avisar a Pilar.', 'Pilar abre la feria con un discurso corto: gana quien disfruta y quien deja disfrutar.'),
    q('La feria de Celeste', 'pilar', 'dana', 'Celebra el final de la feria con Dana.', 'La feria termina. Dana te espera en la Ruta 25 con la última limonada.', 'Dana brinda por la próxima feria. Pilar ya ha reservado la plaza para el año que viene.', reward='ITEM_SUPER_POTION', quantity=2, bonus=dict(money=1500)),
])

# 5 ---------------------------------------------------------------------------------------
actor2('ramon', 'Ramón', 'VermilionCity', 'Llevo veinte años en el muelle. Los barcos cambian; el mar no.', 'OBJ_EVENT_GFX_HIKER')
actor2('ines_fan', 'Irma', 'VermilionCity_PokemonFanClub', 'Escribo el boletín del Club de Fans. Necesito historias de verdad.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('kiko', 'Kiko', 'Route11', 'Cuido el faro de la Ruta 11. No es un faro, es una farola, pero yo lo llamo faro.')
T_KIKO = trainer('KIKO', 20, 'Male', 'Youngster Frlg', ['Drowzee', 'Ekans', 'Spearow'])
campaign('MAREAS DE CARMÍN', 'Un viejo estibador y una redactora cuentan la historia del puerto antes de que cambie.', [
    q('Historias del muelle', 'ines_fan', 'ramon', 'Entrevista a Ramón en el muelle de Carmín.', 'Irma quiere escribir sobre el puerto para el boletín del Club de Fans. Ramón trabaja en el muelle desde hace veinte años.', 'Ramón acepta hablar, pero solo si lo acompañas un día de trabajo.', flag='FLAG_BADGE02_GET'),
    q('Un día en el muelle', 'ramon', 'ramon', 'Entrega dos Superpociones a Ramón para los Pokémon del puerto.', 'Los Machop del muelle trabajan mucho. Ramón te pide dos Superpociones para que descansen bien.', 'Ramón reparte las Superpociones. Los Machop del muelle se lo agradecen a su manera.', kind='delivery', proof=('ITEM_SUPER_POTION', 2)),
    q('La farola de la Ruta 11', 'ramon', 'kiko', 'Busca a Kiko en la Ruta 11.', 'Ramón dice que Kiko conoce el mejor punto para ver los barcos. Búscalo en la Ruta 11.', 'Kiko te enseña su farola. Dice que desde allí se ven los barcos que llegan de noche.'),
    q('Vecinos de la ruta', 'kiko', 'kiko', 'Captura dos Pokémon en la Ruta 11 y vuelve con Kiko.', 'Kiko quiere incluir en el boletín a los Pokémon de la ruta. Registra dos capturas allí.', 'Kiko anota los nombres. El boletín tendrá su propia sección de vecinos.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_MAP', 'MAP_ROUTE11', 2)),
    q('El reto de Kiko', 'kiko', 'kiko', 'Acepta el combate de Kiko.', 'Kiko quiere aparecer en el boletín con una historia propia. Te desafía a un combate.', 'Kiko sonríe aunque pierda. Dice que la historia será buena igualmente.', kind='battle', proof=T_KIKO),
    q('Notas para Irma', 'kiko', 'ines_fan', 'Lleva las notas al Club de Fans.', 'Con las historias de Ramón y Kiko, Irma ya puede escribir el boletín. Llévale las notas.', 'Irma lee las notas con atención. Le faltan las palabras del propio Ramón.'),
    q('La última pregunta', 'ines_fan', 'ramon', 'Haz a Ramón la última pregunta del boletín.', 'Irma quiere cerrar el boletín con una pregunta: qué le diría Ramón a quien empieza a trabajar en el puerto.', 'Ramón piensa un rato. Responde que el mar enseña a esperar, y que esperar también es trabajar.'),
    q('Mareas de Carmín', 'ramon', 'ines_fan', 'Entrega la respuesta de Ramón a Irma.', 'Lleva la respuesta de Ramón al Club de Fans. Irma cerrará el boletín con ella.', 'El boletín se agota en una mañana. Ramón guarda un ejemplar junto a su caja de herramientas.', reward='ITEM_GREAT_BALL', quantity=5, bonus=dict(money=2000)),
])

# 6 ---------------------------------------------------------------------------------------
actor2('lope', 'Lope', 'DiglettsCave_B1F', 'Mido los túneles de Diglett. Nunca están igual dos días seguidos.', 'OBJ_EVENT_GFX_HIKER')
actor2('ari', 'Ari', 'Route2', 'Mi huerto está junto a la cueva. Los Diglett me ayudan a remover la tierra.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
campaign('EL TÚNEL DE DIGLETT', 'Una agricultora y un topógrafo descubren que los Diglett trabajan mejor de lo que pensaban.', [
    q('Tierra removida', 'ari', 'ari', 'Habla con Ari en la Ruta 2.', 'Ari cultiva un huerto junto a la Cueva Diglett. Algo está removiendo su tierra cada noche.', 'Ari sospecha de los Diglett, pero no quiere echarlos. Quiere entenderlos.', flag='FLAG_BADGE02_GET'),
    q('Quién cava de noche', 'ari', 'ari', 'Observa un Diglett y vuelve con Ari.', 'Ari quiere confirmar que son Diglett. Observa uno en la cueva.', 'Ari ríe. Son Diglett, y su tierra está más esponjosa que nunca.', kind='field', goal=('QUEST_DEX_SEEN', 'SPECIES_DIGLETT')),
    q('El topógrafo', 'ari', 'lope', 'Busca a Lope en la Cueva Diglett.', 'Lope mide los túneles de la cueva. Ari quiere saber si los túneles llegan hasta su huerto.', 'Lope confirma que un túnel nuevo pasa bajo el huerto. Los Diglett buscan raíces blandas.'),
    q('Mapas que cambian', 'lope', 'lope', 'Responde las preguntas de Lope.', 'Lope explica cómo trabajan los Diglett y quiere saber si lo has entendido.', 'Lope asiente. Quien entiende a los Diglett deja de verlos como plaga.', kind='puzzle', questions=(
        ('Lope pregunta: ¿los Diglett airean la tierra?', True),
        ('Lope sigue: ¿conviene tapar sus túneles?', False),
        ('Lope termina: ¿prefieren la tierra blanda?', True))),
    q('Bayas para compartir', 'lope', 'ari', 'Entrega tres Bayas Aranja a Ari.', 'Lope sugiere que Ari plante bayas en el borde del huerto para los Diglett. Llévale tres Bayas Aranja.', 'Ari planta las bayas junto al túnel. Los Diglett tendrán su propio rincón.', kind='delivery', proof=('ITEM_ORAN_BERRY', 3)),
    q('Un huerto compartido', 'ari', 'lope', 'Cuenta a Lope el nuevo plan de Ari.', 'Ari quiere que Lope dibuje el túnel en su mapa para no pisarlo al plantar.', 'Lope marca el túnel con una línea verde. Será el primer mapa con un huerto dentro.'),
    q('Medir sin molestar', 'lope', 'lope', 'Gana tres combates contra salvajes cerca de la cueva y vuelve.', 'Lope quiere medir el túnel nuevo, pero algunos Pokémon lo defienden. Gana tres combates contra salvajes para que pueda trabajar.', 'Lope termina la medición. Los Pokémon vuelven a su sitio en cuanto él se aparta.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_WILD_WIN', '0', 3)),
    q('El túnel de Diglett', 'lope', 'ari', 'Entrega el mapa a Ari.', 'Lope te da el mapa terminado para Ari. Con él sabrá dónde plantar sin dañar los túneles.', 'Ari cuelga el mapa en la puerta del huerto. Las cosechas mejoran; los Diglett, también.', reward='ITEM_RARE_CANDY', quantity=1, bonus=dict(money=1500)),
])

# 7 ---------------------------------------------------------------------------------------
actor2('volta', 'Volta', 'Route10_PokemonCenter_1F', 'Revisé la Central durante años. Ahora la luz de la ruta depende de que vuelva a funcionar.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('chispa', 'Chispa', 'Route9', 'Entreno Pokémon eléctricos. Dicen que tengo el pelo de punta por eso.')
T_CHISPA = trainer('CHISPA', 24, 'Male', 'Camper Frlg', ['Voltorb', 'Magnemite', 'Pikachu'])
campaign('LA CENTRAL APAGADA', 'Volta quiere devolver la luz a la Ruta 10 sin echar a los Pokémon que viven en la Central.', [
    q('Una ruta a oscuras', 'volta', 'volta', 'Habla con Volta en el Centro de la Ruta 10.', 'La Ruta 10 lleva semanas sin luz. Volta trabajó en la Central y cree saber por qué.', 'Volta explica que la Central funciona con la ayuda de Pokémon eléctricos que se han ido.', flag='FLAG_BADGE03_GET'),
    q('Chispas en la Ruta 9', 'volta', 'chispa', 'Busca a Chispa en la Ruta 9.', 'Chispa entrena Pokémon eléctricos en la Ruta 9. Volta quiere pedirle consejo.', 'Chispa dice que los Pokémon eléctricos se fueron porque la Central estaba demasiado ruidosa.'),
    q('Un reto con chispa', 'chispa', 'chispa', 'Acepta el combate de Chispa.', 'Chispa solo ayuda a quien conoce bien a los Pokémon eléctricos. Te reta a un combate.', 'Chispa se rasca el pelo, que se pone aún más de punta. Acepta ayudar.', kind='battle', proof=T_CHISPA),
    q('Corriente viva', 'chispa', 'chispa', 'Captura tres Pokémon de tipo Eléctrico y vuelve con Chispa.', 'Chispa quiere saber qué especies eléctricas hay cerca. Registra tres capturas de tipo Eléctrico.', 'Chispa repasa tu lista. Hay suficientes Pokémon eléctricos para una Central tranquila.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_TYPE', 'TYPE_ELECTRIC', 3)),
    q('Silencio para la Central', 'chispa', 'volta', 'Lleva el informe de Chispa a Volta.', 'Chispa propone apagar las máquinas viejas y dejar solo las silenciosas. Llévale la idea a Volta.', 'Volta asiente. Con menos ruido, los Pokémon podrían volver.'),
    q('Piezas de repuesto', 'volta', 'volta', 'Entrega dos Antiparalizadores a Volta.', 'Volta revisará las máquinas, y a veces dan calambrazos. Te pide dos Antiparalizadores para el botiquín.', 'Volta guarda los Antiparalizadores. Ya puede trabajar sin miedo.', kind='delivery', proof=('ITEM_PARALYZE_HEAL', 2)),
    q('La luz vuelve', 'volta', 'chispa', 'Cuenta a Chispa que la Central está tranquila.', 'Volta ha apagado las máquinas ruidosas. Ve a contárselo a Chispa.', 'Chispa sonríe: sus Pokémon ya notan la Central más tranquila.'),
    q('La Central apagada', 'chispa', 'volta', 'Cierra el proyecto con Volta.', 'Chispa te da su lista de especies para Volta. Con ella sabrán quién vive en la Central.', 'La Ruta 10 vuelve a tener luz por la noche. En la Central, los Pokémon eléctricos duermen sin ruido.', reward='ITEM_ULTRA_BALL', quantity=3, bonus=dict(money=2500)),
])

# 8 ---------------------------------------------------------------------------------------
actor2('lira', 'Lira', 'LavenderTown_PokemonCenter_1F', 'Toco el violín en Lavanda. La música ayuda a recordar sin hundirse.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('basilio', 'Basilio', 'PokemonTower_1F', 'Cuido la entrada de la torre. Escucho más de lo que hablo.')
actor2('vela', 'Vela', 'LavenderTown', 'Mi Growlithe se fue hace un año. Todavía no sé cómo despedirme.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
campaign('MÚSICA EN LAVANDA', 'Lira quiere tocar en la Torre Pokémon, y el pueblo debe decidir si el silencio también es una forma de recordar.', [
    q('Una melodía para Lavanda', 'lira', 'basilio', 'Habla con Basilio en la Torre Pokémon.', 'Lira quiere tocar música en la Torre Pokémon para quienes la visitan. Pregunta a Basilio qué opina.', 'Basilio duda. Algunos visitantes prefieren el silencio; otros quizá agradecerían la música.', flag='FLAG_BADGE03_GET'),
    q('Escuchar a Vela', 'basilio', 'vela', 'Habla con Vela en Pueblo Lavanda.', 'Basilio sugiere preguntar a Vela, que perdió a su Growlithe hace un año y visita la torre a menudo.', 'Vela cuenta que a veces el silencio de la torre le pesa demasiado.'),
    q('Ver sin miedo', 'vela', 'vela', 'Observa un Gastly y vuelve con Vela.', 'Vela tiene miedo de los Gastly de la torre. Cree que, si alguien los observa y le cuenta cómo son, perderá el miedo.', 'Vela escucha tu descripción. Los Gastly solo juegan; quizá también acompañan.', kind='field', goal=('QUEST_DEX_SEEN', 'SPECIES_GASTLY')),
    q('Una flor para la entrada', 'vela', 'basilio', 'Entrega una Muñeca Poké a Basilio.', 'Vela quiere dejar algo en la entrada de la torre. Basilio sugiere una Muñeca Poké, que los niños dejan como recuerdo.', 'Basilio coloca la muñeca junto a la puerta. Vela sonríe por primera vez en mucho tiempo.', kind='delivery', proof=('ITEM_POKE_DOLL', 1)),
    q('Preguntar al pueblo', 'basilio', 'lira', 'Lleva la opinión de Basilio y Vela a Lira.', 'Basilio y Vela tienen opiniones distintas sobre la música. Lira quiere escucharlas antes de decidir.', 'Lira entiende las dos posturas. Te pide ayuda para decidir.'),
    q('Música o silencio', 'lira', 'lira', 'Ayuda a Lira a decidir.', 'Lira podría tocar dentro de la torre o solo en la plaza del pueblo, dejando la torre en silencio.', 'Lira decide tocar en la plaza. La torre conserva su silencio, y quien quiera música sabe dónde encontrarla.', kind='choice', choice='musica_en_la_torre', question='Lira pregunta: ¿toco dentro de la Torre Pokémon?', alt_ending='Lira decide tocar en la torre una vez por semana, muy suave, en la planta de entrada.'),
    q('La primera canción', 'lira', 'vela', 'Invita a Vela a escuchar a Lira.', 'Lira ha compuesto una canción para Growlithe. Ve a invitar a Vela.', 'Vela escucha la canción con los ojos cerrados. Dice que ya sabe cómo despedirse.', choice='musica_en_la_torre', alt_ending='Vela escucha la canción en la entrada de la torre. Dice que, por primera vez, el lugar no le pesa.'),
    q('Música en Lavanda', 'vela', 'lira', 'Agradece a Lira de parte de Vela.', 'Vela te pide que le des las gracias a Lira. Quiere que la canción se toque cada año.', 'Lira promete tocarla cada año. Lavanda tiene ahora una canción propia para recordar.', reward='ITEM_REVIVE', quantity=2, bonus=dict(money=1500)),
])
