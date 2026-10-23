"""Generate graphics/bag/bag_male.png, bag_female.png and bag.pal from the backpack concept.

Source: graphics/rojo_fuego/menu_art_v2/concepts/backpack-v1.png (our own concept, see
menu_art_v2/README.md). The concept is reduced to the 64x64 sprite grid, mapped onto a fixed
15-colour palette by material roles and cleaned of isolated pixels. Each pocket then gets its
own frame, in the order item_menu_icons.c expects (tile offsets 0, 64, ... 320):
closed, Items, Key Items, Poke Balls, TMs/HMs, Berries. An open pocket shows a dark mouth with
a hint of its contents. Both genders share bag.pal, so the female bag swaps the red and teal ramps.
Standard library only: python tools/rojo_fuego/make_bag_frames.py [--preview out.png]
"""
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from make_menu_icons import read_rgba  # noqa: E402
from pngutil import write_png, write_rgb_png  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "graphics/rojo_fuego/menu_art_v2/concepts/backpack-v1.png"
OUT_DIR = ROOT / "graphics/bag"
SIZE = 64
BAG_W = 62
OUTLINE_SHARE = 0.5

# Index 0 is transparent. Ramps go from light to dark.
PALETTE = [
    (115, 197, 164),                                                      # 0 transparent
    (16, 18, 32),                                                         # 1 outline
    (248, 120, 104), (224, 64, 64), (184, 40, 48), (136, 24, 40), (88, 16, 32),   # 2-6 red
    (96, 200, 192), (40, 144, 152), (24, 104, 120), (16, 72, 92), (8, 44, 60),    # 7-11 teal
    (248, 216, 96), (200, 136, 40),                                       # 12-13 gold
    (248, 232, 200),                                                      # 14 cream stitching
    (255, 255, 255),                                                      # 15 white
]
OUTLINE, RED, TEAL, GOLD_LIGHT, GOLD_DARK, CREAM, WHITE = 1, 2, 7, 12, 13, 14, 15
INTERIOR = OUTLINE


def load_concept():
    width, rows = read_rgba(SOURCE)
    height = len(rows)

    def solid(x, y):
        r, g, b, a = rows[y][x * 4:x * 4 + 4]
        return a >= 128 and r + g + b < 700

    xs = [x for y in range(0, height, 2) for x in range(0, width, 2) if solid(x, y)]
    ys = [y for y in range(0, height, 2) for x in range(0, width, 2) if solid(x, y)]
    return rows, solid, (min(xs), min(ys), max(xs) + 1, max(ys) + 1)


_NEAREST = {}


def nearest(rgb):
    if rgb not in _NEAREST:
        _NEAREST[rgb] = min(range(1, 16), key=lambda i: sum((a - b) ** 2 for a, b in zip(PALETTE[i], rgb)))
    return _NEAREST[rgb]


def reduce_concept():
    """Box-reduce the concept to BAG_W pixels wide; each cell takes its dominant colour."""
    rows, solid, (x0, y0, x1, y1) = load_concept()
    bag_h = round(BAG_W * (y1 - y0) / (x1 - x0))
    grid = [[0] * BAG_W for _ in range(bag_h)]
    for j in range(bag_h):
        for i in range(BAG_W):
            ax, bx = x0 + (x1 - x0) * i // BAG_W, x0 + (x1 - x0) * (i + 1) // BAG_W
            ay, by = y0 + (y1 - y0) * j // bag_h, y0 + (y1 - y0) * (j + 1) // bag_h
            votes, total = {}, 0
            for y in range(ay, by):
                for x in range(ax, bx):
                    total += 1
                    if solid(x, y):
                        k = nearest(tuple(rows[y][x * 4:x * 4 + 3]))
                        votes[k] = votes.get(k, 0) + 1
            if sum(votes.values()) < total * 0.45:
                continue
            # Outline pixels are thin in the source; let a strong minority keep the edge crisp.
            if votes.get(OUTLINE, 0) >= total * OUTLINE_SHARE:
                grid[j][i] = OUTLINE
            else:
                grid[j][i] = max(votes, key=lambda k: votes[k])
    return grid


def clean(grid):
    """Replace pixels that share no colour with their 8 neighbours by the neighbours' majority."""
    h, w = len(grid), len(grid[0])
    out = [row[:] for row in grid]
    for y in range(h):
        for x in range(w):
            c = grid[y][x]
            if c in (0, OUTLINE, GOLD_LIGHT, GOLD_DARK):
                continue
            near = [grid[y + dy][x + dx] for dy in (-1, 0, 1) for dx in (-1, 0, 1)
                    if (dy or dx) and 0 <= y + dy < h and 0 <= x + dx < w]
            if c not in near:
                body = [n for n in near if n not in (0, OUTLINE)] or near
                out[y][x] = max(set(body), key=body.count)
    return out


def outline(grid):
    """Close the silhouette: any opaque pixel touching transparency becomes outline."""
    h, w = len(grid), len(grid[0])
    out = [row[:] for row in grid]
    for y in range(h):
        for x in range(w):
            if grid[y][x] and any(not (0 <= y + dy < h and 0 <= x + dx < w) or grid[y + dy][x + dx] == 0
                                  for dy, dx in ((-1, 0), (1, 0), (0, -1), (0, 1))):
                out[y][x] = OUTLINE
    return out


def place(grid):
    """Centre the bag horizontally and sit it on the frame's bottom edge."""
    frame = [[0] * SIZE for _ in range(SIZE)]
    h, w = len(grid), len(grid[0])
    left, top = (SIZE - w) // 2, SIZE - 1 - h
    for y in range(h):
        for x in range(w):
            frame[top + y][left + x] = grid[y][x]
    return frame


def base_frame():
    return place(outline(clean(reduce_concept())))


# Contents that peek out of an open pocket. o outline, w white, c cream, r/R/d red light/mid/dark,
# t/T teal light/mid, g/G gold light/dark; '.' leaves the pocket mouth visible.
INK = {"o": OUTLINE, "w": WHITE, "c": CREAM, "r": RED, "R": RED + 1, "d": RED + 2,
       "t": TEAL, "T": TEAL + 1, "g": GOLD_LIGHT, "G": GOLD_DARK}
POTION = """
..ooo..
..oGo..
.ooGoo.
otwtTTo
otwTTTo
otTTTTo
otTTTTo
.oTTTo.
..ooo..
"""
KEY = """
.ooo......
ogwgoooooo
oG.Ggggggo
oGGGoooGoo
.ooo...oo.
"""
BALL = """
..oooo..
.orrRRo.
orRRRRRo
ooowwooo
owwwwwco
owwwwwco
.owwwco.
..oooo..
"""
DISC = """
..oooo..
.owtttTo
owtttTTo
ottooTTo
ottooTTo
otttTTTo
.otTTTo.
..oooo..
"""
BERRY = """
...oo..
..oTTo.
.oorroo
orwrrRo
orrrRRo
orrRRdo
.oRRdo.
..ooo..
"""


def open_pocket(frame, cx, cy, rx, ry, ramp, contents, shift=0):
    """Draw a pocket mouth centred on (cx, cy); `contents` stand in it, sunk two rows deep."""
    def inside(x, y, grow=0):
        return ((x - cx) / (rx + grow)) ** 2 + ((y - cy) / (ry + grow)) ** 2 <= 1

    box = [(x, y) for y in range(cy - ry - 1, cy + ry + 2) for x in range(cx - rx - 1, cx + rx + 2)
           if 0 <= x < SIZE and 0 <= y < SIZE]
    for x, y in box:
        if inside(x, y):
            frame[y][x] = INTERIOR
        elif inside(x, y, 1):
            frame[y][x] = ramp if y > cy else ramp + 3
    art = [line for line in contents.split() if line]
    assert len({len(line) for line in art}) == 1, "contents rows must share one width"
    left, top = cx + shift - len(art[0]) // 2, cy + 2 - len(art)
    for j, line in enumerate(art):
        for i, ch in enumerate(line):
            x, y = left + i, top + j
            if ch != "." and 0 <= x < SIZE and 0 <= y < SIZE and not (y > cy and inside(x, y, 1)):
                frame[y][x] = INK[ch]


# (centre x, centre y, radius x, radius y, fabric ramp around the mouth, contents, contents shift)
POCKETS = [
    None,                                   # closed
    (34, 46, 7, 2, RED, POTION, 0),         # Items: front pocket, left half
    (40, 39, 12, 2, RED, KEY, 7),           # Key Items: main compartment under the flap
    (13, 37, 4, 2, RED, BALL, 0),           # Poke Balls: left side pocket
    (59, 36, 2, 2, TEAL, DISC, 0),          # TMs/HMs: right side pocket
    (48, 46, 7, 2, RED, BERRY, 0),          # Berries: front pocket, right half
]


def swap_ramps(frame):
    swap = {RED + i: TEAL + i for i in range(5)}
    swap.update({v: k for k, v in swap.items()})
    return [[swap.get(c, c) for c in row] for row in frame]


def frames(base, female):
    out = []
    for pocket in POCKETS:
        frame = swap_ramps(base) if female else [row[:] for row in base]
        if pocket:
            cx, cy, rx, ry, ramp, contents, shift = pocket
            if female:
                ramp = TEAL if ramp == RED else RED
            open_pocket(frame, cx, cy, rx, ry, ramp, contents, shift)
        out.append(frame)
    return out


def main():
    base = base_frame()
    sheets = {name: [row for frame in frames(base, name == "female") for row in frame]
              for name in ("male", "female")}
    if "--preview" in sys.argv:
        side = [a + [0] * 4 + b for a, b in zip(sheets["male"], sheets["female"])]
        write_rgb_png(sys.argv[sys.argv.index("--preview") + 1], PALETTE, side, scale=4)
        return
    for name, sheet in sheets.items():
        write_png(OUT_DIR / f"bag_{name}.png", PALETTE, sheet)
    pal = ["JASC-PAL", "0100", "16"] + [" ".join(map(str, c)) for c in PALETTE]
    (OUT_DIR / "bag.pal").write_bytes(("\n".join(pal) + "\n").encode("ascii"))
    print("graphics/bag: bag_male.png, bag_female.png (64x384), bag.pal")


if __name__ == "__main__":
    main()
