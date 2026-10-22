#!/usr/bin/env python3
"""Rojo Fuego+ map content: Omni Ring upgrades and Mega Stone guardians.

Reuses item balls from the original FireRed maps, so every spot is already reachable and
already has its own flag:
  - UPGRADES: the item ball now holds an Omni Ring module or an Omni Cell.
  - GUARDIANS: the item ball becomes an overworld Pokémon; beating or catching it gives
    a Mega Stone.
  - Z_TRIALS: an NPC who is always on the map offers a two-battle trial (helper, then a boosted
    Totem); passing it gives a Z-Crystal, then the NPC goes back to its usual lines.

Run from the repository root (inside WSL):  python3 tools/rojo_fuego/content.py
It is safe to run again: already converted entries are left alone.
"""
import json
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parents[2]
ITEM_BALL_SCRIPTS = ROOT / "data/scripts/item_ball_scripts_frlg.inc"
GUARDIAN_SCRIPTS = ROOT / "data/scripts/rojo_fuego_guardians.inc"
MAPS = ROOT / "data/maps"

# item ball script -> new item (story order: Mega, Z, Dynamax, Tera; then the cells)
UPGRADES = {
    "MtMoon_B2F_EventScript_ItemRevive": "ITEM_OMNI_MODULE_MEGA",
    "SSAnne_B1F_Room3_EventScript_ItemEther": "ITEM_OMNI_MODULE_Z",
    "SilphCo_4F_EventScript_ItemEscapeRope": "ITEM_OMNI_MODULE_DYNAMAX",
    "PokemonMansion_1F_EventScript_ItemEscapeRope": "ITEM_OMNI_MODULE_TERA",
    "RockTunnel_1F_EventScript_ItemEscapeRope": "ITEM_OMNI_CELL",
    "RocketHideout_B2F_EventScript_ItemXSpeed": "ITEM_OMNI_CELL",
    "PokemonTower_3F_EventScript_ItemEscapeRope": "ITEM_OMNI_CELL",
    "SafariZone_East_EventScript_ItemMaxPotion": "ITEM_OMNI_CELL",
    "PowerPlant_EventScript_ItemElixir": "ITEM_OMNI_CELL",
    "VictoryRoad_2F_EventScript_ItemGuardSpec": "ITEM_OMNI_CELL",
    "OneIsland_KindleRoad_EventScript_ItemEther": "ITEM_OMNI_CELL",
    "SevenIsland_SevaultCanyon_EventScript_ItemMaxElixir": "ITEM_OMNI_CELL",
}

# item ball script -> (species, level, mega stone)
GUARDIANS = {
    "ViridianForest_EventScript_ItemPotion2": ("BEEDRILL", 12, "ITEM_BEEDRILLITE"),
    "MtMoon_1F_EventScript_ItemPotion": ("CLEFAIRY", 14, "ITEM_CLEFABLITE"),
    "Route2_EventScript_ItemEther": ("PIDGEOTTO", 18, "ITEM_PIDGEOTITE"),
    "Route11_EventScript_ItemAwakening": ("WEEPINBELL", 24, "ITEM_VICTREEBELITE"),
    "RockTunnel_B1F_EventScript_ItemRevive": ("ONIX", 26, "ITEM_STEELIXITE"),
    "PokemonTower_4F_EventScript_ItemAwakening": ("HAUNTER", 30, "ITEM_GENGARITE"),
    "SafariZone_West_EventScript_ItemMaxPotion": ("KANGASKHAN", 30, "ITEM_KANGASKHANITE"),
    "SafariZone_North_EventScript_ItemProtein": ("PINSIR", 30, "ITEM_PINSIRITE"),
    "SafariZone_Center_EventScript_ItemNugget": ("SCYTHER", 30, "ITEM_SCIZORITE"),
    "SilphCo_6F_EventScript_ItemXSpecial": ("KADABRA", 36, "ITEM_ALAKAZITE"),
    "SeafoamIslands_1F_EventScript_ItemIceHeal": ("GYARADOS", 40, "ITEM_GYARADOSITE"),
    "SeafoamIslands_B1F_EventScript_ItemRevive": ("SLOWBRO", 40, "ITEM_SLOWBRONITE"),
    "PokemonMansion_3F_EventScript_ItemMaxPotion": ("AERODACTYL", 42, "ITEM_AERODACTYLITE"),
    "OneIsland_KindleRoad_EventScript_ItemCarbos": ("HOUNDOOM", 44, "ITEM_HOUNDOOMINITE"),
    "FourIsland_EventScript_ItemStardust": ("STARMIE", 45, "ITEM_STARMINITE"),
    "ThreeIsland_BerryForest_EventScript_ItemMaxEther": ("HERACROSS", 45, "ITEM_HERACRONITE"),
    "SixIsland_WaterPath_EventScript_ItemElixir": ("DRAGONAIR", 50, "ITEM_DRAGONINITE"),
    "SevenIsland_SevaultCanyon_EventScript_ItemNugget": ("PUPITAR", 52, "ITEM_TYRANITARITE"),
    "ThreeIsland_BerryForest_EventScript_ItemFullHeal": ("VENUSAUR", 55, "ITEM_VENUSAURITE"),
    "MtEmber_Exterior_EventScript_ItemDireHit": ("CHARIZARD", 55, "ITEM_CHARIZARDITE_X"),
    "MtEmber_Exterior_EventScript_ItemUltraBall": ("CHARIZARD", 55, "ITEM_CHARIZARDITE_Y"),
    "FourIsland_IcefallCave_B1F_EventScript_ItemFullRestore": ("BLASTOISE", 55, "ITEM_BLASTOISINITE"),
    "CeruleanCave_2F_EventScript_ItemFullRestore": ("MACHAMP", 62, "ITEM_MEWTWONITE_X"),
    "CeruleanCave_1F_EventScript_ItemFullRestore": ("ALAKAZAM", 62, "ITEM_MEWTWONITE_Y"),
}


Z_TRIAL_SCRIPTS = ROOT / "data/scripts/rojo_fuego_ztrials.inc"

# Z-Crystal trials, one per type, run by an NPC that is always on the map.
# NPC script -> (type name, crystal, required flag, helper species, helper level, Totem species, Totem level)
Z_TRIALS = {
    "PalletTown_EventScript_FatMan": ("NORMAL", "ITEM_NORMALIUM_Z", "FLAG_BADGE01_GET", "RATTATA", 14, "RATICATE", 18),
    "PewterCity_EventScript_Lass": ("ROCA", "ITEM_ROCKIUM_Z", "FLAG_BADGE01_GET", "GEODUDE", 14, "GRAVELER", 18),
    "ViridianCity_EventScript_Boy": ("TIERRA", "ITEM_GROUNDIUM_Z", "FLAG_BADGE02_GET", "DIGLETT", 20, "DUGTRIO", 24),
    "CeruleanCity_EventScript_BaldingMan": ("AGUA", "ITEM_WATERIUM_Z", "FLAG_BADGE02_GET", "POLIWAG", 20, "POLIWHIRL", 24),
    "VermilionCity_EventScript_Sailor": ("ELÉCTRICO", "ITEM_ELECTRIUM_Z", "FLAG_BADGE03_GET", "MAGNEMITE", 24, "MAGNETON", 28),
    "LavenderTown_EventScript_Boy": ("FANTASMA", "ITEM_GHOSTIUM_Z", "FLAG_BADGE04_GET", "GASTLY", 28, "HAUNTER", 32),
    "CeladonCity_EventScript_Woman": ("PLANTA", "ITEM_GRASSIUM_Z", "FLAG_BADGE04_GET", "GLOOM", 28, "VILEPLUME", 32),
    "FuchsiaCity_EventScript_OldMan": ("VENENO", "ITEM_POISONIUM_Z", "FLAG_BADGE05_GET", "KOFFING", 34, "WEEZING", 38),
    "CeladonCity_EventScript_OldMan2": ("PSÍQUICO", "ITEM_PSYCHIUM_Z", "FLAG_BADGE06_GET", "DROWZEE", 36, "HYPNO", 40),
    "CinnabarIsland_EventScript_OldMan": ("FUEGO", "ITEM_FIRIUM_Z", "FLAG_BADGE07_GET", "PONYTA", 40, "RAPIDASH", 44),
    "OneIsland_EventScript_BaldingMan": ("LUCHA", "ITEM_FIGHTINIUM_Z", "FLAG_BADGE07_GET", "MANKEY", 42, "MACHAMP", 46),
    "TwoIsland_EventScript_Woman": ("HADA", "ITEM_FAIRIUM_Z", "FLAG_BADGE07_GET", "CLEFAIRY", 42, "CLEFABLE", 46),
    "ThreeIsland_EventScript_Woman": ("SINIESTRO", "ITEM_DARKINIUM_Z", "FLAG_BADGE07_GET", "HOUNDOUR", 42, "HOUNDOOM", 46),
    "FourIsland_EventScript_FatMan": ("HIELO", "ITEM_ICIUM_Z", "FLAG_IS_CHAMPION", "SNEASEL", 50, "WEAVILE", 54),
    "FiveIsland_EventScript_Fisher": ("VOLADOR", "ITEM_FLYINIUM_Z", "FLAG_IS_CHAMPION", "FEAROW", 50, "DODRIO", 54),
    "SixIsland_EventScript_Boy": ("BICHO", "ITEM_BUGINIUM_Z", "FLAG_IS_CHAMPION", "HERACROSS", 52, "SCIZOR", 56),
    "SevenIsland_EventScript_OldWoman": ("DRAGÓN", "ITEM_DRAGONIUM_Z", "FLAG_IS_CHAMPION", "DRAGONAIR", 55, "DRAGONITE", 59),
    "IndigoPlateau_PokemonCenter_1F_EventScript_GymGuy": ("ACERO", "ITEM_STEELIUM_Z", "FLAG_IS_CHAMPION", "MAGNETON", 58, "STEELIX", 62),
}


# NPCs that run a hand-written script from data/scripts/rojo_fuego_npcs.inc: NPC script -> new script
NPC_SCRIPTS = {
    "CeladonCity_DepartmentStore_5F_EventScript_Sailor": "RF_EventScript_HyperTrainer",
}


def apply_npc_scripts():
    for npc_script, new_script in NPC_SCRIPTS.items():
        for map_json in MAPS.glob("*_Frlg/map.json"):
            data = json.loads(map_json.read_text(encoding="utf-8"))
            objects = [o for o in data.get("object_events", []) if o.get("script") in (npc_script, new_script)]
            if objects:
                for obj in objects:
                    obj["script"] = new_script
                map_json.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
                break
        else:
            raise SystemExit(f"no map object runs {npc_script}")


def z_trial_label(npc_script):
    return "RF_EventScript_ZTrial_" + npc_script.replace("_EventScript_", "_")


def apply_z_trials():
    scripts = ["@ Rojo Fuego+ Z-Crystal trials. Generated by tools/rojo_fuego/content.py; do not edit.\n"]
    for index, (npc_script, trial) in enumerate(Z_TRIALS.items()):
        type_name, crystal, required_flag, helper, helper_level, totem, totem_level = trial
        new_label = z_trial_label(npc_script)
        done_flag = f"FLAG_RF_Z_TRIAL_{index:02d}"

        # Point the NPC at the trial; the trial falls back to the NPC's own script once done
        for map_json in MAPS.glob("*_Frlg/map.json"):
            data = json.loads(map_json.read_text(encoding="utf-8"))
            objects = [o for o in data.get("object_events", []) if o.get("script") in (npc_script, new_label)]
            if objects:
                for obj in objects:
                    obj["script"] = new_label
                map_json.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
                break
        else:
            raise SystemExit(f"no map object runs {npc_script}")

        scripts.append(f"""
{new_label}::
	lock
	faceplayer
	goto_if_set {done_flag}, {new_label}_Done
	goto_if_unset {required_flag}, {new_label}_NotReady
	msgbox {new_label}_Text_Offer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {new_label}_Decline
	msgbox RF_Text_ZTrial_Helper
	closemessage
	setwildbattle SPECIES_{helper}, {helper_level}
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, {new_label}_Totem
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, {new_label}_Totem
	goto {new_label}_Failed

{new_label}_Totem::
	msgbox RF_Text_ZTrial_Totem
	closemessage
	playmoncry SPECIES_{totem}, CRY_MODE_ENCOUNTER
	waitmoncry
	setwildbattle SPECIES_{totem}, {totem_level}
	settotemboost B_POSITION_OPPONENT_LEFT, 1, 1, 1, 1, 1
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, {new_label}_Passed
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, {new_label}_Passed
	goto {new_label}_Failed

{new_label}_Passed::
	setflag {done_flag}
	msgbox RF_Text_ZTrial_Passed
	giveitem {crystal}
	release
	end

{new_label}_Failed::
	msgbox RF_Text_ZTrial_Failed
	release
	end

{new_label}_Decline::
	msgbox RF_Text_ZTrial_Decline
	release
	end

{new_label}_NotReady::
	msgbox RF_Text_ZTrial_NotReady
	release
	end

{new_label}_Done::
	release
	goto {npc_script}

{new_label}_Text_Offer:
	.string "Soy capitán del tipo {type_name}.\\p"
	.string "Si superas mi desafío, te daré\\n"
	.string "un cristal Z. ¿Lo intentas?$"
""")

    scripts.append("""
RF_Text_ZTrial_Helper:
	.string "¡Primero vence a mi ayudante!$"

RF_Text_ZTrial_Totem:
	.string "¡Bien hecho! Ahora llega el\\n"
	.string "Pokémon dominante. ¡Es muy fuerte!$"

RF_Text_ZTrial_Passed:
	.string "¡Has superado el desafío!\\n"
	.string "Toma, te has ganado esto.$"

RF_Text_ZTrial_Failed:
	.string "El desafío ha terminado…\\n"
	.string "Vuelve cuando estés preparado.$"

RF_Text_ZTrial_Decline:
	.string "Aquí estaré cuando quieras.$"

RF_Text_ZTrial_NotReady:
	.string "Mi desafío es para entrenadores\\n"
	.string "más fuertes. ¡Vuelve más adelante!$"
""")
    Z_TRIAL_SCRIPTS.write_text("".join(scripts), encoding="utf-8")


def apply_upgrades():
    text = ITEM_BALL_SCRIPTS.read_text(encoding="utf-8")
    for label, item in UPGRADES.items():
        pattern = re.compile(rf"(^{label}::\n\tfinditem )ITEM_\w+", re.M)
        text, count = pattern.subn(rf"\g<1>{item}", text)
        if count != 1:
            raise SystemExit(f"item ball script not found: {label}")
    ITEM_BALL_SCRIPTS.write_text(text, encoding="utf-8")


def find_object(label):
    """Returns (map.json path, parsed map, object dict) for the object that runs this script."""
    for map_json in MAPS.glob("*_Frlg/map.json"):
        data = json.loads(map_json.read_text(encoding="utf-8"))
        for obj in data.get("object_events", []):
            if obj.get("script") in (label, guardian_label(label)):
                return map_json, data, obj
    raise SystemExit(f"no map object runs {label}")


def guardian_label(label):
    return "RF_EventScript_Guardian_" + label.replace("_EventScript_Item", "_")


def apply_guardians():
    scripts = ["@ Rojo Fuego+ Mega Stone guardians. Generated by tools/rojo_fuego/content.py; do not edit.\n"]
    for label, (species, level, stone) in GUARDIANS.items():
        map_json, data, obj = find_object(label)
        new_label = guardian_label(label)
        obj["graphics_id"] = f"OBJ_EVENT_GFX_SPECIES({species})"
        obj["script"] = new_label
        flag = obj["flag"]
        map_json.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

        scripts.append(f"""
{new_label}::
	lock
	faceplayer
	playmoncry SPECIES_{species}, CRY_MODE_ENCOUNTER
	msgbox RF_Text_GuardianAppears
	waitmoncry
	setwildbattle SPECIES_{species}, {level}
	settotemboost B_POSITION_OPPONENT_LEFT, 0, 1, 0, 0, 1
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, {new_label}_Reward
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, {new_label}_Reward
	release
	end

{new_label}_Reward::
	setflag {flag}
	removeobject VAR_LAST_TALKED
	msgbox RF_Text_GuardianDroppedStone
	giveitem {stone}
	release
	end
""")

    scripts.append("""
RF_Text_GuardianAppears:
	.string "¡Este Pokémon protege algo\\n"
	.string "que brilla! Se prepara para luchar…$"

RF_Text_GuardianDroppedStone:
	.string "¡El Pokémon dejó caer una piedra\\n"
	.string "que brillaba con fuerza!$"
""")
    GUARDIAN_SCRIPTS.write_text("".join(scripts), encoding="utf-8")


if __name__ == "__main__":
    apply_upgrades()
    apply_guardians()
    apply_z_trials()
    apply_npc_scripts()
    print(f"{len(UPGRADES)} upgrades, {len(GUARDIANS)} guardians, {len(Z_TRIALS)} Z trials")
