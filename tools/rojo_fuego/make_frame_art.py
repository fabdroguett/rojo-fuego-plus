#!/usr/bin/env python3
"""Pokémon Ultimate Kanto window frame (option "MARCO" 1, the default).

The frame shares its 16-colour palette with the text drawn inside the window, and the night theme
(RFUI_LoadWindowPalette) repaints indices 1-3 as panel, ink and shadow. FireRed's frame used 1-8 for
its bevel, which turned into white and black bands. This frame only uses the border indices
10-15: a 1 px edge (13), a panel band (11) and a highlight on the top inner line (12).
"""
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from make_kanto_ui_art import png  # noqa: E402
from make_title_art import rgb  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
TARGET = ROOT / 'graphics/text_window/1.png'

# Day colours for the same roles (the light theme loads this palette as is).
PALETTE = [rgb(c) for c in ('#90c8ff', '#fffff7', '#212931', '#dee6de', '#e8483a', '#f7b0a8', '#2f8f5a', '#a8e0c0',
                            '#3060c0', '#a8c8f0', '#a5adad', '#fffff7', '#e6efe6', '#5a636b', '#fffff7', '#212931')]
EDGE, PANEL, HIGHLIGHT = 13, 11, 12


def main():
    w = h = 24
    px = [0] * (w * h)
    # Outer edge on a 22x22 square inset by 1 px, with cut corners.
    lo, hi = 1, 22
    for y in range(lo, hi + 1):
        for x in range(lo, hi + 1):
            corner = (y in (lo, hi)) and (x in (lo, hi))
            if corner:
                continue
            if y in (lo, hi) or x in (lo, hi):
                px[y * w + x] = EDGE
            else:
                px[y * w + x] = PANEL
    for x in range(lo + 1, hi):
        px[(lo + 1) * w + x] = HIGHLIGHT  # top inner line catches the light
    png(TARGET, px, w, h, PALETTE, indexed=True)
    print('frame art: MARCO 1')


if __name__ == '__main__':
    main()
