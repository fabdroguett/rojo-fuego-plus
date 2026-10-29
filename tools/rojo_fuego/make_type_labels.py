#!/usr/bin/env python3
"""Spanish labels on the 32x16 type icons in graphics/types/*.png (summary, Pokédex, move relearner).

The icons' own condensed bold letters are cut out of the original English labels (ink = colour 15) and
reused, with V, Q and Z drawn in the same style and a one-pixel accent for Á É Í Ó Ú. Each Spanish name
is centred on the face (colour 2) with a shadow (colour 14) one pixel right and below.

usage: make_type_labels.py ORIGINAL_ICON_DIR   (the untouched English icons, e.g. from git)
"""
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from pngutil import read_png, write_png  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
DIR = ROOT / 'graphics/types'
ENGLISH = {
    'normal': 'NORMAL', 'fight': 'FIGHT', 'flying': 'FLYING', 'poison': 'POISON', 'ground': 'GROUND',
    'rock': 'ROCK', 'bug': 'BUG', 'ghost': 'GHOST', 'steel': 'STEEL', 'fire': 'FIRE', 'water': 'WATER',
    'grass': 'GRASS', 'electric': 'ELECTR', 'psychic': 'PSYCHC', 'ice': 'ICE', 'dragon': 'DRAGON',
    'dark': 'DARK', 'fairy': 'FAIRY', 'contest_cool': 'COOL', 'contest_beauty': 'BEAUTY',
    'contest_cute': 'CUTE', 'contest_smart': 'SMART', 'contest_tough': 'TOUGH',
}
NAMES = {
    'normal': ('NORMAL',), 'fight': ('LUCHA',), 'flying': ('VOLADOR', 'VOLAD.'), 'poison': ('VENENO',),
    'ground': ('TIERRA',), 'rock': ('ROCA',), 'bug': ('BICHO',), 'ghost': ('FANTASMA', 'FANTAS.'),
    'steel': ('ACERO',), 'fire': ('FUEGO',), 'water': ('AGUA',), 'grass': ('PLANTA',),
    'electric': ('ELÉCTRICO', 'ELÉCTR.', 'ELÉC.'), 'psychic': ('PSÍQUICO', 'PSÍQUI.', 'PSÍQ.'), 'ice': ('HIELO',),
    'dragon': ('DRAGÓN',), 'dark': ('SINIESTRO', 'SINIES.', 'SINI.'), 'fairy': ('HADA',),
    'stellar': ('ASTRAL',), 'mystery': ('???',),
    'contest_cool': ('CARISMA', 'CARIS.'), 'contest_beauty': ('BELLEZA', 'BELLE.'), 'contest_cute': ('DULZURA', 'DULZ.'),
    'contest_smart': ('INGENIO', 'INGEN.'), 'contest_tough': ('DUREZA', 'DUREZ.'),
}
HAND = {  # 7 rows, '#' = ink
    'V': ['#..#', '#..#', '#..#', '#..#', '#..#', '.##.', '.##.'],
    'Q': ['.##.', '#..#', '#..#', '#..#', '#.##', '#..#', '.###'],
    'Z': ['####', '...#', '..#.', '.##.', '.#..', '#...', '####'],
    '.': ['.', '.', '.', '.', '.', '.', '#'],
    '?': ['.##.', '#..#', '...#', '..#.', '.#..', '....', '.#..'],
}
FACE, INK, SHADE = 2, 15, 14
X0, X1, TOP = 1, 30, 4  # the whole face (colour 2) between the rims
ACCENTS = {'Á': 'A', 'É': 'E', 'Í': 'I', 'Ó': 'O', 'Ú': 'U'}


def cut_letters(orig_dir):
    glyphs = {}
    for name, word in ENGLISH.items():
        path = Path(orig_dir) / f'{name}.png'
        if not path.exists():
            continue
        w, h, pal, px = read_png(path)
        inked = [x for x in range(w) if any(px[y][x] == INK for y in range(TOP, TOP + 7))]
        runs, cur = [], []
        for x in inked:
            if cur and x != cur[-1] + 1:
                runs.append(cur)
                cur = []
            cur.append(x)
        if cur:
            runs.append(cur)
        if len(runs) != len(word):
            continue
        for ch, run in zip(word, runs):
            if ch in glyphs:
                continue
            glyphs[ch] = [''.join('#' if px[TOP + y][x] == INK else '.' for x in run) for y in range(7)]
    glyphs.update({k: v for k, v in HAND.items() if k not in glyphs or k in '.?'})
    return glyphs


def glyph(ch, glyphs):
    return glyphs[ACCENTS.get(ch, ch)]


def wordw(word, glyphs):
    return sum(len(glyph(ch, glyphs)[0]) + 1 for ch in word) - 1


def main():
    glyphs = cut_letters(sys.argv[1])
    for name, words in NAMES.items():
        path = DIR / f'{name}.png'
        if not path.exists():
            continue
        missing = {ACCENTS.get(c, c) for w in words for c in w} - set(glyphs)
        word = next((wd for wd in words if not ({ACCENTS.get(c, c) for c in wd} - set(glyphs))
                     and wordw(wd, glyphs) <= X1 - X0 + 1), None)
        if word is None:
            print('no fit', name, missing)
            continue
        w, h, pal, px = read_png(Path(sys.argv[1]) / f'{name}.png')
        for y in range(2, 13):
            for x in range(X0, X1 + 1):
                if px[y][x] in (INK, SHADE):
                    px[y][x] = FACE
        x = X0 + (X1 - X0 + 1 - wordw(word, glyphs)) // 2
        for ch in word:
            rows = glyph(ch, glyphs)
            gw = len(rows[0])
            for dx, dy, colour in ((1, 1, SHADE), (0, 0, INK)):
                for gy, row in enumerate(rows):
                    for gx, cell in enumerate(row):
                        if cell == '#':
                            px[TOP + gy + dy][x + gx + dx] = colour
                if ch in ACCENTS:  # accent tick above the letter, on the face row
                    px[TOP - 2 + dy][x + gw // 2 + dx] = colour
                    px[TOP - 2 + dy][x + gw // 2 + 1 + dx] = colour if dx == 0 else px[TOP - 2 + dy][x + gw // 2 + 1 + dx]
            x += gw + 1
        write_png(path, pal, px)
        print(name, word)


if __name__ == '__main__':
    main()
