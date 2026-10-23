"""Capture native frames from an explicitly linked, disposable UI fixture ROM.

Requires running rojo-fuego-check container and /tmp/rf-mgba-harness.
Never use this fixture ROM as a release. See UI_VALIDATION.md.
"""
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[3]
OUT = ROOT / "build_artifacts/ui"
OUT.mkdir(parents=True, exist_ok=True)


def docker(*args, **kwargs):
    return subprocess.run(["docker", *args], check=True, **kwargs)


nm = docker("exec", "-w", "/work", "rojo-fuego-check", "arm-none-eabi-nm",
            "pokefirered.elf", capture_output=True, text=True).stdout
symbols = {p[2]: int(p[0], 16) for line in nm.splitlines() if len(p := line.split()) == 3}
base = "run 600\nmash T 30\nrun 180\nmash A 3900\nmash B 240\nrun 60\n"


def callback(name):
    return f'w32 {symbols["gMain"] + 4:08x} {symbols[name] | 1:08x}\n'


def shot(name):
    return f"shot /tmp/rf-ui-{name}.ppm\n"


cases = {
    "options": ("Options", "run 240\n" + shot("options") + "hold > 1\nrun 30\n" + shot("options-changed") + "hold B 1\nrun 180\n" + shot("options-closed")),
    "trainer": ("Trainer", "run 240\n" + shot("trainer") + "hold A 1\nrun 240\n" + shot("trainer-back") + "hold B 1\nrun 180\n" + shot("trainer-closed")),
    "pokedex": ("Pokedex", "run 240\n" + shot("pokedex")
                + "hold A 1\nrun 240\n" + shot("pokedex-info")),
    "field-party": ("FieldParty", "run 240\n" + shot("field-party")
                    + "hold D 1\nrun 60\n" + shot("field-party-focus")
                    + "hold A 1\nrun 60\n" + shot("field-party-action")
                    + "hold A 1\nrun 240\n" + shot("field-party-summary")
                    + "hold B 1\nrun 240\nhold D 1\nrun 20\nhold A 1\nrun 60\n" + shot("field-party-switch")
                    + "hold D 1\nrun 40\nhold A 1\nrun 240\n" + shot("field-party-reordered")
                    + callback("RFUiFixture_CheckOrder") + "run 1\n" + f'r8 {symbols["gRFUiFixture_OrderOK"]:08x}\n'),
    "start": ("Start", "run 60\n" + shot("start")
              + "hold > 1\nrun 20\n" + shot("start-focus")
              + "hold D 1\nrun 20\n" + shot("start-row")
              + "hold B 1\nrun 30\n" + shot("start-closed")),
    "empty": ("DexNavEmpty", "run 180\n" + shot("empty")),
    "unknown": ("DexNavUnknown", "run 180\nhold > 1\nrun 20\nhold > 1\nrun 20\n" + shot("unknown")),
    "dexnav": ("DexNav", "run 180\n" + shot("dexnav")
               + "hold T 1\nrun 30\n" + shot("collection")
               + "hold B 1\nrun 30\nhold R 1\nrun 30\n" + shot("registered")
               + "hold > 1\nrun 30\n" + shot("seen")
               + "hold S 1\nrun 30\n" + shot("filter")),
    "summary": ("Summary", "run 180\n" + shot("summary")
                + "hold > 1\nrun 60\n" + shot("stats")
                + "hold > 1\nrun 60\n" + shot("moves")),
    "bag": ("Bag", "run 180\n" + shot("bag") + "hold > 1\nrun 90\n" + shot("bag-balls") + "hold > 1\nrun 90\n" + shot("bag-tm") + "hold < 1\nrun 90\nhold < 1\nrun 90\nhold A 1\nrun 40\n" + shot("bag-actions")),
    "bag-female": ("BagFemale", "run 180\n" + shot("bagf-0")
                   + "".join("hold > 1\nrun 90\n" + shot(f"bagf-{n}") for n in range(1, 5))),
    "quests": ("Quests", "run 60\n" + shot("quests") + "hold B 1\nrun 30\n" + shot("quests-closed")),
    "keys": ("Keys", "run 60\n" + shot("keys") + "hold B 1\nrun 30\n" + shot("keys-closed")),
    "pc": ("PC", "run 60\n" + shot("pc-menu") + "hold D 1\nrun 10\nhold D 1\nrun 10\nhold A 1\nrun 240\n" + shot("pc")),
    "naming": ("Naming", "run 240\n" + shot("naming") + "hold R 1\nrun 40\n" + shot("naming-page")),
    "townmap": ("TownMap", "run 240\n" + shot("townmap") + "hold > 1\nrun 30\n" + shot("townmap-move")),
    "flymap": ("FlyMap", "run 240\n" + shot("flymap")),
    "achievements": ("Achievements", "run 240\n" + shot("achievements") + "hold D 1\nrun 30\n" + shot("achievements-row")),
    "registered": ("Registered", "run 60\n" + shot("registered")),
    "mart": ("Mart", "run 120\n" + shot("mart-menu") + "hold A 1\nrun 240\n" + shot("mart-buy")
             + "hold A 1\nrun 60\n" + shot("mart-quantity")),
    "relearner": ("Relearner", "run 300\n" + shot("relearner") + "hold D 1\nrun 30\n" + shot("relearner-row")),
    "evolution": ("Evolution", "run 300\n" + shot("evolution-start") + "run 900\n" + shot("evolution-mid")
                  + "mash A 600\n" + shot("evolution-end")),
    "popup": ("WildIntimidate", "run 300\nhold A 1\nrun 20\nhold A 1\nrun 20\n" + "".join(shot(f"popup-{n}") + "run 25\n" for n in range(1, 9))),
    "battle": ("Wild", "run 500\nhold A 1\nrun 240\n" + shot("battle-actions")
               + "hold A 1\nrun 60\n" + shot("battle-moves")
               + "hold > 1\nrun 20\n" + shot("battle-moves-focus")),
    "party": ("Wild", "run 500\nhold A 1\nrun 240\nhold D 1\nrun 6\nhold A 1\nrun 240\n" + shot("party")
              + "hold D 1\nrun 10\n" + shot("party-move")
              + "hold B 1\nrun 180\n" + shot("party-cancel")
              + "hold A 1\nrun 180\nhold D 1\nrun 10\nhold A 1\nrun 30\n" + shot("party-action")
              + "hold A 1\nrun 240\n" + shot("party-switch")
              + f'r8 {symbols["gBattlerPartyIndexes"]:08x}\n'),
}

selected = sys.argv[1:] or list(cases)
cases["party-slow"] = cases["party"]
cases["voiceover"] = ("Tutorial", "run 500\nhold A 1\nrun 240\n"
                      + callback("RFUiFixture_Voiceover") + "run 240\n" + shot("voiceover"))
cases["faint"] = ("Faint", "run 500\nhold A 1\nrun 240\nhold A 1\nrun 60\nhold A 1\nrun 600\n" + shot("faint-prompt")
                  + "mash A 120\nrun 240\n" + shot("faint-party")
                  + "hold B 1\nrun 30\nhold D 1\nrun 10\nhold A 1\nrun 30\n" + shot("faint-action")
                  + "hold A 1\nrun 360\n" + shot("faint-switch")
                  + f'r8 {symbols["gBattlerPartyIndexes"]:08x}\n')
for case in selected:
    fixture, commands = cases[case]
    for mode in ("dark", "light"):
        setup = callback("RFUiFixture_Day") + "run 1\n" if mode == "light" else ""
        if case.endswith("slow"):
            setup += callback("RFUiFixture_Slow") + "run 1\n"
        script = base + setup + callback("RFUiFixture_" + fixture) + commands
        if case.endswith("slow"):
            script = script.replace("run 500\n", "run 1000\n").replace("run 240\n", "run 600\n").replace("run 180\n", "run 400\n")
        # Unique paths let a single batched run retain every state/theme.
        script = script.replace("/tmp/rf-ui-", f"/tmp/rf-ui-{mode}-")
        if case.endswith("slow"):
            script = script.replace(f"/tmp/rf-ui-{mode}-", f"/tmp/rf-ui-{mode}-slow-")
        path = OUT / f"{case}-{mode}.txt"
        path.write_text(script)
        docker("cp", str(path), "rojo-fuego-check:/tmp/rf-ui.txt")
        result = docker("exec", "rojo-fuego-check", "/tmp/rf-mgba-harness",
                        "/work/pokefirered.gba", "/tmp/rf-ui.txt", capture_output=True, text=True)
        (OUT / f"{case}-{mode}.log").write_text(result.stdout)
        if case in ("party", "party-slow", "faint", "field-party"):
            assert result.stdout.strip().endswith("= 01"), f"Party switch failed: {case} {mode}: {result.stdout}"
        for line in script.splitlines():
            if line.startswith("shot "):
                remote = line.split()[1]
                ppm = OUT / Path(remote).name
                docker("cp", "rojo-fuego-check:" + remote, str(ppm))
                subprocess.run([sys.executable, str(Path(__file__).with_name("ppm2png.py")), str(ppm)], check=True)
        print(case, mode, result.stdout.strip(), flush=True)
