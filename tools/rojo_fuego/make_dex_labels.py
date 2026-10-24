"""Spanish labels for the HGSS Pokédex interface sprites (graphics/pokedex/hgss/tileset_interface.png).

The sheet is 32 px wide, 8x8 tiles in 1D sprite order; each label is a sprite of the shape and
first tile used by pokedex.c (sSpriteAnim_SearchText etc.). A label is rebuilt from its tiles,
its English text is erased and the Spanish one drawn with m5x7 (graphics/rojo_fuego/fonts, CC0)
in the original style: ink plus a shadow right of and below it. NATIONAL only needs its T turned
into a C, drawn by hand in the 3 px lettering of that sprite.
Standard library only: python tools/rojo_fuego/make_dex_labels.py
"""
from pathlib import Path
import json
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from pngutil import read_png, write_png  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
SHEET = ROOT / "graphics/pokedex/hgss/tileset_interface.png"
FONT = ROOT / "graphics/rojo_fuego/fonts/m5x7/m5x7-bitmap.json"
WHITE, GREY, BLACK = 1, 4, 15
# name: (first tile, width, height, text, ink, shadow, text box x0, y0 (top of capitals), x1, erase y0, y1)
LABELS = {
    "search": (40, 32, 16, "BUSCAR", WHITE, BLACK, 0, 4, 32, 0, 16),
    "menu": (56, 32, 16, "MENÚ", WHITE, BLACK, 0, 4, 32, 0, 16),
    "seen": (64, 64, 32, "VIST.", BLACK, GREY, 16, 6, 41, 0, 16),
    "own": (96, 64, 32, "CAPT.", BLACK, GREY, 16, 6, 41, 0, 16),
}
# NATIONAL -> NACIONAL: columns 9..12, rows 4..11 of the 32x16 sprite at tile 168.
NATIONAL_TILE, NATIONAL_X, NATIONAL_Y = 168, 9, 4
# List screen strip: tilemap rows 18-19, columns 13..23 (x 104..191); separator line on row 8.
LIST_SHEET = ROOT / "graphics/pokedex/hgss/tileset_menu_list.png"
LIST_TILEMAP = ROOT / "graphics/pokedex/hgss/tilemap_list_screen.bin"
STRIP_ROWS, STRIP_COLS, STRIP_LINE_Y = (18, 19), (13, 24), 8
STRIP_LABELS = [("MENÚ", 11, 38), ("BUSCAR", 45, 79)]
# POKé DEX logo: tilemap rows 0-2, columns 12..23; its letters (colour 1, the panel colour, dark
# at night) inside the box x 22..77, y 4..20 become colour 12, which RFLoadDexChrome keeps light.
LOGO_ROWS, LOGO_COLS, LOGO_BOX, LOGO_INK = (0, 1, 2), (12, 24), (22, 78, 4, 21), 12
# Stat labels beside the list's stat bars (graphics/pokedex/hgss/stat_bars.png, its own palette:
# 14 ink, 15 shadow), one every 10 rows from row 1, in x 0..10.
STAT_SHEET = ROOT / "graphics/pokedex/hgss/stat_bars.png"
STAT_LABELS = ["PS", "AT", "DF", "AE", "DE", "VE"]
STAT_INK, STAT_SHADOW = 14, 15
# Search screen buttons (tileset_menu_search.png through tilemap_search_screen_national.bin; the
# highlight only switches palette banks). Text box in screen pixels: x0, x1, top row of capitals;
# fill and shadow colours are read from the button itself. OK stays OK.
SEARCH_SHEET = ROOT / "graphics/pokedex/hgss/tileset_menu_search.png"
SEARCH_TILEMAP = ROOT / "graphics/pokedex/hgss/tilemap_search_screen_national.bin"
SEARCH_INK = 3
SIDE_SHARED_X = 32
# Shadow colour per button, as in the original artwork: 9 on the green top bar (fill 8), 11 on the
# orange side buttons (fill 10).
SEARCH_SHADOWS = {"BUSCAR": 9, "LISTA": 9, "SALIR": 9, "LETRA": 11, "COLOR": 11, "TIPO": 11, "ORDEN": 11, "MODO": 11}
SEARCH_LABELS = [
    ("BUSCAR", 5, 35, 4), ("LISTA", 53, 83, 4), ("SALIR", 101, 131, 4),
    # Side buttons stop at x 32: their right-edge tiles (175, 190) are shared between rows.
    ("LETRA", 5, 32, 20), ("COLOR", 5, 32, 36), ("TIPO", 5, 32, 52), ("ORDEN", 5, 32, 68), ("MODO", 5, 32, 84),
]
LETTER_C = [" FF4", "F44 ", "F4  ", "F4  ", "F4  ", "F4  ", "4FF4", " 44 "]


def sprite_pixels(pixels, tile, width, height):
    """(x, y) in the sprite -> (sheet x, sheet y) for a 1D-mapped sprite."""
    tiles_wide = width // 8
    for y in range(height):
        for x in range(width):
            n = tile + (y // 8) * tiles_wide + x // 8
            yield x, y, (n % 4) * 8 + x % 8, (n // 4) * 8 + y % 8


def glyph_columns(rows, baseline):
    """Ink columns of a glyph (bit 0 = left), capitals' top at row 0; condensed by one column
    where two identical neighbours allow it without changing the shape."""
    placed = {y - (baseline - 6): bits for y, bits in enumerate(rows) if bits}
    width = max(bits.bit_length() for bits in placed.values())
    columns = [tuple(placed.get(y, 0) >> x & 1 for y in range(-4, 9)) for x in range(width)]
    while columns and not any(columns[0]):
        columns.pop(0)
    return columns


def condense(columns):
    for i in sorted(range(1, len(columns)), key=lambda i: abs(i - len(columns) / 2)):
        if columns[i] == columns[i - 1]:
            return columns[:i] + columns[i + 1:]
    return columns


def render(text, glyphs, baseline, max_width):
    """Ink set {(x, y)} with y relative to the capitals' top, condensing if it does not fit."""
    letters = [glyph_columns(glyphs[c], baseline) for c in text]
    if sum(len(c) + 1 for c in letters) > max_width:
        letters = [condense(c) for c in letters]
    ink, x = set(), 0
    for columns in letters:
        for cx, column in enumerate(columns):
            for cy, on in enumerate(column):
                if on:
                    ink.add((x + cx, cy - 4))
        x += len(columns) + 1
    return ink, x - 1


def draw_text(pixels, coords, text, ink, shadow, x0, x1, y0, glyphs, baseline, condensed=False):
    """Draws `text` centred in [x0, x1) with the capitals' top at y0. `coords` maps label
    (x, y) to sheet (x, y)."""
    dots, text_width = render(text, glyphs, baseline, 0 if condensed else x1 - x0 - 1)
    assert text_width + 1 <= x1 - x0, f"{text} is {text_width} px wide"
    left = x0 + (x1 - x0 - text_width - 1) // 2
    for x, y in dots:
        for dx, dy in ((1, 0), (0, 1), (1, 1)):
            if (x + dx, y + dy) not in dots and (left + x + dx, y0 + y + dy) in coords:
                sx, sy = coords[(left + x + dx, y0 + y + dy)]
                pixels[sy][sx] = shadow
    for x, y in dots:
        sx, sy = coords[(left + x, y0 + y)]
        pixels[sy][sx] = ink


def list_strip_coords(rows=STRIP_ROWS, cols=STRIP_COLS):
    """Screen-strip (x, y) -> list atlas (x, y) for a block of the list tilemap (unflipped tiles)."""
    data = LIST_TILEMAP.read_bytes()
    entries = struct.unpack(f"<{len(data) // 2}H", data)
    coords = {}
    for row, map_y in enumerate(rows):
        for col in range(cols[1] - cols[0]):
            tile = entries[map_y * 32 + cols[0] + col] & 0x3FF
            for y in range(8):
                for x in range(8):
                    coords[(col * 8 + x, row * 8 + y)] = ((tile % 16) * 8 + x, (tile // 16) * 8 + y)
    return coords


def screen_coords(tilemap, x0, x1, y0, y1, sheet_tiles_wide=16):
    """Screen (x, y) -> sheet (x, y) through a 32-wide tilemap, honouring tile flips."""
    data = tilemap.read_bytes()
    entries = struct.unpack(f"<{len(data) // 2}H", data)
    coords = {}
    for y in range(y0, y1):
        for x in range(x0, x1):
            entry = entries[(y // 8) * 32 + x // 8]
            tile = entry & 0x3FF
            tx = 7 - x % 8 if entry & 0x400 else x % 8
            ty = 7 - y % 8 if entry & 0x800 else y % 8
            coords[(x, y)] = ((tile % sheet_tiles_wide) * 8 + tx, (tile // sheet_tiles_wide) * 8 + ty)
    return coords


def main():
    data = json.loads(FONT.read_text(encoding="utf-8"))
    glyphs, baseline = data["glyphs"], data["baseline"]
    _, _, palette, pixels = read_png(SHEET)
    for name, (tile, width, height, text, ink, shadow, x0, y0, x1, ey0, ey1) in LABELS.items():
        coords = {(x, y): (sx, sy) for x, y, sx, sy in sprite_pixels(pixels, tile, width, height)}
        for (x, y), (sx, sy) in coords.items():
            if x0 <= x < x1 and ey0 <= y < ey1:
                pixels[sy][sx] = 0
        draw_text(pixels, coords, text, ink, shadow, x0, x1, y0, glyphs, baseline, condensed=width == 64)
    coords = {(x, y): (sx, sy) for x, y, sx, sy in sprite_pixels(pixels, NATIONAL_TILE, 32, 16)}
    for row, line in enumerate(LETTER_C):
        for col, ch in enumerate(line):
            sx, sy = coords[(NATIONAL_X + col, NATIONAL_Y + row)]
            pixels[sy][sx] = {" ": 0, "F": BLACK, "4": GREY}[ch]
    write_png(SHEET, palette, pixels)

    # List screen: MENU / SEARCH under the START / SELECT buttons are background tiles used once.
    _, _, palette, pixels = read_png(LIST_SHEET)
    coords = list_strip_coords()
    for text, x0, x1 in STRIP_LABELS:
        for (x, y), (sx, sy) in coords.items():
            if x0 <= x < x1 and 6 <= y < 16:
                pixels[sy][sx] = GREY if y == STRIP_LINE_Y else WHITE
        draw_text(pixels, coords, text, BLACK, GREY, x0, x1, 8, glyphs, baseline)
    x0, x1, y0, y1 = LOGO_BOX
    for (x, y), (sx, sy) in list_strip_coords(LOGO_ROWS, LOGO_COLS).items():
        if x0 <= x < x1 and y0 <= y < y1 and pixels[sy][sx] == WHITE:
            pixels[sy][sx] = LOGO_INK
    write_png(LIST_SHEET, palette, pixels)

    _, _, palette, pixels = read_png(STAT_SHEET)
    for i, text in enumerate(STAT_LABELS):
        top = 1 + 10 * i
        coords = {(x, y): (x, y) for x in range(11) for y in range(top - 1, top + 9)}
        for x, y in coords:
            pixels[y][x] = 0
        draw_text(pixels, coords, text, STAT_INK, STAT_SHADOW, 0, 11, top, glyphs, baseline, condensed=True)
    write_png(STAT_SHEET, palette, pixels)

    _, _, palette, pixels = read_png(SEARCH_SHEET)
    for text, x0, x1, top in SEARCH_LABELS:
        coords = screen_coords(SEARCH_TILEMAP, x0, x1, top, top + 8)
        # The row above the capitals is plain button fill; the text rows hold more shadow than fill.
        sx, sy = screen_coords(SEARCH_TILEMAP, x0 + 4, x0 + 5, top - 1, top)[(x0 + 4, top - 1)]
        fill = pixels[sy][sx]
        erase = coords
        if x1 == SIDE_SHARED_X:  # the English words reached into the shared edge tiles
            erase = {**coords, **screen_coords(SEARCH_TILEMAP, SIDE_SHARED_X, SIDE_SHARED_X + 4, top, top + 8)}
        for sx, sy in erase.values():
            pixels[sy][sx] = fill
        draw_text(pixels, coords, text, SEARCH_INK, SEARCH_SHADOWS[text], x0, x1, top, glyphs, baseline)
    write_png(SEARCH_SHEET, palette, pixels)
    print("Pokédex labels: " + ", ".join(l[3] for l in LABELS.values()) + ", NACIONAL, "
          + ", ".join(t for t, _, _ in STRIP_LABELS) + ", " + " ".join(STAT_LABELS) + ", "
          + ", ".join(t for t, _, _, _ in SEARCH_LABELS))


if __name__ == "__main__":
    main()
