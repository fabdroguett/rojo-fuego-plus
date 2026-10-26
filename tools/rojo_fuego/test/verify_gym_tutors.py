"""Run all eight native badge tutors and a locked case using synthetic Mew."""
from pathlib import Path
import subprocess
import sys

from runtime import run as environment


ROOT = Path(__file__).resolve().parents[3]
OUT = ROOT / "build_artifacts/gym-tutors"
OUT.mkdir(parents=True, exist_ok=True)


nm = environment("exec", "rojo-fuego-check", "arm-none-eabi-nm", "/work/pokefirered.elf",
            capture_output=True, text=True).stdout
symbols = {p[2]: int(p[0], 16) for line in nm.splitlines() if len(p := line.split()) == 3}


def callback(name):
    return f'w32 {symbols["gMain"] + 4:08x} {symbols[name] | 1:08x}\nrun 1\n'


for index, locked in [(i, 0) for i in range(8)] + [(0, 1)]:
    name = f"gym-{index}" + ("-locked" if locked else "")
    script = ("run 600\nmash T 30\nrun 180\nmash A 3900\nmash B 240\nrun 60\n"
              + f'w32 {symbols["gRFTrainingFixtureGym"]:08x} {index:x}\n'
              + f'w32 {symbols["gRFTrainingFixtureGymLocked"]:08x} {locked:x}\n'
              + callback("RFTrainingFixture_GymTutor") + "mash A 1200\n"
              + "hold U 1\nrun 30\nhold U 1\nrun 30\nhold A 1\nrun 120\nmash A 1800\n"
              + callback("RFTrainingFixture_CheckGym")
              + f'r32 {symbols["gRFTrainingFixtureResult"]:08x}\n'
              + f"shot /tmp/rf-{name}.ppm\n")
    path = OUT / f"{name}.txt"
    path.write_text(script, encoding="utf-8")
    environment("cp", str(path), "rojo-fuego-check:/tmp/rf-gym-tutor.txt")
    result = environment("exec", "rojo-fuego-check", "/tmp/rf-mgba-harness", "/work/pokefirered.gba",
                    "/tmp/rf-gym-tutor.txt", capture_output=True, text=True)
    (OUT / f"{name}.log").write_text(result.stdout, encoding="utf-8")
    environment("cp", f"rojo-fuego-check:/tmp/rf-{name}.ppm", str(OUT / f"{name}.ppm"))
    subprocess.run([sys.executable, str(Path(__file__).with_name("ppm2png.py")), str(OUT / f"{name}.ppm")], check=True)
    print(name, result.stdout.strip(), flush=True)
    assert result.stdout.strip().lower().endswith("= 00000001"), f"Tutor failed: {name}; inspect capture"
