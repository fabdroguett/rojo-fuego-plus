#!/usr/bin/env python3
"""Rojo Fuego+ wild encounters: every Pokémon from later generations, catchable in Kanto.

The original FireRed/LeafGreen tables stay as they are and keep running in the morning and
during the day (they are the time-of-day fallback). This script adds an Evening and a Night table
for every land and water table, filled with base forms from Generations 2-9 (regional forms
included, legendaries and Mythicals left out):
  - each species is placed where its types fit the area (caves get Rock/Ground/Steel/..., the
    Pokémon Tower gets Ghosts, water tables get Water/Ice, ...),
  - strong families only show up in high level areas,
  - every eligible species is placed at least once; the remaining slots repeat species so that
    many of them can be found in more than one place.
Slot levels are copied from the original table, so the level curve of the game does not change.

Run from the repository root (inside WSL):  python3 tools/rojo_fuego/encounters.py
It rebuilds the Evening/Night tables from scratch every time.
"""
import json
import pathlib
import random
import re
from collections import defaultdict

ROOT = pathlib.Path(__file__).resolve().parents[2]
ENCOUNTERS = ROOT / "src/data/wild_encounters.json"
SPECIES_INFO = sorted((ROOT / "src/data/pokemon/species_info").glob("gen_*_families.h"))
TIMES = ("Evening", "Night")
NIGHT_TYPES = {"DARK", "GHOST", "PSYCHIC", "FAIRY", "POISON"}

HABITAT_TYPES = {
    "GRASS": {"NORMAL", "FLYING", "GRASS", "BUG", "ELECTRIC", "FAIRY", "POISON", "GROUND", "FIGHTING", "PSYCHIC", "DARK"},
    "FOREST": {"BUG", "GRASS", "POISON", "FAIRY", "DARK", "GHOST"},
    "CAVE": {"ROCK", "GROUND", "STEEL", "DARK", "GHOST", "POISON", "DRAGON", "FIGHTING"},
    "ICE": {"ICE", "WATER", "ROCK"},
    "TOWER": {"GHOST", "DARK", "PSYCHIC"},
    "POWER_PLANT": {"ELECTRIC", "STEEL"},
    "MANSION": {"FIRE", "POISON", "PSYCHIC", "GHOST"},
    "VOLCANO": {"FIRE", "ROCK", "GROUND", "FIGHTING", "DRAGON"},
    "SAFARI": {"NORMAL", "GRASS", "GROUND", "BUG", "FAIRY", "FIGHTING", "POISON", "ROCK"},
    "ROCKY": {"ROCK", "GROUND", "FIGHTING", "DRAGON", "STEEL", "FLYING"},
    "WATER": {"WATER", "ICE"},
}

# Areas whose tables are special in the original game and stay untouched
SKIP_LABELS = ("TanobyRuins", "AlteringCave")


def land_habitat(label):
    rules = [
        (("Forest", "PatternBush"), "FOREST"),
        (("IcefallCave", "Seafoam"), "ICE"),
        (("PokemonTower",), "TOWER"),
        (("PowerPlant",), "POWER_PLANT"),
        (("PokemonMansion",), "MANSION"),
        (("MtEmberExterior", "KindleRoad"), "VOLCANO"),
        (("SafariZone",), "SAFARI"),
        (("RuinValley", "SevaultCanyon", "CapeBrink"), "ROCKY"),
        (("Cave", "MtMoon", "RockTunnel", "VictoryRoad", "RubyPath", "SummitPath"), "CAVE"),
    ]
    for keys, habitat in rules:
        if any(key in label for key in keys):
            return habitat
    return "GRASS"


def parse_defines(texts):
    """First definition of every object-like #define (the updated-types branch comes first)."""
    defines = {}
    for text in texts:
        for name, value in re.findall(r"^\s*#define (\w+)\s+(.+?)\s*$", text, re.M):
            defines.setdefault(name, value)
    return defines


def resolve(token, defines):
    for _ in range(10):
        token = token.strip()
        if token not in defines:
            break
        token = defines[token]
    return token


def parse_types(expr, defines):
    """Handles MON_TYPES(...), { TYPE_A, TYPE_B } and (P_UPDATED_TYPES >= GEN_6 ? TYPE_A : TYPE_B)."""
    expr = resolve(expr, defines)
    m = re.match(r"MON_TYPES\((.*)\)$", expr) or re.match(r"\{(.*)\}$", expr)
    parts = m.group(1).split(",") if m else [expr]
    types = set()
    for part in parts:
        part = resolve(part, defines)
        if "?" in part:  # updated types are the first branch
            part = part.split("?", 1)[1].split(":", 1)[0]
        types |= {t.replace("TYPE_", "") for t in re.findall(r"TYPE_\w+", part)}
    return types


def parse_species():
    """Returns {species: info} for every species entry, plus the set of evolution targets."""
    species = {}
    evolution_targets = set()
    entry_re = re.compile(r"^    \[(SPECIES_\w+)\] =\n    \{\n(.*?)^    \},", re.M | re.S)
    texts = [path.read_text(encoding="utf-8") for path in SPECIES_INFO]
    defines = parse_defines(texts)
    for text in texts:
        for name, body in entry_re.findall(text):
            def stat(field):
                m = re.search(rf"\.{field}\s*=\s*(\d+)", body)
                return int(m.group(1)) if m else 0
            types = re.search(r"\.types = (.+?),?\s*$", body, re.M)
            type_list = parse_types(types.group(1), defines) if types else set()
            # The evolutions field runs until the next field; #if/#else variants in between count too
            targets = set()
            start = body.find(".evolutions")
            if start >= 0:
                segment = body[start:]
                next_field = re.search(r"\n        \.", segment)
                targets = set(re.findall(r"SPECIES_\w+", segment[:next_field.start()] if next_field else segment))
            evolution_targets |= targets
            dex = re.search(r"\.natDexNum = NATIONAL_DEX_(\w+)", body)
            flags = set(re.findall(r"\.(is\w+) = TRUE", body))
            species[name] = {
                "types": set(type_list),
                "bst": sum(stat(f) for f in ("baseHP", "baseAttack", "baseDefense", "baseSpeed", "baseSpAttack", "baseSpDefense")),
                "evolves_to": targets,
                "dex": dex.group(1) if dex else None,
                "flags": flags,
            }
    return species, evolution_targets


def national_dex_numbers():
    text = (ROOT / "include/constants/pokedex.h").read_text(encoding="utf-8")
    numbers = {}
    for name, value in re.findall(r"^\s*NATIONAL_DEX_(\w+)\s*(?:=\s*(\d+))?,", text, re.M):
        numbers[name] = int(value) if value else len(numbers) + 1
    return numbers


def eligible_species(species, evolution_targets, already_wild):
    dex_numbers = national_dex_numbers()
    excluded = {"isSubLegendary", "isRestrictedLegendary", "isMythical", "isUltraBeast", "isParadox",
                "isMegaEvolution", "isGigantamax", "isTotem", "isPrimalReversion", "isUltraBurst", "isTeraForm"}
    regional = {"isAlolanForm", "isGalarianForm", "isHisuianForm", "isPaldeanForm"}
    result = []
    for name, info in species.items():
        if info["dex"] is None or info["flags"] & excluded or name in evolution_targets:
            continue
        is_main_form = name == f"SPECIES_{info['dex']}"
        is_regional = bool(info["flags"] & regional)
        if not (is_main_form or is_regional):
            continue
        # Generation 1 main forms that are already wild in FireRed stay in their original tables
        if dex_numbers.get(info["dex"], 0) <= 151 and not is_regional and name in already_wild:
            continue
        result.append(name)
    return sorted(result)


def family_max_bst(name, species, seen=None):
    seen = seen or set()
    if name in seen or name not in species:
        return 0
    seen.add(name)
    info = species[name]
    return max([info["bst"]] + [family_max_bst(t, species, seen) for t in info["evolves_to"]])


def min_area_level(name, species):
    info = species[name]
    family_bst = family_max_bst(name, species)
    if family_bst >= 580 and info["bst"] < 360:
        return 40  # pseudo-legendary lines (and Slakoth)
    if info["bst"] >= 450:
        return 30
    if family_bst >= 525 and info["bst"] <= 330:
        return 20  # starters and other strong three-stage lines
    if info["bst"] >= 380:
        return 18
    return 0


def main():
    random.seed(1996)
    species, evolution_targets = parse_species()

    data = json.loads(ENCOUNTERS.read_text(encoding="utf-8"))
    group = next(g for g in data["wild_encounter_groups"] if g["label"] == "gWildMonHeaders")
    # Rebuild from scratch: drop tables made by an earlier run
    group["encounters"] = [e for e in group["encounters"]
                           if not any(e["base_label"].endswith("_" + t) for t in TIMES)]

    already_wild = set()
    for entry in group["encounters"]:
        if entry["base_label"].endswith("_FireRed"):
            for area in ("land_mons", "water_mons", "rock_smash_mons", "fishing_mons"):
                already_wild |= {m["species"] for m in entry.get(area, {}).get("mons", [])}
    pool = eligible_species(species, evolution_targets, already_wild)

    # Tables to fill: (FireRed entry, area, time) -> list of slot species
    tables = []
    for entry in group["encounters"]:
        label = entry["base_label"]
        if not label.endswith("_FireRed") or any(s in label for s in SKIP_LABELS):
            continue
        for area in ("land_mons", "water_mons"):
            if area not in entry:
                continue
            mons = entry[area]["mons"]
            avg_level = sum((m["min_level"] + m["max_level"]) / 2 for m in mons) / len(mons)
            habitat = "WATER" if area == "water_mons" else land_habitat(label)
            for time in TIMES:
                tables.append({"entry": entry, "area": area, "time": time, "habitat": habitat,
                               "level": avg_level, "slots": [None] * len(mons)})

    def fits(name, table):
        types = species[name]["types"]
        if not types & HABITAT_TYPES[table["habitat"]]:
            return False
        if table["habitat"] != "WATER" and types <= {"WATER", "ICE"} and table["habitat"] != "ICE":
            return False  # pure water Pokémon stay in the water
        return table["level"] >= min_area_level(name, species)

    def time_fits(name, table):
        night = bool(species[name]["types"] & NIGHT_TYPES)
        return night == (table["time"] == "Night")

    # Pass 1: give every eligible species one slot, most constrained species first
    usage = defaultdict(int)
    unplaced = []
    candidates = {n: [t for t in tables if fits(n, t)] for n in pool}
    for name in sorted(pool, key=lambda n: (len(candidates[n]), n)):
        options = [t for t in candidates[name] if None in t["slots"]]
        preferred = [t for t in options if time_fits(name, t)] or options
        if not preferred:
            unplaced.append(name)
            continue
        table = min(preferred, key=lambda t: (t["slots"].count(None) == 0, -t["slots"].count(None), random.random()))
        strong = min_area_level(name, species) >= 30
        free = [i for i, s in enumerate(table["slots"]) if s is None]
        slot = free[-1] if strong else free[0]  # strong species take the rarest free slot
        table["slots"][slot] = name
        usage[name] += 1

    # Pass 2: fill the rest with fitting species, least used first, for variety
    for table in tables:
        fitting = [n for n in pool if fits(n, table)]
        timed = [n for n in fitting if time_fits(n, table)] or fitting
        for i, slot in enumerate(table["slots"]):
            if slot is not None:
                continue
            if not timed:
                timed = fitting or [s for s in table["slots"] if s] or ["SPECIES_RATTATA"]
            choice = min(timed, key=lambda n: (usage[n], random.random()))
            table["slots"][i] = choice
            usage[choice] += 1

    # Write the new tables right after their FireRed entry, mirrored for LeafGreen
    by_label = defaultdict(dict)
    for table in tables:
        by_label[table["entry"]["base_label"]].setdefault(table["time"], {})[table["area"]] = table

    new_encounters = []
    entries = group["encounters"]
    for entry in entries:
        new_encounters.append(entry)
        label = entry["base_label"]
        base = label.replace("_LeafGreen", "_FireRed")
        if base not in by_label:
            continue
        for time in TIMES:
            new_entry = {"map": entry["map"], "base_label": f"{label}_{time}"}
            for area, table in by_label[base][time].items():
                source = entry[area]
                new_entry[area] = {
                    "encounter_rate": source["encounter_rate"],
                    "mons": [{"min_level": m["min_level"], "max_level": m["max_level"], "species": s}
                             for m, s in zip(source["mons"], table["slots"])],
                }
            new_encounters.append(new_entry)
    group["encounters"] = new_encounters

    ENCOUNTERS.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    placed = sum(1 for n in pool if usage[n] > 0)
    print(f"{len(pool)} eligible species, {placed} placed, {len(tables)} new tables")
    if unplaced:
        print("Not placed (no fitting area):", " ".join(unplaced))


if __name__ == "__main__":
    main()
