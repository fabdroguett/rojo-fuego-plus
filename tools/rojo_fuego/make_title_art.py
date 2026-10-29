#!/usr/bin/env python3
"""Rojo Fuego Plus title screen, authored at native 240x160 resolution.

Writes src/data/rf_title_art.h (tiles, tilemaps and palettes for four backgrounds plus the ember
sprite) and a preview of the composed screen in build_artifacts/ui-redesign/title-preview.png.
Pure Python, like the other art generators; no third-party modules.

Layers (priority order, front to back):
  BG0  text: "PULSA START" and the footer
  BG1  logo "ROJO FUEGO" with the PLUS badge, and the volcano with its foreground ridge
  BG2  far mountain range, periodic over 256 px so it can scroll forever
  BG3  dusk sky gradient with stars
"""
from pathlib import Path
import math
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from make_kanto_ui_art import font, png  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / 'src/data/rf_title_art.h'
PREVIEW = ROOT / 'build_artifacts/ui-redesign/title-preview.png'
W, H = 240, 160
CRATER_X, CRATER_Y = 120, 90  # volcano crater; embers rise from here (exported to C)
MAP_W = 32  # tilemap width in tiles (256 px)


def rgb(hexcode):
    v = int(hexcode.lstrip('#'), 16)
    return ((v >> 16) * 31 // 255, ((v >> 8) & 255) * 31 // 255, (v & 255) * 31 // 255)


# --- Palettes ---------------------------------------------------------------------------------
# Sky: 0 backdrop, 1-10 dusk gradient top to horizon, 11-12 stars, 13-15 twinkle slots.
SKY = [rgb(c) for c in ('#0b0c18', '#10122a', '#171936', '#221d42', '#33204a', '#4c2550',
                        '#6c2b50', '#90344c', '#b54445', '#d65e3f', '#ef8644', '#f7efde',
                        '#8f90b8', '#f7efde', '#c7c8e6', '#8f90b8')]
# Far range: 0 transparent, 1 body, 2 lit rim, 3 mist.
RANGE = [rgb(c) for c in ('#000000', '#3a2140', '#7d3a4e', '#5a2c4a')] + [(0, 0, 0)] * 12
# Logo and volcano: 0 transparent, 1 outline, 2-5 fire fill top to bottom, 6 highlight,
# 7 badge red, 8 badge text, 9 volcano body, 10 volcano rim, 11 ground, 12-14 lava cycle, 15 glow.
LOGO = [rgb(c) for c in ('#000000', '#1c0a10', '#ffe08a', '#ffb24d', '#f7743a', '#d8392e',
                         '#fff7e0', '#f76352', '#fffff7', '#24141f', '#a8443a', '#120b14',
                         '#f2742f', '#ffc25a', '#e04a2c', '#ff9a3c')]
# Text: 0 transparent, 1 ink, 2 outline, 3 muted.
TEXT = [rgb(c) for c in ('#000000', '#f7efde', '#120b14', '#b5b5ad')] + [(0, 0, 0)] * 12
# Ember sprite: 0 transparent, 1 core, 2 hot, 3 warm, 4 ash.
EMBER = [rgb(c) for c in ('#000000', '#fff7c8', '#ffc25a', '#f2742f', '#a8443a')] + [(0, 0, 0)] * 11


# --- Helpers ----------------------------------------------------------------------------------
def canvas(width=256, height=160, fill=0):
    return [[fill] * width for _ in range(height)]


def hash01(*values):
    h = 2166136261
    for v in values:
        h = ((h ^ (v & 0xFFFFFFFF)) * 16777619) & 0xFFFFFFFF
    return h / 0xFFFFFFFF


def periodic_noise(x, period, octaves, seed):
    total = 0.0
    for i, (freq, amp) in enumerate(octaves):
        phase = hash01(seed, i) * math.tau
        total += amp * math.sin(math.tau * freq * x / period + phase)
    return total


# --- Sky (BG3) --------------------------------------------------------------------------------
BAYER = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]


def draw_sky():
    img = canvas()
    stops = [(0, 1), (22, 2), (40, 3), (56, 4), (70, 5), (82, 6), (92, 7), (101, 8), (109, 9), (116, 10)]
    for y in range(160):
        lo = max(s for s in stops if s[0] <= y)
        hi = next((s for s in stops if s[0] > y), None)
        for x in range(256):
            color = lo[1]
            if hi:
                t = (y - lo[0]) / (hi[0] - lo[0])
                if t * 16 > BAYER[y % 4][x % 4] + 4:
                    color = hi[1]
            img[y][x] = color
    for i in range(46):  # stars, densest at the top
        x = int(hash01(7, i) * 256)
        y = int(hash01(11, i) ** 1.7 * 70)
        if 8 <= y <= 50 and 20 <= x <= 222:  # keep the logo area clean
            continue
        img[y][x] = 11 if hash01(13, i) > 0.55 else (13 + i % 3)
        if hash01(17, i) > 0.92 and 0 < x < 255 and 0 < y:
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                img[y + dy][x + dx] = 12
    return img


# --- Far range (BG2) --------------------------------------------------------------------------
def draw_range():
    img = canvas()
    for x in range(256):
        top = int(104 + periodic_noise(x, 256, [(2, 7), (5, 4), (11, 2), (23, 1)], 3))
        for y in range(top, 160):
            img[y][x] = 1
        img[top][x] = 2
        if periodic_noise(x, 256, [(5, 1)], 9) > 0.3:  # rims catch the dusk on the sunward slopes
            img[top + 1][x] = 2
        for y in range(max(top + 6, 128), 160):  # mist settles in the valleys
            if (x + y) % 2 == 0 and y < 136:
                img[y][x] = 3
    return img


# --- Logo, badge and volcano (BG1) ------------------------------------------------------------
GLYPHS = {
    'U': ['11000011', '11000011', '11000011', '11000011', '11000011', '11000011', '11000011', '11100111', '01111110', '00111100'],
    'L': ['11000000', '11000000', '11000000', '11000000', '11000000', '11000000', '11000000', '11000000', '11111111', '11111111'],
    'T': ['11111111', '11111111', '00011000', '00011000', '00011000', '00011000', '00011000', '00011000', '00011000', '00011000'],
    'I': ['111111', '111111', '001100', '001100', '001100', '001100', '001100', '001100', '111111', '111111'],
    'M': ['11000011', '11100111', '11111111', '11011011', '11011011', '11000011', '11000011', '11000011', '11000011', '11000011'],
    'A': ['00111100', '01111110', '11100111', '11000011', '11000011', '11111111', '11111111', '11000011', '11000011', '11000011'],
    'E': ['11111111', '11111111', '11000000', '11000000', '11111100', '11111100', '11000000', '11000000', '11111111', '11111111'],
    'K': ['11000111', '11001110', '11011100', '11111000', '11110000', '11111000', '11011100', '11001110', '11000111', '11000011'],
    'N': ['11000011', '11100011', '11110011', '11111011', '11011111', '11001111', '11000111', '11000011', '11000011', '11000011'],
    'O': ['00111100', '01111110', '11100111', '11000011', '11000011', '11000011', '11000011', '11100111', '01111110', '00111100'],
}


def stamp_word(mask, word, scale, gap, y0):
    """Draws a word of block glyphs centred on the screen; returns (x0, width, height)."""
    widths = [len(GLYPHS[ch][0]) * scale for ch in word]
    width = sum(widths) + gap * (len(word) - 1)
    x = x0 = (W - width) // 2
    for ch, gw in zip(word, widths):
        for gy, row in enumerate(GLYPHS[ch]):
            for gx, bit in enumerate(row):
                if bit == '1':
                    for sy in range(scale):
                        for sx in range(scale):
                            mask[y0 + gy * scale + sy][x + gx * scale + sx] = 1
        x += gw + gap
    return x0, width, 10 * scale


def fill_word(img, mask, y0, height, steps):
    """Fire gradient from the top with a bright bevel on the upper edge of each stroke."""
    for y in range(y0, y0 + height):
        for x in range(256):
            if mask[y][x]:
                t = (y - y0) / height
                img[y][x] = steps[min(len(steps) - 1, int(t * len(steps)))]
                if not mask[y - 1][x]:
                    img[y][x] = 6


def outline(img, mask, reach=3):
    for y in range(1, 159):
        for x in range(1, 255):
            if not mask[y][x] and img[y][x] == 0 and any(
                    mask[y + dy][x + dx] for dy in (-2, -1, 0, 1, 2) for dx in (-2, -1, 0, 1, 2)
                    if 0 <= y + dy < 160 and 0 <= x + dx < 256 and abs(dx) + abs(dy) <= reach):
                img[y][x] = 1


def draw_logo(img):
    """POKÉMON (cream, doubled 5x7 capitals) over ULTIMATE and a larger KANTO, both in fire."""
    small = font()
    # POKÉMON: the 5x7 face at 2x with wide tracking.
    word = 'POKÉMON'
    rows = {ch: small[ch] for ch in word}
    gw, track = 10, 4
    width = len(word) * gw + (len(word) - 1) * track
    x, y0 = (W - width) // 2, 6
    pmask = canvas(256, 160, 0)
    for ch in word:
        glyph = rows[ch]
        for gy, bits in enumerate(glyph):
            for gx in range(5):
                if bits & (16 >> gx):
                    for sy in range(2):
                        for sx in range(2):
                            pmask[y0 + gy * 2 + sy][x + gx * 2 + sx] = 1
        x += gw + track
    for y in range(160):
        for xx in range(256):
            if pmask[y][xx]:
                img[y][xx] = 8 if not pmask[y - 1][xx] or y < y0 + 6 else 6
    outline(img, pmask, reach=2)
    # ULTIMATE and KANTO share one mask so their outlines merge.
    mask = canvas(256, 160, 0)
    _, _, h1 = stamp_word(mask, 'ULTIMATE', 2, 2, 26)
    _, _, h2 = stamp_word(mask, 'KANTO', 3, 3, 50)
    fill_word(img, mask, 26, h1, [2, 3, 4, 5])
    fill_word(img, mask, 50, h2, [2, 2, 3, 4, 4, 5])
    outline(img, mask)
    return img


def draw_volcano(img):
    peak_x, peak_y, crater = CRATER_X, CRATER_Y, 8
    tops = {}
    for x in range(256):
        d = abs(x - peak_x)
        slope = peak_y + max(0, d - crater) * 0.64 + (max(0, d - 38) ** 1.25) * 0.08
        slope += periodic_noise(x, 256, [(13, 1.2), (29, 0.6)], 21) * min(1, d / 12)
        top = int(min(160, slope))
        tops[x] = top
        for y in range(top, 160):
            if img[y][x] == 0:
                img[y][x] = 9
    # Volume: a dusk-lit rim on the left flank, ridges that run down from the crater.
    for x in range(peak_x - 70, peak_x - crater + 1):
        if 0 <= x < 256 and tops[x] < 160:
            for k in range(2 if x > peak_x - 40 else 1):
                if img[tops[x] + k][x] == 9:
                    img[tops[x] + k][x] = 10
    for ridge, (dx, length) in enumerate(((-14, 26), (-26, 30), (12, 22), (24, 28))):
        x = peak_x + dx * 0.35
        for i in range(length):
            y = peak_y + 6 + i
            x += dx / 40 + 0.3 * math.sin(i / 3 + ridge)
            xi = int(x)
            if 0 <= xi < 256 and y < 160 and img[y][xi] == 9 and (i % 5) != 4:
                img[y][xi] = 10 if dx < 0 else 11
    # Crater: a glowing pool with a halo on the rim.
    for x in range(peak_x - crater, peak_x + crater + 1):
        edge = abs(x - peak_x) >= crater - 1
        img[peak_y][x] = 10 if edge else 13
        img[peak_y + 1][x] = 10 if edge else 15
        img[peak_y + 2][x] = 9 if edge else 12 + (x % 3)
    for x in range(peak_x - crater + 2, peak_x + crater - 1):
        if img[peak_y - 1][x] == 0:
            img[peak_y - 1][x] = 15
    # Lava: two flows that narrow as they cool, using the cycling colours 12-14.
    for flow, (start, drift, length) in enumerate(((peak_x - 3, -0.38, 44), (peak_x + 4, 0.30, 34))):
        x = float(start)
        for i in range(length):
            y = peak_y + 3 + i
            x += drift + 0.45 * math.sin(i / 5.0 + flow * 2)
            width = 3 if i < length * 0.3 else 2 if i < length * 0.75 else 1
            for w in range(width):
                xi = int(x) + w - width // 2
                if 0 <= xi < 256 and y < 160 and img[y][xi] in (9, 10, 11):
                    img[y][xi] = 12 + (i // 2 + w + flow) % 3
            if i >= length - 4 and int(x) + 1 < 256:  # the cooled tip
                img[y][int(x)] = 10
    # Foreground ridge with grass and pine silhouettes against the valley mist.
    for x in range(256):
        top = int(127 + periodic_noise(x, 256, [(3, 3), (9, 1.5), (21, 0.8)], 33))
        if hash01(51, x) > 0.7:
            top -= 1 + int(hash01(53, x) * 2)  # grass tufts
        for y in range(top, 160):
            img[y][x] = 11
    for tree in range(9):
        tx = int(hash01(61, tree) * 256)
        base = int(127 + periodic_noise(tx, 256, [(3, 3), (9, 1.5), (21, 0.8)], 33))
        height = 9 + int(hash01(63, tree) * 8)
        for i in range(height):
            half = (i * 4) // height if i > 1 else 0
            if i % 3 == 2:
                half = max(0, half - 1)
            for dx in range(-half, half + 1):
                xx = (tx + dx) % 256
                img[base - height + i][xx] = 11
    return img


def glyph_width(rows):
    used = [x for bits in rows for x in range(5) if bits & (16 >> x)]
    return (max(used) + 1) if used else 3


def small_text(img, text, x, y, color, small):
    """Proportional 5x7 text; returns the x after the last glyph."""
    for ch in text:
        if ch == ' ':
            x += 4
            continue
        rows = small[ch][1:8]
        for gy, bits in enumerate(rows):
            for gx in range(5):
                if bits & (16 >> gx):
                    img[y + gy][x + gx] = color
        x += glyph_width(rows) + 1
    return x


def small_width(text, small):
    return sum(4 if ch == ' ' else glyph_width(small[ch][1:8]) + 1 for ch in text) - 1


# --- Text (BG0) -------------------------------------------------------------------------------
def draw_text():
    img = canvas()
    small = font()

    def put(text, y, color, outline=True):
        small_text(img, text, (W - small_width(text, small)) // 2, y, color, small)
        if outline:
            for yy in range(y - 1, y + 9):
                for xx in range(1, 255):
                    if img[yy][xx] == 0 and any(img[yy + dy][xx + dx] == color for dy in (-1, 0, 1) for dx in (-1, 0, 1)
                                                if 0 <= yy + dy < 160):
                        img[yy][xx] = 2
    put('PULSA START', 133, 1)
    put('PROYECTO DE FANS - NO OFICIAL', 150, 3, outline=False)
    return img


# --- Ember sprite -----------------------------------------------------------------------------
EMBER_FRAMES = [
    ['00000000', '00000000', '00044000', '00423400', '00432400', '00044000', '00000000', '00000000'],
    ['00000000', '00000000', '00000000', '00032000', '00021000', '00000000', '00000000', '00000000'],
    ['00000000', '00000000', '00000000', '00003000', '00000000', '00000000', '00000000', '00000000'],
]


# --- Pokémon on the title ---------------------------------------------------------------------
# (species folder, x, y): Charizard keeps watch beside the volcano, Pikachu on the near ridge.
TITLE_MONS = [('charizard', 170, 70), ('pikachu', 10, 74)]


def read_mon(species):
    """Two 64x64 front frames and a palette warmed and dimmed for the dusk light."""
    from pngutil import read_png
    w, h, _, rows = read_png(ROOT / 'graphics/pokemon' / species / 'anim_front.png')
    pal_lines = (ROOT / 'graphics/pokemon' / species / 'normal.pal').read_text().splitlines()[3:19]
    pal = []
    for line in pal_lines:
        r, g, b = (int(v) for v in line.split())
        # Warm key light from the horizon and a slight dimming, like the rest of the scene.
        r, g, b = min(255, r * 0.92 + 18), g * 0.80 + 6, b * 0.70 + 10
        pal.append((int(r) * 31 // 255, int(g) * 31 // 255, int(b) * 31 // 255))
    frames = [[row[:64] for row in rows[f * 64:(f + 1) * 64]] for f in range(2)]
    return frames, pal


def obj_tiles_64(frame):
    """64x64 OBJ in 1D mapping: tiles row-major, 8 per row."""
    return [tuple(frame[ty * 8 + y][tx * 8 + x] for y in range(8) for x in range(8)) for ty in range(8) for tx in range(8)]


# --- Tiles, maps and output -------------------------------------------------------------------
def tile_of(img, tx, ty):
    return tuple(img[ty * 8 + y][tx * 8 + x] for y in range(8) for x in range(8))


def flips(t):
    rows = [t[i * 8:(i + 1) * 8] for i in range(8)]
    h = tuple(v for r in rows for v in reversed(r))
    v = tuple(v for r in reversed(rows) for v in r)
    hv = tuple(v for r in reversed(rows) for v in reversed(r))
    return {t: 0, h: 1, v: 2, hv: 3}


def tilemap(img, palette_slot, width_tiles, tiles=None, base=0):
    """Deduplicates tiles (with flips) and returns (tiles, map entries). Tile 0 stays blank."""
    tiles = tiles if tiles is not None else [tuple([0] * 64)]
    index = {}
    for i, t in enumerate(tiles):
        for variant, flip in flips(t).items():
            index.setdefault(variant, (i, flip))
    entries = []
    for ty in range(20):
        for tx in range(MAP_W):
            if tx >= width_tiles:
                entries.append(0)
                continue
            t = tile_of(img, tx, ty)
            if t not in index:
                tiles.append(t)
                for variant, flip in flips(t).items():
                    index.setdefault(variant, (len(tiles) - 1, flip))
            i, flip = index[t]
            entries.append((base + i) | (flip << 10) | (palette_slot << 12))
    return tiles, entries


def pack4(tiles):
    out = bytearray()
    for t in tiles:
        for i in range(0, 64, 2):
            out.append(t[i] | (t[i + 1] << 4))
    return bytes(out)


def c_u32(name, data):
    words = [int.from_bytes(data[i:i + 4], 'little') for i in range(0, len(data), 4)]
    lines = [f'static const u32 {name}[{len(words)}] = {{']
    lines += ['    ' + ', '.join(f'0x{w:08X}' for w in words[i:i + 8]) + ',' for i in range(0, len(words), 8)]
    return lines + ['};']


def c_u16(name, values, per=16):
    lines = [f'static const u16 {name}[{len(values)}] = {{']
    lines += ['    ' + ', '.join(f'0x{v:04X}' for v in values[i:i + per]) + ',' for i in range(0, len(values), per)]
    return lines + ['};']


def bgr(c):
    return c[0] | (c[1] << 5) | (c[2] << 10)


def main():
    sky, far = draw_sky(), draw_range()
    front = draw_volcano(draw_logo(canvas()))
    text = draw_text()
    # Palette slots: 0 sky, 1 range, 2 logo/volcano, 3 text.
    sky_tiles, sky_map = tilemap(sky, 0, 32)
    far_tiles, far_map = tilemap(far, 1, 32)
    front_tiles, front_map = tilemap(front, 2, 32)
    text_tiles, text_map = tilemap(text, 3, 32)
    budget = {'sky': len(sky_tiles), 'range': len(far_tiles), 'front': len(front_tiles), 'text': len(text_tiles)}
    assert all(n <= 512 for n in budget.values()), budget
    ember = [int(c) for frame in EMBER_FRAMES for row in frame for c in row]
    ember_tiles = [tuple(ember[i * 64:(i + 1) * 64]) for i in range(len(EMBER_FRAMES))]

    out = ['// Generated by tools/rojo_fuego/make_title_art.py; authored at native resolution.',
           f'// Tiles: sky {budget["sky"]}, range {budget["range"]}, front {budget["front"]}, text {budget["text"]}.', '']
    for name, pal in (('Sky', SKY), ('Range', RANGE), ('Front', LOGO), ('Text', TEXT), ('Ember', EMBER)):
        out += c_u16(f'sRFTitle{name}Pal', [bgr(c) for c in pal]) + ['']
    for name, tiles, entries in (('Sky', sky_tiles, sky_map), ('Range', far_tiles, far_map),
                                 ('Front', front_tiles, front_map), ('Text', text_tiles, text_map)):
        out += c_u32(f'sRFTitle{name}Tiles', pack4(tiles)) + ['']
        out += c_u16(f'sRFTitle{name}Map', entries) + ['']
    out += c_u32('sRFTitleEmberTiles', pack4(ember_tiles)) + ['']
    mons = []
    for i, (species, mx, my) in enumerate(TITLE_MONS):
        frames, mpal = read_mon(species)
        mons.append((frames, mpal, mx, my))
        out += c_u16(f'sRFTitleMon{i}Pal', [bgr(c) for c in mpal]) + ['']
        out += c_u32(f'sRFTitleMon{i}Tiles', pack4(obj_tiles_64(frames[0]) + obj_tiles_64(frames[1]))) + ['']
    out += [f'#define RF_TITLE_CRATER_X {CRATER_X}', f'#define RF_TITLE_CRATER_Y {CRATER_Y}', '',
            '// Positions (top-left) of the title Pokémon, in TITLE_MONS order.',
            'static const s16 sRFTitleMonPos[][2] = {' + ', '.join(f'{{{x}, {y}}}' for _, x, y in TITLE_MONS) + '};', '']
    HEADER.write_text('\n'.join(out), encoding='utf-8')

    # Preview: compose the four layers (front to back) at the start scroll, with a few embers.
    pals = [SKY, RANGE, LOGO, TEXT]
    preview = []
    for y in range(H):
        for x in range(W):
            color = None
            for layer, pal in ((text, TEXT), (front, LOGO), (far, RANGE)):
                if layer[y][x]:
                    color = pal[layer[y][x]]
                    break
            preview.append(color or SKY[sky[y][x]])
    for frames, mpal, mx, my in mons:
        for yy in range(64):
            for xx in range(64):
                v = frames[0][yy][xx]
                if v and 0 <= mx + xx < W and 0 <= my + yy < H and not text[my + yy][mx + xx]:
                    preview[(my + yy) * W + mx + xx] = mpal[v]
    for i in range(9):
        ex, ey = int(CRATER_X - 20 + hash01(41, i) * 40), int(40 + hash01(43, i) * 50)
        frame = EMBER_FRAMES[i % 3]
        for yy in range(8):
            for xx in range(8):
                v = int(frame[yy][xx])
                if v and 0 <= ex + xx < W and 0 <= ey + yy < H:
                    preview[(ey + yy) * W + ex + xx] = EMBER[v]
    palette = sorted(set(preview))
    lookup = {c: i for i, c in enumerate(palette)}
    assert len(palette) <= 256
    write_rgb_png(PREVIEW, [lookup[c] for c in preview], W, H, palette)
    print('title art:', budget, 'header', HEADER.relative_to(ROOT), 'preview', PREVIEW.relative_to(ROOT))
    del pals


def write_rgb_png(path, pixels, width, height, palette):
    import struct
    import zlib

    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data) & 0xffffffff)
    raw = bytearray()
    for y in range(height):
        raw.append(0)
        for i in pixels[y * width:(y + 1) * width]:
            raw.extend(round(c * 255 / 31) for c in palette[i])
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0))
                     + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b''))


if __name__ == '__main__':
    main()
