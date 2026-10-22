#!/usr/bin/env python3
# Minimal reader/writer for the indexed PNGs used by the decomp graphics (no third-party modules).
import struct, zlib

def read_png(path):
    """Returns (width, height, palette [(r,g,b)], pixels [[index]]) for an 8-bit or 4-bit indexed PNG."""
    data = open(path, 'rb').read()
    pos, chunks = 8, []
    while pos < len(data):
        length, kind = struct.unpack('>I4s', data[pos:pos + 8])
        chunks.append((kind, data[pos + 8:pos + 8 + length]))
        pos += 12 + length
    ihdr = dict(chunks)[b'IHDR']
    w, h, depth, ctype = struct.unpack('>IIBB', ihdr[:10])
    assert ctype == 3, 'not an indexed png'
    plte = dict(chunks)[b'PLTE']
    palette = [tuple(plte[i:i + 3]) for i in range(0, len(plte), 3)]
    raw = zlib.decompress(b''.join(d for k, d in chunks if k == b'IDAT'))
    stride = (w * depth + 7) // 8
    rows, prev, i = [], bytearray(stride), 0
    bpp = 1
    for _ in range(h):
        f = raw[i]; line = bytearray(raw[i + 1:i + 1 + stride]); i += 1 + stride
        for x in range(stride):
            a = line[x - bpp] if x >= bpp else 0
            b = prev[x]; c = prev[x - bpp] if x >= bpp else 0
            if f == 1: line[x] = (line[x] + a) & 0xFF
            elif f == 2: line[x] = (line[x] + b) & 0xFF
            elif f == 3: line[x] = (line[x] + (a + b) // 2) & 0xFF
            elif f == 4:
                p = a + b - c; pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[x] = (line[x] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 0xFF
        prev = line
        if depth == 8:
            rows.append(list(line))
        else:
            px = []
            for byte in line:
                for s in range(8 - depth, -1, -depth):
                    px.append((byte >> s) & ((1 << depth) - 1))
            rows.append(px[:w])
    return w, h, palette, rows

def write_png(path, palette, pixels, depth=4):
    h, w = len(pixels), len(pixels[0])
    raw = bytearray()
    for row in pixels:
        raw.append(0)
        if depth == 8:
            raw += bytes(row)
        else:
            for x in range(0, w, 2):
                raw.append((row[x] << 4) | (row[x + 1] if x + 1 < w else 0))
    def chunk(t, d):
        return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xFFFFFFFF)
    plte = b''.join(bytes(c) for c in palette)
    out = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, depth, 3, 0, 0, 0))
    out += chunk(b'PLTE', plte) + chunk(b'IDAT', zlib.compress(bytes(raw), 9)) + chunk(b'IEND', b'')
    open(path, 'wb').write(out)

def write_rgb_png(path, palette, pixels, scale=1):
    h, w = len(pixels), len(pixels[0])
    raw = bytearray()
    for row in pixels:
        line = b''.join(bytes(palette[p]) * scale for p in row)
        for _ in range(scale):
            raw += b'\x00' + line
    def chunk(t, d):
        return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xFFFFFFFF)
    out = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w * scale, h * scale, 8, 2, 0, 0, 0))
    out += chunk(b'IDAT', zlib.compress(bytes(raw))) + chunk(b'IEND', b'')
    open(path, 'wb').write(out)
