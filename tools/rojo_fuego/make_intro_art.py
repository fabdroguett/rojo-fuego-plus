#!/usr/bin/env python3
"""Pokémon Ultimate Kanto new-game intro art, authored at native 240x160.

Writes src/data/rf_intro_art.h: the professor's lab at dusk (BG1 tiles, tilemap and palette), the
controls-guide palette in the same night tones, and the dusk platform palette. Also rewrites
graphics/oak_speech/platform.pal and a preview in build_artifacts/ui-redesign/intro-preview.png.
Pure Python, like the other art generators.
"""
from pathlib import Path
import math
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from make_title_art import (BAYER, bgr, c_u16, c_u32, hash01, pack4, rgb, tilemap,  # noqa: E402
                            write_rgb_png)

ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / 'src/data/rf_intro_art.h'
PREVIEW = ROOT / 'build_artifacts/ui-redesign/intro-preview.png'
W, H = 240, 160

# Lab palette: 0 backdrop, 1-6 dusk sky top to horizon, 7 far hills, 8 wall dark, 9 wall,
# 10 shelf wood, 11 warm spines, 12 cool spines, 13 floor, 14 floor seam, 15 window frame.
LAB = [rgb(c) for c in ('#141019', '#1b1832', '#2c1f45', '#4e2550', '#7d2f4c', '#b54445', '#e9794a',
                        '#3a2140', '#1c1520', '#271d2c', '#3b2a33', '#c25a3a', '#6b5470', '#231a26',
                        '#30242f', '#4d3b4f')]
# Controls guide and welcome pages: 0 backdrop, 1-8 panel ramp (edge to fill), 9-10 unused,
# 11 green, 12 red, 13 blue, 14 white, 15 dark (as in the original sheet).
GUIDE = [rgb(c) for c in ('#141019', '#4d3b4f', '#3e3042', '#342839', '#2c2231', '#282030', '#241c2a',
                          '#211a25', '#1d1721', '#ff00ff', '#ff00ff', '#73dead', '#f76352', '#ffb24d',
                          '#f7efde', '#0a080d')]
# Welcome pages (palette slot 2 of the sheet): edge, panel ramp, watermark and dotted rules.
WELCOME = [rgb(c) for c in ('#141019', '#6b5470', '#4d3b4f', '#2c2231', '#211a25', '#261e2b', '#3e3042',
                            '#4d3b4f', '#3a2e3d', '#261e2b', '#000000', '#000000', '#000000', '#000000',
                            '#000000', '#000000')]
# Platform sprite: same index roles as graphics/oak_speech/platform.png, warm stone at dusk.
PLATFORM = [rgb(c) for c in ('#bdbda4', '#3b2a3a', '#45313f', '#4f3845', '#5c404b', '#6b4a52', '#7a545a',
                             '#8a5f60', '#9c6b66', '#000000', '#ff00ff', '#6e3c2a', '#82482f', '#985636',
                             '#ad6640', '#5c404b')]


def canvas(fill=0):
    return [[fill] * 256 for _ in range(160)]


def draw_lab():
    img = canvas(9)
    # Back wall with three tall windows onto the dusk; the windows show sky and far hills.
    windows = [(18, 54), (93, 54), (168, 54)]
    stops = [(0, 1), (14, 2), (28, 3), (42, 4), (54, 5), (66, 6)]
    for wx, ww in windows:
        for y in range(10, 78):
            for x in range(wx, wx + ww):
                v = y - 10
                lo = max(s for s in stops if s[0] <= v)
                hi = next((s for s in stops if s[0] > v), None)
                c = lo[1]
                if hi and (v - lo[0]) / (hi[0] - lo[0]) * 16 > BAYER[y % 4][x % 4] + 4:
                    c = hi[1]
                img[y][x] = c
            hill = int(66 + 4 * math.sin((x + wx) / 9.0) + 2 * math.sin((x + wx) / 3.7))
        for x in range(wx, wx + ww):
            top = int(64 + 4 * math.sin(x / 9.0) + 2 * math.sin(x / 3.7))
            for y in range(top, 78):
                img[y][x] = 7
        for i in range(5):  # a few early stars
            sx, sy = wx + int(hash01(3, wx, i) * ww), 12 + int(hash01(5, wx, i) * 16)
            img[sy][sx] = 6
        # Frame and mullions.
        for y in range(8, 80):
            for x in (wx - 2, wx - 1, wx + ww, wx + ww + 1):
                img[y][x] = 15
            img[y][wx + ww // 2] = 15
        for x in range(wx - 2, wx + ww + 2):
            for y in (8, 9, 42, 78, 79):
                img[y][x] = 15
    # Low bookshelves under the windows, with warm and cool spines.
    for x in range(0, 256):
        for y in range(80, 100):
            img[y][x] = 10 if y in (80, 89, 99) else 8
    for x in range(2, 254, 3):
        for shelf_top, shelf_bottom in ((81, 89), (90, 99)):
            h = 4 + int(hash01(7, x, shelf_top) * 4)
            c = 11 if hash01(11, x, shelf_top) > 0.6 else 12 if hash01(13, x, shelf_top) > 0.4 else 9
            for y in range(shelf_bottom - h, shelf_bottom):
                img[y][x] = c
                img[y][x + 1] = c if hash01(17, x) > 0.3 else 8
    # Floor: boards that recede toward the window wall.
    for y in range(100, 160):
        for x in range(256):
            img[y][x] = 13
        spacing = 6 + (y - 100) // 6
        if (y - 100) % spacing == 0:
            for x in range(256):
                img[y][x] = 14
    for k in range(-8, 9):
        for y in range(100, 160):
            x = int(120 + k * (14 + (y - 100) * 0.9))
            if 0 <= x < 256:
                img[y][x] = 14
    # Warm light pooled where the professor stands.
    for y in range(102, 116):
        half = int((1 - ((y - 109) / 7.5) ** 2) ** 0.5 * 60)
        for x in range(120 - half, 120 + half):
            if img[y][x] in (13, 14):
                img[y][x] = 10 if (x + y) % 2 == 0 or abs(x - 120) < half // 2 else 14
    return img


def pal_jasc(colors):
    return 'JASC-PAL\r\n0100\r\n16\r\n' + ''.join(f'{c[0] * 255 // 31} {c[1] * 255 // 31} {c[2] * 255 // 31}\r\n' for c in colors)


def main():
    lab = draw_lab()
    tiles, entries = tilemap(lab, 0, 32)
    assert len(tiles) <= 512, len(tiles)
    out = ['// Generated by tools/rojo_fuego/make_intro_art.py; authored at native resolution.',
           f'// Lab tiles: {len(tiles)}.', '']
    out += c_u16('sRFIntroLabPal', [bgr(c) for c in LAB]) + ['']
    out += c_u16('sRFIntroGuidePal', [bgr(c) for c in GUIDE]) + ['']
    out += c_u16('sRFIntroWelcomePal', [bgr(c) for c in WELCOME]) + ['']
    out += c_u32('sRFIntroLabTiles', pack4(tiles)) + ['']
    out += c_u16('sRFIntroLabMap', entries) + ['']
    HEADER.write_text('\n'.join(out), encoding='utf-8')
    (ROOT / 'graphics/oak_speech/platform.pal').write_bytes(pal_jasc(PLATFORM).encode('ascii'))
    preview = [LAB[lab[y][x]] for y in range(H) for x in range(W)]
    palette = sorted(set(preview))
    write_rgb_png(PREVIEW, [palette.index(c) for c in preview], W, H, palette)
    print('intro art: lab tiles', len(tiles))


if __name__ == '__main__':
    main()
