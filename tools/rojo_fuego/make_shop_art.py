#!/usr/bin/env python3
"""Pokémon Ultimate Kanto shop (buy menu) background: four night panels over the map.

Writes graphics/shop/menu.png (tile sheet, tile 0 blank) and graphics/shop/menu.bin (32x32 tilemap,
palette bits left to the code). Layout, in tiles (the windows in src/shop.c match it):
  list panel   x 0-15,  y 0-12  header "TIENDA" + item list
  money panel  x 17-29, y 0-2
  info panel   x 17-29, y 3-12  item icon, quantity in bag, quantity and price
  text panel   x 0-29,  y 13-19 description
The code loads at most 0x3A0 bytes (29 tiles) of this sheet at tile 0x3E3.
"""
from pathlib import Path
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from make_kanto_ui_art import font, png  # noqa: E402
from make_title_art import flips, rgb, tile_of  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
DIR = ROOT / 'graphics/shop'
MAX_TILES = 0x3A0 // 32

# 1 panel (= RF_UI_NIGHT_PANEL after 5-bit rounding), 2 edge, 3 top highlight, 4 ember accent, 5 header ink, 6 header shadow.
PALETTE = [rgb(c) for c in ('#000000', '#201828', '#6b5470', '#2c2231', '#ffb24d', '#f7efde', '#0a080d', '#000000',
                            '#000000', '#000000', '#000000', '#000000', '#000000', '#000000', '#000000', '#000000')]
PANELS = [(0, 0, 16, 13), (17, 0, 13, 3), (17, 3, 13, 10), (0, 13, 30, 7)]


def label(img, text, x, y):
    """Header text in the UI font, proportional spacing, ink 5 with a shadow 6."""
    small = font()
    for ch in text:
        rows = small[ch]
        cols = [gx for gx in range(5) if any(bits & (16 >> gx) for bits in rows)]
        lo, hi = (min(cols), max(cols)) if cols else (0, 2)
        for gy, bits in enumerate(rows):
            for gx in range(lo, hi + 1):
                if bits & (16 >> gx):
                    img[y + gy + 1][x + gx - lo + 1] = 6
                    img[y + gy][x + gx - lo] = 5
        x += hi - lo + 2


def draw():
    img = [[0] * 256 for _ in range(160)]
    for tx, ty, tw, th in PANELS:
        x0, y0, x1, y1 = tx * 8 + 1, ty * 8 + 1, (tx + tw) * 8 - 2, (ty + th) * 8 - 2
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                if (y in (y0, y1)) and (x in (x0, x1)):
                    continue
                img[y][x] = 2 if (y in (y0, y1) or x in (x0, x1)) else (3 if y == y0 + 1 else 1)
    # List header: "TIENDA" and an ember rule under it.
    label(img, 'TIENDA', 8, 4)
    label(img, 'DINERO', 17 * 8 + 6, 9)
    for x in range(4, 124):
        img[14][x] = 4
    return img


def main():
    img = draw()
    tiles = [tuple([0] * 64)]
    index = {tiles[0]: (0, 0)}
    entries = []
    for ty in range(32):
        for tx in range(32):
            if ty >= 20 or tx >= 30:
                entries.append(0)
                continue
            t = tile_of(img, tx, ty)
            if t not in index:
                tiles.append(t)
                for variant, flip in flips(t).items():
                    index.setdefault(variant, (len(tiles) - 1, flip))
            i, flip = index[t]
            entries.append(0 if i == 0 else i | (flip << 10))
    assert len(tiles) <= MAX_TILES, f'{len(tiles)} tiles; the shop loads at most {MAX_TILES}'
    sheet_w = 8 * len(tiles)
    pixels = [0] * (sheet_w * 8)
    for i, t in enumerate(tiles):
        for y in range(8):
            for x in range(8):
                pixels[y * sheet_w + i * 8 + x] = t[y * 8 + x]
    png(DIR / 'menu.png', pixels, sheet_w, 8, PALETTE, indexed=True)
    (DIR / 'menu.bin').write_bytes(struct.pack('<1024H', *entries))
    print('shop art: tiles', len(tiles))


if __name__ == '__main__':
    main()
