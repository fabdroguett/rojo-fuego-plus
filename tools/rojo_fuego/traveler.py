#!/usr/bin/env python3
"""Rojo Fuego+ traveling challenger (daily challenges, from Crystal Clear).

Places Noa in the Pokémon Center of seven Kanto cities. Each copy has its own hide flag;
src/rf_traveler.c shows only today's one on every map load. The battle and its rules live in
data/scripts/rojo_fuego_npcs.inc (RF_EventScript_Traveler).

    python3 tools/rojo_fuego/traveler.py
edits the map.json files (only once: a map that already has her is left as it is).
"""
import json
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
MAPS = ROOT / "data/maps"

# Same order as sCityNames in src/rf_traveler.c
CITIES = ["ViridianCity", "PewterCity", "CeruleanCity", "VermilionCity",
          "CeladonCity", "FuchsiaCity", "SaffronCity"]

# A tile left free by every Pokémon Center 1F (LAYOUT_POKEMON_CENTER_1F_FRLG)
X, Y = 10, 4
SCRIPT = "RF_EventScript_Traveler"


def main():
    for i, city in enumerate(CITIES):
        path = MAPS / f"{city}_PokemonCenter_1F_Frlg" / "map.json"
        data = json.loads(path.read_text(encoding="utf-8"))
        objects = data["object_events"]
        if any(o.get("script") == SCRIPT for o in objects):
            continue
        if any(o.get("x") == X and o.get("y") == Y for o in objects):
            raise SystemExit(f"{path}: tile {X},{Y} is taken")
        objects.append({
            "type": "object",
            "graphics_id": "OBJ_EVENT_GFX_BEAUTY",
            "x": X,
            "y": Y,
            "elevation": 3,
            "movement_type": "MOVEMENT_TYPE_FACE_DOWN",
            "movement_range_x": 1,
            "movement_range_y": 1,
            "trainer_type": "TRAINER_TYPE_NONE",
            "trainer_sight_or_berry_tree_id": "0",
            "script": SCRIPT,
            "flag": f"FLAG_RF_TRAVELER_HIDE_{i}",
        })
        path.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"{len(CITIES)} Pokémon Centers")


if __name__ == "__main__":
    main()
