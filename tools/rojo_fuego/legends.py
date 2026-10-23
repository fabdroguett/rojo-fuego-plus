#!/usr/bin/env python3
"""Rojo Fuego+ legendary sagas: every Legendary, Mythical, Ultra Beast and Paradox Pokémon.

Each saga is a mission (chapters LEYENDAS DE KANTO / LEYENDAS DEL MUNDO / ULTRAENTES Y
PARADOJAS). Accepting it makes its Pokémon appear as overworld objects in special places; each
one is a one-time static battle. A capture is stored in SaveBlock3 (rf_legends.c) and hides the
object for good; a defeat or escape leaves it there to try again. The mission is ready to claim
once the whole saga is caught.

Spots are chosen from the real map data: a walkable, open tile (all eight neighbours walkable)
that can be reached on foot from the map's warps or connections, away from other events.

    python tools/rojo_fuego/legends.py
rewrites data/scripts/rojo_fuego_legends.inc, src/data/rf_legends.h,
src/data/rf_quests_legends.h, include/constants/rf_legends.h, the legend block of
include/constants/rf_quests.h, and the legend objects of the map.json files (only those
objects are touched; running it again gives the same result).
"""
import json
import pathlib
import re
from collections import deque

ROOT = pathlib.Path(__file__).resolve().parents[2]
MAPS = ROOT / "data/maps"
SCRIPT_PREFIX = "RF_EventScript_Legend_"
FIRST_QUEST = 43
CHAPTER_KANTO, CHAPTER_WORLD, CHAPTER_BEYOND = 6, 7, 8
B = {n: f"FLAG_BADGE0{n}_GET" for n in range(1, 9)}
CLEAR = "FLAG_SYS_GAME_CLEAR"

# (name, chapter, required flag, level, reward item, reward count, description, [(species, map)])
SAGAS = [
    ("Guardianes de los lagos", CHAPTER_KANTO, B[3], 40, "ITEM_ULTRA_BALL", 5,
     "Uxie, Mesprit y Azelf meditan junto\nal agua en las rutas 24, 25 y 4.\nAtrápalos a los tres.",
     [("UXIE", "Route24"), ("MESPRIT", "Route25"), ("AZELF", "Route4")]),
    ("Espadas de la justicia", CHAPTER_KANTO, B[4], 42, "ITEM_ULTRA_BALL", 5,
     "Cobalion, Terrakion, Virizion y\nKeldeo entrenan en las rutas 3, 9,\n2 y 22. Demuestra tu valía.",
     [("COBALION", "Route3"), ("TERRAKION", "Route9"), ("VIRIZION", "Route2"), ("KELDEO", "Route22")]),
    ("Espíritus de Alola", CHAPTER_KANTO, B[4], 42, "ITEM_ULTRA_BALL", 5,
     "Los cuatro Tapu custodian las rutas\n12, 13, 14 y 15 al sur de Lavanda.\nAtrápalos a todos.",
     [("TAPU_KOKO", "Route12"), ("TAPU_LELE", "Route13"), ("TAPU_BULU", "Route14"), ("TAPU_FINI", "Route15")]),
    ("Fuerzas de la naturaleza", CHAPTER_KANTO, B[5], 45, "ITEM_ULTRA_BALL", 5,
     "Tornadus, Thundurus, Landorus y\nEnamorus agitan las rutas 16, 17,\n18 y 7. Calma las tormentas.",
     [("TORNADUS", "Route16"), ("THUNDURUS", "Route17"), ("LANDORUS", "Route18"), ("ENAMORUS", "Route7")]),
    ("Bestias legendarias", CHAPTER_KANTO, B[6], 50, "ITEM_RARE_CANDY", 3,
     "Raikou, Entei y Suicune descansan\nen la Central, la Mansión Pokémon\ny las Islas Espuma.",
     [("RAIKOU", "PowerPlant"), ("ENTEI", "PokemonMansion_1F"), ("SUICUNE", "SeafoamIslands_1F")]),
    ("Tesoros funestos", CHAPTER_KANTO, B[6], 50, "ITEM_RARE_CANDY", 3,
     "Cuatro reliquias malditas despiertan\nen las rutas 8 y 10, la Cueva\nDiglett y el sótano de la Mansión.",
     [("WO_CHIEN", "Route8"), ("CHIEN_PAO", "Route10"), ("TING_LU", "DiglettsCave_B1F"), ("CHI_YU", "PokemonMansion_B1F")]),
    ("Gólems legendarios", CHAPTER_KANTO, B[7], 55, "ITEM_PP_UP", 3,
     "Regirock, Regice, Registeel, Regieleki\ny Regidrago duermen en cuevas de\nKanto y de las Islas Sete.",
     [("REGIROCK", "RockTunnel_B1F"), ("REGICE", "FourIsland_IcefallCave_B1F"), ("REGISTEEL", "MtEmber_RubyPath_B1F"),
      ("REGIELEKI", "PowerPlant"), ("REGIDRAGO", "MtEmber_Exterior")]),
    ("Dragones eón", CHAPTER_KANTO, B[7], 55, "ITEM_PP_UP", 3,
     "Latias y Latios sobrevuelan la Playa\nTesoro de Isla Prima y el Cabo\nKabo de Isla Secunda.",
     [("LATIAS", "OneIsland_TreasureBeach"), ("LATIOS", "TwoIsland_CapeBrink")]),
    ("Sueños y pesadillas", CHAPTER_KANTO, B[8], 60, "ITEM_PP_MAX", 2,
     "Cresselia y Darkrai se enfrentan en\nel Puente Unión y el Bosque Baya\nde Isla Tera.",
     [("CRESSELIA", "ThreeIsland_BondBridge"), ("DARKRAI", "ThreeIsland_BerryForest")]),
    ("Corazón volcánico", CHAPTER_KANTO, B[8], 60, "ITEM_PP_MAX", 2,
     "Heatran y Volcanion habitan el Monte\nAscuas y la Senda Ígnea de\nIsla Prima.",
     [("HEATRAN", "MtEmber_RubyPath_1F"), ("VOLCANION", "OneIsland_KindleRoad")]),
    ("Colosos del clima", CHAPTER_WORLD, CLEAR, 70, "ITEM_MASTER_BALL", 1,
     "Kyogre, Groudon y Rayquaza chocan\nen el Camino Agua, el Cañón Sete\ny la Isla Remota.",
     [("KYOGRE", "SixIsland_WaterPath"), ("GROUDON", "SevenIsland_SevaultCanyon"), ("RAYQUAZA", "SixIsland_OutcastIsland")]),
    ("Rey de los gólems", CHAPTER_WORLD, CLEAR, 70, "ITEM_PP_MAX", 2,
     "Regigigas espera en el Valle Ruinas\nde Isla Sexta. Dicen que solo\ndespierta ante un gran entrenador.",
     [("REGIGIGAS", "SixIsland_RuinValley")]),
    ("La creación", CHAPTER_WORLD, CLEAR, 70, "ITEM_MASTER_BALL", 1,
     "Dialga, Palkia y Giratina deforman\nel tiempo y el espacio dentro\nde la Cueva Celeste.",
     [("DIALGA", "CeruleanCave_1F"), ("PALKIA", "CeruleanCave_1F"), ("GIRATINA", "CeruleanCave_B1F")]),
    ("Ideales y verdades", CHAPTER_WORLD, CLEAR, 70, "ITEM_MASTER_BALL", 1,
     "Reshiram, Zekrom y Kyurem aguardan\nen la Cueva Perdida, la Senda\nVerde y la Cueva Helada.",
     [("RESHIRAM", "FiveIsland_LostCave_Entrance"), ("ZEKROM", "SixIsland_GreenPath"), ("KYUREM", "FourIsland_IcefallCave_1F")]),
    ("Vida, muerte y orden", CHAPTER_WORLD, CLEAR, 70, "ITEM_PP_MAX", 3,
     "Xerneas, Yveltal y Zygarde vigilan\nel Bosque Baya, el Pilar Memoria\ny el Bosque Bordado.",
     [("XERNEAS", "ThreeIsland_BerryForest"), ("YVELTAL", "FiveIsland_MemorialPillar"), ("ZYGARDE", "SixIsland_PatternBush")]),
    ("El sol y la luna", CHAPTER_WORLD, CLEAR, 65, "ITEM_PP_MAX", 3,
     "Cosmog, Type: Null, Solgaleo, Lunala\ny Necrozma brillan en las Islas\nQuinta, Sete y la Cueva Tecno.",
     [("COSMOG", "FiveIsland_Meadow"), ("TYPE_NULL", "FiveIsland_Meadow"), ("SOLGALEO", "SevenIsland_SevaultCanyon_Entrance"),
      ("LUNALA", "SevenIsland_TanobyRuins"), ("NECROZMA", "SixIsland_AlteringCave")]),
    ("Héroes de Galar", CHAPTER_WORLD, CLEAR, 70, "ITEM_MASTER_BALL", 1,
     "Zacian, Zamazenta, Eternatus y Kubfu\nesperan en las rutas 23, 22 y 21\ny en la Calle Victoria.",
     [("ZACIAN", "Route23"), ("ZAMAZENTA", "Route22"), ("ETERNATUS", "VictoryRoad_3F"), ("KUBFU", "Route21_North")]),
    ("La corona de Galar", CHAPTER_WORLD, CLEAR, 70, "ITEM_PP_MAX", 3,
     "Calyrex, Glastrier y Spectrier cabalgan\npor el Paraíso Rosado, la Cueva\nHelada y la ruta 8.",
     [("CALYREX", "FiveIsland_ResortGorgeous"), ("GLASTRIER", "FourIsland_IcefallCave_Entrance"), ("SPECTRIER", "Route8")]),
    ("Leyendas de Paldea", CHAPTER_WORLD, CLEAR, 70, "ITEM_MASTER_BALL", 1,
     "Koraidon, Miraidon y Terapagos\nrecorren las rutas 11 y 6 y la\nentrada del Cañón Sete.",
     [("KORAIDON", "Route11"), ("MIRAIDON", "Route6"), ("TERAPAGOS", "SevenIsland_SevaultCanyon_Entrance")]),
    ("Los tres leales", CHAPTER_WORLD, CLEAR, 70, "ITEM_PP_MAX", 3,
     "Okidogi, Munkidori, Fezandipiti y\nOgerpon causan alboroto en las\nrutas 5, 7, 6 y 25.",
     [("OKIDOGI", "Route5"), ("MUNKIDORI", "Route7"), ("FEZANDIPITI", "Route6"), ("OGERPON", "Route25")]),
    ("Guardianes del cielo", CHAPTER_WORLD, CLEAR, 70, "ITEM_MASTER_BALL", 1,
     "Lugia, Ho-Oh y Deoxys bajaron a las\nIslas Espuma, al Monte Ascuas y a\nlas Ruinas Sete.",
     [("LUGIA", "SeafoamIslands_B2F"), ("HO_OH", "MtEmber_Exterior"), ("DEOXYS", "SevenIsland_TanobyRuins")]),
    ("Singulares de Kanto", CHAPTER_WORLD, CLEAR, 65, "ITEM_PP_MAX", 3,
     "Mew, Celebi, Jirachi, Zarude y Arceus\nse esconden en la Mansión, el Bosque\nVerde, Isla Quinta y la Meseta.",
     [("MEW", "PokemonMansion_B1F"), ("CELEBI", "ViridianForest"), ("JIRACHI", "FiveIsland_Meadow"),
      ("ZARUDE", "ViridianForest"), ("ARCEUS", "IndigoPlateau_Exterior")]),
    ("Singulares del mar", CHAPTER_WORLD, CLEAR, 65, "ITEM_PP_MAX", 3,
     "Manaphy, Phione, Shaymin y Victini\njuegan en el Paraíso Rosado, la\nPlaya Tesoro, el Bosque Baya y ruta 11.",
     [("MANAPHY", "FiveIsland_ResortGorgeous"), ("PHIONE", "OneIsland_TreasureBeach"),
      ("SHAYMIN", "ThreeIsland_BerryForest"), ("VICTINI", "Route11")]),
    ("Singulares lejanos", CHAPTER_WORLD, CLEAR, 65, "ITEM_PP_MAX", 3,
     "Genesect, Meloetta, Diancie, Hoopa\ny otros singulares aparecen por las\nrutas y cuevas de Kanto.",
     [("GENESECT", "Route10"), ("MELOETTA", "Route16"), ("DIANCIE", "DiglettsCave_B1F"), ("HOOPA", "Route23"),
      ("MAGEARNA", "PowerPlant"), ("MARSHADOW", "PokemonTower_1F"), ("ZERAORA", "Route10"),
      ("MELTAN", "Route6"), ("MELMETAL", "Route6"), ("PECHARUNT", "Route8")]),
    ("Grieta Ultra I", CHAPTER_BEYOND, CLEAR, 65, "ITEM_BEAST_BALL", 5,
     "Nihilego, Buzzwole, Pheromosa,\nXurkitree y Celesteela cruzaron a la\nZona Safari y las rutas 21 y 23.",
     [("NIHILEGO", "Route21_South"), ("BUZZWOLE", "SafariZone_East"), ("PHEROMOSA", "SafariZone_North"),
      ("XURKITREE", "SafariZone_West"), ("CELESTEELA", "Route23")]),
    ("Grieta Ultra II", CHAPTER_BEYOND, CLEAR, 65, "ITEM_BEAST_BALL", 5,
     "Kartana, Guzzlord, Stakataka,\nBlacephalon y Poipole rondan por las\nrutas 14, 16, 18, 10 y 15.",
     [("KARTANA", "Route14"), ("GUZZLORD", "Route16"), ("STAKATAKA", "Route18"), ("BLACEPHALON", "Route10"), ("POIPOLE", "Route15")]),
    ("Ecos del pasado", CHAPTER_BEYOND, CLEAR, 70, "ITEM_RARE_CANDY", 5,
     "Diez Pokémon paradoja de aspecto\nprimitivo vagan por Monte Ascuas\ny las Islas Sete.",
     [("GREAT_TUSK", "MtEmber_Exterior"), ("SCREAM_TAIL", "FiveIsland_Meadow"), ("BRUTE_BONNET", "ThreeIsland_BerryForest"),
      ("FLUTTER_MANE", "PokemonMansion_1F"), ("SLITHER_WING", "OneIsland_KindleRoad"), ("SANDY_SHOCKS", "SevenIsland_SevaultCanyon"),
      ("ROARING_MOON", "SixIsland_RuinValley"), ("WALKING_WAKE", "SixIsland_GreenPath"), ("GOUGING_FIRE", "MtEmber_Exterior"),
      ("RAGING_BOLT", "SixIsland_OutcastIsland")]),
    ("Ecos del futuro", CHAPTER_BEYOND, CLEAR, 70, "ITEM_RARE_CANDY", 5,
     "Diez Pokémon paradoja de hierro\naparecieron en rutas, cuevas y la\nCentral de Kanto.",
     [("IRON_TREADS", "Route3"), ("IRON_BUNDLE", "FourIsland_IcefallCave_B1F"), ("IRON_HANDS", "PowerPlant"),
      ("IRON_JUGULIS", "Route17"), ("IRON_MOTH", "Route8"), ("IRON_THORNS", "RockTunnel_1F"), ("IRON_VALIANT", "Route7"),
      ("IRON_LEAVES", "Route12"), ("IRON_BOULDER", "Route4"), ("IRON_CROWN", "Route24")]),
]
CHAPTER_NAMES = ["LEYENDAS DE KANTO", "LEYENDAS DEL MUNDO", "ULTRAENTES Y PARADOJAS"]

GOOD_BEHAVIORS = {"MB_NORMAL", "MB_TALL_GRASS", "MB_SHORT_GRASS", "MB_CAVE", "MB_SAND", "MB_ASHGRASS",
                  "MB_MOUNTAIN_TOP", "MB_FOOTPRINTS", "MB_INDOOR_ENCOUNTER", "MB_PUDDLE", "MB_FLOWER"}
LEDGES = {"MB_JUMP_EAST", "MB_JUMP_WEST", "MB_JUMP_NORTH", "MB_JUMP_SOUTH"}
WALKABLE = GOOD_BEHAVIORS | LEDGES | {"MB_LONG_GRASS", "MB_DEEP_SAND", "MB_THIN_ICE", "MB_CRACKED_ICE",
                                      "MB_ICE", "MB_SECRET_BASE_WALL", "MB_NO_RUNNING", "MB_STAIRS_OUTSIDE_ABANDONED_SHIP"}


def behaviors():
    text = (ROOT / "include/constants/metatile_behaviors.h").read_text(encoding="utf-8")
    body = text[text.index("enum {"):text.index("};")]
    return re.findall(r"^\s*(MB_\w+)", body, re.M)


def tileset_attribute_files():
    headers = (ROOT / "src/data/tilesets/headers.h").read_text(encoding="utf-8")
    metatiles = (ROOT / "src/data/tilesets/metatiles.h").read_text(encoding="utf-8")
    files = dict(re.findall(r"(gMetatileAttributes_\w+)\[\] = INCBIN_U16\(\"([^\"]+)\"\)", metatiles))
    result = {}
    for name, body in re.findall(r"const struct Tileset (gTileset_\w+) =\s*\{(.*?)\};", headers, re.S):
        attr = re.search(r"\.metatileAttributes = (gMetatileAttributes_\w+)", body)
        if attr and attr.group(1) in files:
            result[name] = ROOT / files[attr.group(1)]
    return result


class MapGrid:
    def __init__(self, folder, layouts, attr_files, mb_names):
        self.folder = folder
        self.data = json.loads((MAPS / folder / "map.json").read_text(encoding="utf-8"))
        layout = layouts[self.data["layout"]]
        self.w, self.h = layout["width"], layout["height"]
        raw = (ROOT / layout["blockdata_filepath"]).read_bytes()
        self.blocks = [raw[i] | raw[i + 1] << 8 for i in range(0, len(raw), 2)]
        prim = attr_files[layout["primary_tileset"]].read_bytes()
        sec = attr_files[layout["secondary_tileset"]].read_bytes()
        self.prim = [prim[i] | prim[i + 1] << 8 for i in range(0, len(prim), 2)]
        self.sec = [sec[i] | sec[i + 1] << 8 for i in range(0, len(sec), 2)]
        self.mb_names = mb_names

    def behavior(self, x, y):
        mid = self.blocks[y * self.w + x] & 0x3FF
        attrs = self.prim if mid < 640 else self.sec
        index = mid if mid < 640 else mid - 640
        value = attrs[index] if index < len(attrs) else 0
        number = value & 0xFF
        return self.mb_names[number] if number < len(self.mb_names) else "?"

    def collision(self, x, y):
        return (self.blocks[y * self.w + x] >> 10) & 3

    def elevation(self, x, y):
        return self.blocks[y * self.w + x] >> 12

    def walkable(self, x, y):
        return 0 <= x < self.w and 0 <= y < self.h and self.collision(x, y) == 0 and self.behavior(x, y) in WALKABLE

    def find_spots(self, count, own_scripts):
        events = self.data
        blocked = set()
        for obj in events.get("object_events", []):
            if obj.get("script", "").startswith(SCRIPT_PREFIX):
                continue
            blocked.add((obj["x"], obj["y"]))
        warps = [(w["x"], w["y"]) for w in events.get("warp_events", [])]
        others = [(e["x"], e["y"]) for key in ("coord_events", "bg_events") for e in events.get(key, [])]
        # Entry points: warps and walkable border tiles on connected sides
        starts = [p for p in warps]
        for conn in events.get("connections") or []:
            d = conn["direction"]
            for i in range(self.w if d in ("up", "down") else self.h):
                p = {"up": (i, 0), "down": (i, self.h - 1), "left": (0, i), "right": (self.w - 1, i)}.get(d)
                if p and self.walkable(*p):
                    starts.append(p)
        dist = {}
        queue = deque()
        for p in starts:
            dist[p] = 0
            queue.append(p)
        while queue:
            x, y = queue.popleft()
            for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
                if (nx, ny) in dist or (nx, ny) in blocked or not self.walkable(nx, ny):
                    continue
                e1, e2 = self.elevation(x, y), self.elevation(nx, ny)
                if e1 != e2 and e1 not in (0, 15) and e2 not in (0, 15):
                    continue
                dist[(nx, ny)] = dist[(x, y)] + 1
                queue.append((nx, ny))
        # Prefer fully open spots; narrow caves fall back to tiles with walkable sides
        # (a legend there still leaves a way around it).
        neighbourhoods = (
            [(dx, dy) for dx in (-1, 0, 1) for dy in (-1, 0, 1)],
            [(0, 0), (1, 0), (-1, 0), (0, 1), (0, -1)],
        )
        candidates = []
        for around in neighbourhoods:
            for (x, y), d in dist.items():
                if self.behavior(x, y) not in GOOD_BEHAVIORS or not 2 <= x < self.w - 2 or not 2 <= y < self.h - 2:
                    continue
                if not all(self.walkable(x + dx, y + dy) and (x + dx, y + dy) not in blocked for dx, dy in around):
                    continue
                if any(abs(x - a) + abs(y - b) < 4 for a, b in warps + others) or any(abs(x - a) + abs(y - b) < 3 for a, b in blocked):
                    continue
                candidates.append((d, x, y))
            if len(candidates) >= count:
                break
        if not candidates:
            raise SystemExit(f"{self.folder}: no open reachable spot")
        candidates.sort()
        chosen = []
        # Spread from the farthest open spots, keeping legends apart
        for d, x, y in reversed(candidates[len(candidates) // 3:] or candidates):
            if all(abs(x - a) + abs(y - b) >= 5 for a, b in chosen):
                chosen.append((x, y))
            if len(chosen) == count:
                break
        if len(chosen) < count:
            raise SystemExit(f"{self.folder}: only {len(chosen)} spots for {count} legends")
        return [(x, y, self.elevation(x, y)) for x, y in chosen]


def species_names():
    text = (ROOT / "include/constants/species.h").read_text(encoding="utf-8")
    return set(re.findall(r"^\s*(SPECIES_\w+)\s*=", text, re.M))


def write(path, text):
    path.write_bytes(text.encode("utf-8"))


def main():
    known = species_names()
    layouts = {l["id"]: l for l in json.loads((ROOT / "data/layouts/layouts.json").read_text(encoding="utf-8"))["layouts"] if l}
    attr_files = tileset_attribute_files()
    mb_names = behaviors()

    legends = []  # (index, saga, species, folder)
    for saga, (_, _, _, _, _, _, _, members) in enumerate(SAGAS):
        for species, folder in members:
            name = f"SPECIES_{species}"
            if name not in known:
                raise SystemExit(f"unknown species {name}")
            if not (MAPS / f"{folder}_Frlg/map.json").exists():
                raise SystemExit(f"unknown map {folder}")
            legends.append((len(legends), saga, name, f"{folder}_Frlg"))
    if len(legends) > 128:
        raise SystemExit("RF_LEGEND_CAPACITY is 128")

    # Place objects map by map
    by_map = {}
    for legend in legends:
        by_map.setdefault(legend[3], []).append(legend)
    for path in MAPS.glob("*_Frlg/map.json"):
        data = json.loads(path.read_text(encoding="utf-8"))
        objects = data.get("object_events", [])
        kept = [o for o in objects if not o.get("script", "").startswith(SCRIPT_PREFIX)]
        if len(kept) != len(objects) and path.parent.name not in by_map:
            data["object_events"] = kept
            write(path, json.dumps(data, indent=2, ensure_ascii=False) + "\n")
    for folder, members in sorted(by_map.items()):
        grid = MapGrid(folder, layouts, attr_files, mb_names)
        spots = grid.find_spots(len(members), True)
        data = grid.data
        data["object_events"] = [o for o in data.get("object_events", []) if not o.get("script", "").startswith(SCRIPT_PREFIX)]
        for (index, saga, species, _), (x, y, elevation) in zip(members, spots):
            data["object_events"].append({
                "type": "object",
                "graphics_id": f"OBJ_EVENT_GFX_SPECIES({species[8:]})",
                "x": x, "y": y,
                "elevation": elevation if elevation not in (0, 15) else 3,
                "movement_type": "MOVEMENT_TYPE_LOOK_AROUND",
                "movement_range_x": 0, "movement_range_y": 0,
                "trainer_type": "TRAINER_TYPE_NONE",
                "trainer_sight_or_berry_tree_id": "0",
                "script": f"{SCRIPT_PREFIX}{index:03d}",
                "flag": "0",
            })
        if len(data["object_events"]) > 64:
            raise SystemExit(f"{folder}: too many object events")
        write(MAPS / folder / "map.json", json.dumps(data, indent=2, ensure_ascii=False) + "\n")
        for (index, _, species, _), (x, y, _) in zip(members, spots):
            print(f"{index:03d} {species[8:]:<14} {folder} ({x},{y})")

    # Scripts
    lines = ["@ Rojo Fuego+ legendary sagas. Generated by tools/rojo_fuego/legends.py; do not edit.", ""]
    for index, saga, species, _ in legends:
        level = SAGAS[saga][3]
        label = f"{SCRIPT_PREFIX}{index:03d}"
        lines += [
            f"{label}::",
            "\tlock",
            "\tfaceplayer",
            f"\tbufferspeciesname STR_VAR_1, {species}",
            f"\tplaymoncry {species}, CRY_MODE_ENCOUNTER",
            "\tmsgbox RF_Text_LegendAppears",
            "\twaitmoncry",
            f"\tsetwildbattle {species}, {level}",
            "\tdowildbattle",
            "\tspecialvar VAR_RESULT, GetBattleOutcome",
            "\tgoto_if_ne VAR_RESULT, B_OUTCOME_CAUGHT, RF_EventScript_LegendStays",
            f"\tsetvar VAR_0x8004, {index}",
            "\tcallnative Script_RF_LegendCaught",
            "\tremoveobject VAR_LAST_TALKED",
            "\tmsgbox RF_Text_LegendCaught",
            "\trelease",
            "\tend",
            "",
        ]
    lines += [
        "RF_EventScript_LegendStays::",
        "\tmsgbox RF_Text_LegendStays",
        "\trelease",
        "\tend",
        "",
        "RF_Text_LegendAppears::",
        "\t.string \"¡Una presencia imponente!\\n¡{STR_VAR_1} te desafía!$\"",
        "",
        "RF_Text_LegendCaught::",
        "\t.string \"La leyenda queda registrada en tu\\nmisión. ¡Gran captura!$\"",
        "",
        "RF_Text_LegendStays::",
        "\t.string \"La leyenda sigue aquí. Prepárate y\\nvuelve a intentarlo.$\"",
        "",
    ]
    write(ROOT / "data/scripts/rojo_fuego_legends.inc", "\n".join(lines))

    # C tables
    count, sagas = len(legends), len(SAGAS)
    write(ROOT / "include/constants/rf_legends.h", "\n".join([
        "// Generated by tools/rojo_fuego/legends.py. Do not edit by hand.",
        "#ifndef GUARD_CONSTANTS_RF_LEGENDS_H",
        "#define GUARD_CONSTANTS_RF_LEGENDS_H",
        "",
        "// Save bits are indexed by legend ID: only append new legends.",
        "#define RF_LEGEND_CAPACITY 128",
        "#define RF_LEGEND_BYTES (RF_LEGEND_CAPACITY / 8)",
        f"#define RF_LEGEND_COUNT {count}",
        f"#define RF_LEGEND_SAGA_COUNT {sagas}",
        "",
        "#endif",
        "",
    ]))
    c = ["// Generated by tools/rojo_fuego/legends.py. Do not edit by hand.", ""]
    c += [f"extern const u8 {SCRIPT_PREFIX}{i:03d}[];" for i, *_ in legends]
    c += ["", "static const struct RFLegend sRFLegends[RF_LEGEND_COUNT] =", "{"]
    for index, saga, species, _ in legends:
        c.append(f"    {{{species}, RF_QUEST_LEGEND_{saga:02d}, {SCRIPT_PREFIX}{index:03d}}},")
    c += ["};", ""]
    write(ROOT / "src/data/rf_legends.h", "\n".join(c))

    q = ["// Generated by tools/rojo_fuego/legends.py. Included inside sRFQuests[]. Do not edit by hand."]
    for saga, (name, chapter, flag, level, reward, reward_count, description, members) in enumerate(SAGAS):
        if any(len(line) > 40 for line in description.split("\n")) or description.count("\n") > 2:
            raise SystemExit(f"description too long: {name}")
        q += [
            f"    [RF_QUEST_LEGEND_{saga:02d}] = {{",
            f"        .name = COMPOUND_STRING(\"{name}\"),",
            "        .description = COMPOUND_STRING(\"" + description.replace("\n", "\\n") + "\"),",
            f"        .prerequisite = RF_QUEST_NONE, .requiredFlag = {flag}, .condition = QUEST_LEGENDS,",
            f"        .argument = {saga}, .rewardItem = {reward}, .rewardCount = {reward_count}, .chapter = {chapter},",
            "    },",
        ]
    write(ROOT / "src/data/rf_quests_legends.h", "\n".join(q) + "\n")

    constants = ROOT / "include/constants/rf_quests.h"
    text = constants.read_text(encoding="utf-8")
    block = ["// BEGIN legends (tools/rojo_fuego/legends.py)"]
    block += [f"#define RF_QUEST_LEGEND_{s:02d} {FIRST_QUEST + s}" for s in range(sagas)]
    block += [f"#define RF_QUEST_COUNT {FIRST_QUEST + sagas}", "// END legends"]
    if "// BEGIN legends" in text:
        text = re.sub(r"// BEGIN legends.*?// END legends", "\n".join(block), text, flags=re.S)
    else:
        text = text.replace(f"#define RF_QUEST_COUNT {FIRST_QUEST}\n", "\n".join(block) + "\n")
    write(constants, text)
    placed = {}
    for folder, members in by_map.items():
        for obj in json.loads((MAPS / folder / "map.json").read_text(encoding="utf-8"))["object_events"]:
            if obj.get("script", "").startswith(SCRIPT_PREFIX):
                placed[int(obj["script"][len(SCRIPT_PREFIX):])] = (folder[:-5], obj["x"], obj["y"])
    requirement = {CLEAR: "Campeón"} | {flag: f"{n} medallas" for n, flag in B.items()}
    doc = ["# Sagas legendarias", "",
           "Generado por `tools/rojo_fuego/legends.py`. Cada saga es una misión de los capítulos",
           "LEYENDAS DE KANTO, LEYENDAS DEL MUNDO y ULTRAENTES Y PARADOJAS. Al aceptarla, sus",
           "Pokémon aparecen como objetos en el mapa; cada uno es un combate único. Capturarlo lo",
           "guarda en SaveBlock3 (`rfLegendCaught`) y lo retira; si se le derrota o se huye, sigue ahí.",
           "La misión queda lista para reclamar al capturar toda la saga.", "",
           f"{count} Pokémon en {sagas} sagas y {len(by_map)} mapas.", ""]
    for saga, (name, chapter, flag, level, reward, reward_count, _, members) in enumerate(SAGAS):
        doc += [f"## {name}", "",
                f"{CHAPTER_NAMES[chapter - CHAPTER_KANTO]} · requisito: {requirement[flag]} · Nv. {level} · "
                f"premio: {reward_count} × `{reward}`", "", "| Pokémon | Mapa | Casilla |", "|---|---|---|"]
        for index, s, species, _ in legends:
            if s == saga:
                folder, x, y = placed[index]
                doc.append(f"| {species[8:].replace('_', ' ').title()} | {folder} | ({x}, {y}) |")
        doc.append("")
    write(ROOT / "docs/LEYENDAS.md", "\n".join(doc))
    print(f"{count} legends in {sagas} sagas, {len(by_map)} maps")


if __name__ == "__main__":
    main()
