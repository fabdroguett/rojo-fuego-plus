"""Verify a disposable progression fixture ROM using synthetic in-memory game data."""
from pathlib import Path
import json
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[3]
OUT = ROOT / "build_artifacts/progression"
berries_off = "--berries-off" in sys.argv
if berries_off:
    OUT = OUT / "berries-off"
OUT.mkdir(parents=True, exist_ok=True)


def docker(*args, **kwargs):
    return subprocess.run(["docker", *args], check=True, **kwargs)


nm = docker("exec", "rojo-fuego-check", "arm-none-eabi-nm", "/work/pokefirered.elf",
            capture_output=True, text=True).stdout
symbols = {p[2]: int(p[0], 16) for line in nm.splitlines() if len(p := line.split()) == 3}
assert "RFProgress_Traveler" in symbols, "Requires the explicitly linked disposable fixture"


def callback(name):
    return f'w32 {symbols["gMain"] + 4:08x} {symbols[name] | 1:08x}\n'


script = ("run 600\nmash T 30\nrun 180\nmash A 3900\nmash B 240\nrun 60\n"
          + callback("RFProgress_Briefing") + "mash A 1200\n"
          + callback("RFProgress_Delivery") + "mash A 1800\n"
          + (callback("RFProgress_DisableBerry") + "run 1\n" if berries_off else "")
          + callback("RFProgress_Traveler")
          + "run 120\nshot /tmp/rf-progress-intro.ppm\nmash A 18000\n"
          + "shot /tmp/rf-progress-end.ppm\n"
          + callback("RFProgress_CheckTraveler") + "run 1\n"
          + f'r32 {symbols["gRFProgressResult"]:08x}\n'
          + callback("RFProgress_Report") + "mash A 1200\n"
          + callback("RFProgress_CheckQuests") + "run 1\n"
          + f'r32 {symbols["gRFProgressStages"]:08x}\n'
          + callback("RFProgress_Relearner") + "run 300\nshot /tmp/rf-relearner.ppm\n"
          + "mash A 1200\nshot /tmp/rf-relearner-learned.ppm\n"
          + callback("RFProgress_CheckRelearner") + "run 1\n"
          + f'r32 {symbols["gRFProgressLearned"]:08x}\n'
          + callback("RFProgress_Rematch") + "mash A 24000\nshot /tmp/rf-rematch.ppm\n"
          + callback("RFProgress_CheckRematch") + "run 1\n"
          + f'r32 {symbols["gRFProgressRematch"]:08x}\n'
          + f'dump {symbols["gRFProgressDebug"]:08x} 18 /tmp/rf-rematch-debug.bin\n'
          + f'dump {symbols["gRFSaveLayout"]:08x} 34 /tmp/rf-save-layout.bin\n')
path = OUT / "traveler.txt"
path.write_text(script, encoding="utf-8")
docker("cp", str(path), "rojo-fuego-check:/tmp/rf-progression.txt")
result = docker("exec", "rojo-fuego-check", "/tmp/rf-mgba-harness",
                "/work/pokefirered.gba", "/tmp/rf-progression.txt", capture_output=True, text=True)
(OUT / "traveler.log").write_text(result.stdout, encoding="utf-8")
for name in ("rf-progress-intro.ppm", "rf-progress-end.ppm", "rf-save-layout.bin",
             "rf-relearner.ppm", "rf-relearner-learned.ppm", "rf-rematch.ppm", "rf-rematch-debug.bin"):
    docker("cp", "rojo-fuego-check:/tmp/" + name, str(OUT / name))
layout = dict(zip(("SaveBlock1", "SaveBlock2", "SaveBlock3", "BoxPokemon", "Pokemon",
                   "PokemonStorage", "boxes_offset", "box_names_offset", "wallpapers_offset",
                   "fusions_offset", "sector_data", "SaveBlock3_capacity", "storage_capacity"),
                  struct.unpack("<13I", (OUT / "rf-save-layout.bin").read_bytes())))
(OUT / "save-layout.json").write_text(json.dumps(layout, indent=2) + "\n", encoding="utf-8")
print(result.stdout, flush=True)
print(json.dumps(layout, indent=2), flush=True)
print("Rematch diagnostic:", struct.unpack("<6I", (OUT / "rf-rematch-debug.bin").read_bytes()), flush=True)
checks = [line.lower() for line in result.stdout.splitlines() if " = " in line]
assert len(checks) == 4 and checks[0].endswith("= 0000003f") and checks[1].endswith("= 0000001f") \
    and checks[2].endswith("= 0000000f") and checks[3].endswith("= 00000001"), \
    "Noa/mission/relearner/rematch regression: inspect artifacts"
