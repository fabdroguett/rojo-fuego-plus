"""Reproducible UI checks for the disposable training fixture (synthetic party only)."""
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[3]
OUT = ROOT / "build_artifacts/training"
OUT.mkdir(parents=True, exist_ok=True)


def docker(*args, **kwargs):
    return subprocess.run(["docker", *args], check=True, **kwargs)


nm = docker("exec", "rojo-fuego-check", "arm-none-eabi-nm", "/work/pokefirered.elf",
            capture_output=True, text=True).stdout
symbols = {p[2]: int(p[0], 16) for line in nm.splitlines() if len(p := line.split()) == 3}
assert "RFTrainingFixture_Begin" in symbols, "Requires the disposable training fixture"


def callback(name):
    return f'w32 {symbols["gMain"] + 4:08x} {symbols[name] | 1:08x}\nrun 1\n'


phases = {
    "begin": "run 600\nmash T 30\nrun 180\nmash A 3900\nmash B 240\nrun 60\n"
             + callback("RFTrainingFixture_Begin") + "run 120\n",
    "ev": "hold D 1\nrun 20\nhold A 1\nrun 180\n",
    "party": "mash A 120\nrun 120\n",
    "stat": "hold A 1\nrun 240\n",
    "value": "hold A 1\nrun 150\n",
    "actions": "mash A 120\nrun 60\n",
    "preview": "hold A 1\nrun 180\n",
    "cancel": "hold D 1\nrun 20\nhold A 1\nrun 150\n"
              + callback("RFTrainingFixture_CheckPreview")
              + f'r32 {symbols["gRFTrainingFixtureResult"]:08x}\n',
}
def press(key, wait=120):
    return f"hold {key} 1\nrun {wait}\n"


def check(name):
    return callback(name) + f'r32 {symbols["gRFTrainingFixtureResult"]:08x}\n'


phases["all"] = (
    phases["begin"] + press("D", 20) + press("A") + press("A", 240) + press("A", 240)
    + press("A") + press("A", 60) + press("A")
    + "shot /tmp/rf-training-confirm.ppm\n"
    + press("D", 20) + press("A") + check("RFTrainingFixture_CheckPreview")
    + press("A", 60) + press("A") + press("A")
    + check("RFTrainingFixture_CheckEV") + "shot /tmp/rf-training-ev-applied.ppm\n"
    + press("A", 60) + press("B", 60)
    + callback("RFTrainingFixture_IV") + "run 60\n"
    + press("D", 20) + press("D", 20) + press("A")
    + press("A", 240) + press("A", 240) + press("A")
    + press("A", 60) + press("A", 60) + press("D", 20) + press("A") + press("A")
    + check("RFTrainingFixture_CheckIV") + "shot /tmp/rf-training-iv-applied.ppm\n"
)
phase = sys.argv[1] if len(sys.argv) > 1 else "all"
script = ("" if phase in ("begin", "all") else "load /tmp/rf-training.ss\n") + phases[phase]
script += f"shot /tmp/rf-training-{phase}.ppm\nsave /tmp/rf-training.ss\n"
path = OUT / f"{phase}.txt"
path.write_text(script, encoding="utf-8")
docker("cp", str(path), "rojo-fuego-check:/tmp/rf-training.txt")
result = docker("exec", "rojo-fuego-check", "/tmp/rf-mgba-harness", "/work/pokefirered.gba",
                "/tmp/rf-training.txt", capture_output=True, text=True)
(OUT / f"{phase}.log").write_text(result.stdout, encoding="utf-8")
for line in script.splitlines():
    if line.startswith("shot "):
        remote = line.split()[1]
        path = OUT / Path(remote).name
        docker("cp", "rojo-fuego-check:" + remote, str(path))
        subprocess.run([sys.executable, str(Path(__file__).with_name("ppm2png.py")), str(path)], check=True)
print(phase, result.stdout, flush=True)
if phase == "all":
    checks = [line.lower().split(" = ")[-1] for line in result.stdout.splitlines() if " = " in line]
    assert checks == ["00000001", "0000000f", "0000000f"], "Training UI regression: inspect captures"
