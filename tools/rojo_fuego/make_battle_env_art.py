#!/usr/bin/env python3
"""Pokémon Ultimate Kanto battle backgrounds (every environment) and the evolution scene backdrop.

Rewrites graphics/battle_environment/{tall_grass,long_grass}/{tiles.png,map.bin,palette.pal}.
Visible area is 32x14 tiles (the text box covers rows 14-19). Layout kept from the originals:
the opponent's platform is centred at (176, 63) and the player's at (64, 116); rows 12-13 of
the left half keep palette 3 (the battle intro cross-fades it), so palettes 2-4 are identical.
"""
from pathlib import Path
import math
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from make_title_art import BAYER, flips, hash01, tile_of  # noqa: E402
from pngutil import write_png  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
MAX_TILES = 256

PALETTE = ['#140f1c',                                                   # 0 backdrop
           '#2c2142', '#4a2f58', '#7a3f62', '#b9555c', '#ec8559', '#ffc07a',  # 1-6 sky top to horizon
           '#1a1424', '#2a2034',                                        # 7-8 tree line
           '#24413a', '#2d5244', '#386650',                             # 9-11 field
           '#5f9064', '#86b472', '#ffb24d']                             # 12-13 platform, 14 sunlit rim
PALETTE += ['#000000'] * (16 - len(PALETTE))
HORIZON = 40


def canvas():
    return [[0] * 256 for _ in range(160)]


def sky(img):
    # Colours 1-6 top to horizon, ordered-dithered between neighbours.
    for y in range(HORIZON):
        t = y / (HORIZON - 1) * 5
        base, frac = int(t), t - int(t)
        for x in range(256):
            img[y][x] = 1 + min(5, base + (1 if frac * 16 > BAYER[y % 4][x % 4] else 0))


def tree_line(img):
    # Two rows of rounded crowns, repeating every 64 px so the tiles stay few.
    for x in range(256):
        u = x % 64
        far = HORIZON - 9 + int(4 * hash01(u // 6, 3)) - (2 if u % 6 in (2, 3) else 0)
        near = HORIZON - 4 + int(3 * hash01(u // 9, 7)) - (2 if u % 9 in (3, 4, 5) else 0)
        for y in range(far, HORIZON + 2):
            img[y][x] = 8
        for y in range(near, HORIZON + 3):
            img[y][x] = 7


def field(img):
    for y in range(HORIZON + 3, 160):
        band = 9 if y < HORIZON + 14 else (10 if y < 84 else 11)
        for x in range(256):
            img[y][x] = band
            # Grass blades on a 16x8 repeat, sparser toward the horizon.
            if (x % 16, y % 8) in ((3, 2), (4, 1), (11, 6), (12, 5)) and y > HORIZON + 6:
                img[y][x] = band + 1 if band < 11 else 12
            if y in (HORIZON + 14, 84) and (x // 2 + y) % 4 == 0:
                img[y][x] = band - 1


def platform(img, cx, cy, rx, ry):
    for y in range(cy - ry - 1, cy + ry + 2):
        for x in range(cx - rx - 1, cx + rx + 2):
            if not (0 <= y < 160 and 0 <= x < 256):
                continue
            d = ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2
            if d >= 0.985:  # drop the lone tip pixels
                continue
            inner = ((x - cx) / (rx - 2.5)) ** 2 + ((y - cy + 1.5) / (ry - 2.5)) ** 2
            if inner > 1 or d > 0.93:
                img[y][x] = 14 if y < cy else 10          # sunlit back rim, shaded front lip
            elif y > cy + ry * 0.35:
                img[y][x] = 12 if (x + y) % 2 or y > cy + ry * 0.6 else 13
            else:
                img[y][x] = 13


# Indoor scene (building tiles, shared by the building, plain, gym, leader and link/frontier palettes).
# 0 backdrop, 1-4 wall top to bottom, 5 baseboard, 6 lamp, 7 lamp halo, 8-10 floorboards, 11 seams,
# 12-13 rug, 14 rug rim, 15 rug emblem. Variants only recolour the rug (12-15).
INDOOR = ['#140f1c', '#1e1626', '#2a1f33', '#3a2a40', '#4d3550', '#5e3f4a', '#ffc07a', '#b9655a',
          '#3b2a2a', '#4a3431', '#5a3f38', '#2a1d20']
RUGS = {
    'building/palette.pal': ['#6a2f3a', '#8a3d45', '#ffb24d', '#b9555c'],
    'plain/palette.pal': ['#4f4a63', '#666080', '#ffb24d', '#8a84a6'],
    'building/gym.pal': ['#2f5a5a', '#3f7470', '#73dead', '#5a9a8a'],
    'building/leader.pal': ['#8a2f2a', '#b8443a', '#ffd27a', '#f76352'],
    'stadium/battle_frontier.pal': ['#2f3f6a', '#40558a', '#8ab4ff', '#5a74b0'],
}


def indoor():
    img = canvas()
    wall_end = 44
    for y in range(160):
        for x in range(256):
            if y < wall_end:
                t = y / (wall_end - 1) * 3
                base, frac = int(t), t - int(t)
                img[y][x] = 1 + min(3, base + (1 if frac * 16 > BAYER[y % 4][x % 4] else 0))
            elif y < wall_end + 4:
                img[y][x] = 5 if y < wall_end + 3 else 11
            else:
                row = (y - wall_end - 4) // 10
                img[y][x] = 11 if (y - wall_end - 4) % 10 == 9 else (8, 9, 10)[min(2, row // 2)]
                if (x + row * 24) % 48 == 0 and (y - wall_end - 4) % 10 != 9:
                    img[y][x] = 11
    # Wall lamps every 64 px with a dithered halo.
    for lx in range(32, 256, 64):
        for y in range(4, 34):
            for x in range(lx - 11, lx + 12):
                d = ((x - lx) / 11) ** 2 + ((y - 18) / 11) ** 2
                if d < 1 and (d < 0.45 or BAYER[y % 4][x % 4] > d * 16):
                    img[y][x] = 7
        for y in range(13, 23):
            for x in range(lx - 2, lx + 3):
                img[y][x] = 6
    return img


def rug(img, cx, cy, rx, ry):
    for y in range(cy - ry - 1, cy + ry + 2):
        for x in range(cx - rx - 1, cx + rx + 2):
            if not (0 <= y < 160 and 0 <= x < 256):
                continue
            d = ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2
            if d >= 0.985:  # drop the lone tip pixels
                continue
            if d > 0.86:
                img[y][x] = 14 if y < cy else 12
            elif 0.30 < d < 0.40:
                img[y][x] = 15
            else:
                img[y][x] = 13 if y < cy + ry * 0.4 else 12


def encode(img, keep_pal3):
    tiles = [tuple([0] * 64)]
    index = {tiles[0]: (0, 0)}
    entries = []
    for ty in range(64):
        for tx in range(32):
            if ty >= 14:
                entries.append(0)
                continue
            t = tile_of(img, tx, ty)
            if t not in index:
                tiles.append(t)
                for variant, flip in flips(t).items():
                    index.setdefault(variant, (len(tiles) - 1, flip))
            i, flip = index[t]
            pal = 3 if keep_pal3 and ty >= 12 and tx < 16 else 2
            entries.append(i | (flip << 10) | (pal << 12))
    assert len(tiles) <= MAX_TILES, f'{len(tiles)} tiles; the sheet holds {MAX_TILES}'
    sheet = [[0] * 128 for _ in range(128)]
    for i, t in enumerate(tiles):
        oy, ox = divmod(i, 16)
        for y in range(8):
            for x in range(8):
                sheet[oy * 8 + y][ox * 8 + x] = t[y * 8 + x]
    return sheet, struct.pack('<2048H', *entries[:2048]), len(tiles)


def colours_of(hexes):
    return [tuple(int(h[i:i + 2], 16) for i in (1, 3, 5)) for h in hexes]


def jasc(colours):
    return ('JASC-PAL\r\n0100\r\n48\r\n' + ''.join(f'{r} {g} {b}\r\n' for _ in range(3) for r, g, b in colours)).encode('ascii')


# Outdoor/cave scenes sharing one index layout: 0 backdrop, 1-6 back gradient (top to horizon),
# 7-8 silhouette far/near, 9-11 ground bands, 12-13 platform, 14 platform rim, 15 sparkle/accent.
SCENES = {
    'cave': dict(horizon=52, back='stalactites', ground='rock', rim='low', pal=[
        '#0c0a10', '#1a1422', '#211a2b', '#2a2135', '#342940', '#3f324c', '#4a3a58',
        '#0e0b13', '#16111c', '#2a2230', '#332a3a', '#3d3245', '#54485c', '#6a5c72', '#9a86a6', '#73dead']),
    'water': dict(horizon=40, back='islands', ground='sea', rim='foam', pal=[
        '#140f1c', '#2c2142', '#4a2f58', '#7a3f62', '#b9555c', '#ec8559', '#ffc07a',
        '#1a1424', '#2a2034', '#1d2c4a', '#24385a', '#2c466a', '#3a5a82', '#4f74a0', '#cfe3f0', '#ffc07a']),
    'pond_water': dict(horizon=40, back='reeds', ground='pond', rim='foam', pal=[
        '#140f1c', '#2c2142', '#4a2f58', '#7a3f62', '#b9555c', '#ec8559', '#ffc07a',
        '#1a1424', '#24301f', '#1e3340', '#244050', '#2b4d5c', '#386672', '#4c8088', '#cfe8e0', '#ffc07a']),
    'sand': dict(horizon=44, back='dunes', ground='sand', rim='sun', pal=[
        '#140f1c', '#3a2348', '#663059', '#9a4260', '#d0605a', '#f29058', '#ffcc80',
        '#6e3f3c', '#8a5444', '#a86a4c', '#b97a52', '#c98c5c', '#d9a46a', '#e8bc7e', '#ffd99a', '#fff0c0']),
    'rock': dict(horizon=44, back='mountains', ground='rock', rim='sun', pal=[
        '#140f1c', '#2c2142', '#4a2f58', '#7a3f62', '#b9555c', '#ec8559', '#ffc07a',
        '#241b2c', '#33263a', '#3a2f36', '#463840', '#52424a', '#6a5a5e', '#857278', '#ffb24d', '#ffd27a']),
    'underwater': dict(horizon=72, back='rays', ground='seabed', rim='low', pal=[
        '#06101c', '#0a1a2e', '#0d2340', '#112c4e', '#16365c', '#1c416a', '#234d78',
        '#0b1726', '#10213a', '#1d3540', '#24404a', '#2c4c54', '#3c6064', '#4f787a', '#9ad4d0', '#cfeff0']),
}
VARIANTS = {  # extra palettes reusing a scene's tiles (Emerald legendary arenas)
    'cave/groudon.pal': ('cave', {9: '#3a1a14', 10: '#4a2018', 11: '#5a281c', 12: '#7a3424', 13: '#9a4430',
                                  14: '#ff8a4d', 15: '#ffb24d'}),
    'water/kyogre.pal': ('water', {1: '#101a30', 2: '#142440', 3: '#1a2e52', 4: '#203a62', 5: '#284872', 6: '#305680'}),
}


def back_gradient(img, horizon):
    for y in range(horizon):
        t = y / max(1, horizon - 1) * 5
        base, frac = int(t), t - int(t)
        for x in range(256):
            img[y][x] = 1 + min(5, base + (1 if frac * 16 > BAYER[y % 4][x % 4] else 0))


def silhouette(img, kind, horizon):
    for x in range(256):
        if kind == 'islands':
            u = x % 128
            if 16 < u < 64:
                h = max(0, 7 - abs(u - 40) // 4)
            elif 80 < u < 120:
                h = max(0, 4 - abs(u - 100) // 5)
            else:
                h = 0
            for y in range(horizon - h, horizon):
                img[y][x] = 7
        elif kind == 'reeds':
            u = x % 32
            h = 3 + int(5 * hash01(x // 3, 11)) if u % 3 == 0 else 2
            for y in range(horizon - h, horizon + 1):
                img[y][x] = 8 if u % 3 else 7
        elif kind == 'dunes':
            far = horizon - 8 + int(4 * math.sin(x / 23.0) + 2 * math.sin(x / 9.0))
            near = horizon - 3 + int(3 * math.sin(x / 31.0 + 1))
            for y in range(far, horizon + 1):
                img[y][x] = 7
            for y in range(near, horizon + 1):
                img[y][x] = 8
        elif kind == 'mountains':
            far = horizon - 22 + abs((x % 64) - 32) * 2 // 3
            near = horizon - 12 + abs(((x + 40) % 96) - 48) // 3
            for y in range(max(0, far), horizon + 1):
                img[y][x] = 7
            for y in range(max(0, near), horizon + 1):
                img[y][x] = 8
            if far <= horizon - 19:  # sunlit snow caps
                for y in range(far, far + 2):
                    img[y][x] = 15
        elif kind == 'stalactites':
            u = x % 32
            h = (14 - abs(u - 10)) if u < 22 else (8 - abs(u - 27) * 2)
            h = max(2, h) + int(4 * hash01(x // 32, 5))
            for y in range(h):
                img[y][x] = 7 if y < h - 2 else 8
            for gx, gy, r in ((22, 51, 5), (30, 51, 3), (86, 51, 4), (112, 51, 6), (121, 51, 3)):
                dx = abs((x % 128) - gx)  # mint crystal clusters growing from the cave floor
                for y in range(gy - r * 2, gy + 1):
                    if dx <= r - (gy - y) // 2:
                        img[y][x] = 15 if dx < (r - (gy - y) // 2) or y == gy else 14
        elif kind == 'rays':
            for y in range(horizon):
                if ((x - y // 2) % 64) < 10 and BAYER[y % 4][x % 4] > 6 + y // 12:
                    img[y][x] = min(6, img[y][x] + 1)
            if (x * 7) % 61 == 3:
                for y in (20 + x % 30, 26 + x % 30):
                    img[y][x] = 15


def ground(img, kind, horizon):
    for y in range(horizon, 160):
        depth = y - horizon
        band = 9 if depth < 12 else (10 if depth < 40 else 11)
        for x in range(256):
            v = band
            if kind in ('sea', 'pond'):
                if (x + (y // 4) * 13) % 24 < (3 if depth < 12 else 6) and y % 4 == 0:
                    v = band + 1 if band < 11 else 12
                if kind == 'sea' and depth < 30 and abs(x - 200) < 10 + depth // 2 and y % 3 == 0 and (x + y) % 4:
                    v = 15  # sun glitter path
            elif kind == 'rock':
                if (x % 32, y % 16) in ((5, 3), (6, 3), (7, 4), (21, 11), (22, 10), (23, 10)):
                    v = band - 1 if band > 9 else 8
                if (x % 16, y % 8) == (12, 6):
                    v = min(13, band + 1)
            elif kind == 'sand':
                if (x + y * 3) % 20 == 0 and y % 3 == 0:
                    v = min(13, band + 1)
            elif kind == 'seabed':
                if (x % 16, y % 8) in ((3, 2), (11, 6)):
                    v = min(13, band + 1)
            img[y][x] = v


def disc(img, cx, cy, rx, ry, rim):
    for y in range(cy - ry - 1, cy + ry + 2):
        for x in range(cx - rx - 1, cx + rx + 2):
            if not (0 <= y < 160 and 0 <= x < 256):
                continue
            d = ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2
            if d >= 0.985:
                continue
            inner = ((x - cx) / (rx - 2.5)) ** 2 + ((y - cy + 1.5) / (ry - 2.5)) ** 2
            if inner > 1 or d > 0.93:
                if rim == 'foam':
                    img[y][x] = 14 if (x // 3 + y) % 3 else 13
                elif rim == 'sun':
                    img[y][x] = 14 if y < cy else 11
                else:
                    img[y][x] = 14 if y < cy - ry // 2 else 12
            elif y > cy + ry * 0.35:
                img[y][x] = 12 if (x + y) % 2 or y > cy + ry * 0.6 else 13
            else:
                img[y][x] = 13


def scene(cfg):
    img = canvas()
    h = cfg['horizon']
    back_gradient(img, h)
    ground(img, cfg['ground'], h)
    silhouette(img, cfg['back'], h)
    disc(img, 176, 63, 62, 15, cfg['rim'])
    disc(img, 64, 118, 74, 18, cfg['rim'])
    return img


def main():
    base = ROOT / 'graphics/battle_environment'
    img = canvas()
    sky(img)
    tree_line(img)
    field(img)
    platform(img, 176, 63, 62, 15)
    platform(img, 64, 118, 74, 18)
    sheet, tilemap, count = encode(img, True)
    colours = colours_of(PALETTE)
    for env in ('tall_grass', 'long_grass'):
        write_png(base / env / 'tiles.png', colours, sheet)
        (base / env / 'map.bin').write_bytes(tilemap)
        (base / env / 'palette.pal').write_bytes(jasc(colours))
    print('grass tiles', count)

    img = indoor()
    rug(img, 176, 63, 60, 14)
    rug(img, 64, 118, 72, 17)
    sheet, tilemap, count = encode(img, False)
    write_png(base / 'building/tiles.png', colours_of(INDOOR + RUGS['building/palette.pal']), sheet)
    (base / 'building/map.bin').write_bytes(tilemap)
    for path, rug_colours in RUGS.items():
        (base / path).write_bytes(jasc(colours_of(INDOOR + rug_colours)))
    print('indoor tiles', count)

    # Evolution scene: the grass dusk with a single platform centred under the evolving Pokémon.
    img = canvas()
    sky(img)
    tree_line(img)
    field(img)
    platform(img, 120, 92, 64, 14)
    sheet, tilemap, count = encode(img, False)
    colours = colours_of(PALETTE)
    evo = ROOT / 'graphics/evolution_scene'
    write_png(evo / 'rf_dusk.png', colours, sheet)
    # BG3 is 64x32 and the scene scrolls it by 256 px, so the right screenblock carries the image too.
    (evo / 'rf_dusk.bin').write_bytes(tilemap[:2048] * 2)
    (evo / 'rf_dusk.pal').write_bytes(jasc(colours))
    print('evolution tiles', count)

    for env, cfg in SCENES.items():
        sheet, tilemap, count = encode(scene(cfg), True)
        colours = colours_of(cfg['pal'])
        write_png(base / env / 'tiles.png', colours, sheet)
        (base / env / 'map.bin').write_bytes(tilemap)
        (base / env / 'palette.pal').write_bytes(jasc(colours))
        print(env, 'tiles', count)
    for path, (env, changes) in VARIANTS.items():
        hexes = list(SCENES[env]['pal'])
        for k, v in changes.items():
            hexes[k] = v
        (base / path).write_bytes(jasc(colours_of(hexes)))


if __name__ == '__main__':
    main()
