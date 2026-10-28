"""Exercise the real bag in a disposable fixture ROM; write native captures under ui/.

Link ui_fixture.c and bag_reference_fixture.c with their RFUiFixture_/RFBagFixture_
entry points retained. Never deliver this fixture ROM. See UI_VALIDATION.md.
"""
import argparse
import json
from pathlib import Path
import re
import subprocess
import sys

from runtime import run as environment

ROOT = Path(__file__).resolve().parents[3]
parser = argparse.ArgumentParser()
parser.add_argument("--workdir", default="/work")
parser.add_argument("--case", action="append", default=[])
parser.add_argument("--output", type=Path, default=ROOT / "build_artifacts/ui/bag-reference")
args = parser.parse_args()
OUT = args.output.resolve()
OUT.mkdir(parents=True, exist_ok=True)
work = args.workdir.rstrip("/")
nm = environment("exec", "-w", work, "rojo-fuego-check", "arm-none-eabi-nm", "pokefirered.elf",
                 capture_output=True, text=True).stdout
symbols = {p[2]: int(p[0], 16) for line in nm.splitlines() if len(p := line.split()) == 3}
symbols["gMainCallback2"] = symbols["gMain"] + 4
base = "run 600\nmash T 30\nrun 180\nmash A 3900\nmash B 240\nrun 60\n"


def callback(name):
    return f'w32 {symbols["gMain"] + 4:08x} {symbols[name] | 1:08x}\n'


def read(name):
    return f"r32 {symbols[name]:08x}\n"


def shot(name):
    return f"shot /tmp/rf-bag-reference-{name}.ppm\n"


def inspect(name):
    return callback(name) + "run 1\n" + callback("CB2_BagMenuRun") + "run 30\n"


cases = {
    "sort": ("RFBagFixture_Sort", "run 180\nhold T 1\nrun 60\n" + shot("sort-menu")
             + "hold A 1\nrun 180\nhold B 1\nrun 90\n" + shot("sorted")
             + inspect("RFBagFixture_ReadSort") + read("gRFBagFixtureResult"),
             {"gRFBagFixtureResult": 1}),
    "field-medicine": ("RFBagFixture_FieldMedicine", "run 180\nhold A 1\nrun 60\n"
                       + "hold A 1\nrun 180\n" + shot("field-party")
                       + "hold A 1\nrun 180\n" + shot("field-healed")
                       + "mash B 180\nrun 180\n" + shot("field-return") + read("gMainCallback2")
                       + callback("RFBagFixture_ReadField") + "run 1\n"
                       + read("gRFBagFixtureHP") + read("gRFBagFixturePotions") + read("gRFBagFixtureResult"),
                       {"gMainCallback2": symbols["CB2_Overworld"] | 1, "gRFBagFixtureResult": 1}),
    "battle-cancel": ("RFBagFixture_Battle", "run 500\nhold A 1\nrun 240\n"
                      + "hold > 1\nrun 20\nhold A 1\nrun 240\n" + shot("battle-cancel-open")
                      + "hold B 1\nrun 240\n" + shot("battle-cancel-return") + read("gMainCallback2"),
                      {"gMainCallback2": symbols["BattleMainCB2"] | 1}),
    "empty": ("RFBagFixture_Empty", "run 180\n" + shot("empty")
              + "hold B 1\nrun 180\n" + shot("empty-return") + read("gMainCallback2"),
              {"gMainCallback2": symbols["CB2_Overworld"] | 1}),
    "edges": ("RFBagFixture_Edges", "run 180\n" + shot("quantities")
              + "hold D 1\nrun 30\n" * 6 + shot("long-names")
              + "hold B 1\nrun 180\n" + read("gMainCallback2"),
              {"gMainCallback2": symbols["CB2_Overworld"] | 1}),
    "descriptions": ("RFBagFixture_Edges", "run 180\n"
                     + inspect("RFBagFixture_NoDescription") + shot("no-description")
                     + inspect("RFBagFixture_LongDescription") + shot("long-description")
                     + read("gRFBagFixtureResult"), {"gRFBagFixtureResult": 1}),
    "keys": ("RFBagFixture_Keys", "run 180\n" + shot("keys")
             + "hold A 1\nrun 60\n" + shot("key-actions")
             + "hold > 1\nrun 20\nhold A 1\nrun 60\n" + shot("key-registered")
             + "hold B 1\nrun 180\n" + callback("RFBagFixture_ReadKey")
             + "run 1\n" + read("gRFBagFixtureResult"), {"gRFBagFixtureResult": 1}),
    "stress": ("RFBagFixture_Edges", "run 180\n"
               + ("hold D 1\nrun 8\n" * 12 + "hold R 1\nrun 20\n") * 14
               + inspect("RFBagFixture_ReadResources") + shot("stress")
               + read("gRFBagFixtureMatrices") + read("gRFBagFixturePalettes")
               + read("gRFBagFixtureSprites") + read("gRFBagFixtureOBJBytes") + read("gRFBagFixtureResult"),
               {"gRFBagFixtureResult": 1}),
    "battle": ("RFBagFixture_Battle", "run 500\nhold A 1\nrun 240\n" + shot("battle-actions")
               + "hold > 1\nrun 20\nhold A 1\nrun 240\n" + shot("battle-bag")
               + "hold A 1\nrun 60\n" + shot("battle-item-actions")
               + "hold A 1\nrun 240\n" + shot("battle-party")
               + "hold A 1\nrun 180\n" + shot("battle-healed")
               + "mash A 180\nmash B 60\nrun 180\n" + shot("battle-return")
               + callback("RFBagFixture_ReadBattle") + "run 1\n"
               + read("gRFBagFixtureHP") + read("gRFBagFixturePotions") + read("gRFBagFixtureResult"),
               {"gRFBagFixturePotions": 1, "gRFBagFixtureResult": 1}),
}

report = []
for name in args.case or list(cases):
    fixture, steps, expected = cases[name]
    for theme in ("dark", "light"):
        setup = callback("RFUiFixture_Day") + "run 1\n" if theme == "light" else ""
        script = base + setup + callback(fixture) + steps
        script = script.replace("/tmp/rf-bag-reference-", f"/tmp/rf-bag-reference-{theme}-")
        local = OUT / f"{name}-{theme}.txt"
        local.write_text(script, encoding="utf-8")
        remote = f"/tmp/rf-bag-test-{name}-{theme}.txt"
        environment("cp", str(local), "rojo-fuego-check:" + remote)
        result = environment("exec", "-w", work, "rojo-fuego-check", "/tmp/rf-mgba-harness",
                             work + "/pokefirered.gba", remote, capture_output=True, text=True)
        (OUT / f"{name}-{theme}.log").write_text(result.stdout, encoding="utf-8")
        for line in script.splitlines():
            if line.startswith("shot "):
                ppm = OUT / Path(line.split()[1]).name
                environment("cp", "rojo-fuego-check:" + line.split()[1], str(ppm))
                subprocess.run([sys.executable, str(Path(__file__).with_name("ppm2png.py")), str(ppm)], check=True)
        values = {int(address, 16): int(value, 16)
                  for address, value in re.findall(r"([0-9a-fA-F]{8}) = ([0-9a-fA-F]+)", result.stdout)}
        checked = {symbol: values.get(symbols[symbol]) for symbol in expected}
        passed = checked == expected
        observed = {symbol: values[symbols[symbol]] for symbol in symbols
                    if symbol.startswith("gRFBagFixture") and symbols[symbol] in values}
        report.append({"case": name, "theme": theme, "passed": passed, "checks": checked, "observed": observed})
        (OUT / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(name, theme, "PASS" if passed else "FAIL", result.stdout.strip(), flush=True)
        assert passed, f"{name} {theme}: expected {expected}, got {checked}"
