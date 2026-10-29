#!/usr/bin/env python3
"""Repaint the summary screen's section headings (drawn inside graphics/summary_screen/*tiles.png) in Spanish.

Each heading sits on one 8 px tile row of the sheet; the word is erased to that row's background and the
Spanish word is drawn with the UI font in the original letter and shadow colours, never wider than the
original word (the page tilemaps only show that many tiles). Run it on the original sheets (it reads the old word to size the new one).
"""
from collections import Counter
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from make_kanto_ui_art import font  # noqa: E402
from pngutil import read_png, write_png  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
# (tile row, first x, last x, Spanish word)
HEADINGS = [  # candidates in order of preference; the first that fits is used
    (8, 0, 71, ('PERFIL',)), (9, 0, 71, ('HABILIDAD', 'HABILID.', 'HABIL.')), (11, 0, 71, ('ESTADO', 'EST.')),
    (12, 0, 71, ('ATAQUES', 'MOVS.')), (13, 0, 71, ('DESCRIPCIÓN', 'DESCRIPCION')), (14, 0, 71, ('NOTAS DEL ENTRENADOR', 'NOTAS')),
    (8, 72, 127, ('OBJETO', 'OBJ.')), (9, 72, 127, ('CINTAS',)), (10, 72, 127, ('VALORES', 'STATS')),
    (11, 72, 127, ('EXP.', 'EXP')), (12, 80, 127, ('EFECTO',)),
]

def glyph_cols(rows):
    cols = [gx for gx in range(5) if any(bits & (16 >> gx) for bits in rows)]
    return (min(cols), max(cols)) if cols else (0, 1)


def word_width(word, glyphs):
    return sum(glyph_cols(glyphs[c])[1] - glyph_cols(glyphs[c])[0] + 2 for c in word) - 1


def repaint(path):
    w, h, pal, px = read_png(path)
    glyphs = font()
    top = min(gy for rows in glyphs.values() for gy, bits in enumerate(rows) if bits)  # first inked row
    for row, x0, x1, words in HEADINGS:
        y0 = row * 8
        region = [(x, y) for y in range(y0, y0 + 8) for x in range(x0, x1 + 1)]
        counts = Counter(px[y][x] for x, y in region)
        # Letters use colour 2 of their bank and colour 6 for the shadow; the rest is the row's ground.
        near = Counter(px[y][x] for x, y in region if x < x0 + 16)  # the word's own ground, not a neighbour's
        bg = next(v for v, _ in near.most_common() if v % 16 not in (2, 6))
        # The letter pixels keep their own raw values (often bank 0) whatever bank the ground uses.
        # In the night theme colour 6 is the gold heading and 2 the panel, so 6 draws the letter face.
        shade = next((v for v, _ in counts.most_common() if v % 16 == 2), 2)
        ink = next((v for v, _ in counts.most_common() if v % 16 == 6), 6)
        cols = sorted({x for x, y in region if px[y][x] in (ink, shade)})
        start = cols[0] if cols else x0
        used = [start]
        for x in cols[1:]:  # only the original word: stop at the first gap wider than a space
            if x - used[-1] > 6:
                break
            used.append(x)
        # The tilemap shows every tile the original word touched, so the word may fill those tiles.
        budget = ((max(used) // 8 + 1) * 8 - start - 1) if used else (x1 - x0)
        fitting = [wd for wd in words if word_width(wd, glyphs) <= budget]
        assert fitting, (words, budget)
        word = fitting[0]
        print(row, word, word_width(word, glyphs), '/', budget)
        for x, y in region:
            px[y][x] = bg
        x = start
        for ch in word:
            lo, hi = glyph_cols(glyphs[ch])
            for gy, bits in enumerate(glyphs[ch]):
                for gx in range(lo, hi + 1):
                    if bits & (16 >> gx) and gy - top + 1 < 8:
                        px[y0 + gy - top + 1][x + gx - lo + 1] = shade
            for gy, bits in enumerate(glyphs[ch]):
                for gx in range(lo, hi + 1):
                    if bits & (16 >> gx) and gy - top < 8:
                        px[y0 + gy - top][x + gx - lo] = ink
            x += hi - lo + 2
    write_png(path, pal, px, depth=8)


for name in ('tiles.png', 'iv_ev_tiles.png'):
    repaint(ROOT / 'graphics/summary_screen' / name)
print('summary headings repainted')
