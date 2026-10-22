"""Convert Mikitari's permitted Castform art to shared-palette GBA battle assets.

Usage: python import_castform_sand.py "path/to/Fakemon Pack"
Source: https://eeveeexpo.com/resources/1517/ (free use; credit Mikitari).
Only CASTFORM_4 is imported; unrelated Fakemon in the pack are not used.
Requires Pillow. This script performs nearest-neighbor size/palette conversion only.
"""
from collections import Counter, defaultdict
from pathlib import Path
import argparse
import hashlib
import json

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]


def fit_sprite(image, baseline):
    image = image.convert("RGBA").resize((80, 80), Image.Resampling.NEAREST)
    box = image.getchannel("A").getbbox()
    if box is None:
        raise ValueError("Empty source sprite")
    sprite = image.crop(box)
    if sprite.width > 64 or sprite.height > baseline:
        scale = min(64 / sprite.width, baseline / sprite.height)
        sprite = sprite.resize((int(sprite.width * scale), int(sprite.height * scale)), Image.Resampling.NEAREST)
    canvas = Image.new("RGBA", (64, 64))
    canvas.alpha_composite(sprite, ((64 - sprite.width) // 2, baseline - sprite.height))
    return canvas


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    args = parser.parse_args()
    originals = {}
    frames = {}
    for kind, baseline in (("Front", 56), ("Back", 64)):
        for shiny in (False, True):
            folder = kind + (" shiny" if shiny else "")
            path = args.source / folder / "CASTFORM_4.png"
            originals[folder] = hashlib.sha256(path.read_bytes()).hexdigest()
            frames[kind, shiny] = fit_sprite(Image.open(path), baseline)
    normal_colors = sorted({pixel[:3] for kind in ("Front", "Back")
                            for pixel in frames[kind, False].get_flattened_data() if pixel[3]})
    if len(normal_colors) > 15:
        raise ValueError(f"Needs {len(normal_colors)} opaque colors; GBA limit is 15")
    colors = [(0, 0, 0)] + normal_colors
    colors += [(0, 0, 0)] * (16 - len(colors))
    color_ids = {color: index + 1 for index, color in enumerate(normal_colors)}
    shiny_votes = defaultdict(Counter)
    output = ROOT / "graphics/pokemon/castform/sandy"
    output.mkdir(parents=True, exist_ok=True)
    for kind, name in (("Front", "front.png"), ("Back", "back.png")):
        normal = list(frames[kind, False].get_flattened_data())
        shiny = list(frames[kind, True].get_flattened_data())
        indices = []
        for original, alternate in zip(normal, shiny):
            index = color_ids[original[:3]] if original[3] else 0
            indices.append(index)
            if index and alternate[3]:
                shiny_votes[index][alternate[:3]] += 1
        image = Image.new("P", (64, 64))
        image.putpalette([channel for color in colors for channel in color] + [0] * (768 - 48))
        image.putdata(indices)
        image.save(output / name, transparency=0, bits=4)
    shiny_colors = [shiny_votes[i].most_common(1)[0][0] if shiny_votes[i] else color
                    for i, color in enumerate(colors)]
    for name, palette in (("normal.pal", colors), ("shiny.pal", shiny_colors)):
        text = "JASC-PAL\n0100\n16\n" + "\n".join(" ".join(map(str, color)) for color in palette) + "\n"
        (output / name).write_text(text, encoding="ascii")
    (output / "source.json").write_text(json.dumps({
        "artist": "Mikitari", "resource": "https://eeveeexpo.com/resources/1517/",
        "permission": "Feel free to use in any project.", "source_files_sha256": originals,
        "conversion": "nearest-neighbor fit to 64x64; shared normal/shiny palette; original menu icon reused",
    }, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
