#!/usr/bin/env python3
"""Rojo Fuego+ achievements: the list, and the files generated from it.

Writes:
  include/constants/rf_achievements.h  ACH_* ids, counter ids, condition types, sizes
  src/data/rf_achievements.h           the achievement table and the species/item sets it uses
  tools/rojo_fuego/achievements.json   the list for the design page (read by people, not the game)
  tools/rojo_fuego/retroachievements_local.txt  RetroAchievements local set (see write_ra())

Run from the repository root (inside WSL):  python3 tools/rojo_fuego/achievements.py
"""
import json
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
TIER_POINTS = {"B": 10, "S": 25, "G": 50, "P": 100}
TIER_NAMES = {"B": "ACH_TIER_BRONZE", "S": "ACH_TIER_SILVER", "G": "ACH_TIER_GOLD", "P": "ACH_TIER_PLATINUM"}
CATEGORIES = [
    ("H", "Historia"), ("D", "Pokédex"), ("C", "Captura"), ("B", "Combate"), ("T", "Entrenamiento"),
    ("E", "Exploración"), ("K", "Crianza"), ("O", "Colección"), ("X", "Sistemas"), ("R", "Desafíos"),
    ("Z", "Ocultos"), ("M", "Maestría"),
]

# Counters kept in the save and raised by hooks in the game code (see src/achievements.c)
COUNTERS = [
    "SUPER_EFFECTIVE", "CRITS", "OHKO", "PARALYZED", "BURNED", "SLEPT", "FROZEN", "POISONED",
    "MEGA", "TERA", "DYNAMAX", "ZMOVE", "SWITCHES", "FLED", "SHINY_CAUGHT", "FIRST_BALL_CATCHES",
    "NIGHT_CATCHES", "OUTBREAK_CATCHES", "LEGEND_CATCHES", "SAFARI_CATCHES", "FISHING_CATCHES",
    "WONDER_TRADES", "VIAL_USES", "RARE_CANDIES", "EXP_CANDIES", "VITAMINS", "HYPER_TRAININGS",
    "RELEASES", "WALL_BUMPS", "TRAINER_WINS", "WIN_STREAK", "BEST_WIN_STREAK", "FAINTED_FOES",
    "CRIT_CAPTURES", "DEXNAV_CATCHES", "CLUTCH_WINS", "MINTS", "CAPSULES", "WONDER_SHINY",
    "WONDER_LEGEND", "KEYS_CHANGED", "BALL_KINDS", "LEAGUE_FAINTS",
]

ach = []


def add(aid, cat, tier, name, desc, cond, reward, hidden=False):
    ach.append({"id": aid, "cat": cat, "tier": tier, "name": name, "desc": desc, "cond": cond,
                "reward": reward, "hidden": hidden})


# Condition helpers: each returns (type, arg, value)
def flag(f): return ("ACH_COND_FLAG", f, 0)
def dex(n): return ("ACH_COND_DEX_CAUGHT", 0, n)
def seen(n): return ("ACH_COND_DEX_SEEN", 0, n)
def species(*names): return ("ACH_COND_SPECIES_SET", ["SPECIES_" + n for n in names], 0)
def stat(s, n): return ("ACH_COND_GAME_STAT", "GAME_STAT_" + s, n)
def cnt(c, n): return ("ACH_COND_COUNTER", "ACH_COUNTER_" + c, n)
def money(n): return ("ACH_COND_MONEY", 0, n)
def level(n): return ("ACH_COND_PARTY_LEVEL", 0, n)
def level_all(n): return ("ACH_COND_PARTY_ALL_LEVEL", 0, n)
def item(i): return ("ACH_COND_ITEM_SET", ["ITEM_" + i], 1)
def items(n, *names): return ("ACH_COND_ITEM_SET", ["ITEM_" + i for i in names], n)
def typed(t, n): return ("ACH_COND_TYPE_CAUGHT", "TYPE_" + t, n)
def special(s, n=0): return ("ACH_COND_SPECIAL", "ACH_SPECIAL_" + s, n)
def event(): return ("ACH_COND_EVENT", 0, 0)
def pct(n): return ("ACH_COND_PERCENT", 0, n)
def tier_all(t): return ("ACH_COND_TIER_ALL", TIER_NAMES[t], 0)
def hidden_all(): return ("ACH_COND_HIDDEN_ALL", 0, 0)


# Reward helpers
def give(i, q=1): return ("ITEM_" + i, q, None)
def title(t): return ("ITEM_NONE", 0, t)


# ---------------------------------------------------------------- Historia
add("H01", "H", "B", "El viaje comienza", "Recibe tu primer Pokémon.", flag("FLAG_SYS_POKEMON_GET"), give("POKE_BALL", 5))
add("H02", "H", "B", "Rival a la vista", "Vence a tu rival en el laboratorio de Oak.", flag("FLAG_BEAT_RIVAL_IN_OAKS_LAB"), give("POTION", 3))
add("H03", "H", "B", "Medalla Roca", "Vence a Brock.", flag("FLAG_BADGE01_GET"), give("EXP_CANDY_S", 5))
add("H04", "H", "B", "Medalla Cascada", "Vence a Misty.", flag("FLAG_BADGE02_GET"), give("EXP_CANDY_S", 5))
add("H05", "H", "B", "Medalla Trueno", "Vence al Teniente Surge.", flag("FLAG_BADGE03_GET"), give("EXP_CANDY_M", 3))
add("H06", "H", "S", "Medalla Arcoíris", "Vence a Erika.", flag("FLAG_BADGE04_GET"), give("EXP_CANDY_M", 3))
add("H07", "H", "S", "Medalla Alma", "Vence a Koga.", flag("FLAG_BADGE05_GET"), give("EXP_CANDY_L", 2))
add("H08", "H", "S", "Medalla Pantano", "Vence a Sabrina.", flag("FLAG_BADGE06_GET"), give("EXP_CANDY_L", 2))
add("H09", "H", "S", "Medalla Volcán", "Vence a Blaine.", flag("FLAG_BADGE07_GET"), give("EXP_CANDY_XL"))
add("H10", "H", "G", "Medalla Tierra", "Vence a Giovanni en su gimnasio.", flag("FLAG_BADGE08_GET"), give("EXP_CANDY_XL", 2))
add("H11", "H", "S", "Scope Silph", "Consigue el Scope Silph.", item("SILPH_SCOPE"), give("NUGGET"))
add("H12", "H", "S", "Silph liberada", "Expulsa al Team Rocket de Silph S.A.", flag("FLAG_GOT_LAPRAS_FROM_SILPH"), give("ABILITY_CAPSULE"))
add("H13", "H", "B", "La flauta del Sr. Fuji", "Rescata al Sr. Fuji en la Torre Pokémon.", flag("FLAG_RESCUED_MR_FUJI"), give("HYPER_POTION", 3))
add("H14", "H", "B", "Despierta, dormilón", "Despierta al Snorlax de la Ruta 12.", flag("FLAG_WOKE_UP_ROUTE_12_SNORLAX"), give("LEFTOVERS"))
add("H15", "H", "B", "Todos a bordo", "Consigue el billete del S.S. Anne.", flag("FLAG_GOT_SS_TICKET"), give("REVIVE", 3))
add("H16", "H", "B", "Ayuda a Bill", "Ayuda a Bill en su cabaña.", flag("FLAG_HELPED_BILL_IN_SEA_COTTAGE"), give("GREAT_BALL", 5))
add("H17", "H", "B", "¿Hélix o Domo?", "Elige un fósil en el Monte Moon.", flag("FLAG_GOT_FOSSIL_FROM_MT_MOON"), give("GREAT_BALL", 5))
add("H18", "H", "S", "La Élite: Lorelei", "Vence a Lorelei.", flag("FLAG_DEFEATED_LORELEI"), give("NEVER_MELT_ICE"))
add("H19", "H", "S", "La Élite: Bruno", "Vence a Bruno.", flag("FLAG_DEFEATED_BRUNO"), give("BLACK_BELT"))
add("H20", "H", "S", "La Élite: Agatha", "Vence a Agatha.", flag("FLAG_DEFEATED_AGATHA"), give("SPELL_TAG"))
add("H21", "H", "G", "La Élite: Lance", "Vence a Lance.", flag("FLAG_DEFEATED_LANCE"), give("DRAGON_FANG"))
add("H22", "H", "G", "Campeón de Kanto", "Gana la Liga Pokémon.", flag("FLAG_IS_CHAMPION"), title("Campeón"))
add("H23", "H", "B", "Más allá del mar", "Llega a Isla Prima.", flag("FLAG_WORLD_MAP_ONE_ISLAND"), give("GREAT_BALL", 10))
add("H24", "H", "S", "El Rubí", "Encuentra el Rubí en el Monte Ascuas.", flag("FLAG_GOT_RUBY"), give("ULTRA_BALL", 5))
add("H25", "H", "G", "El Zafiro", "Recupera el Zafiro.", flag("FLAG_RECOVERED_SAPPHIRE"), give("OVAL_CHARM"))
add("H26", "H", "S", "Almacén limpio", "Echa al Team Rocket del almacén de Isla Quinta.", flag("FLAG_DEFEATED_ROCKETS_IN_WAREHOUSE"), give("ULTRA_BALL", 10))
add("H27", "H", "B", "Perdida en el bosque", "Rescata a Lostelle en el Bosque Baya.", flag("FLAG_RESCUED_LOSTELLE"), give("SITRUS_BERRY", 3))
add("H28", "H", "S", "Pokédex Nacional", "Consigue la Pokédex Nacional.", flag("FLAG_SYS_NATIONAL_DEX"), give("ULTRA_BALL", 5))
add("H29", "H", "S", "Ruinas Tanoby", "Abre las Ruinas Tanoby.", flag("FLAG_SYS_UNLOCKED_TANOBY_RUINS"), give("WHITE_HERB"))
add("H30", "H", "G", "Ronda de revanchas", "Vence la revancha de los siete líderes.", special("REMATCHES", 7), give("GOLD_BOTTLE_CAP"))
add("H31", "H", "B", "Primera revancha", "Vence la revancha de un líder.", special("REMATCHES", 1), give("PP_UP"))
add("H32", "H", "G", "Diez veces campeón", "Entra en el Salón de la Fama 10 veces.", stat("ENTERED_HOF", 10), title("Decacampeón"))
add("H33", "H", "S", "Bicicleta", "Consigue la bicicleta.", flag("FLAG_GOT_BICYCLE"), give("MAX_REPEL", 5))
add("H34", "H", "B", "Caña de pescar", "Consigue la Caña Vieja.", flag("FLAG_GOT_OLD_ROD"), give("NET_BALL", 5))
add("H35", "H", "S", "Supercaña", "Consigue la Supercaña.", flag("FLAG_GOT_SUPER_ROD"), give("DIVE_BALL", 10))

# ---------------------------------------------------------------- Pokédex
for i, (n, tier, reward) in enumerate([
        (10, "B", give("GREAT_BALL", 5)), (25, "B", give("GREAT_BALL", 10)), (50, "S", give("ULTRA_BALL", 10)),
        (100, "S", give("RARE_CANDY", 3)), (151, "G", give("MASTER_BALL")), (200, "S", give("RARE_CANDY", 5)),
        (251, "G", give("BOTTLE_CAP", 3)), (300, "G", give("RARE_CANDY", 10)), (386, "G", give("ABILITY_PATCH")),
        (450, "G", give("GOLD_BOTTLE_CAP")), (500, "G", give("MASTER_BALL", 2)), (550, "P", title("Maestro Pokémon"))]):
    add(f"D{i + 1:02d}", "D", tier, f"{n} especies", f"Captura {n} especies distintas.", dex(n), reward)
add("D13", "D", "S", "Ojo de lince", "Ve 300 especies distintas.", seen(300), give("BOTTLE_CAP"))
add("D14", "D", "G", "Visto todo", "Ve 600 especies distintas.", seen(600), give("BOTTLE_CAP", 3))
add("D15", "D", "G", "Aves legendarias", "Captura a Articuno, Zapdos y Moltres.", species("ARTICUNO", "ZAPDOS", "MOLTRES"), give("MASTER_BALL"))
add("D16", "D", "G", "El clon", "Captura a Mewtwo.", species("MEWTWO"), give("LONELY_MINT", 1))
add("D17", "D", "S", "Bestias errantes", "Captura a Raikou, Entei y Suicune.", species("RAIKOU", "ENTEI", "SUICUNE"), give("ULTRA_BALL", 20))
add("D18", "D", "S", "Vida del pasado", "Captura a Omanyte, Kabuto y Aerodactyl.", species("OMANYTE", "KABUTO", "AERODACTYL"), give("BIG_NUGGET"))
add("D19", "D", "S", "Familia Eevee", "Captura todas las evoluciones de Eevee.", species("VAPOREON", "JOLTEON", "FLAREON", "ESPEON", "UMBREON", "LEAFEON", "GLACEON", "SYLVEON"), give("RARE_CANDY", 5))
add("D20", "D", "G", "Iniciales de Kanto", "Captura a Bulbasaur, Charmander y Squirtle.", species("BULBASAUR", "CHARMANDER", "SQUIRTLE"), give("EXP_CANDY_XL", 2))
add("D21", "D", "G", "Iniciales de Johto", "Captura a Chikorita, Cyndaquil y Totodile.", species("CHIKORITA", "CYNDAQUIL", "TOTODILE"), give("EXP_CANDY_XL", 2))
add("D22", "D", "G", "Iniciales de Hoenn", "Captura a Treecko, Torchic y Mudkip.", species("TREECKO", "TORCHIC", "MUDKIP"), give("EXP_CANDY_XL", 2))
add("D23", "D", "G", "Iniciales de Sinnoh", "Captura a Turtwig, Chimchar y Piplup.", species("TURTWIG", "CHIMCHAR", "PIPLUP"), give("EXP_CANDY_XL", 2))
add("D24", "D", "G", "Iniciales de Teselia", "Captura a Snivy, Tepig y Oshawott.", species("SNIVY", "TEPIG", "OSHAWOTT"), give("EXP_CANDY_XL", 2))
add("D25", "D", "G", "Iniciales de Kalos", "Captura a Chespin, Fennekin y Froakie.", species("CHESPIN", "FENNEKIN", "FROAKIE"), give("EXP_CANDY_XL", 2))
add("D26", "D", "G", "Iniciales de Alola", "Captura a Rowlet, Litten y Popplio.", species("ROWLET", "LITTEN", "POPPLIO"), give("EXP_CANDY_XL", 2))
add("D27", "D", "G", "Iniciales de Galar", "Captura a Grookey, Scorbunny y Sobble.", species("GROOKEY", "SCORBUNNY", "SOBBLE"), give("EXP_CANDY_XL", 2))
add("D28", "D", "G", "Iniciales de Paldea", "Captura a Sprigatito, Fuecoco y Quaxly.", species("SPRIGATITO", "FUECOCO", "QUAXLY"), give("EXP_CANDY_XL", 2))
add("D29", "D", "P", "Todos los iniciales", "Captura los 27 iniciales de las nueve regiones.",
    species("BULBASAUR", "CHARMANDER", "SQUIRTLE", "CHIKORITA", "CYNDAQUIL", "TOTODILE", "TREECKO", "TORCHIC", "MUDKIP",
            "TURTWIG", "CHIMCHAR", "PIPLUP", "SNIVY", "TEPIG", "OSHAWOTT", "CHESPIN", "FENNEKIN", "FROAKIE",
            "ROWLET", "LITTEN", "POPPLIO", "GROOKEY", "SCORBUNNY", "SOBBLE", "SPRIGATITO", "FUECOCO", "QUAXLY"),
    title("Profesor"))
add("D30", "D", "G", "Pseudolegendarios", "Captura a Dratini, Larvitar, Bagon, Beldum, Gible, Deino, Goomy, Jangmo-o, Dreepy y Frigibax.",
    species("DRATINI", "LARVITAR", "BAGON", "BELDUM", "GIBLE", "DEINO", "GOOMY", "JANGMO_O", "DREEPY", "FRIGIBAX"), give("DRAGON_SCALE"))
TYPE_REWARDS = [("NORMAL", "Normal", "SILK_SCARF"), ("FIRE", "Fuego", "CHARCOAL"), ("WATER", "Agua", "MYSTIC_WATER"),
                ("GRASS", "Planta", "MIRACLE_SEED"), ("ELECTRIC", "Eléctrico", "MAGNET"), ("ICE", "Hielo", "NEVER_MELT_ICE"),
                ("FIGHTING", "Lucha", "BLACK_BELT"), ("POISON", "Veneno", "POISON_BARB"), ("GROUND", "Tierra", "SOFT_SAND"),
                ("FLYING", "Volador", "SHARP_BEAK"), ("PSYCHIC", "Psíquico", "TWISTED_SPOON"), ("BUG", "Bicho", "SILVER_POWDER"),
                ("ROCK", "Roca", "HARD_STONE"), ("GHOST", "Fantasma", "SPELL_TAG"), ("DRAGON", "Dragón", "DRAGON_FANG"),
                ("DARK", "Siniestro", "BLACK_GLASSES"), ("STEEL", "Acero", "METAL_COAT"), ("FAIRY", "Hada", "FAIRY_FEATHER")]
for i, (t, name, reward) in enumerate(TYPE_REWARDS):
    add(f"D{31 + i:02d}", "D", "S", f"Especialista {name}", f"Captura 30 especies de tipo {name}.", typed(t, 30), give(reward))
for i, (t, name, _) in enumerate(TYPE_REWARDS):
    add(f"D{49 + i:02d}", "D", "B", f"Aprendiz {name}", f"Captura 10 especies de tipo {name}.", typed(t, 10), give("GREAT_BALL", 3))

# ---------------------------------------------------------------- Captura
add("C01", "C", "B", "¡Te tengo!", "Captura tu primer Pokémon.", stat("POKEMON_CAPTURES", 1), give("POKE_BALL", 10))
add("C02", "C", "S", "Cien en la caja", "Captura 100 Pokémon.", stat("POKEMON_CAPTURES", 100), give("ULTRA_BALL", 10))
add("C03", "C", "G", "Quinientos", "Captura 500 Pokémon.", stat("POKEMON_CAPTURES", 500), give("RARE_CANDY", 10))
add("C04", "C", "P", "Mil capturas", "Captura 1000 Pokémon.", stat("POKEMON_CAPTURES", 1000), give("MASTER_BALL"))
add("C05", "C", "B", "Probando balls", "Captura con 5 tipos de Poké Ball distintos.", cnt("BALL_KINDS", 5), give("LUXURY_BALL", 5))
add("C06", "C", "S", "Maestro de las balls", "Captura con 15 tipos de Poké Ball distintos.", cnt("BALL_KINDS", 15), give("QUICK_BALL", 10))
add("C07", "C", "S", "A la primera", "Captura 50 Pokémon al primer lanzamiento.", cnt("FIRST_BALL_CATCHES", 50), give("QUICK_BALL", 5))
add("C08", "C", "B", "¡Captura crítica!", "Consigue una captura crítica.", cnt("CRIT_CAPTURES", 1), give("ULTRA_BALL", 5))
add("C09", "C", "S", "Críticas en serie", "Consigue 25 capturas críticas.", cnt("CRIT_CAPTURES", 25), give("ULTRA_BALL", 15))
add("C10", "C", "G", "¡Brilla!", "Captura tu primer Pokémon variocolor.", cnt("SHINY_CAUGHT", 1), give("LUXURY_BALL", 10))
add("C11", "C", "P", "Cazashiny", "Captura 10 Pokémon variocolor.", cnt("SHINY_CAUGHT", 10), title("Cazashiny"))
add("C12", "C", "B", "Pesca de domingo", "Captura 10 Pokémon pescando.", cnt("FISHING_CATCHES", 10), give("LURE_BALL", 5))
add("C13", "C", "S", "Lobo de mar", "Captura 100 Pokémon pescando.", cnt("FISHING_CATCHES", 100), give("DIVE_BALL", 10))
add("C14", "C", "S", "Marea de Pokémon", "Captura 10 Pokémon de apariciones masivas.", cnt("OUTBREAK_CATCHES", 10), give("REPEAT_BALL", 5))
add("C15", "C", "B", "Cazador nocturno", "Captura 10 Pokémon de noche.", cnt("NIGHT_CATCHES", 10), give("DUSK_BALL", 10))
add("C16", "C", "S", "Ave nocturna", "Captura 100 Pokémon de noche.", cnt("NIGHT_CATCHES", 100), give("DUSK_BALL", 20))
add("C17", "C", "B", "Safari", "Captura 10 Pokémon en la Zona Safari.", cnt("SAFARI_CATCHES", 10), give("SAFARI_BALL", 30))
add("C18", "C", "S", "Cazador de leyendas", "Captura 5 Pokémon legendarios o singulares.", cnt("LEGEND_CATCHES", 5), give("MASTER_BALL"))
add("C19", "C", "G", "Leyenda viva", "Captura 15 Pokémon legendarios o singulares.", cnt("LEGEND_CATCHES", 15), give("MASTER_BALL", 2))
add("C20", "C", "B", "Escondido en la hierba", "Captura un Pokémon encontrado con el DexNav.", cnt("DEXNAV_CATCHES", 1), give("WIDE_LENS"))
add("C21", "C", "S", "Rastreador", "Captura 50 Pokémon encontrados con el DexNav.", cnt("DEXNAV_CATCHES", 50), give("ULTRA_BALL", 10))
add("C22", "C", "S", "Racha DexNav", "Llega a una cadena de 25 con el DexNav.", special("DEXNAV_CHAIN", 25), give("BOTTLE_CAP"))
add("C23", "C", "G", "Racha perfecta", "Llega a una cadena de 100 con el DexNav.", special("DEXNAV_CHAIN", 100), give("GOLD_BOTTLE_CAP"))
add("C24", "C", "S", "Habilidad rara", "Ten un Pokémon con habilidad oculta en el equipo.", special("HIDDEN_ABILITY"), give("ABILITY_PATCH"))
add("C25", "C", "B", "Salvajes", "Gana o captura en 50 combates salvajes.", stat("WILD_BATTLES", 50), give("REPEL", 10))
add("C26", "C", "S", "Explorador salvaje", "Participa en 500 combates salvajes.", stat("WILD_BATTLES", 500), give("MAX_REPEL", 10))
add("C27", "C", "S", "Doscientos cincuenta", "Captura 250 Pokémon.", stat("POKEMON_CAPTURES", 250), give("ULTRA_BALL", 15))
add("C28", "C", "S", "Trío brillante", "Captura 3 Pokémon variocolor.", cnt("SHINY_CAUGHT", 3), give("LUXURY_BALL", 5))
add("C29", "C", "P", "Coleccionista de brillos", "Captura 25 Pokémon variocolor.", cnt("SHINY_CAUGHT", 25), give("MASTER_BALL", 2))
add("C30", "C", "G", "Todas las balls", "Captura con 25 tipos de Poké Ball distintos.", cnt("BALL_KINDS", 25), give("BEAST_BALL", 5))

# ---------------------------------------------------------------- Combate
add("B01", "B", "B", "Primera victoria", "Vence a tu primer entrenador.", cnt("TRAINER_WINS", 1), give("POTION", 5))
add("B02", "B", "B", "Cazaentrenadores", "Vence a 50 entrenadores.", cnt("TRAINER_WINS", 50), give("NUGGET"))
add("B03", "B", "S", "Terror de las rutas", "Vence a 200 entrenadores.", cnt("TRAINER_WINS", 200), give("BIG_NUGGET"))
add("B04", "B", "G", "Leyenda de los caminos", "Vence a 500 entrenadores.", cnt("TRAINER_WINS", 500), title("Barredor"))
add("B05", "B", "B", "Cien combates", "Combate 100 veces.", stat("TOTAL_BATTLES", 100), give("PP_UP"))
add("B06", "B", "S", "Mil combates", "Combate 1000 veces.", stat("TOTAL_BATTLES", 1000), give("PP_MAX", 3))
add("B07", "B", "B", "¡Es muy eficaz!", "Usa 100 ataques superefectivos.", cnt("SUPER_EFFECTIVE", 100), give("HYPER_POTION", 5))
add("B08", "B", "S", "Maestro de tipos", "Usa 1000 ataques superefectivos.", cnt("SUPER_EFFECTIVE", 1000), give("EXPERT_BELT"))
add("B09", "B", "S", "De un golpe", "Debilita a 50 Pokémon de un solo ataque.", cnt("OHKO", 50), give("CHOICE_BAND"))
add("B10", "B", "G", "Golpe certero", "Debilita a 500 Pokémon de un solo ataque.", cnt("OHKO", 500), give("LIFE_ORB"))
add("B11", "B", "B", "¡Golpe crítico!", "Asesta 100 golpes críticos.", cnt("CRITS", 100), give("SCOPE_LENS"))
add("B12", "B", "S", "Críticos sin fin", "Asesta 500 golpes críticos.", cnt("CRITS", 500), give("RAZOR_CLAW"))
add("B13", "B", "B", "Chispazo", "Paraliza a 50 Pokémon.", cnt("PARALYZED", 50), give("CHERI_BERRY", 5))
add("B14", "B", "B", "Quemadura", "Quema a 50 Pokémon.", cnt("BURNED", 50), give("RAWST_BERRY", 5))
add("B15", "B", "B", "Canción de cuna", "Duerme a 50 Pokémon.", cnt("SLEPT", 50), give("CHESTO_BERRY", 5))
add("B16", "B", "S", "Congelado", "Congela a 10 Pokémon.", cnt("FROZEN", 10), give("ASPEAR_BERRY", 5))
add("B17", "B", "B", "Tóxico", "Envenena a 50 Pokémon.", cnt("POISONED", 50), give("PECHA_BERRY", 5))
add("B18", "B", "S", "Megaevolución", "Megaevoluciona por primera vez.", cnt("MEGA", 1), give("OMNI_CELL"))
add("B19", "B", "G", "Megaexperto", "Megaevoluciona 50 veces.", cnt("MEGA", 50), title("Mega"))
add("B20", "B", "S", "Teracristal", "Teracristaliza por primera vez.", cnt("TERA", 1), give("RARE_CANDY", 3))
add("B21", "B", "G", "Teraexperto", "Teracristaliza 50 veces.", cnt("TERA", 50), give("OMNI_CELL"))
add("B22", "B", "S", "Gigante", "Usa el Dinamax por primera vez.", cnt("DYNAMAX", 1), give("RARE_CANDY", 3))
add("B23", "B", "G", "Dinaexperto", "Usa el Dinamax 50 veces.", cnt("DYNAMAX", 50), give("GOLD_BOTTLE_CAP"))
add("B24", "B", "S", "Poder Z", "Usa un movimiento Z por primera vez.", cnt("ZMOVE", 1), give("RARE_CANDY", 3))
add("B25", "B", "G", "Experto Z", "Usa 50 movimientos Z.", cnt("ZMOVE", 50), give("GOLD_BOTTLE_CAP"))
add("B26", "B", "G", "Las cuatro mecánicas", "Usa megaevolución, Tera, Dinamax y movimiento Z.", special("ALL_GIMMICKS"), title("Estratega"))
add("B27", "B", "B", "Cambio de planes", "Cambia de Pokémon 100 veces en combate.", cnt("SWITCHES", 100), give("EJECT_BUTTON"))
add("B28", "B", "S", "Racha de diez", "Gana 10 combates de entrenador seguidos.", cnt("BEST_WIN_STREAK", 10), give("PP_UP", 3))
add("B29", "B", "G", "Racha de cincuenta", "Gana 50 combates de entrenador seguidos.", cnt("BEST_WIN_STREAK", 50), give("GOLD_BOTTLE_CAP"))
add("B30", "B", "P", "Invicto", "Gana 100 combates de entrenador seguidos.", cnt("BEST_WIN_STREAK", 100), title("Invicto"))
add("B31", "B", "S", "Contra las cuerdas", "Gana un combate con tu último Pokémon por debajo del 10 % de PS.", cnt("CLUTCH_WINS", 1), give("SITRUS_BERRY", 5))
add("B32", "B", "B", "Cien caídos", "Debilita a 100 Pokémon rivales.", cnt("FAINTED_FOES", 100), give("EXP_CANDY_M", 3))
add("B33", "B", "S", "Mil caídos", "Debilita a 1000 Pokémon rivales.", cnt("FAINTED_FOES", 1000), give("EXP_CANDY_XL", 3))
add("B34", "B", "G", "Sin bajas", "Gana la Liga sin que se debilite ningún Pokémon tuyo.", event(), give("LIFE_ORB"))
add("B35", "B", "P", "Mil victorias", "Vence a 1000 entrenadores.", cnt("TRAINER_WINS", 1000), title("Leyenda viviente"))
add("B36", "B", "S", "Megaaficionado", "Megaevoluciona 10 veces.", cnt("MEGA", 10), give("RARE_CANDY", 3))
add("B37", "B", "S", "Teraaficionado", "Teracristaliza 10 veces.", cnt("TERA", 10), give("RARE_CANDY", 3))
add("B38", "B", "S", "Dinaaficionado", "Usa el Dinamax 10 veces.", cnt("DYNAMAX", 10), give("RARE_CANDY", 3))
add("B39", "B", "S", "Aficionado Z", "Usa 10 movimientos Z.", cnt("ZMOVE", 10), give("RARE_CANDY", 3))
add("B40", "B", "S", "Estados alterados", "Paraliza, quema, duerme o envenena a 500 Pokémon en total.", special("STATUS_TOTAL", 500), give("TOXIC_ORB"))

# ---------------------------------------------------------------- Entrenamiento
add("T01", "T", "B", "Creciendo", "Sube un Pokémon a nivel 25.", level(25), give("EXP_CANDY_S", 5))
add("T02", "T", "S", "Veterano", "Sube un Pokémon a nivel 50.", level(50), give("EXP_CANDY_M", 5))
add("T03", "T", "G", "Nivel máximo", "Sube un Pokémon a nivel 100.", level(100), give("GOLD_BOTTLE_CAP"))
add("T04", "T", "P", "Equipo de leyenda", "Ten 6 Pokémon de nivel 100 en el equipo.", level_all(100), title("Entrenador de élite"))
add("T05", "T", "S", "Equipo sólido", "Ten 6 Pokémon de nivel 50 o más en el equipo.", level_all(50), give("EXP_CANDY_L", 3))
add("T06", "T", "B", "¡Está evolucionando!", "Evoluciona un Pokémon.", stat("EVOLVED_POKEMON", 1), give("RARE_CANDY"))
add("T07", "T", "S", "Evolucionista", "Evoluciona 50 Pokémon.", stat("EVOLVED_POKEMON", 50), give("SHINY_STONE"))
add("T08", "T", "G", "Teoría de la evolución", "Evoluciona 150 Pokémon.", stat("EVOLVED_POKEMON", 150), give("RARE_CANDY", 10))
add("T09", "T", "B", "Frescor mentolado", "Usa una menta.", cnt("MINTS", 1), give("JOLLY_MINT"))
add("T10", "T", "S", "Esfuerzo total", "Ten en el equipo un Pokémon con 510 EV.", special("EV_510"), give("POWER_WEIGHT"))
add("T11", "T", "G", "Perfección", "Ten en el equipo un Pokémon con los 6 IV a 31.", special("IV_PERFECT"), give("GOLD_BOTTLE_CAP"))
add("T12", "T", "S", "Mejores amigos", "Lleva a un Pokémon a la amistad máxima.", special("FRIENDSHIP_ONE"), give("SOOTHE_BELL"))
add("T13", "T", "G", "Amistad total", "Ten 6 Pokémon con amistad máxima en el equipo.", special("FRIENDSHIP_ALL"), give("LUXURY_BALL", 10))
add("T14", "T", "B", "Otra habilidad", "Usa una Cápsula o un Parche Habilidad.", cnt("CAPSULES", 1), give("ABILITY_CAPSULE"))
add("T15", "T", "S", "Goloso", "Usa 100 Caramelos EXP.", cnt("EXP_CANDIES", 100), give("EXP_CANDY_XL", 3))
add("T16", "T", "S", "Atajo", "Usa 50 Caramelos raros.", cnt("RARE_CANDIES", 50), give("RARE_CANDY", 10))
add("T17", "T", "S", "Vitaminas", "Usa 50 vitaminas.", cnt("VITAMINS", 50), give("RARE_CANDY", 5))
add("T18", "T", "S", "Entrenamiento extremo", "Usa el Entrenamiento extremo.", cnt("HYPER_TRAININGS", 1), give("BOTTLE_CAP", 2))
add("T19", "T", "G", "Más allá del límite", "Usa el Entrenamiento extremo 20 veces.", cnt("HYPER_TRAININGS", 20), give("GOLD_BOTTLE_CAP"))
add("T20", "T", "B", "Virus amistoso", "Ten un Pokémon con Pokérus en el equipo.", special("POKERUS"), give("PP_UP"))
add("T21", "T", "S", "Especialista en EV", "Ten un Pokémon con 252 EV en una estadística.", special("EV_252"), give("POWER_BRACER"))

# ---------------------------------------------------------------- Exploración
for i, (f, name, tier, reward) in enumerate([
        ("VIRIDIAN_FOREST", "Bosque Verde", "B", give("POTION", 5)), ("MT_MOON_1F", "Monte Moon", "B", give("MOON_STONE")),
        ("DIGLETTS_CAVE_B1F", "Cueva Diglett", "B", give("SOFT_SAND")), ("ROCK_TUNNEL_1F", "Túnel Roca", "B", give("HARD_STONE")),
        ("POKEMON_TOWER_1F", "Torre Pokémon", "B", give("CLEANSE_TAG")), ("POWER_PLANT", "Central de Energía", "S", give("MAGNET")),
        ("SAFARI_ZONE_CENTER", "Zona Safari", "B", give("SAFARI_BALL", 30)), ("SEAFOAM_ISLANDS_1F", "Islas Espuma", "S", give("NEVER_MELT_ICE")),
        ("POKEMON_MANSION_1F", "Mansión Pokémon", "S", give("FIRE_STONE")), ("VICTORY_ROAD_1F", "Calle Victoria", "S", give("MAX_REVIVE", 2)),
        ("CERULEAN_CAVE_1F", "Cueva Celeste", "G", give("ULTRA_BALL", 10)), ("MT_EMBER_EXTERIOR", "Monte Ascuas", "S", give("CHARCOAL")),
        ("THREE_ISLAND_BERRY_FOREST", "Bosque Baya", "S", give("SITRUS_BERRY", 3)), ("FOUR_ISLAND_ICEFALL_CAVE_ENTRANCE", "Cueva Helada", "S", give("ICE_STONE")),
        ("FIVE_ISLAND_LOST_CAVE_ENTRANCE", "Cueva Perdida", "S", give("SPELL_TAG")), ("SIX_ISLAND_PATTERN_BUSH", "Arbusto Patrón", "B", give("SILVER_POWDER")),
        ("SIX_ISLAND_ALTERING_CAVE", "Cueva Cambiante", "S", give("ULTRA_BALL", 5)), ("TRAINER_TOWER_LOBBY", "Torre Entrenador", "S", give("PP_UP", 2))]):
    add(f"E{i + 1:02d}", "E", tier, name, f"Visita {name}.", flag("FLAG_WORLD_MAP_" + f), reward)
add("E19", "E", "S", "Viajero de Kanto", "Visita todas las ciudades y pueblos de Kanto.", special("ALL_KANTO_TOWNS"), give("MAX_REPEL", 10))
add("E20", "E", "G", "Las Islas Sete", "Visita las siete Islas Sete.", special("ALL_SEVII_ISLANDS"), give("EXP_CHARM"))
add("E21", "E", "B", "Buen paso", "Camina 10.000 pasos.", stat("STEPS", 10000), give("NUGGET"))
add("E22", "E", "S", "Maratón", "Camina 50.000 pasos.", stat("STEPS", 50000), give("BIG_NUGGET"))
add("E23", "E", "G", "La vuelta al mundo", "Camina 200.000 pasos.", stat("STEPS", 200000), give("BIG_NUGGET", 5))
add("E24", "E", "B", "Buen olfato", "Encuentra 10 objetos ocultos.", special("HIDDEN_ITEMS", 10), give("NUGGET"))
add("E25", "E", "S", "Detector humano", "Encuentra 60 objetos ocultos.", special("HIDDEN_ITEMS", 60), give("IRON_BALL"))
add("E26", "E", "B", "Saltarín", "Salta 100 bordillos.", stat("JUMPED_DOWN_LEDGES", 100), give("QUICK_CLAW"))
add("E27", "E", "S", "Cortador", "Usa Corte 50 veces.", stat("USED_CUT", 50), give("SHARP_BEAK"))
add("E28", "E", "S", "Rompedor", "Usa Golpe Roca 50 veces.", stat("USED_ROCK_SMASH", 50), give("HARD_STONE"))

TOWNS = [("PALLET_TOWN", "Pueblo Paleta"), ("VIRIDIAN_CITY", "Ciudad Verde"), ("PEWTER_CITY", "Ciudad Plateada"),
         ("CERULEAN_CITY", "Ciudad Celeste"), ("VERMILION_CITY", "Ciudad Carmín"), ("LAVENDER_TOWN", "Pueblo Lavanda"),
         ("CELADON_CITY", "Ciudad Azulona"), ("FUCHSIA_CITY", "Ciudad Fucsia"), ("SAFFRON_CITY", "Ciudad Azafrán"),
         ("CINNABAR_ISLAND", "Isla Canela"), ("INDIGO_PLATEAU_EXTERIOR", "Meseta Añil"), ("ONE_ISLAND", "Isla Prima"),
         ("TWO_ISLAND", "Isla Secunda"), ("THREE_ISLAND", "Isla Tera"), ("FOUR_ISLAND", "Isla Cuarta"),
         ("FIVE_ISLAND", "Isla Quinta"), ("SIX_ISLAND", "Isla Sexta"), ("SEVEN_ISLAND", "Isla Séptima")]
for i, (f, name) in enumerate(TOWNS):
    add(f"E{29 + i:02d}", "E", "B", f"Rumbo a {name}", f"Visita {name}.", flag("FLAG_WORLD_MAP_" + f), give("POTION", 2))
for i, (f, name, reward) in enumerate([
        ("UNDERGROUND_PATH_NORTH_SOUTH_TUNNEL", "el Túnel Norte-Sur", give("REPEL", 3)),
        ("UNDERGROUND_PATH_EAST_WEST_TUNNEL", "el Túnel Este-Oeste", give("REPEL", 3)),
        ("SSANNE_EXTERIOR", "el S.S. Anne", give("GREAT_BALL", 3)), ("ROCKET_HIDEOUT_B1F", "el Escondite Rocket", give("BLACK_GLASSES")),
        ("SILPH_CO_1F", "Silph S.A.", give("ULTRA_BALL", 3)), ("POKEMON_LEAGUE_LORELEIS_ROOM", "la Liga Pokémon", give("FULL_RESTORE", 2)),
        ("FIVE_ISLAND_ROCKET_WAREHOUSE", "el Almacén Rocket", give("ULTRA_BALL", 3)), ("SIX_ISLAND_DOTTED_HOLE_1F", "el Hoyo Punteado", give("PP_UP")),
        ("THREE_ISLAND_DUNSPARCE_TUNNEL", "el Túnel Dunsparce", give("NUGGET")),
        ("SEVEN_ISLAND_TANOBY_RUINS_MONEAN_CHAMBER", "la Cámara Monean", give("ULTRA_BALL", 3)),
        ("SEVEN_ISLAND_SEVAULT_CANYON_TANOBY_KEY", "el Cañón Sevault", give("NUGGET")),
        ("ROUTE10_POKEMON_CENTER_1F", "el Centro de la Ruta 10", give("POTION", 3))]):
    add(f"E{47 + i:02d}", "E", "B", f"Visita a {name.split(' ', 1)[-1] if name.startswith('el ') else name}",
        f"Visita {name}.", flag("FLAG_WORLD_MAP_" + f), reward)
add("E59", "E", "P", "Medio millón de pasos", "Camina 500.000 pasos.", stat("STEPS", 500000), title("Trotamundos"))

# ---------------------------------------------------------------- Crianza
add("K01", "K", "B", "¡Un huevo!", "Eclosiona tu primer huevo.", stat("HATCHED_EGGS", 1), give("LUCKY_EGG"))
add("K02", "K", "S", "Criador", "Eclosiona 50 huevos.", stat("HATCHED_EGGS", 50), give("OVAL_CHARM"))
add("K03", "K", "S", "Cien huevos", "Eclosiona 100 huevos.", stat("HATCHED_EGGS", 100), give("DESTINY_KNOT"))
add("K04", "K", "G", "Granja Pokémon", "Eclosiona 500 huevos.", stat("HATCHED_EGGS", 500), title("Criador experto"))
add("K05", "K", "B", "Guardería abierta", "Usa la Guardería.", stat("USED_DAYCARE", 1), give("EVERSTONE"))
add("K06", "K", "S", "Clientela fija", "Usa la Guardería 20 veces.", stat("USED_DAYCARE", 20), give("POWER_ANKLET"))

# ---------------------------------------------------------------- Colección
add("O01", "O", "B", "Ahorros", "Ten 10.000 ¥.", money(10000), give("NUGGET"))
add("O02", "O", "S", "Rico", "Ten 100.000 ¥.", money(100000), give("AMULET_COIN"))
add("O03", "O", "G", "Millonario", "Ten 999.999 ¥.", money(999999), title("Magnate"))
add("O04", "O", "B", "De compras", "Compra en tiendas 20 veces.", stat("SHOPPED", 20), give("PREMIER_BALL", 10))
add("O05", "O", "S", "Buen cliente", "Compra en tiendas 200 veces.", stat("SHOPPED", 200), give("BIG_NUGGET"))
MEGA_STONES = ["BEEDRILLITE", "CLEFABLITE", "PIDGEOTITE", "VICTREEBELITE", "STEELIXITE", "GENGARITE", "KANGASKHANITE",
               "PINSIRITE", "SCIZORITE", "ALAKAZITE", "GYARADOSITE", "SLOWBRONITE", "AERODACTYLITE", "HOUNDOOMINITE",
               "STARMINITE", "HERACRONITE", "DRAGONINITE", "TYRANITARITE", "VENUSAURITE", "CHARIZARDITE_X",
               "CHARIZARDITE_Y", "BLASTOISINITE", "MEWTWONITE_X", "MEWTWONITE_Y"]
Z_CRYSTALS = ["NORMALIUM_Z", "FIRIUM_Z", "WATERIUM_Z", "ELECTRIUM_Z", "GRASSIUM_Z", "ICIUM_Z", "FIGHTINIUM_Z",
              "POISONIUM_Z", "GROUNDIUM_Z", "FLYINIUM_Z", "PSYCHIUM_Z", "BUGINIUM_Z", "ROCKIUM_Z", "GHOSTIUM_Z",
              "DRAGONIUM_Z", "DARKINIUM_Z", "STEELIUM_Z", "FAIRIUM_Z"]
add("O06", "O", "B", "Primera megapiedra", "Consigue una megapiedra de un guardián.", items(1, *MEGA_STONES), give("OMNI_CELL"))
add("O07", "O", "S", "Coleccionista mega", "Consigue 12 megapiedras de guardianes.", items(12, *MEGA_STONES), give("GOLD_BOTTLE_CAP"))
add("O08", "O", "G", "Megacolección", "Consigue las 24 megapiedras de los guardianes.", items(24, *MEGA_STONES), title("Guardián de las piedras"))
add("O09", "O", "B", "Primer cristal", "Consigue un cristal Z de un desafío.", special("Z_TRIALS", 1), give("RARE_CANDY", 2))
add("O10", "O", "S", "Medio camino Z", "Supera 9 desafíos Z.", special("Z_TRIALS", 9), give("GOLD_BOTTLE_CAP"))
add("O11", "O", "G", "Coleccionista Z", "Supera los 18 desafíos Z.", special("Z_TRIALS", 18), title("Gran capitán"))
add("O12", "O", "S", "Herbolario", "Usa 10 mentas.", cnt("MINTS", 10), give("BOTTLE_CAP", 2))
add("O13", "O", "G", "Piedras preciosas", "Ten las diez piedras evolutivas a la vez.",
    items(10, "FIRE_STONE", "THUNDER_STONE", "WATER_STONE", "LEAF_STONE", "MOON_STONE", "SUN_STONE", "SHINY_STONE",
          "DUSK_STONE", "DAWN_STONE", "ICE_STONE"), give("BIG_NUGGET", 2))
add("O14", "O", "S", "Amuletos", "Consigue el Amuleto Oval, el Amuleto Captura y el Amuleto EXP.",
    items(3, "OVAL_CHARM", "CATCHING_CHARM", "EXP_CHARM"), give("GOLD_BOTTLE_CAP"))
add("O15", "O", "P", "Amuleto Iris", "Consigue el Amuleto Iris.", item("SHINY_CHARM"), title("Coleccionista"))

# ---------------------------------------------------------------- Sistemas nuevos
add("X01", "X", "B", "Anillo despierto", "Instala tu primer módulo en el Omni Ring.", special("OMNI_MODULES", 1), give("RARE_CANDY"))
add("X02", "X", "G", "Anillo completo", "Instala los cuatro módulos en el Omni Ring.", special("OMNI_MODULES", 4), give("GOLD_BOTTLE_CAP"))
add("X03", "X", "S", "Batería llena", "Lleva el Omni Ring a 9 cargas.", special("OMNI_CHARGES", 9), give("MAX_ELIXIR", 3))
add("X04", "X", "B", "Intercambio Prodigio", "Haz tu primer Intercambio Prodigio.", cnt("WONDER_TRADES", 1), give("RARE_CANDY"))
add("X05", "X", "S", "Caja sorpresa", "Haz 30 Intercambios Prodigio.", cnt("WONDER_TRADES", 30), give("BOTTLE_CAP", 2))
add("X06", "X", "G", "Prodigio brillante", "Recibe un variocolor en un Intercambio Prodigio.", cnt("WONDER_SHINY", 1), give("LUXURY_BALL", 10))
add("X07", "X", "B", "Botiquín", "Usa el Botiquín 20 veces.", cnt("VIAL_USES", 20), give("FULL_RESTORE", 3))
add("X08", "X", "B", "Llave maestra", "Cambia una clave del Key System.", cnt("KEYS_CHANGED", 1), give("POTION", 3))
add("X09", "X", "S", "Retador", "Juega en dificultad Reto.", special("CHALLENGE_MODE"), give("PP_MAX"))
add("X10", "X", "B", "Búsqueda", "Busca 10 Pokémon con el DexNav.", stat("DEXNAV_SCANNED", 10), give("GREAT_BALL", 5))
add("X11", "X", "S", "Investigador de campo", "Busca 200 Pokémon con el DexNav.", stat("DEXNAV_SCANNED", 200), give("ULTRA_BALL", 10))
add("X12", "X", "G", "Experto en una especie", "Llega a nivel de búsqueda 100 con una especie.", special("DEXNAV_LEVEL", 100), give("GOLD_BOTTLE_CAP"))
add("X13", "X", "S", "Premios de Oak", "Recibe 4 premios de la Pokédex.", special("DEX_REWARDS", 4), give("ULTRA_BALL", 10))
add("X14", "X", "G", "Todos los premios de Oak", "Recibe los 8 premios de la Pokédex.", special("DEX_REWARDS", 8), title("Ayudante de Oak"))

# ---------------------------------------------------------------- Desafíos (checked when entering the Hall of Fame)
add("R01", "R", "P", "Modo Reto", "Entra en el Salón de la Fama en dificultad Reto.", event(), title("Retador"))
add("R02", "R", "P", "El elegido", "Entra en el Salón de la Fama con un solo Pokémon.", event(), title("Solitario"))
add("R03", "R", "P", "Contrarreloj", "Entra en el Salón de la Fama en menos de 8 horas.", event(), title("Speedrunner"))
add("R04", "R", "P", "Rayo humano", "Entra en el Salón de la Fama en menos de 4 horas.", event(), title("Relámpago"))
add("R05", "R", "G", "Monotipo", "Entra en el Salón de la Fama con un equipo que comparte un tipo.", event(), title("Especialista"))
add("R06", "R", "P", "Bajo nivel", "Entra en el Salón de la Fama sin Pokémon por encima del nivel 55.", event(), title("Minimalista"))
add("R07", "R", "P", "Sin evolucionar", "Entra en el Salón de la Fama sin haber evolucionado ningún Pokémon.", event(), give("EVIOLITE"))
add("R08", "R", "P", "Pocas capturas", "Entra en el Salón de la Fama habiendo capturado 6 Pokémon o menos.", event(), title("Austero"))
add("R09", "R", "G", "Seis gemelos", "Entra en el Salón de la Fama con 6 Pokémon de la misma especie.", event(), title("Gemelos"))
add("R10", "R", "G", "Generación nueva", "Entra en el Salón de la Fama solo con Pokémon de la Gen 5 en adelante.", event(), title("Moderno"))
add("R11", "R", "P", "Sin Centros", "Entra en el Salón de la Fama sin haber curado en un Centro Pokémon.", event(), title("Superviviente"))
add("R12", "R", "G", "Contra todo pronóstico", "Entra en el Salón de la Fama con un Magikarp en el equipo.", event(), title("Salpicador"))

# ---------------------------------------------------------------- Ocultos
add("Z01", "Z", "B", "¡Salpicadura!", "Usa Salpicadura 100 veces.", stat("USED_SPLASH", 100), give("RARE_CANDY"), hidden=True)
add("Z02", "Z", "B", "Último recurso", "Usa Forcejeo 20 veces.", stat("USED_STRUGGLE", 20), give("LEPPA_BERRY", 5), hidden=True)
add("Z03", "Z", "B", "Retirada estratégica", "Huye de 100 combates.", cnt("FLED", 100), give("SMOKE_BALL"), hidden=True)
add("Z04", "Z", "B", "¡Bonk!", "Choca contra una pared 500 veces.", cnt("WALL_BUMPS", 500), give("ROCKY_HELMET"), hidden=True)
add("Z05", "Z", "S", "Despedidas", "Libera a 50 Pokémon.", cnt("RELEASES", 50), give("LUXURY_BALL", 5), hidden=True)
add("Z06", "Z", "S", "¡Siete, siete, siete!", "Saca el premio gordo en las tragaperras.", stat("SLOT_JACKPOTS", 1), give("BIG_NUGGET"), hidden=True)
add("Z07", "Z", "B", "Lector empedernido", "Mira la Pokédex 200 veces.", stat("CHECKED_POKEDEX", 200), give("GREAT_BALL", 5), hidden=True)
add("Z08", "Z", "B", "Ahorrador", "Guarda la partida 100 veces.", stat("SAVED_GAME", 100), give("NUGGET"), hidden=True)
add("Z09", "Z", "S", "Cliente del Centro", "Cura en un Centro Pokémon 100 veces.", stat("USED_POKECENTER", 100), give("FULL_RESTORE", 3), hidden=True)
add("Z10", "Z", "G", "Prodigio legendario", "Recibe un legendario en un Intercambio Prodigio.", cnt("WONDER_LEGEND", 1), give("MASTER_BALL"), hidden=True)
add("Z11", "Z", "S", "El récord del Magikarp", "Consigue el Magikarp récord.", flag("FLAG_GOT_RECORD_SETTING_MAGIKARP"), give("DRAGON_SCALE"), hidden=True)
add("Z12", "Z", "G", "Fiel a sí mismo", "Ten un Magikarp de nivel 100.", special("MAGIKARP_100"), give("MYSTIC_WATER"), hidden=True)

# ---------------------------------------------------------------- Maestría
add("M01", "M", "B", "Primeros pasos", "Desbloquea el 10 % de los logros.", pct(10), give("GREAT_BALL", 10))
add("M02", "M", "S", "Un cuarto", "Desbloquea el 25 % de los logros.", pct(25), give("RARE_CANDY", 5))
add("M03", "M", "G", "La mitad", "Desbloquea el 50 % de los logros.", pct(50), give("GOLD_BOTTLE_CAP"))
add("M04", "M", "G", "Tres cuartos", "Desbloquea el 75 % de los logros.", pct(75), give("MASTER_BALL", 2))
add("M05", "M", "P", "Casi perfecto", "Desbloquea el 90 % de los logros.", pct(90), give("GOLD_BOTTLE_CAP", 3))
add("M06", "M", "G", "Todo el bronce", "Desbloquea todos los logros de bronce.", tier_all("B"), give("BIG_NUGGET", 3))
add("M07", "M", "G", "Sin secretos", "Desbloquea todos los logros ocultos.", hidden_all(), title("Detective"))
add("M08", "M", "P", "Maestro de logros", "Desbloquea todos los demás logros.", pct(99), title("Maestro de logros"))

# Append only. 318/319 use the two spare bits; ACH_BYTES remains 40.
add("RF_STORY_FIRST", "H", "S", "Primera crónica", "Concluye una campaña de Kanto.", event(), title("Cronista"))
add("RF_STORY_ALL", "H", "G", "Memorias compartidas", "Concluye las ocho campañas narrativas.", event(), title("Memoria de Kanto"))

SPECIALS = sorted({a["cond"][1] for a in ach if a["cond"][0] == "ACH_COND_SPECIAL"})


def c_string(s):
    return 'COMPOUND_STRING("' + s.replace('"', '\\"') + '")'


def wrap(text, width=34):
    """Splits a description in two lines for the game's description window."""
    words, lines, line = text.split(), [], ""
    for w in words:
        if len(line) + len(w) + 1 > width and line:
            lines.append(line)
            line = w
        else:
            line = (line + " " + w).strip()
    lines.append(line)
    if len(lines) > 2:
        lines = [lines[0], " ".join(lines[1:])]
    return "\\n".join(lines)


def write_ra():
    """RetroAchievements local set for RALibretro / RAIntegration ("<game id>-User.txt").

    Each achievement fires when its bit in gRfAchievementMirror goes from 0 to 1. rcheevos maps
    GBA EWRAM (0x02000000) to address 0x008000; the mirror address comes from pokefirered.map, so
    run this after building. Bit conditions use 0xM (bit 0) to 0xT (bit 7).
    """
    map_file = ROOT / "pokefirered.map"
    if not map_file.exists():
        print("pokefirered.map not found: build first to write the RetroAchievements set")
        return
    mirror = None
    for line in map_file.read_text(encoding="utf-8", errors="replace").splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[1] == "gRfAchievementMirror":
            mirror = int(parts[0], 16)
    if mirror is None:
        print("gRfAchievementMirror not in pokefirered.map")
        return
    base = 0x008000 + (mirror - 0x02000000)
    synced = base + (len(ach) + 7) // 8  # MIRROR_SYNCED byte; must already be set on the previous frame
    lines = ["1.0", "Pokemon Rojo Fuego+"]
    for i, a in enumerate(ach):
        address = base + i // 8
        bit = "MNOPQRST"[i % 8]
        memaddr = f"d0xH{synced:06x}=165_d0x{bit}{address:06x}=0_0x{bit}{address:06x}=1"
        title = a["name"].replace('"', "'")
        desc = a["desc"].replace('"', "'")
        lines.append(f'{111000001 + i}:"{memaddr}":"{title}":"{desc}"::::Rojo Fuego+:{TIER_POINTS[a["tier"]] // 5}:::::00000')
    out = ROOT / "tools/rojo_fuego/retroachievements_local.txt"
    out.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"RetroAchievements set: {out.relative_to(ROOT)} (mirror at 0x{mirror:08x})")


def main():
    ids = [a["id"] for a in ach]
    assert len(ids) == len(set(ids)), "duplicate achievement id"

    const = ["// Generated by tools/rojo_fuego/achievements.py; do not edit.",
             "#ifndef GUARD_CONSTANTS_RF_ACHIEVEMENTS_H", "#define GUARD_CONSTANTS_RF_ACHIEVEMENTS_H", "",
             f"#define ACH_COUNT {len(ach)}", "#define ACH_BYTES ((ACH_COUNT + 7) / 8)", ""]
    for i, a in enumerate(ach):
        const.append(f"#define ACH_{a['id']} {i}")
    const += ["", "enum AchievementCounter", "{"] + [f"    ACH_COUNTER_{c}," for c in COUNTERS] + ["    ACH_COUNTER_COUNT", "};", ""]
    const += ["enum AchievementSpecial", "{"] + [f"    {s}," for s in SPECIALS] + ["    ACH_SPECIAL_COUNT", "};", ""]
    const += ["enum AchievementCategory", "{"] + [f"    ACH_CAT_{c}," for c, _ in CATEGORIES] + ["    ACH_CAT_COUNT", "};", ""]
    const += ["#endif // GUARD_CONSTANTS_RF_ACHIEVEMENTS_H", ""]
    (ROOT / "include/constants/rf_achievements.h").write_text("\n".join(const), encoding="utf-8")

    data = ["// Generated by tools/rojo_fuego/achievements.py; do not edit.", ""]
    sets = []
    for a in ach:
        ctype, arg, value = a["cond"]
        if isinstance(arg, list):
            name = f"sAchSet_{a['id']}"
            data.append(f"static const u16 {name}[] = {{{', '.join(arg)}}};")
            sets.append(a["id"])
    data.append("")
    data.append("static const u8 *const sAchCategoryNames[ACH_CAT_COUNT] =\n{")
    for c, n in CATEGORIES:
        data.append(f"    [ACH_CAT_{c}] = COMPOUND_STRING(\"{n}\"),")
    data.append("};\n")
    data.append("const struct Achievement gAchievements[ACH_COUNT] =\n{")
    for a in ach:
        ctype, arg, value = a["cond"]
        item_const, qty, title_text = a["reward"]
        if isinstance(arg, list):
            arg_c = f"0, .set = sAchSet_{a['id']}, .setSize = ARRAY_COUNT(sAchSet_{a['id']})"
            value = value if ctype != "ACH_COND_SPECIES_SET" else len(arg)
        else:
            arg_c = str(arg)
        data.append(f"    [ACH_{a['id']}] =\n    {{\n"
                    f"        .name = {c_string(a['name'])},\n"
                    f"        .description = COMPOUND_STRING(\"{wrap(a['desc'])}\"),\n"
                    f"        .category = ACH_CAT_{a['cat']},\n"
                    f"        .tier = {TIER_NAMES[a['tier']]},\n"
                    f"        .hidden = {'TRUE' if a['hidden'] else 'FALSE'},\n"
                    f"        .condType = {ctype},\n"
                    f"        .condArg = {arg_c},\n"
                    f"        .condValue = {value},\n"
                    f"        .rewardItem = {item_const},\n"
                    f"        .rewardQuantity = {qty},\n"
                    f"        .rewardTitle = {c_string(title_text) if title_text else 'NULL'},\n"
                    f"    }},")
    data.append("};\n")
    (ROOT / "src/data/rf_achievements.h").write_text("\n".join(data), encoding="utf-8")

    page = [{"id": a["id"], "cat": a["cat"], "tier": a["tier"], "hidden": a["hidden"], "name": a["name"],
             "desc": a["desc"],
             "reward": (f"Título “{a['reward'][2]}”" if a["reward"][2] else
                        f"{a['reward'][0].replace('ITEM_', '').replace('_', ' ').title()} ×{a['reward'][1]}")}
            for a in ach]
    (ROOT / "tools/rojo_fuego/achievements.json").write_text(json.dumps(page, ensure_ascii=False, indent=1), encoding="utf-8")
    print(f"{len(ach)} achievements, {len(COUNTERS)} counters, {len(SPECIALS)} special checks")
    write_ra()


if __name__ == "__main__":
    main()
