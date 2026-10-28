"""Batch C3, part B: mid Kanto, from Celadon City to Cinnabar Island.

Appended to story_content.CAMPAIGNS; see story_content.RECONVERTED_C3 for slot assignment.
"""
from story_content import CAMPAIGNS, CHOICES, PLACES, RECONVERTED_C3, TRAINERS, actor2, q

PLACES.update({
    'CeladonCity': 'Ciudad Azulona', 'CeladonCity_GameCorner': 'Casino de Azulona', 'Route7': 'Ruta 7',
    'Route8': 'Ruta 8', 'SaffronCity_Dojo': 'Dojo de Azafrán', 'SaffronCity_PokemonCenter_1F': 'Centro Pokémon de Azafrán',
    'Route12': 'Ruta 12', 'Route13': 'Ruta 13', 'Route14': 'Ruta 14', 'Route15': 'Ruta 15', 'Route16': 'Ruta 16',
    'Route18': 'Ruta 18', 'FuchsiaCity': 'Ciudad Fucsia', 'FuchsiaCity_WardensHouse': 'Casa del Guarda, Fucsia',
    'FuchsiaCity_PokemonCenter_1F': 'Centro Pokémon de Fucsia', 'Route19': 'Ruta 19', 'Route20': 'Ruta 20',
    'CinnabarIsland': 'Isla Canela', 'CinnabarIsland_PokemonCenter_1F': 'Centro Pokémon de Canela',
    'CeladonCity_PokemonCenter_1F': 'Centro Pokémon de Azulona', 'SeafoamIslands_1F': 'Islas Espuma',
    'PowerPlant': 'Central Energía',
})
CHOICES.update({'casino_cerrado': 4, 'dojo_abierto': 5, 'safari_sin_redes': 6, 'canela_reconstruye': 7})


def campaign(title, summary, rows):
    CAMPAIGNS.append((title, summary, rows))
    RECONVERTED_C3.append(title)


def trainer(name, level, gender, pic, species):
    TRAINERS.append((name, level, gender, pic, species))
    return len(TRAINERS) - 1


# 9 ---------------------------------------------------------------------------------------
actor2('flora', 'Flora', 'CeladonCity', 'Cuido los parterres de Azulona. Cada flor tiene un Pokémon que la visita.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('genaro', 'Genaro', 'Route7', 'Vendo semillas en la Ruta 7. Las que no se venden, las planto.', 'OBJ_EVENT_GFX_HIKER')
actor2('aroa', 'Aroa', 'CeladonCity_PokemonCenter_1F', 'Hago perfumes con pétalos caídos. Nunca arranco una flor.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
campaign('JARDINES DE AZULONA', 'Flora quiere que los jardines de Azulona vuelvan a atraer Pokémon, sin arrancar ni una flor.', [
    q('Parterres vacíos', 'flora', 'flora', 'Habla con Flora en Ciudad Azulona.', 'Flora cuida los jardines de Azulona, pero este año casi no llegan Pokémon a las flores.', 'Flora sospecha que faltan plantas que den néctar. Necesita ayuda para elegir cuáles.', flag='FLAG_BADGE04_GET'),
    q('Quién visita las flores', 'flora', 'flora', 'Captura tres Pokémon de tipo Planta y vuelve con Flora.', 'Flora quiere saber qué Pokémon de tipo Planta hay cerca para elegir flores que les gusten.', 'Flora repasa tu lista. Ya sabe qué plantar en cada parterre.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_TYPE', 'TYPE_GRASS', 3)),
    q('El vendedor de semillas', 'flora', 'genaro', 'Busca a Genaro en la Ruta 7.', 'Genaro vende semillas en la Ruta 7. Flora quiere encargarle variedades de néctar.', 'Genaro tiene las semillas, pero pide a cambio algo con que regar los brotes.'),
    q('Agua para los brotes', 'genaro', 'genaro', 'Entrega dos Aguas Frescas a Genaro.', 'Genaro necesita agua limpia para que las semillas germinen antes de venderlas. Entrégale dos Aguas Frescas.', 'Genaro riega los brotes con cuidado. En unos días estarán listos para Flora.', kind='delivery', proof=('ITEM_FRESH_WATER', 2)),
    q('Pétalos caídos', 'genaro', 'aroa', 'Habla con Aroa en el Centro Pokémon de Azulona.', 'Genaro dice que Aroa sabe qué flores atraen a más Pokémon: hace perfumes con sus pétalos.', 'Aroa recomienda flores de olor suave. Las de olor fuerte asustan a los Oddish.'),
    q('Preguntas de perfumista', 'aroa', 'aroa', 'Responde las preguntas de Aroa.', 'Aroa quiere comprobar que entiendes el jardín antes de dar su receta.', 'Aroa sonríe. Entiendes que un jardín es un lugar de paso, no una vitrina.', kind='puzzle', questions=(
        ('Aroa pregunta: ¿se arrancan las flores para el perfume?', False),
        ('Aroa sigue: ¿las flores suaves atraen a más Pokémon?', True),
        ('Aroa termina: ¿un jardín necesita rincones sin podar?', True))),
    q('Plantar juntos', 'aroa', 'flora', 'Lleva las semillas y la receta a Flora.', 'Con las semillas de Genaro y el consejo de Aroa, Flora puede replantar los parterres.', 'Flora planta las flores nuevas. Deja un rincón sin podar, como recomendó Aroa.'),
    q('Jardines de Azulona', 'flora', 'genaro', 'Cuenta a Genaro que los Pokémon han vuelto.', 'Una semana después, los parterres están llenos de Pokémon. Ve a contárselo a Genaro.', 'Genaro ríe y promete reservar semillas para el año que viene. Azulona vuelve a oler a flores.', reward='ITEM_LEAF_STONE', quantity=1, bonus=dict(money=2000)),
])

# 10 --------------------------------------------------------------------------------------
actor2('dimas', 'Dimas', 'CeladonCity_GameCorner', 'Trabajé en el casino veinte años. Ahora quiero saber qué pasó con los premios.', 'OBJ_EVENT_GFX_HIKER')
actor2('rocio', 'Rocío', 'Route8', 'Mi Porygon salió de una máquina de premios. Nunca supe de dónde venía.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
T_DIMAS = trainer('DIMAS', 30, 'Male', 'Hiker Frlg', ['Raticate', 'Arbok', 'Golbat'])
campaign('FICHAS SIN DUEÑO', 'Un antiguo empleado del casino busca el rastro de los Pokémon que se entregaban como premio.', [
    q('El empleado del casino', 'dimas', 'dimas', 'Habla con Dimas en el Casino de Azulona.', 'Dimas trabajó en el casino durante años. Quiere saber qué fue de los Pokémon que se daban como premio.', 'Dimas recuerda nombres y fechas. Cree que muchos Pokémon de premio nunca tuvieron un hogar estable.', flag='FLAG_BADGE04_GET'),
    q('Un premio con nombre', 'dimas', 'rocio', 'Busca a Rocío en la Ruta 8.', 'Dimas tiene la ficha de un Porygon entregado hace años. La dueña, Rocío, vive junto a la Ruta 8.', 'Rocío cuida bien a su Porygon, pero siempre se preguntó de dónde venía.'),
    q('Ver sin comprar', 'rocio', 'rocio', 'Observa un Porygon y vuelve con Rocío.', 'Rocío quiere saber cómo es un Porygon sano para compararlo con el suyo. Observa uno.', 'Rocío compara tus notas con su Porygon. Está sano y feliz.', kind='field', goal=('QUEST_DEX_SEEN', 'SPECIES_PORYGON')),
    q('El reto de Dimas', 'rocio', 'dimas', 'Acepta el combate de Dimas.', 'Dimas quiere comprobar que sabes tratar a los Pokémon antes de compartir los archivos del casino.', 'Dimas guarda sus Pokémon y abre el archivo. Son cientos de fichas sin seguimiento.', kind='battle', proof=T_DIMAS),
    q('Un archivo olvidado', 'dimas', 'dimas', 'Entrega tres Pokéballs a Dimas para su colección de registros.', 'Dimas quiere guardar una Poké Ball junto a cada ficha que encuentre un dueño, como recordatorio.', 'Dimas coloca las Poké Balls junto a las fichas. Cada una espera un nombre.', kind='delivery', proof=('ITEM_POKE_BALL', 3)),
    q('Cerrar el mostrador', 'dimas', 'dimas', 'Ayuda a Dimas a decidir sobre el mostrador de premios.', 'Dimas propone pedir que el casino deje de entregar Pokémon como premio y dé objetos en su lugar.', 'Dimas presenta la propuesta. El mostrador de premios solo entregará objetos.', kind='choice', choice='casino_cerrado', question='Dimas pregunta: ¿pedimos que cierren el mostrador?', alt_ending='Dimas decide que el mostrador siga, pero con una ficha de seguimiento para cada Pokémon entregado.'),
    q('Contárselo a Rocío', 'dimas', 'rocio', 'Cuenta a Rocío lo que se ha decidido.', 'Rocío quiere saber qué pasará con los Pokémon de premio a partir de ahora.', 'Rocío se alegra. Ningún Pokémon saldrá de una máquina sin saber a dónde va.', choice='casino_cerrado', alt_ending='Rocío se ofrece a visitar a cada Pokémon entregado durante el primer mes.'),
    q('Fichas sin dueño', 'rocio', 'dimas', 'Lleva la ficha de Porygon a Dimas para cerrarla.', 'Rocío firma la ficha de su Porygon. Llévasela a Dimas para cerrar el primer caso.', 'Dimas archiva la ficha con una nota: “hogar encontrado”. Es la primera de muchas.', reward='ITEM_UP_GRADE', quantity=1, choice='casino_cerrado', alt_ending='Dimas archiva la ficha y abre otra en blanco. El seguimiento empieza hoy.', bonus=dict(money=2500)),
])

# 11 --------------------------------------------------------------------------------------
actor2('kai', 'Kai', 'SaffronCity_Dojo', 'Soy el aprendiz más joven del Dojo. Barro el suelo más que entreno.', 'OBJ_EVENT_GFX_HIKER')
actor2('mei', 'Mei', 'SaffronCity_PokemonCenter_1F', 'Entreno sola. El Dojo no admite a gente de fuera.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
T_KAI = trainer('KAI', 32, 'Male', 'Black Belt Frlg', ['Machoke', 'Hitmonchan', 'Primeape'])
T_MEI = trainer('MEI', 33, 'Female', 'Lass Frlg', ['Hitmonlee', 'Poliwrath', 'Mankey'])
campaign('PUERTAS DEL DOJO', 'Un aprendiz del Dojo y una entrenadora de fuera buscan la forma de entrenar juntos.', [
    q('El aprendiz que barre', 'kai', 'kai', 'Habla con Kai en el Dojo de Azafrán.', 'Kai es el aprendiz más joven del Dojo. Dice que quiere aprender algo más que a barrer.', 'Kai cuenta que una entrenadora de fuera pidió entrar y el Dojo la rechazó. Le pareció injusto.', flag='FLAG_BADGE05_GET'),
    q('La entrenadora de fuera', 'kai', 'mei', 'Busca a Mei en el Centro Pokémon de Azafrán.', 'Mei entrena sus Pokémon de tipo Lucha sin maestro. Kai quiere conocerla.', 'Mei duda. No quiere volver a pedir permiso para que la rechacen.'),
    q('Probar la técnica', 'mei', 'mei', 'Acepta el combate de Mei.', 'Mei quiere comprobar si su entrenamiento sirve. Te reta a un combate.', 'Mei respira hondo. Ha aprendido sola más de lo que creía.', kind='battle', proof=T_MEI),
    q('Lucha en la hierba', 'mei', 'mei', 'Captura dos Pokémon de tipo Lucha y vuelve con Mei.', 'Mei quiere enseñar a Kai qué Pokémon de tipo Lucha viven libres en Kanto. Registra dos capturas.', 'Mei anota las especies. Le servirán para explicar su estilo.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_TYPE', 'TYPE_FIGHTING', 2)),
    q('El turno de Kai', 'mei', 'kai', 'Acepta el combate de Kai.', 'Kai quiere demostrar al maestro que ya sabe combatir. Te pide un combate en el Dojo.', 'El maestro observa en silencio. Al terminar, asiente una vez.', kind='battle', proof=T_KAI),
    q('Abrir las puertas', 'kai', 'kai', 'Ayuda a Kai a proponer algo al maestro.', 'Kai quiere pedir que el Dojo abra un día a la semana para entrenadores de fuera, como Mei.', 'El maestro acepta. Los sábados, el Dojo abrirá las puertas.', kind='choice', choice='dojo_abierto', question='Kai pregunta: ¿propongo abrir el Dojo a gente de fuera?', alt_ending='Kai decide no pedirlo aún. Propondrá que él mismo entrene con Mei fuera del Dojo.'),
    q('Avisar a Mei', 'kai', 'mei', 'Cuenta a Mei la noticia.', 'Kai quiere que Mei sea la primera en saberlo.', 'Mei no se lo cree hasta que lo repites. El sábado estará allí.', choice='dojo_abierto', alt_ending='Mei acepta entrenar con Kai en el parque. Dice que así ninguno tendrá que pedir permiso.'),
    q('Puertas del Dojo', 'mei', 'kai', 'Lleva el cinturón de Mei a Kai.', 'Mei te da su cinturón gastado para Kai: el primer regalo entre compañeros de entrenamiento.', 'Kai guarda el cinturón junto a la escoba. Ahora entrena y barre, y no le importa.', reward='ITEM_BLACK_BELT', quantity=1, choice='dojo_abierto', alt_ending='Kai cuelga el cinturón en el parque, en la rama donde entrenan cada tarde.', bonus=dict(money=2500)),
])

# 12 --------------------------------------------------------------------------------------
actor2('pesca', 'Pesca', 'Route12', 'Pesco en la Ruta 12 desde que tengo memoria. El Snorlax era mi vecino.', 'OBJ_EVENT_GFX_HIKER')
actor2('nuria', 'Nuria', 'Route13', 'Estudio las vallas de la Ruta 13. Las colocaron mal y nadie las ha movido.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('guille', 'Guille', 'Route14', 'Entreno a mis Pidgeotto para mensajería entre rutas.')
campaign('EL PUENTE DE LOS PESCADORES', 'Los vecinos del litoral de Lavanda quieren unir las rutas 12 a 14 sin dañar los nidos de la costa.', [
    q('Un pescador sin vecino', 'pesca', 'pesca', 'Habla con Pesca en la Ruta 12.', 'Pesca pesca en el puente de la Ruta 12. Desde que se fue Snorlax, la ruta tiene más paso y más ruido.', 'Pesca quiere poner carteles para que la gente no moleste a los Pokémon del agua.', flag='FLAG_BADGE05_GET'),
    q('Pescar y soltar', 'pesca', 'pesca', 'Captura dos Pokémon en la Ruta 12 y vuelve con Pesca.', 'Pesca quiere saber qué especies viven en el agua de la ruta. Registra dos capturas allí.', 'Pesca apunta las especies. Los carteles tendrán dibujos de cada una.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_MAP', 'MAP_ROUTE12', 2)),
    q('Vallas mal puestas', 'pesca', 'nuria', 'Busca a Nuria en la Ruta 13.', 'Nuria estudia el laberinto de vallas de la Ruta 13. Pesca quiere colocar allí uno de los carteles.', 'Nuria dice que las vallas cortan el paso de los Pokémon. Propone abrir un hueco.'),
    q('El hueco correcto', 'nuria', 'nuria', 'Responde las preguntas de Nuria.', 'Nuria quiere estar segura de dónde abrir el hueco sin dejar suelto a nadie.', 'Nuria marca el hueco en su mapa. Los Pokémon podrán pasar y la gente no se perderá.', kind='puzzle', questions=(
        ('Nuria pregunta: ¿un hueco junto a la hierba ayuda?', True),
        ('Nuria sigue: ¿hay que quitar todas las vallas?', False),
        ('Nuria termina: ¿conviene un cartel junto al hueco?', True))),
    q('El mensajero alado', 'nuria', 'guille', 'Busca a Guille en la Ruta 14.', 'Nuria quiere avisar a los vecinos de la Ruta 14. Guille entrena Pidgeotto mensajeros.', 'Guille acepta llevar los avisos, pero necesita alimento para sus Pidgeotto.'),
    q('Alimento para el viaje', 'guille', 'guille', 'Entrega tres Bayas Aranja a Guille.', 'Los Pidgeotto de Guille vuelan mejor con bayas. Entrégale tres Bayas Aranja.', 'Guille reparte las bayas. Los Pidgeotto salen con los avisos atados a la pata.', kind='delivery', proof=('ITEM_ORAN_BERRY', 3)),
    q('Correo por el aire', 'guille', 'pesca', 'Lleva a Pesca la respuesta de los vecinos.', 'Los Pidgeotto han vuelto con las respuestas. Todos apoyan los carteles.', 'Pesca sonríe. Mañana colocarán el primero en el puente.'),
    q('El puente de los pescadores', 'pesca', 'nuria', 'Cuenta a Nuria que los carteles ya están puestos.', 'Los carteles y el hueco están listos. Ve a contárselo a Nuria.', 'Nuria camina por el hueco nuevo. Un Oddish la adelanta sin mirarla siquiera.', reward='ITEM_SUPER_ROD', quantity=1, bonus=dict(money=2000)),
])

# 13 --------------------------------------------------------------------------------------
actor2('ofelia', 'Ofelia', 'FuchsiaCity_WardensHouse', 'Ayudo al Guarda con los registros del Safari. Hay más papeles que Pokémon.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('teo_safari', 'Telmo', 'FuchsiaCity', 'Hago fotos de los Pokémon del Safari desde la valla. Nunca entro.')
actor2('marisa', 'Marisa', 'Route15', 'Llevo los Pokémon heridos del Safari al Centro. Algunos vuelven; otros se quedan conmigo.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
campaign('LA ZONA SAFARI', 'La ayudante del Guarda quiere cambiar cómo se visita el Safari, con datos y no solo con opiniones.', [
    q('Papeles del Safari', 'ofelia', 'ofelia', 'Habla con Ofelia en la Casa del Guarda.', 'Ofelia lleva los registros del Safari. Cree que las redes y los cebos estresan a los Pokémon.', 'Ofelia necesita datos para convencer al Guarda. Te pide ayuda.', flag='FLAG_BADGE05_GET'),
    q('Fotos desde la valla', 'ofelia', 'teo_safari', 'Busca a Telmo en Ciudad Fucsia.', 'Telmo fotografía el Safari desde fuera. Sus fotos podrían mostrar cómo reaccionan los Pokémon.', 'Telmo enseña las fotos. Los Pokémon huyen cada vez que alguien lanza un cebo.'),
    q('Observar a Chansey', 'teo_safari', 'teo_safari', 'Observa un Chansey y vuelve con Telmo.', 'A Telmo le falta una foto de Chansey. Observa uno para describirle cómo se mueve.', 'Telmo dibuja un Chansey a partir de tu descripción. Le servirá hasta que consiga la foto.', kind='field', goal=('QUEST_DEX_SEEN', 'SPECIES_CHANSEY')),
    q('La enfermera del Safari', 'teo_safari', 'marisa', 'Busca a Marisa en la Ruta 15.', 'Marisa atiende a los Pokémon heridos del Safari. Ofelia quiere su testimonio.', 'Marisa cuenta que muchos llegan agotados de huir de las redes.'),
    q('Botiquín para Marisa', 'marisa', 'marisa', 'Entrega dos Superpociones a Marisa.', 'Marisa se ha quedado sin medicinas. Entrégale dos Superpociones.', 'Marisa atiende a un Exeggcute cansado. Al rato, vuelve a saltar.', kind='delivery', proof=('ITEM_SUPER_POTION', 2)),
    q('Redes o paciencia', 'marisa', 'ofelia', 'Ayuda a Ofelia a decidir su propuesta.', 'Ofelia tiene los datos de Telmo y el testimonio de Marisa. Puede proponer prohibir las redes, o solo limitarlas.', 'Ofelia propone un Safari sin redes: solo paciencia y Poké Balls.', kind='choice', choice='safari_sin_redes', question='Ofelia pregunta: ¿proponemos un Safari sin redes?', alt_ending='Ofelia propone limitar las redes a dos por visita, con un descanso obligatorio entre lanzamientos.'),
    q('Avisar a Marisa', 'ofelia', 'marisa', 'Cuenta a Marisa la decisión del Guarda.', 'El Guarda ha aceptado la propuesta de Ofelia. Marisa querrá saberlo.', 'Marisa respira aliviada. Espera recibir menos Pokémon agotados.', choice='safari_sin_redes', alt_ending='Marisa acepta el término medio. Dice que dos redes son mejor que veinte.'),
    q('La Zona Safari', 'marisa', 'ofelia', 'Entrega a Ofelia el primer informe de Marisa.', 'Un mes después, Marisa ha escrito un informe. Llévaselo a Ofelia.', 'El informe muestra menos Pokémon heridos. Ofelia lo cuelga en la Casa del Guarda.', reward='ITEM_ULTRA_BALL', quantity=3, choice='safari_sin_redes', alt_ending='El informe muestra una mejora pequeña pero clara. Ofelia lo cuelga junto a las normas nuevas.', bonus=dict(money=2500)),
])

# 14 --------------------------------------------------------------------------------------
actor2('ciro', 'Ciro', 'Route16', 'Bajo en bici por la Ruta 17 cada mañana. Subir ya es otra historia.')
actor2('lucia', 'Lucía', 'Route18', 'Organizo la carrera ciclista. Este año habrá Pokémon de ánimo en la cuneta.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
T_CIRO = trainer('CIRO', 34, 'Male', 'Biker Frlg', ['Koffing', 'Grimer', 'Muk'])
campaign('LA CARRERA DE LA RUTA 17', 'Ciclistas y entrenadores preparan una carrera que respete a los Pokémon de la pista.', [
    q('Ruedas en la pista', 'lucia', 'lucia', 'Habla con Lucía en la Ruta 18.', 'Lucía organiza una carrera ciclista por la Pista Ciclista. Quiere que sea segura para todos, Pokémon incluidos.', 'Lucía necesita a alguien que revise el recorrido y hable con los ciclistas.', flag='FLAG_BADGE05_GET'),
    q('El ciclista de la Ruta 16', 'lucia', 'ciro', 'Busca a Ciro en la Ruta 16.', 'Ciro conoce la pista mejor que nadie. Lucía quiere que sea el capitán de la carrera.', 'Ciro acepta, pero quiere comprobar antes que sabes defenderte en la pista.'),
    q('Reto en la pista', 'ciro', 'ciro', 'Acepta el combate de Ciro.', 'Ciro te desafía a un combate antes de hablar de la carrera.', 'Ciro se ríe. Dice que en la pista no hay rivales, solo compañeros que van más rápido o más lento.', kind='battle', proof=T_CIRO),
    q('Vecinos del asfalto', 'ciro', 'ciro', 'Captura dos Pokémon de tipo Veneno y vuelve con Ciro.', 'Muchos Pokémon de tipo Veneno viven junto a la pista. Ciro quiere saber cuáles para avisar a los corredores.', 'Ciro anota las especies. Pondrá carteles en las curvas.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_TYPE', 'TYPE_POISON', 2)),
    q('Botiquín de carrera', 'ciro', 'lucia', 'Entrega dos Antídotos a Lucía.', 'Lucía quiere un botiquín en la meta por si algún Pokémon o ciclista se envenena.', 'Lucía guarda los Antídotos en la mesa de la meta.', kind='delivery', proof=('ITEM_ANTIDOTE', 2)),
    q('Vuelta de reconocimiento', 'ciro', 'lucia', 'Ve desde Ciro hasta Lucía en 60 segundos.', 'Ciro quiere comprobar el recorrido. Sal de la Ruta 16 y llega hasta Lucía, en la Ruta 18, en menos de un minuto.', 'Lucía detiene el cronómetro. El recorrido es bueno; solo falta dar la salida.', kind='race', seconds=60),
    q('Dar la salida', 'lucia', 'ciro', 'Lleva la bandera de salida a Ciro.', 'Lucía quiere que Ciro dé la salida. Llévale la bandera.', 'Ciro agita la bandera. Los ciclistas salen despacio en las curvas con carteles.'),
    q('La carrera de la Ruta 17', 'ciro', 'lucia', 'Celebra el final de la carrera con Lucía.', 'La carrera ha terminado sin un solo accidente. Lucía te espera en la meta.', 'Lucía entrega una medalla a cada participante y otra a los Pokémon de la cuneta, que se la comen.', reward='ITEM_FULL_HEAL', quantity=3, bonus=dict(money=2000)),
])

# 15 --------------------------------------------------------------------------------------
actor2('marina', 'Marina', 'Route19', 'Enseño a nadar con Pokémon. La primera regla: el mar manda.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('bruno_faro', 'Bruno', 'Route20', 'Vigilo las corrientes de la Ruta 20. Las Islas Espuma las cambian cada invierno.', 'OBJ_EVENT_GFX_HIKER')
T_MARINA = trainer('MARINA', 36, 'Female', 'Swimmer F Frlg', ['Staryu', 'Seaking', 'Tentacruel'])
campaign('CORRIENTES DE LA RUTA 20', 'Una monitora de natación y un vigía preparan un mapa de corrientes para quienes cruzan el mar.', [
    q('La escuela de natación', 'marina', 'marina', 'Habla con Marina en la Ruta 19.', 'Marina enseña a nadar con Pokémon. Varias alumnas se han desviado por corrientes nuevas.', 'Marina quiere un mapa de corrientes. Para eso necesita al vigía de la Ruta 20.', flag='FLAG_BADGE05_GET'),
    q('Nadar con cabeza', 'marina', 'marina', 'Acepta el combate de Marina.', 'Marina solo trabaja con entrenadores que conocen el agua. Te reta a un combate.', 'Marina asiente. Sabes moverte en el agua sin perder el norte.', kind='battle', proof=T_MARINA),
    q('El vigía', 'marina', 'bruno_faro', 'Busca a Bruno en la Ruta 20.', 'Bruno vigila las corrientes desde una roca de la Ruta 20.', 'Bruno conoce las corrientes, pero le faltan datos de las especies que las siguen.'),
    q('Quién nada aquí', 'bruno_faro', 'bruno_faro', 'Captura tres Pokémon de tipo Agua y vuelve con Bruno.', 'Las especies de tipo Agua siguen las corrientes. Bruno quiere saber cuáles hay.', 'Bruno marca en su mapa dónde vio cada especie. Las corrientes empiezan a tener forma.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_TYPE', 'TYPE_WATER', 3)),
    q('Las Islas Espuma', 'bruno_faro', 'bruno_faro', 'Observa un Seel y vuelve con Bruno.', 'Los Seel de las Islas Espuma indican dónde está el agua fría. Observa uno.', 'Bruno anota el dato. Donde hay Seel, la corriente baja del norte.', kind='field', goal=('QUEST_DEX_SEEN', 'SPECIES_SEEL')),
    q('Preguntas del vigía', 'bruno_faro', 'bruno_faro', 'Responde las preguntas de Bruno.', 'Bruno quiere comprobar que sabrás explicar el mapa a las alumnas de Marina.', 'Bruno te da el mapa. Dice que lo explicarás mejor que él.', kind='puzzle', questions=(
        ('Bruno pregunta: ¿se nada contra la corriente para salir de ella?', False),
        ('Bruno sigue: ¿donde hay Seel el agua es fría?', True),
        ('Bruno termina: ¿hay que avisar antes de cruzar?', True))),
    q('El mapa de corrientes', 'bruno_faro', 'marina', 'Lleva el mapa a Marina.', 'Lleva el mapa de Bruno a la escuela de Marina.', 'Marina cuelga el mapa en la orilla. Las alumnas lo estudian antes de entrar.'),
    q('Corrientes de la Ruta 20', 'marina', 'bruno_faro', 'Avisa a Bruno de que las alumnas cruzan seguras.', 'Las alumnas de Marina han cruzado sin desviarse. Ve a contárselo a Bruno.', 'Bruno sonríe desde su roca. Dice que el mar sigue mandando, pero ahora se le entiende mejor.', reward='ITEM_MYSTIC_WATER', quantity=1, bonus=dict(money=2500)),
])

# 16 --------------------------------------------------------------------------------------
actor2('adela', 'Adela', 'CinnabarIsland', 'Vivía en Canela antes de la erupción. Quiero ver la isla viva otra vez.', 'OBJ_EVENT_GFX_WOMAN_1_FRLG')
actor2('ernesto', 'Ernesto', 'CinnabarIsland_PokemonCenter_1F', 'Estudio volcanes. El de Canela no ha dicho su última palabra.')
T_ADELA = trainer('ADELA', 40, 'Female', 'Lass Frlg', ['Ponyta', 'Magmar', 'Rapidash'])
campaign('CENIZAS DE CANELA', 'Una antigua vecina y un vulcanólogo deciden cómo volver a vivir en una isla que todavía arde.', [
    q('Volver a Canela', 'adela', 'adela', 'Habla con Adela en Isla Canela.', 'Adela vivió en Canela de niña. Quiere saber si la isla puede volver a ser un hogar.', 'Adela cuenta que el pueblo se fue tras la erupción. Ella quiere entender qué cambió.', flag='FLAG_BADGE07_GET'),
    q('El vulcanólogo', 'adela', 'ernesto', 'Busca a Ernesto en el Centro Pokémon de Canela.', 'Ernesto estudia el volcán. Adela quiere saber si es seguro reconstruir.', 'Ernesto dice que el volcán está tranquilo, pero necesita datos de los Pokémon de fuego.'),
    q('Fuego vivo', 'ernesto', 'ernesto', 'Captura tres Pokémon de tipo Fuego y vuelve con Ernesto.', 'Los Pokémon de tipo Fuego se alejan cuando el volcán se agita. Ernesto quiere saber cuántos hay.', 'Ernesto repasa tu lista. Si los Pokémon de fuego se quedan, el volcán duerme.', kind='field', goal=('QUEST_COUNT', 'RF_QUEST_EVENT_CATCH_TYPE', 'TYPE_FIRE', 3)),
    q('La memoria de Adela', 'ernesto', 'adela', 'Acepta el combate de Adela.', 'Adela entrena Pokémon de fuego desde niña. Quiere demostrar que el fuego también puede cuidar.', 'Adela recoge a su Rapidash. Dice que el fuego que le quitó la casa no es el mismo que la acompaña.', kind='battle', proof=T_ADELA),
    q('Provisiones para la obra', 'adela', 'ernesto', 'Entrega dos Antiquemar a Ernesto.', 'Ernesto quiere un botiquín para quienes trabajen cerca del volcán. Entrégale dos Antiquemar.', 'Ernesto guarda los Antiquemar en la caja de la obra.', kind='delivery', proof=('ITEM_BURN_HEAL', 2)),
    q('Reconstruir o recordar', 'ernesto', 'adela', 'Ayuda a Adela a decidir el futuro de Canela.', 'Adela puede impulsar la reconstrucción del pueblo, o pedir que la isla quede como reserva y memoria.', 'Adela decide reconstruir. Canela volverá a tener casas, con un plan de evacuación en cada puerta.', kind='choice', choice='canela_reconstruye', question='Adela pregunta: ¿reconstruimos el pueblo de Canela?', alt_ending='Adela decide que la isla sea una reserva. Las ruinas quedarán como memoria y refugio para los Pokémon de fuego.'),
    q('El plan de Ernesto', 'adela', 'ernesto', 'Cuenta a Ernesto la decisión de Adela.', 'Ernesto quiere saber qué ha decidido Adela para preparar sus mediciones.', 'Ernesto instalará sensores en cada calle nueva. La isla avisará antes de despertar.', choice='canela_reconstruye', alt_ending='Ernesto se alegra. Una reserva le permitirá estudiar el volcán sin prisas.'),
    q('Cenizas de Canela', 'ernesto', 'adela', 'Entrega a Adela el informe de Ernesto.', 'Ernesto te da su informe para Adela. Es la base de todo lo que vendrá.', 'Adela guarda el informe junto a una foto de su casa antigua. Canela tiene futuro otra vez.', reward='ITEM_FIRE_STONE', quantity=1, choice='canela_reconstruye', alt_ending='Adela guarda el informe junto a una foto de la isla. La reserva abrirá el próximo verano.', bonus=dict(money=3000)),
])
