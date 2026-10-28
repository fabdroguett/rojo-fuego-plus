"""Build reviewed native bag art; retain the old sampler for diagnostic comparisons.

Standard library only. Originals remain in graphics/rojo_fuego/bag_user/source.
"""
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
from make_menu_icons import read_rgba

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "graphics/rojo_fuego/bag_user/source"
ASSETS = [
    ("BACKPACK", "backpack_mochila.png", (14, 14)),
    ("MEDICINE", "icon_bottle_purple.png", (14, 14)),
    ("BATTLE", "icon_pokeball_red.png", (14, 14)),
    ("EVOLUTION", "icon_green_stone.png", (14, 14)),
    ("HELD", "icon_key_gold.png", (14, 14)),
    ("TREASURE", "icon_chest_redgold.png", (14, 14)),
    ("OTHER", "icon_blue_disc.png", (14, 14)),
    ("TREASURE_ALTERNATE", "icon_gold_bundle.png", (14, 14)),
    ("TAB_LEFT", "arrow_tab_left_orange.png", (8, 12)),
    ("TAB_RIGHT", "arrow_tab_right_orange.png", (8, 12)),
    ("RETURN", "icon_return_arrow.png", (12, 12)),
    ("CURSOR", "cursor_pointer_yellow.png", (10, 14)),
    ("SCROLL_UP", "scroll_up_teal.png", (6, 4)),
    ("SCROLL_DOWN", "scroll_down_teal.png", (6, 4)),
]
GROUPS = [0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 2, 0, 2, 2]


def sample(filename, canvas, bounds):
    width, rows = read_rgba(SOURCE / filename)
    points = [(x, y) for y, row in enumerate(rows) for x in range(width) if row[x * 4 + 3] >= 128]
    assert points, filename
    left, right = min(x for x, _ in points), max(x for x, _ in points) + 1
    top, bottom = min(y for _, y in points), max(y for _, y in points) + 1
    scale = min(bounds[0] / (right - left), bounds[1] / (bottom - top))
    tw, th = max(1, round((right - left) * scale)), max(1, round((bottom - top) * scale))
    ox, oy = (canvas - tw) // 2, (canvas - th) // 2
    pixels = [None] * (canvas * canvas)
    for y in range(th):
        sy = min(bottom - 1, top + int((y + .5) * (bottom - top) / th))
        for x in range(tw):
            sx = min(right - 1, left + int((x + .5) * (right - left) / tw))
            rgba = rows[sy][sx * 4:sx * 4 + 4]
            if rgba[3] >= 128:
                pixels[(oy + y) * canvas + ox + x] = tuple(round(c * 31 / 255) for c in rgba[:3])
    return pixels, {"sourceSize": [width, len(rows)], "crop": [left, top, right, bottom],
                    "canvas": canvas, "fittedSize": [tw, th]}


def palette_for(images):
    histogram = Counter()
    for pixels in images:
        colors = Counter(p for p in pixels if p is not None)
        total = sum(colors.values())
        histogram.update({c: n / total for c, n in colors.items()})
    boxes = [list(histogram)]
    def spread(box):
        return max(max(c[i] for c in box) - min(c[i] for c in box) for i in range(3))
    while len(boxes) < 15:
        candidates = [i for i, b in enumerate(boxes) if len(b) > 1]
        if not candidates:
            break
        index = max(candidates, key=lambda i: spread(boxes[i]) * sum(histogram[c] for c in boxes[i]))
        box = boxes.pop(index)
        axis = max(range(3), key=lambda i: max(c[i] for c in box) - min(c[i] for c in box))
        box.sort(key=lambda c: (c[axis], c))
        half, weight, split = sum(histogram[c] for c in box) / 2, 0, 1
        for split in range(1, len(box)):
            weight += histogram[box[split - 1]]
            if weight >= half:
                break
        boxes += [box[:split], box[split:]]
    colors = []
    for box in boxes:
        total = sum(histogram[c] for c in box)
        colors.append(tuple(round(sum(c[i] * histogram[c] for c in box) / total) for i in range(3)))
    return [(0, 0, 0)] + colors + [(0, 0, 0)] * (15 - len(colors))


def tiles(pixels, size, palette):
    indices = [0 if p is None else 1 + min(range(15), key=lambda i: sum((p[k] - palette[i + 1][k]) ** 2 for k in range(3)))
               for p in pixels]
    output = bytearray()
    for ty in range(0, size, 8):
        for tx in range(0, size, 8):
            for y in range(8):
                for x in range(0, 8, 2):
                    i = (ty + y) * size + tx + x
                    output.append(indices[i] | indices[i + 1] << 4)
    assert len(output) == size * size // 2
    return output


def c_words(name, data):
    words = struct.unpack("<" + "I" * (len(data) // 4), data)
    return [f"static const u32 {name}[{len(words)}] = {{"] + [
        "    " + ", ".join(f"0x{v:08X}" for v in words[i:i + 8]) + ","
        for i in range(0, len(words), 8)] + ["};"]


def legacy_main():
    images, manifest = [], []
    for name, filename, bounds in ASSETS:
        pixels, info = sample(filename, 16, bounds)
        images.append(pixels)
        manifest.append({"frame": name, "file": filename, "sha256": hashlib.sha256((SOURCE / filename).read_bytes()).hexdigest(), **info})
    big, _ = sample("icon_return_arrow.png", 32, (28, 28))
    palettes = [palette_for([p for i, p in enumerate(images) if GROUPS[i] == group]
                           + ([big] if group == 2 else [])) for group in range(3)]
    sheet = b"".join(tiles(p, 16, palettes[GROUPS[i]]) for i, p in enumerate(images))
    output = ["// Generated from user PNGs by import_bag_user_sprites.py.",
              "// Transparent index 0; three shared 16-color BGR555 palettes.",
              "static const u16 sRFBagUserPalettes[3][16] = {"]
    output += ["    {" + ", ".join(f"0x{r | g << 5 | b << 10:04X}" for r, g, b in palette) + "}," for palette in palettes]
    output += ["};", "static const u8 sRFBagUserPaletteGroups[14] = {" + ", ".join(map(str, GROUPS)) + "};"]
    output += c_words("sRFBagUserTiles", sheet)
    output += c_words("sRFBagUserReturnTiles", tiles(big, 32, palettes[2]))
    diagnostic = ROOT / "build_artifacts/ui/kanto-art"
    diagnostic.mkdir(parents=True, exist_ok=True)
    (diagnostic / "legacy-bag-art.h").write_text("\n".join(output) + "\n", encoding="utf-8")
    (diagnostic / "legacy-sampler-manifest.json").write_text(json.dumps({"palettesRGB555": palettes, "groups": GROUPS, "sprites": manifest,
        "smallSheetBytes": len(sheet), "returnSheetBytes": 512}, indent=2) + "\n", encoding="utf-8")
    print(f"Converted {len(ASSETS)} assets: {len(sheet) + 512} tile bytes, 3 shared palettes.")


def main():
    from make_kanto_ui_art import main as build_native
    build_native()


if __name__ == "__main__":
    # Keep the legacy sampler available for diagnostics, but never shrink user
    # illustrations over the reviewed native drawings during a normal rebuild.
    import sys
    if "--legacy-reference" in sys.argv:
        legacy_main()
    else:
        main()
