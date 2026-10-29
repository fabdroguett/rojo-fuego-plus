#!/usr/bin/env python3
"""Repaint English labels drawn inside tile sheets with Spanish text (Pokémon Ultimate Kanto).

A label is a strip of tiles (as the screen tilemap lays them out, which may wrap across sheet rows).
The strip is gathered into one image, its text area is cleared to the ground colour, the new text is
centred in the original letter and shadow colours, and the tiles are written back. Run on the original
sheets; the tool reads the old lettering to pick colours.
"""
from collections import Counter
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from make_kanto_ui_art import font  # noqa: E402
from pngutil import read_png, write_png  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]


def strip(tiles_rows):
    """tiles_rows: one list of (col, row) sheet tiles per strip row."""
    return tiles_rows


# PC storage: CLOSE BOX (normal and greyed), laid out by graphics/pokemon_storage/close_box_button.bin.
_CLOSE_TOP = [(c, 4) for c in range(12, 16)] + [(c, 7) for c in range(0, 5)]
_CLOSE_BOTTOM = [(c, 5) for c in range(12, 16)] + [(c, 8) for c in range(0, 5)]
LABELS = {
    'graphics/pokemon_storage/menu.png': [
        dict(tiles=[_CLOSE_TOP, _CLOSE_BOTTOM], text='SALIR', area=(3, 4, 66, 12)),
        dict(tiles=[[(c, 7) for c in range(5, 14)], [(c, 8) for c in range(5, 14)]], text='SALIR', area=(3, 4, 66, 12)),
        # PARTY POKéMON and CANCEL, laid out by graphics/pokemon_storage/party_menu.bin (rows 20-21, 17-18).
        dict(tiles=[[(c, 2) for c in range(8, 16)] + [(6, 6)], [(c, 3) for c in range(8, 16)] + [(7, 6)]],
             text='EQUIPO', area=(5, 4, 67, 11)),
        dict(tiles=[[(9, 4), (10, 4), (11, 4), (9, 6)], [(9, 5), (10, 5), (11, 5), (10, 6)]], text='ATRÁS', area=(2, 4, 29, 12)),
        # PKMN DATA heading, laid out by graphics/pokemon_storage/pkmn_data.bin.
        dict(tiles=[[(c, 0) for c in range(1, 9)], [(c, 1) for c in range(1, 9)]], text='DATOS', area=(5, 3, 59, 12)),
    ],
}


def glyph_cols(rows):
    cols = [gx for gx in range(5) if any(bits & (16 >> gx) for bits in rows)]
    return (min(cols), max(cols)) if cols else (0, 1)


def gather(px, tiles):
    h = 8 * len(tiles)
    w = 8 * len(tiles[0])
    img = [[0] * w for _ in range(h)]
    for ry, row in enumerate(tiles):
        for rx, (c, r) in enumerate(row):
            for y in range(8):
                for x in range(8):
                    img[ry * 8 + y][rx * 8 + x] = px[r * 8 + y][c * 8 + x]
    return img


def scatter(px, tiles, img):
    for ry, row in enumerate(tiles):
        for rx, (c, r) in enumerate(row):
            for y in range(8):
                for x in range(8):
                    px[r * 8 + y][c * 8 + x] = img[ry * 8 + y][rx * 8 + x]


def relabel(img, text, area, glyphs):
    x0, y0, x1, y1 = area
    counts = Counter(img[y][x] for y in range(y0, y1 + 1) for x in range(x0, x1 + 1))
    ground = counts.most_common(1)[0][0]
    others = [v for v, _ in counts.most_common() if v != ground][:2]
    # The letter face is the more frequent of the remaining two colours, the shadow the other one.
    ink = others[0] if others else ground
    shade = others[1] if len(others) > 1 else ground
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            img[y][x] = ground
    top = min(gy for rows in glyphs.values() for gy, bits in enumerate(rows) if bits)
    width = sum(glyph_cols(glyphs[c])[1] - glyph_cols(glyphs[c])[0] + 2 for c in text) - 1
    x = x0 + max(0, (x1 - x0 + 1 - width) // 2)
    ty = y0 + max(0, (y1 - y0 + 1 - 8) // 2)
    for ch in text:
        lo, hi = glyph_cols(glyphs[ch])
        for gy, bits in enumerate(glyphs[ch]):
            for gx in range(lo, hi + 1):
                if bits & (16 >> gx):
                    yy = ty + gy - top
                    if yy + 1 <= y1:
                        img[yy + 1][x + gx - lo + 1] = shade
        for gy, bits in enumerate(glyphs[ch]):
            for gx in range(lo, hi + 1):
                if bits & (16 >> gx):
                    yy = ty + gy - top
                    if yy <= y1:
                        img[yy][x + gx - lo] = ink
        x += hi - lo + 2


def main():
    glyphs = font()
    for rel, labels in LABELS.items():
        w, h, pal, px = read_png(ROOT / rel)
        for label in labels:
            img = gather(px, label['tiles'])
            relabel(img, label['text'], label['area'], glyphs)
            scatter(px, label['tiles'], img)
        write_png(ROOT / rel, pal, px, depth=8)
        print(rel, len(labels), 'labels')


if __name__ == '__main__':
    main()
