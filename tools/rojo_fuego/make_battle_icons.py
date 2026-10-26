"""Generate src/data/rf_battle_icons.h: colour type and command icons for the battle menus.

Each icon is original 12x12 pixel art centred in a 16x16 frame (4bpp, GBA tile order). Pixels
use five shared roles so one per-icon colour ramp paints them at runtime:
  o = 1 outline, d = 2 shade, m = 3 base, l = 4 light, w = 5 highlight, . = 0 transparent.
The battle menu loads the ramp into the cell's own BG palette (see battle_controller_player.c).
A scaled RGB preview is written to graphics/rojo_fuego/icons/battle_icons_preview.png.
Standard library only: python tools/rojo_fuego/make_battle_icons.py
"""
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from pngutil import write_rgb_png  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / "src/data/rf_battle_icons.h"
PREVIEW = ROOT / "graphics/rojo_fuego/icons/battle_icons_preview.png"
ROLES = {".": 0, "o": 1, "d": 2, "m": 3, "l": 4, "w": 5}

ART = {
    "NORMAL": ("""
....oooo....
..oowwlloo..
.owwlllllmo.
.owllllllmo.
olllllllllmo
olllllllllmo
ollllllllmmo
olllllllmmmo
.ollllllmmo.
.olllllmmmo.
..oommmmoo..
....oooo....""", [(7, 7, 6), (12, 12, 10), (19, 19, 17), (26, 26, 23), (31, 31, 29)]),
    "FIGHTING": ("""
.oooooooooo.
olwdlwdlwdwo
olldlldlldlo
ommdmmdmmdmo
oooooooommmo
ollllllomdmo
ommmmmmomdmo
.ooooooommo.
...odmmmmo..
...odmmmdo..
...ooooooo..""", [(10, 2, 2), (18, 4, 3), (26, 8, 6), (31, 17, 12), (31, 26, 22)]),
    "FLYING": ("""
........ooo.
......oollwo
.....olllwo.
...oollllwo.
..olllllwlo.
.ollmllwllo.
ollmlllllo..
olmmlllmo...
.omlmmmo....
..ooooo.....""", [(6, 8, 18), (10, 12, 22), (16, 20, 28), (24, 27, 31), (31, 31, 31)]),
    "POISON": ("""
...oooooo...
..ollllllo..
.olwllllllo.
.olllllllmo.
.oddlllddmo.
.oddlmlddmo.
.olllmmllmo.
..olllllmo..
..odldldmo..
...oooooo...""", [(8, 2, 12), (12, 4, 16), (18, 8, 22), (24, 14, 28), (30, 24, 31)]),
    "GROUND": ("""
.....oo.....
....omlo....
...ommllo...
...omdlllo..
..ommmdllo..
..odmmmdlllo
.odmmmmmdllo
odddmmmmmmlo
oooooooooooo""", [(10, 6, 2), (16, 9, 4), (22, 14, 6), (28, 20, 10), (31, 26, 16)]),
    "ROCK": ("""
....oooo....
..oollllo...
.ollwllmmo..
.olwlllmmmo.
olllllmmmmdo
olllldmmmddo
ommlldmmmddo
ommmmdmmdddo
.odmmmmmddo.
..oddddddo..
...oooooo...""", [(8, 6, 2), (14, 12, 6), (20, 18, 10), (26, 24, 16), (30, 29, 22)]),
    "BUG": ("""
..o......o..
...o....o...
...oooooo...
..ommddmmo..
.olllddlllo.
olwlmddmlwlo
ollmmddmmllo
olmmmddmmmlo
.ommmddmmmo.
.ommmddmmmo.
..odmddmdo..
...oooooo...""", [(4, 8, 2), (8, 14, 3), (12, 20, 4), (18, 26, 6), (26, 31, 14)]),
    "GHOST": ("""
...oooooo...
..ollllllo..
.olwllllllo.
.olldllldlo.
.olldllldlo.
olllllllllmo
ollllllllmmo
olllllllmmmo
ollmolmolmmo
oo.oo.oo.ooo""", [(8, 4, 14), (10, 6, 16), (16, 10, 22), (22, 16, 28), (28, 24, 31)]),
    "STEEL": ("""
..oooooooo..
.owwllllllo.
owllllllllmo
owlloooolmmo
owllo..olmmo
owllo..olmmo
owlloooolmmo
olllllllmmmo
.ommmmmmmmo.
..oooooooo..""", [(6, 7, 9), (10, 11, 14), (17, 18, 21), (24, 25, 27), (31, 31, 31)]),
    "FIRE": ("""
....o.......
....oo......
...odo...o..
...odmo..oo.
..odmmo.odo.
.odmmmmoodmo
.odmmlmmmmmo
odmmlllmlmmo
odmllwwlllmo
odmlwwwwllmo
.odmlwwlmmo.
..oooooooo..""", [(10, 2, 2), (22, 4, 2), (30, 10, 4), (31, 20, 4), (31, 29, 12)]),
    "WATER": ("""
.....oo.....
.....oo.....
....omlo....
....omlo....
...ommllo...
...omlwlo...
..ommlwllo..
..ommllllo..
.odmmmlllmo.
.odmmmmmmmo.
..oddmmmdo..
....oooo....""", [(2, 6, 14), (4, 10, 24), (8, 18, 30), (16, 25, 31), (28, 31, 31)]),
    "GRASS": ("""
........oooo
......oollmo
.....ollllmo
....ollwlmmo
...ollwlmmo.
..ollwlmmmo.
.olwlmmmmo..
.owlmmmdo...
.olmmddo....
.ommdoo.....
odoo........
oo..........""", [(2, 8, 3), (5, 14, 5), (8, 21, 7), (15, 27, 10), (24, 31, 16)]),
    "ELECTRIC": ("""
.....ooooo..
....ollllo..
....olwlo...
...olwlo....
...olllooo..
..olwwlllo..
..ommmmlo...
....omlo....
...omlo.....
...omo......
..omo.......
..oo........""", [(10, 6, 0), (20, 14, 0), (28, 20, 0), (31, 28, 4), (31, 31, 22)]),
    "PSYCHIC": ("""
...oooooo...
..ommmmmmo..
.omlllllllo.
omlmmmmmmlmo
omlmllllmlmo
omlmlmmlmlmo
omlmllmmlmmo
omlmmmmlllmo
.omllllllmo.
..ommmmmmo..
...oooooo...""", [(14, 2, 8), (20, 4, 12), (28, 8, 18), (31, 18, 26), (31, 28, 30)]),
    "ICE": ("""
.....oo.....
..o..ll..o..
..ol.ll.lo..
...olwwlo...
.o..lwwl..o.
olllwwwwlllo
olllwwwwlllo
.o..lwwl..o.
...olwwlo...
..ol.ll.lo..
..o..ll..o..
.....oo.....""", [(4, 12, 20), (8, 18, 26), (12, 22, 28), (16, 26, 31), (28, 31, 31)]),
    "DRAGON": ("""
....ooo.....
...ollloo...
..ollwllloo.
.ollldllllo.
.olllllllllo
ommmllloooo.
ommmmlowwo..
ommmmmooo...
.ommmmmo....
..ommmmmo...
...oooooo...""", [(4, 4, 14), (6, 6, 20), (8, 10, 26), (14, 18, 31), (28, 30, 31)]),
    "DARK": ("""
...oooo.....
.oommmo.....
ommllo......
omllo.......
omlmo.......
omlmo.......
ommmmo.....o
.ommmmoooomo
..oommmmmmo.
....oooooo..""", [(2, 2, 4), (6, 5, 8), (11, 10, 14), (17, 16, 21), (24, 23, 28)]),
    "FAIRY": ("""
.....oo.....
....olmo....
....olmo....
ooooolmmoooo
olllllwmmmmo
.ollllwmmmo.
..olllmmmo..
..ollmmmmo..
.ollmooommo.
.olmo..ommo.
ooo......ooo""", [(16, 4, 14), (22, 8, 18), (28, 14, 24), (31, 22, 28), (31, 30, 31)]),
    "BAG": ("""
....oooo....
...o....o...
..oooooooo..
.ollllllllo.
.olwllllllo.
.oddddddddo.
.ollloollmo.
.olllddllmo.
.ollllllmmo.
.ommmmmmmmo.
..oooooooo..""", [(8, 4, 1), (14, 8, 3), (20, 12, 5), (26, 18, 8), (30, 25, 14)]),
    "BALL": ("""
....oooo....
..oommmmoo..
.omlmmmmmdo.
.omlmmmmmdo.
ommmmoommmdo
ooooolloooo.
olllloolllwo
ollllllllwwo
.ollllllwwo.
.olllllwwwo.
..oowwwwoo..
....oooo....""", [(3, 3, 4), (18, 2, 2), (29, 6, 5), (31, 31, 31), (22, 22, 24)]),
    "RUN": ("""
........oo..
.......ollo.
.......ollo.
....ooooooo.
...olllllmo.
..oo.ollmo..
....ollmmo..
...ollmolo..
..olmo.olo..
.olmo...olo.
ooo......ooo""", [(2, 6, 16), (4, 10, 22), (6, 14, 28), (12, 22, 31), (24, 30, 31)]),
}

# Frame order: index == enum Type for 0..20 (types without own art reuse NORMAL), then commands.
TYPE_ORDER = ["NORMAL", "NORMAL", "FIGHTING", "FLYING", "POISON", "GROUND", "ROCK", "BUG", "GHOST",
              "STEEL", "NORMAL", "FIRE", "WATER", "GRASS", "ELECTRIC", "PSYCHIC", "ICE", "DRAGON",
              "DARK", "FAIRY", "NORMAL"]
COMMANDS = ["BAG", "BALL", "RUN"]
FRAMES = TYPE_ORDER + COMMANDS


def frame_pixels(name):
    rows = [r for r in ART[name][0].strip("\n").split("\n")]
    for r in rows:
        if len(r) != 12 or set(r) - set(ROLES):
            raise SystemExit(f"{name}: bad row {r!r}")
    if len(rows) > 12:
        raise SystemExit(f"{name}: {len(rows)} rows")
    top = (16 - len(rows)) // 2
    frame = [[0] * 16 for _ in range(16)]
    for y, r in enumerate(rows):
        for x, ch in enumerate(r):
            frame[top + y][2 + x] = ROLES[ch]
    return frame


def to_4bpp_tiles(frame):
    out = bytearray()
    for ty in range(2):
        for tx in range(2):
            for y in range(8):
                row = frame[ty * 8 + y][tx * 8:tx * 8 + 8]
                out += bytes(row[x] | (row[x + 1] << 4) for x in range(0, 8, 2))
    return out


def main():
    gfx = bytearray()
    for name in FRAMES:
        gfx += to_4bpp_tiles(frame_pixels(name))
    lines = [
        "// Generated by tools/rojo_fuego/make_battle_icons.py; do not edit by hand.",
        "// Original pixel art for Rojo Fuego Plus. 16x16 frames, 4bpp tiles; roles 1-5 = ramp.",
        f"#define RF_BATTLE_ICON_BAG  {len(TYPE_ORDER)}",
        f"#define RF_BATTLE_ICON_BALL {len(TYPE_ORDER) + 1}",
        f"#define RF_BATTLE_ICON_RUN  {len(TYPE_ORDER) + 2}",
        f"#define RF_BATTLE_ICON_COUNT {len(FRAMES)}",
        "",
        "static const u8 sRFBattleIconGfx[] ALIGNED(4) =",
        "{",
    ]
    for i in range(0, len(gfx), 16):
        lines.append("    " + ", ".join(f"0x{b:02X}" for b in gfx[i:i + 16]) + ",")
    lines += ["};", "", "static const u16 sRFBattleIconRamps[RF_BATTLE_ICON_COUNT][5] =", "{"]
    for i, name in enumerate(FRAMES):
        ramp = ", ".join(f"RGB({r}, {g}, {b})" for r, g, b in ART[name][1])
        lines.append(f"    [{i}] = {{{ramp}}}, // {name}")
    lines += ["};", ""]
    OUTPUT.write_text("\n".join(lines), encoding="utf-8", newline="\n")

    # Preview: every distinct icon on a cream card, 6 per row, scaled x4.
    names = list(dict.fromkeys(FRAMES))
    cols, cell = 6, 20
    rows_n = (len(names) + cols - 1) // cols
    palette = [(250, 246, 230), (40, 70, 72)]
    pixels = [[0] * (cols * cell) for _ in range(rows_n * cell)]
    for n, name in enumerate(names):
        base = len(palette)
        palette += [(r * 8, g * 8, b * 8) for r, g, b in ART[name][1]]
        frame = frame_pixels(name)
        ox, oy = (n % cols) * cell + 2, (n // cols) * cell + 2
        for y in range(16):
            for x in range(16):
                if frame[y][x]:
                    pixels[oy + y][ox + x] = base + frame[y][x] - 1
    write_rgb_png(PREVIEW, palette, pixels, scale=4)
    print(f"{OUTPUT.relative_to(ROOT)}: {len(FRAMES)} frames, {len(gfx)} bytes; preview {PREVIEW.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
