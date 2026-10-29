#!/usr/bin/env python3
"""Pokémon Ultimate Kanto naming screen: night palettes and Spanish button labels.

Rewrites the palettes in graphics/naming_screen/ (menu, keyboard, page swap and buttons) with the
night tokens, and redraws the labels baked into the button sprites ("lower", "others", "UPPER",
"BACK", "OK") in Spanish. Pure Python, like the other art generators.
"""
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from make_kanto_ui_art import font, png  # noqa: E402
from make_title_art import rgb  # noqa: E402
from pngutil import read_png  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
DIR = ROOT / 'graphics/naming_screen'

TEXT, SHADOW, MUTED = '#f7efde', '#0a080d', '#b3a5a6'
EDGE, EDGE_HI, PANEL, PANEL2, INK = '#4d3b4f', '#6b5470', '#211a25', '#2c2231', '#141019'
EMBER, EMBER_DK = '#ffb24d', '#b8622b'

# Index roles follow the original sheets; unused slots keep a harmless dark value.
PALETTES = {
    # Background art: 0 backdrop, 1-9 panels, 12-15 the striped weave behind everything.
    'menu': [INK, PANEL2, EDGE, '#3e3042', EDGE, EDGE_HI, PANEL, '#2a2130', '#241c2a', '#1d1721',
             INK, INK, '#1a1420', '#1e1824', '#221b28', '#262030'],  # 12-15: the background weave
    # Key labels: 1 ink, 2 shadow; 11 the name-entry field; 13-15 the key face of each page.
    'keyboard': [INK, TEXT, SHADOW, MUTED, '#e8483a', EMBER, '#4a6650', '#73dead', '#3e3042', EDGE_HI,
                 INK, PANEL2, INK, '#2c2231', '#2c2231', '#2c2231'],  # 13-15: same key face on every page
    # Page tints: 1 label ink, 2 shadow, 3 muted, 11-15 the key ramp of that page (the same plum on every page; the tab label names the page).
    'page_swap_upper': [INK, TEXT, SHADOW, MUTED, INK, INK, TEXT, SHADOW, TEXT, SHADOW, TEXT,
                        '#1d1721', '#241c2a', '#2c2231', '#3e3042', EDGE],
    'page_swap_lower': [INK, TEXT, SHADOW, MUTED, INK, INK, TEXT, SHADOW, TEXT, SHADOW, TEXT,
                        '#1d1721', '#241c2a', '#2c2231', '#3e3042', EDGE],
    'page_swap_others': [INK, TEXT, SHADOW, MUTED, INK, INK, TEXT, SHADOW, TEXT, SHADOW, TEXT,
                         '#1d1721', '#241c2a', '#2c2231', '#3e3042', EDGE],
    # Buttons (also the page-swap frame): 1 label, 3 face shadow, 4 face highlight, 5 caption,
    # 12-13 face, 14 edge (the code flashes it on the focused button), 15 outline.
    'buttons': [INK, TEXT, SHADOW, INK, '#3e3042', MUTED, INK, INK, INK, INK, INK, INK,
                PANEL2, PANEL2, EDGE_HI, SHADOW],
}

# Labels shown on the page-swap button name the page you switch to.
# Each sheet is displayed on the page before its own (verified in mGBA: the upper page shows the lower
# sheet), so the label on each sheet names the page SELECT opens from there.
PAGE_LABELS = {'page_swap_upper': 'ABC', 'page_swap_lower': 'abc', 'page_swap_others': 'ÁÉÍ'}
BUTTONS = {'back_button': ('ATRÁS', 'B'), 'ok_button': ('LISTO', 'START')}  # 28 px of face: BORRAR does not fit


def to15(colors):
    return [rgb(c) for c in colors]


def write_pal(name, colors):
    lines = ['JASC-PAL', '0100', '16'] + [f'{int(c[1:3], 16)} {int(c[3:5], 16)} {int(c[5:7], 16)}' for c in colors]
    (DIR / f'{name}.pal').write_bytes(('\r\n'.join(lines) + '\r\n').encode('ascii'))


def glyph_width(rows):
    used = [x for bits in rows for x in range(5) if bits & (16 >> x)]
    return (max(used) + 1) if used else 3


def stamp(pixels, width, text, cx, y, ink, shadow=None):
    small = font()
    total = sum(glyph_width(small[ch]) + 1 for ch in text) - 1
    x = cx - total // 2
    for ch in text:
        rows = small[ch]  # 8 rows: row 0 carries the accents of capitals
        for gy, bits in enumerate(rows):
            for gx in range(5):
                if bits & (16 >> gx):
                    height = len(pixels) // width
                    if shadow is not None and 0 <= y + gy + 1 < height:
                        pixels[(y + gy + 1) * width + x + gx + 1] = shadow
                    if 0 <= y + gy < height:
                        pixels[(y + gy) * width + x + gx] = ink
        x += glyph_width(rows) + 1


def draw_button(w, h, top, bottom):
    """A 40x16 panel: edge (index 14, cut corners), face 12, highlight 4 on top, shadow 3 below."""
    pixels = [0] * (w * h)
    x0, x1, y0, y1 = 1, w - 2, top, top + 15
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            corner = (y in (y0, y1)) and (x in (x0, x1))
            if corner:
                continue
            edge = y in (y0, y1) or x in (x0, x1)
            pixels[y * w + x] = 14 if edge else 4 if y == y0 + 1 else 3 if y == y1 - 1 else 12
    return pixels


def main():
    for name, colors in PALETTES.items():
        write_pal(name, colors)
    for name, label in PAGE_LABELS.items():
        w, h = 40, 8
        pixels = [0] * (w * h)
        stamp(pixels, w, label, 12, 0, 1, 2)  # the sprite shows ~24 px
        png(DIR / f'{name}.png', pixels, w, h, to15(PALETTES[name]), indexed=True)
    for name, (label, caption) in BUTTONS.items():
        w, h = 40, 24
        pixels = draw_button(w, h, 0, 15)
        stamp(pixels, w, label, 20, 4, 1, 3)
        stamp(pixels, w, caption, 20, 16, 5)
        png(DIR / f'{name}.png', pixels, w, h, to15(PALETTES['buttons']), indexed=True)
    # Page-swap frame: an edge around the page label (the label sprite and its face sit inside).
    w, h = 40, 32
    pixels = [0] * (w * h)
    for x in range(5, 35):
        pixels[5 * w + x] = pixels[17 * w + x] = 14
    for y in range(6, 17):
        pixels[y * w + 4] = pixels[y * w + 35] = 14
    stamp(pixels, w, 'SELECT', 20, 19, 5)
    png(DIR / 'page_swap_frame.png', pixels, w, h, to15(PALETTES['buttons']), indexed=True)
    print('naming art: palettes', len(PALETTES), 'labels', len(PAGE_LABELS) + len(BUTTONS))


if __name__ == '__main__':
    main()
