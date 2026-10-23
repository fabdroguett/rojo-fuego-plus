"""Rasterize a pixel TTF into the bitmap JSON that make_rf_fonts.py reads.

Pixel fonts such as m5x7 and m6x11plus (Daniel Linssen) are drawn on a 1 px grid at 16 px.
Each charmap.txt character the font defines becomes rows of bits (bit 0 = leftmost pixel),
like Monogram's own monogram-bitmap.json, plus "baseline": the row of the bottom of 'A'.
Needs Pillow and fontTools, unlike the rest of the font tools; run once per font update:
python tools/rojo_fuego/rasterize_font.py FONT.ttf OUT.json
"""
from pathlib import Path
import json
import re
import sys

from fontTools.ttLib import TTFont
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[2]
SIZE, BOX = 16, 24


def main():
    ttf, out = Path(sys.argv[1]), Path(sys.argv[2])
    cmap = TTFont(ttf).getBestCmap()
    font = ImageFont.truetype(str(ttf), SIZE)
    chars = []
    for line in (ROOT / "charmap.txt").read_text(encoding="utf-8").splitlines():
        m = re.match(r"^'(\\'|.)'\s*=", line)
        if m:
            char = "'" if m.group(1) == "\\'" else m.group(1)
            if ord(char) in cmap and char not in chars and char != " ":
                chars.append(char)
    glyphs = {}
    for char in chars:
        image = Image.new("1", (BOX, BOX), 0)
        draw = ImageDraw.Draw(image)
        draw.fontmode = "1"
        draw.text((0, 0), char, font=font, fill=1)
        rows = [sum(1 << x for x in range(BOX) if image.getpixel((x, y))) for y in range(BOX)]
        glyphs[char] = rows
    baseline = max(y for y, bits in enumerate(glyphs["A"]) if bits)
    data = {"source": ttf.name, "size": SIZE, "baseline": baseline, "glyphs": glyphs}
    out.write_text(json.dumps(data, ensure_ascii=False, separators=(",", ":")), encoding="utf-8")
    print(f"{out}: {len(glyphs)} glyphs, baseline row {baseline}")


if __name__ == "__main__":
    main()
