#!/usr/bin/env python3
# Converts binary PPM screenshots to PNG (scaled 2x) so they can be viewed.
import struct, sys, zlib

def convert(src, dst, scale=2):
    data = open(src, 'rb').read()
    parts = data.split(b'\n', 3)
    w, h = map(int, parts[1].split())
    px = parts[3]
    rows = []
    for y in range(h):
        row = px[y * w * 3:(y + 1) * w * 3]
        wide = b''.join(row[x * 3:x * 3 + 3] * scale for x in range(w))
        rows.extend([b'\x00' + wide] * scale)
    raw = b''.join(rows)
    def chunk(t, d):
        return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xFFFFFFFF)
    png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w * scale, h * scale, 8, 2, 0, 0, 0))
    png += chunk(b'IDAT', zlib.compress(raw, 6)) + chunk(b'IEND', b'')
    open(dst, 'wb').write(png)

for src in sys.argv[1:]:
    convert(src, src.rsplit('.', 1)[0] + '.png')
