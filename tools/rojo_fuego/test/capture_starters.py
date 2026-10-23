"""Exercise all 54 Oak choices in mGBA using starter_fixture.c (synthetic save).

Copy the fixture into /work/src/rf_starter_fixture.c and link with -u for its four
RFUiFixture_Starter* entry points. Remove the fixture and relink before delivery.
"""
from pathlib import Path
import json
import re
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[3]
OUT = ROOT / "build_artifacts/ui/starters"
OUT.mkdir(parents=True, exist_ok=True)


def docker(*args):
    return subprocess.check_output(["docker", *args], text=True)


symbols = {p[2]: int(p[0], 16) for line in docker(
    "exec", "rojo-fuego-check", "arm-none-eabi-nm", "/work/pokefirered.elf").splitlines()
    if len(p := line.split()) == 3}


def callback(name):
    return f'w32 {symbols["gMain"] + 4:08x} {symbols[name] | 1:08x}\nrun 3\n'


def key(name):
    return f"hold {name} 1\nrun 12\n"


species = {}
for name, value in re.findall(r"\b(SPECIES_\w+)\s*=\s*(\w+)",
                              (ROOT / "include/constants/species.h").read_text()):
    if value.isdecimal():
        species[name] = int(value)
    elif value in species:
        species[name] = species[value]
table = (ROOT / "src/rf_starter_menu.c").read_text(encoding="utf-8").split("static const u8 *const sGroups")[0]
groups = re.findall(r"\{(SPECIES_\w+), (SPECIES_\w+), (SPECIES_\w+)\}", table)
assert len(groups) == 18 and len(set(sum((list(g) for g in groups), []))) == 54
script = ("run 600\nmash T 30\nrun 180\nmash A 3900\nmash B 240\nrun 60\n"
          + callback("RFUiFixture_StarterLab") + "run 180\nshot /tmp/rf-starter-lab.ppm\n"
          + "save /tmp/rf-starter-lab.ss\n")
cases = []
for group, row in enumerate(groups):
    for choice, name in enumerate(row):
        tag = f"{group:02d}-{choice}"
        script += "load /tmp/rf-starter-lab.ss\n" + callback("RFUiFixture_StarterScript") + "run 120\n"
        script += key("R") * group + key(">") * choice + key("S")
        if choice == 0:
            script += f"shot /tmp/rf-starter-{tag}.ppm\n"
        # First A only previews confirmation; B must return without granting a mon.
        script += key("A")
        if tag == "00-0":
            script += "shot /tmp/rf-starter-confirm.ppm\n"
        script += key("B") + key("A") + key("A")
        script += "run 120\nmash B 900\nrun 180\n" + callback("RFUiFixture_StarterRead")
        script += f'dump {symbols["gRFUiFixture_StarterResult"]:08x} a /tmp/rf-starter-{tag}.bin\n'
        hidden = name not in {"SPECIES_ZORUA", "SPECIES_DEINO", "SPECIES_MIMIKYU"}
        cases.append((tag, name, [1, species[name], 2 if hidden else 0, 3, [0, 2, 1][choice]]))
script += ("load /tmp/rf-starter-lab.ss\n" + callback("RFUiFixture_StarterScript")
           + "run 120\n" + key("B") + "run 300\n" + callback("RFUiFixture_StarterRead")
           + f'dump {symbols["gRFUiFixture_StarterResult"]:08x} a /tmp/rf-starter-cancel.bin\n')
script += callback("RFUiFixture_StarterDay") + "run 120\nshot /tmp/rf-starter-day.ppm\n"
path = OUT / "commands.txt"
path.write_text(script)
docker("cp", str(path), "rojo-fuego-check:/tmp/rf-starter-tests.txt")
log = docker("exec", "rojo-fuego-check", "/tmp/rf-mgba-harness", "/work/pokefirered.gba", "/tmp/rf-starter-tests.txt")
(OUT / "emulator.log").write_text(log)
report = []
for tag, name, expected in cases:
    target = OUT / f"{tag}.bin"
    docker("cp", f"rojo-fuego-check:/tmp/rf-starter-{tag}.bin", str(target))
    actual = list(struct.unpack("<5H", target.read_bytes()))
    assert actual == expected, (name, actual, expected)
    report.append({"species": name, "result": actual})
    if tag.endswith("-0"):
        docker("cp", f"rojo-fuego-check:/tmp/rf-starter-{tag}.ppm", str(OUT / f"{tag}.ppm"))
for tag in ("lab", "day", "confirm"):
    docker("cp", f"rojo-fuego-check:/tmp/rf-starter-{tag}.ppm", str(OUT / f"{tag}.ppm"))
target = OUT / "cancel.bin"
docker("cp", "rojo-fuego-check:/tmp/rf-starter-cancel.bin", str(target))
cancel = list(struct.unpack("<5H", target.read_bytes()))
assert cancel[0] == 0 and cancel[3] == 2, cancel
(OUT / "report.json").write_text(json.dumps({"choices": report, "cancel": cancel}, indent=2))
print("PASS: 54 species, hidden ability/fallback, 3 rival branches, confirmation back and cancellation.")
