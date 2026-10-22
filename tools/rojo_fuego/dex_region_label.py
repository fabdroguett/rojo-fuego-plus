#!/usr/bin/env python3
# Repaints the HGSS Pokédex list's region label sprite (graphics/pokedex/hgss/tileset_interface.png,
# tiles 160-167) from HOENN to KANTO, in the same pixel style: dark strokes, a grey pixel to their
# right and a grey row under the last line. The underline below (rows 12-13) is kept.
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pngutil

PNG = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'graphics', 'pokedex', 'hgss', 'tileset_interface.png')
FIRST_TILE, WIDTH, HEIGHT = 160, 32, 16
STROKE, SHADOW = 15, 4
LABEL_ROWS = 12 # rows below this are the underline
TOP = 4

GLYPHS = {
    'K': ['1..1', '1.1.', '11..', '11..', '1.1.', '1..1', '1..1'],
    'A': ['.11.', '1..1', '1..1', '1111', '1..1', '1..1', '1..1'],
    'N': ['1..1', '11.1', '11.1', '1.11', '1.11', '1..1', '1..1'],
    'T': ['11111', '..1..', '..1..', '..1..', '..1..', '..1..', '..1..'],
    'O': ['.11.', '1..1', '1..1', '1..1', '1..1', '1..1', '.11.'],
}

w, h, pal, px = pngutil.read_png(PNG)
tiles_per_row = w // 8

def pixel_pos(x, y):
    # The sprite is 4x2 tiles stored one after another (1D mapping)
    tile = FIRST_TILE + (y // 8) * (WIDTH // 8) + x // 8
    ty, tx = divmod(tile, tiles_per_row)
    return ty * 8 + y % 8, tx * 8 + x % 8

canvas = [[0] * WIDTH for _ in range(HEIGHT)]
x = 0
for ch in 'KANTO':
    glyph = GLYPHS[ch]
    for gy, row in enumerate(glyph):
        for gx, bit in enumerate(row):
            if bit == '1':
                canvas[TOP + gy][x + gx] = STROKE
    x += len(glyph[0]) + 1
for y in range(TOP, TOP + 7):
    for cx in range(WIDTH - 1):
        if canvas[y][cx] == STROKE and canvas[y][cx + 1] == 0:
            canvas[y][cx + 1] = SHADOW
for cx in range(WIDTH):
    if canvas[TOP + 6][cx] != 0:
        canvas[TOP + 7][cx] = SHADOW

for y in range(LABEL_ROWS):
    for cx in range(WIDTH):
        py, pxx = pixel_pos(cx, y)
        px[py][pxx] = canvas[y][cx]
pngutil.write_png(PNG, pal, px)
print('KANTO label written')
