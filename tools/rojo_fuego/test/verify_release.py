"""Validate a production ROM and boot it with an in-memory save before local delivery."""
from datetime import datetime
from pathlib import Path
import hashlib
import json
import re
import subprocess

ROOT = Path(__file__).resolve().parents[3]
OUT = ROOT / "build_artifacts/releases" / datetime.now().strftime("%Y%m%d-%H%M%S")
OUT.mkdir(parents=True, exist_ok=True)


def docker(*args, **kwargs):
    return subprocess.run(["docker", *args], check=True, **kwargs)


nm = docker("exec", "rojo-fuego-check", "arm-none-eabi-nm", "/work/pokefirered.elf",
            capture_output=True, text=True).stdout
assert not re.search(r"RFProgress|RFUiFixture|RFTrainingFixture|gRFSaveLayout", nm), "Fixture in production ELF"
symbols = {p[2]: int(p[0], 16) for line in nm.splitlines() if len(p := line.split()) == 3}
rom = OUT / "pokefirered.gba"
docker("cp", "rojo-fuego-check:/work/pokefirered.gba", str(rom))
data = rom.read_bytes()
assert len(data) == 32 * 1024 * 1024, "Wrong ROM size"
assert data[0xAC:0xB0] == b"BPRE", "Wrong game code"
assert ((-sum(data[0xA0:0xBD]) - 0x19) & 0xFF) == data[0xBD], "Invalid GBA header checksum"
script = ("run 600\nshot /tmp/rf-release-title.ppm\nmash T 30\nrun 180\n"
          "shot /tmp/rf-release-menu.ppm\nmash A 3900\nmash B 240\nrun 60\n"
          "shot /tmp/rf-release-field.ppm\n"
          + f'r32 {symbols["gMain"] + 4:08x}\n')
path = OUT / "boot.txt"
path.write_text(script, encoding="utf-8")
docker("cp", str(path), "rojo-fuego-check:/tmp/rf-release-boot.txt")
result = docker("exec", "rojo-fuego-check", "/tmp/rf-mgba-harness", "/work/pokefirered.gba",
                "/tmp/rf-release-boot.txt", capture_output=True, text=True)
(OUT / "boot.log").write_text(result.stdout, encoding="utf-8")
for stage in ("title", "menu", "field"):
    docker("cp", f"rojo-fuego-check:/tmp/rf-release-{stage}.ppm", str(OUT / f"{stage}.ppm"))
callback = int(result.stdout.strip().split("=")[-1], 16)
assert callback == symbols["CB2_Overworld"] | 1, "Boot did not reach the field; inspect captures"
manifest = {"romPath": str(rom), "sha256": hashlib.sha256(data).hexdigest(),
            "size": len(data), "gameCode": "BPRE", "bootPassed": True}
(OUT / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
(OUT.parent / "latest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
print(json.dumps(manifest, indent=2))
