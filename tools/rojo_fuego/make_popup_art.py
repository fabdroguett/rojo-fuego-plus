#!/usr/bin/env python3
"""Pokémon Ultimate Kanto location popup: a night panel with a fire accent bar.

Rewrites graphics/map_popup/wood.png (the 80x24 panel bitmap) and wood_outline.png (the 30 frame
tiles), the theme every Kanto map uses. Both carry the popup palette (BG palette 14), which also
colours the map name: 2 text, 3 shadow.
"""
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from make_kanto_ui_art import png  # noqa: E402
from make_title_art import rgb  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
DIR = ROOT / 'graphics/map_popup'

PALETTE = [rgb(c) for c in ('#000000', '#000000', '#f7efde', '#0a080d', '#000000', '#000000', '#000000', '#000000',
                            '#6b5470', '#000000', '#000000', '#211a25', '#2c2231', '#f76352', '#141019', '#000000')]
EDGE, PANEL, HIGHLIGHT, ACCENT, DARK = 8, 11, 12, 13, 14


def panel():
    w, h = 80, 24
    px = [PANEL] * (w * h)
    for x in range(w):
        px[x] = HIGHLIGHT
        px[(h - 1) * w + x] = DARK
    for y in range(h):
        for x in range(3):
            px[y * w + x] = ACCENT
    return px


def outline():
    """Tile order (10 tiles per row): 12 top-edge tiles, 6 side tiles (L/R top, mid, bottom), 12 bottom."""
    w, h = 80, 24
    px = [0] * (w * h)

    def tile(i):
        return (i % 10) * 8, (i // 10) * 8

    def put(i, x, y, c=EDGE):
        tx, ty = tile(i)
        px[(ty + y) * w + tx + x] = c

    for i in range(12):  # top edge: a line on the last row; corners only meet the sides
        for x in range(8):
            if (i == 0 and x < 7) or (i == 11 and x > 0):
                continue
            put(i, x, 7)
    for k, side in enumerate(('L', 'R') * 3):  # sides
        for y in range(8):
            put(12 + k, 7 if side == 'L' else 0, y)
    for i in range(12):  # bottom edge: a line on the first row
        for x in range(8):
            if (i == 0 and x < 7) or (i == 11 and x > 0):
                continue
            put(18 + i, x, 0)
    return px


def main():
    png(DIR / 'wood.png', panel(), 80, 24, PALETTE, indexed=True)
    png(DIR / 'wood_outline.png', outline(), 80, 24, PALETTE, indexed=True)
    print('popup art: panel and frame')


if __name__ == '__main__':
    main()
