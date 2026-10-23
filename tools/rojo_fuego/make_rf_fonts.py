"""Build the selectable Latin font styles (Key System "Fuente") from bitmap JSON sources.

Styles, in enum KeyFontStyle order after the classic fonts, which stay in graphics/fonts/:
- m5x7 (Daniel Linssen, free with attribution), every sheet; condensed in the narrow ones.
- m6x11plus (Daniel Linssen) condensed for the normal and narrow sheets; its 10 px capitals do
  not fit the short and small cells, which use m5x7 as the m5x7 style does.
- Monogram (Vinícius Menézio, CC0) condensed, every sheet.
Sources: graphics/rojo_fuego/fonts/<font>/<font>-bitmap.json (rows of bits, bit 0 = leftmost;
Monogram's own file has no "baseline" and uses row 9). m5x7 and m6x11plus come from
rasterize_font.py. See CREDITS.md.

Each style copies the six classic sheets latin_{normal,narrow,short,short_narrow,small,
small_narrow}.png (16x16 cells: 1 ink, 2 shadow, 3 cell background, 0 outside the width),
redraws every single-byte charmap.txt character its font defines, and writes
graphics/rojo_fuego/fonts/<style>/latin_<sheet>.png plus the widths in src/data/rf_font_widths.h.
Characters a font lacks (♂, ♀, button and PK/MN symbols) keep the classic drawing. The shadow
follows the classic fonts: right, below and diagonal of every ink pixel. "Condensed" drops one
of two identical adjacent columns where a glyph has them, so texts written for the classic
widths still fit. The narrower sheets, the last resort for text that does not fit, stay classic.
Standard library only: python tools/rojo_fuego/make_rf_fonts.py
"""
from pathlib import Path
import json
import re
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from pngutil import read_png, write_png  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
FONT_DIR = ROOT / "graphics/rojo_fuego/fonts"
WIDTHS_H = ROOT / "src/data/rf_font_widths.h"
INK, SHADOW, PAPER = 1, 2, 3
# Sheet -> (classic width table, row of the bottom of 'A' in the classic cell), in enum RFFontSheet order.
SHEETS = {
    "normal": ("gFontNormalLatinGlyphWidths", 11),
    "narrow": ("gFontNarrowLatinGlyphWidths", 11),
    "short": ("gFontShortLatinGlyphWidths", 10),
    "short_narrow": ("gFontShortNarrowLatinGlyphWidths", 10),
    "small": ("gFontSmallLatinGlyphWidths", 10),
    "small_narrow": ("gFontSmallNarrowLatinGlyphWidths", 10),
}
# Style -> sheet -> (font, condensed), in enum KeyFontStyle order.
TALL = {"normal", "narrow"}
NARROW = {sheet for sheet in SHEETS if "narrow" in sheet}
STYLES = {
    "m5x7": {sheet: ("m5x7", sheet in NARROW) for sheet in SHEETS},
    "m6x11plus": {sheet: ("m6x11plus", True) if sheet in TALL else ("m5x7", sheet in NARROW) for sheet in SHEETS},
    "monogram": {sheet: ("monogram", True) for sheet in SHEETS},
}
GLYPH_A = 0xBB  # its classic cell shows which rows of a sheet carry the cell background


def load_font(name):
    data = json.loads((FONT_DIR / name / f"{name}-bitmap.json").read_text(encoding="utf-8"))
    if "glyphs" in data:
        return data["glyphs"], data["baseline"]
    return data, 9  # monogram-bitmap.json as published


def charmap_codes():
    """Character of every single-byte code, taking the first charmap entry for each code
    (the Japanese entries reuse the Latin codes)."""
    chars = {}
    for line in (ROOT / "charmap.txt").read_text(encoding="utf-8").splitlines():
        m = re.match(r"^'(\\'|.)'\s*=\s*([0-9A-F]{2})\s*$", line)
        if m:
            chars.setdefault(int(m.group(2), 16), "'" if m.group(1) == "\\'" else m.group(1))
    return {char: code for code, char in chars.items()}


def glyph_rows(rows, shift):
    """Rows of a glyph moved down by `shift`. If an accent would leave the cell, the blank rows
    between the accent and the letter are closed first, so only an unavoidable excess is cut."""
    rows = list(rows)
    while rows and not rows[-1]:
        rows.pop()
    top = next((y for y, bits in enumerate(rows) if bits), 0)
    while top + shift < 0:
        gaps = [y for y in range(top + 1, len(rows)) if not rows[y] and any(rows[top:y])]
        if not gaps:
            break
        del rows[gaps[0]]
        rows.insert(0, 0)
        top += 1
    return {y + shift: bits for y, bits in enumerate(rows) if bits and 0 <= y + shift < 16}


def draw(rows, shift, condensed, paper_rows):
    """16x16 cell for one glyph, and the width the game advances after it."""
    placed = glyph_rows(rows, shift)
    width = max((bits.bit_length() for bits in placed.values()), default=0)
    columns = [tuple(placed.get(y, 0) >> x & 1 for y in range(16)) for x in range(width)]
    while columns and not any(columns[0]):
        columns.pop(0)
    if condensed:
        for i in sorted(range(1, len(columns)), key=lambda i: abs(i - len(columns) / 2)):
            if columns[i] == columns[i - 1]:
                del columns[i]
                break
    ink = {(x, y) for x, column in enumerate(columns) for y, on in enumerate(column) if on}
    width = len(columns) + 1  # ink plus the shadow column
    cell = [[PAPER if y in paper_rows and x < width else 0 for x in range(16)] for y in range(16)]
    for x, y in ink:
        for dx, dy in ((1, 0), (0, 1), (1, 1)):
            if (x + dx, y + dy) not in ink and y + dy < 16:
                cell[y + dy][x + dx] = SHADOW
    for x, y in ink:
        cell[y][x] = INK
    return cell, width


def main():
    fonts = {name: load_font(name) for name in ("m5x7", "m6x11plus", "monogram")}
    codes = charmap_codes()
    classic_c = (ROOT / "src/fonts.c").read_text(encoding="utf-8")
    header = ["// Generated by tools/rojo_fuego/make_rf_fonts.py. Do not edit by hand.",
              "// Glyph advance widths per style (enum KeyFontStyle minus the classic fonts) and sheet.",
              "static const u8 sRFFontWidths[][RF_FONT_SHEET_COUNT][512] =", "{"]
    for style, sheets in STYLES.items():
        header.append(f"    {{ // {style}")
        for sheet, (table, baseline) in SHEETS.items():
            font, condensed = sheets[sheet]
            glyphs, source_baseline = fonts[font]
            _, _, palette, pixels = read_png(ROOT / f"graphics/fonts/latin_{sheet}.png")
            row, col = divmod(GLYPH_A, 16)
            paper_rows = {y for y in range(16) if any(pixels[row * 16 + y][col * 16 + x] for x in range(16))}
            m = re.search(table + r"\[\] = \{(.*?)\};", classic_c, re.S)
            assert m, f"{table} not found in src/fonts.c"
            widths = [int(v) for v in re.findall(r"\d+", m.group(1))]
            for char, code in codes.items():
                if char == " " or not any(glyphs.get(char, ())):
                    continue
                cell, widths[code] = draw(glyphs[char], baseline - source_baseline, condensed, paper_rows)
                row, col = divmod(code, 16)
                for y in range(16):
                    pixels[row * 16 + y][col * 16:col * 16 + 16] = cell[y]
            out = FONT_DIR / style / f"latin_{sheet}.png"
            out.parent.mkdir(parents=True, exist_ok=True)
            write_png(out, palette, pixels)
            header.append(f"        [RF_FONT_SHEET_{sheet.upper()}] = {{")
            header += ["            " + ", ".join(f"{w:2d}" for w in widths[i:i + 16]).lstrip() + ","
                       for i in range(0, len(widths), 16)]
            header.append("        },")
        header.append("    },")
    header.append("};")
    WIDTHS_H.write_text("\n".join(header) + "\n", encoding="utf-8", newline="\n")
    print(f"{len(STYLES)} styles x {len(SHEETS)} sheets -> graphics/rojo_fuego/fonts, {WIDTHS_H.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
