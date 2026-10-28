"""Hand-authored native Kanto UI art. No resampling, quantization or external packages.

Rebuild: python tools/rojo_fuego/make_kanto_ui_art.py
The row drawings are the editable masters; PNGs, .4bpp, .gbapal and C are generated.
"""
from pathlib import Path
import json
import re
import struct
import zlib

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'graphics/rojo_fuego/kanto_ui'
# 0 transparent, 1..8 UI roles; stable illustration colors 6,7,9..15.
RGB = [(0,0,0),(27,31,27),(2,7,8),(22,27,24),(23,28,25),(8,15,14),
       (8,18,18),(14,27,21),(14,27,21),(31,25,6),(28,9,8),
       (7,14,22),(14,23,28),(22,15,5),(31,30,25),(2,7,8)]
KEY = {'.':0, '#':15, 's':6, 'm':7, 'g':9, 'r':10, 'b':11, 'c':12, 'o':13, 'w':14}
# Each symbol is one final hardware pixel. Shadows are inside silhouettes, never filters.
ART = {
'pokedex': '''
................
....########....
...#rrrrrrrw#...
..#rrrrrrrrrw#..
..#rwrrrrrrrw#..
..#rrrrrrrrrr#..
..#rr######rr#..
..#rr#cmmms#r#..
..#rr#mmmms#r#..
..#rr#sssss#r#..
..#rr######rr#..
..#rrrwrrggro#..
..#rrrrrrrrro#..
...#ooooooo#...
....#######.....
................''',
'dexnav': '''
................
.........##.....
........#mw#....
...######ms#....
..#mmmmmmmms#...
..#mwmmmmmmm#...
..#m######mm#...
..#m#cwwws#m#...
..#m#wcccs#m#...
..#m#wcsss#m#...
..#m######ms#...
..#mmwgmmmmss#..
..#mmmmmmsss#...
...#sssssss#....
....#######.....
................''',
'equipo': '''
................
.....######.....
...##rrrrrr##...
..#rwrrrrrrro#..
..#wrrrrrrroo#..
.#rrrrrrrrroo#..
.#rrrr####rro#..
.#####wcc####.#.
.#wwww#cw#www#..
.#wwww####www#..
..#wwwwwwwwwc#..
..#wwwwwwwwc#...
...##cccccc##...
.....######.....
................
................''',
'mochila': '''
................
.....######.....
....#oggggo#....
....#g####g#....
...##g####g##...
..#wwgggggggg#..
..#wggggggggo#..
..#wggggggggo#..
..#ggg####ggo#..
.#oggg#wwg#ggo#.
.#oggg#ggg#ggo#.
.#oggg#ggg#ggo#.
..#ogg#ooo#go#..
..#ooo#####oo#..
...##########...
................''',
'perfil': '''
................
.....######.....
....#mmmmmm#....
...#mmmmmmmm#...
...#mmwwwwmm#...
....##owoo##....
.....#owoo#.....
.....#oooo#.....
....##ooo##.....
...#mm#o#mm#....
..#mwmm#mmms#...
..#mwmm#mmms#...
..#mmmm#mmss#...
...#sssssss#....
....#######.....
................''',
'guardar': '''
................
...##########...
..#cccccccccc#..
..#c########b#..
..#c#wwwwcm#b#..
..#c#wcccss#b#..
..#c#wcssss#b#..
..#c########b#..
..#cbbbbbbbbb#..
..#c########b#..
..#c#wwwwww#b#..
..#b#wccccw#b#..
..#b#wwwwww#b#..
..#bb######bb#..
...##########...
................''',
'ajustes': '''
................
......####......
.....#mwws#.....
..####mmms####..
..#mmmmmmmmmss#.
...#mm####mss#..
..#mm#wwcc#mss#.
.#mm#w#ss#w#ms#.
.#mm#w#ss#w#ms#.
..#mm#cccc#mss#.
...#mm####mss#..
..#mmmmmmmmmss#.
..####mmms####..
.....#ssss#.....
......####......
................''',
'claves': '''
................
.........####...
........#wwgg#..
.......#wg##go#.
.......#wg##go#.
.......#ggggoo#.
........#ggoo#..
.......#gg###...
......#gg#......
.....#gg#.......
....#gg#........
...#gggo#.......
..#gg#go#.......
...##.##........
................
................''',
'misiones': '''
................
...##########...
..#wwwwwwwwwo#..
..#w########o#..
..#w#gggggg#o#..
..#w#wwwwww#o#..
..#w#ssssww#o#..
..#w#wwwwww#o#..
..#w#ssssww#o#..
..#w#wwwwww#o#..
..#w#ssswww#o#..
..#w#wwwwww#o#..
..#w########o#..
...#oooooooo#...
....########....
................''',
'medicine': '''
................
......####......
.....#wwcc#.....
.....#cccc#.....
......#bb#......
.....#wwcc#.....
....#wcbbbc#....
....#wcbbbc#....
....#cbwwwb#....
....#cbwcwb#....
....#cbwwwb#....
....#cbbbbb#....
....#bbbbbs#....
.....#ssss#.....
......####......
................''',
'evolution': '''
................
......####......
....##wmmm##....
...#wwmmmmmms#..
..#wmmmmmmmmss#.
..#mmmwmmmmmss#.
..#mmwmmmmmsss#.
..#mwmmmmmssss#.
..#mmmmmssssss#.
...#mmmsssss##..
....#sssss##....
.....#####......
................
................
................
................''',
'treasure': '''
................
................
...##########...
..#gwwggggwgg#..
..#grrggggrrg#..
..#grrggggrrg#..
..#gggggggggg#..
..#oooo##oooo#..
..#gwww#gwggg#..
..#grrr#gorrr#..
..#grrr##rrrr#..
..#grrrrrrrro#..
..#oooooooooo#..
...##########...
................
................''',
'other': '''
................
.....######.....
...##ccwwcc##...
..#cccwwwccbb#..
..#cccwwccbbb#..
.#cccwwccbbbb#..
.#ccww####bbb#..
.#ccwc#ww#bbb#..
.#cccc#ww#bbb#..
.#cccc####bbb#..
..#cccbbbbbbb#..
..#ccbbbbbbb#...
...##bbbbbb##...
.....######.....
................
................''',
'bundle': '''
................
................
......####......
.....#wwgg#.....
.....#ggoo#.....
....##oooo##....
...#wwgggggg#...
..#wggggggggo#..
..#gggwoogggo#..
..#gggoogggoo#..
..#gggoogggoo#..
...#ooooooo#....
....#######.....
................
................
................''',
}


def drawing(text):
    lines = text.strip().splitlines()
    assert len(lines) == 16
    assert all(len(s) <= 16 for s in lines), lines
    return [KEY[c] for row in lines for c in row.ljust(16, '.')]


def arrow(direction, size=16):
    p = [0] * (size * size)
    # Explicit stepped silhouette, symmetrical about the center; no rescaling.
    for y, span in enumerate([1,2,3,4,5,5,4,3,2,1]):
        for x in range(span + 2):
            p[(y+3)*16+x+5] = 15 if x in (0,span+1) or y in (0,9) else (14 if x == 1 else 9)
    if direction == 'left':
        p = [p[y*16+15-x] for y in range(16) for x in range(16)]
    if direction in ('up','down'):
        p = [0]*256
        for y, span in enumerate([1,3,5,7]):
            for x in range(span):
                p[(6+y)*16+8-span//2+x] = 15 if y == 3 or x in (0,span-1) else 7
        if direction == 'down':
            p = [p[(15-y)*16+x] for y in range(16) for x in range(16)]
    return p


def return_arrow(size):
    p = [0]*(size*size)
    # Separate native 16 and 32 drawings, with deliberately different edge detail.
    if size == 16:
        for y in range(3,10):
            for x in range(11,14): p[y*size+x] = 15 if x == 13 else 14
        for y in range(7,12):
            for x in range(5,14): p[y*size+x] = 15 if y in (7,11) or x == 13 else (12 if y==10 else 14)
        for y, left in enumerate([6,5,4,3,4,5,6]):
            for x in range(left,8): p[(y+6)*size+x] = 15 if x == left or y in (0,6) else (12 if y>3 else 14)
    else:
        for y in range(5,21):
            for x in range(23,29): p[y*size+x] = 15 if x in (23,28) or y==5 else (12 if x==27 else 14)
        for y in range(16,24):
            for x in range(10,29): p[y*size+x] = 15 if y in (16,23) or x==28 else (12 if y>=21 else 14)
        for y, left in enumerate([12,11,10,9,8,7,6,5,6,7,8,9,10,11,12]):
            for x in range(left,15): p[(y+12)*size+x] = 15 if x==left or y in (0,14) else (12 if y>=8 else 14)
    return p


def control(letter):
    p = [0]*64
    for y in range(8):
        for x in range(8):
            if (x in (0,7) and y in (0,7)): continue
            p[y*8+x] = 15 if x in (0,7) or y in (0,7) else (12 if y==6 else 14)
    for y,row in enumerate(FONT[letter].split('/')):
        if y>=5: break
        for x,c in enumerate(row[:3]):
            if c=='1': p[(y+1)*8+x+2]=15
    return p


# Five-pixel masters; proportional advance trims blank columns. 0/O,1/I,6/8 differ.
RAW = '''
A 01110/10001/10001/11111/10001/10001/10001
B 11110/10001/10001/11110/10001/10001/11110
C 01111/10000/10000/10000/10000/10000/01111
D 11110/10001/10001/10001/10001/10001/11110
E 11111/10000/10000/11110/10000/10000/11111
F 11111/10000/10000/11110/10000/10000/10000
G 01111/10000/10000/10111/10001/10001/01111
H 10001/10001/10001/11111/10001/10001/10001
I 111/010/010/010/010/010/111
J 00111/00010/00010/00010/10010/10010/01100
K 10001/10010/10100/11000/10100/10010/10001
L 10000/10000/10000/10000/10000/10000/11111
M 10001/11011/10101/10101/10001/10001/10001
N 10001/11001/11001/10101/10011/10011/10001
O 01110/10001/10001/10001/10001/10001/01110
P 11110/10001/10001/11110/10000/10000/10000
Q 01110/10001/10001/10001/10101/10010/01101
R 11110/10001/10001/11110/10100/10010/10001
S 01111/10000/10000/01110/00001/00001/11110
T 11111/00100/00100/00100/00100/00100/00100
U 10001/10001/10001/10001/10001/10001/01110
V 10001/10001/10001/10001/10001/01010/00100
W 10001/10001/10001/10101/10101/10101/01010
X 10001/10001/01010/00100/01010/10001/10001
Y 10001/10001/01010/00100/00100/00100/00100
Z 11111/00001/00010/00100/01000/10000/11111
a 00000/00000/01110/00001/01111/10001/01111
b 10000/10000/10110/11001/10001/10001/11110
c 0000/0000/0111/1000/1000/1000/0111
d 00001/00001/01101/10011/10001/10001/01111
e 00000/00000/01110/10001/11111/10000/01111
f 0011/0100/1110/0100/0100/0100/0100
g 00000/01111/10001/10001/01111/00001/01110
h 10000/10000/10110/11001/10001/10001/10001
i 1/0/1/1/1/1/1
j 001/000/001/001/001/101/010
k 1000/1000/1001/1010/1100/1010/1001
l 10/10/10/10/10/10/01
m 00000/00000/11010/10101/10101/10101/10101
n 00000/00000/10110/11001/10001/10001/10001
o 00000/00000/01110/10001/10001/10001/01110
p 00000/11110/10001/10001/11110/10000/10000
q 00000/01111/10001/10001/01111/00001/00001
r 0000/0000/1011/1100/1000/1000/1000
s 0000/0000/0111/1000/0110/0001/1110
t 0100/0100/1111/0100/0100/0100/0011
u 00000/00000/10001/10001/10001/10011/01101
v 00000/00000/10001/10001/10001/01010/00100
w 00000/00000/10001/10001/10101/10101/01010
x 00000/00000/10001/01010/00100/01010/10001
y 00000/10001/10001/10001/01111/00001/01110
z 0000/0000/1111/0001/0010/0100/1111
0 01110/10001/10011/10101/11001/10001/01110
1 010/110/010/010/010/010/111
2 01110/10001/00001/00010/00100/01000/11111
3 11110/00001/00001/01110/00001/00001/11110
4 00010/00110/01010/10010/11111/00010/00010
5 11111/10000/10000/11110/00001/00001/11110
6 00110/01000/10000/11110/10001/10001/01110
7 11111/00001/00010/00100/01000/01000/01000
8 01110/10001/10001/01110/10001/10001/01110
9 01110/10001/10001/01111/00001/00010/01100
. 0/0/0/0/0/1/1
, 00/00/00/00/00/01/10
: 0/1/1/0/1/1/0
; 00/01/01/00/01/01/10
! 1/1/1/1/1/0/1
? 01110/10001/00001/00010/00100/00000/00100
¿ 00100/00000/00100/01000/10000/10001/01110
¡ 1/0/1/1/1/1/1
( 01/10/10/10/10/10/01
) 10/01/01/01/01/01/10
[ 11/10/10/10/10/10/11
] 11/01/01/01/01/01/11
/ 00001/00001/00010/00100/01000/10000/10000
- 000/000/000/111/000/000/000
+ 000/010/010/111/010/010/000
* 00000/10101/01110/11111/01110/10101/00000
% 11001/11010/00010/00100/01000/01011/10011
= 000/000/111/000/111/000/000
< 001/010/100/100/100/010/001
> 100/010/001/001/001/010/100
$ 00100/01111/10100/01110/00101/11110/00100
# 01010/01010/11111/01010/11111/01010/01010
@ 01110/10001/10111/10101/10111/10000/01111
& 01100/10010/10100/01000/10101/10010/01101
¥ 10001/01010/00100/11111/00100/11111/00100
× 00000/00000/10001/01010/00100/01010/10001
' 1/1/0/0/0/0/0
'''
FONT = dict(line.split(' ',1) for line in RAW.strip().splitlines())


def font():
    result = {}
    for char,rows in FONT.items():
        result[char] = [0] + [int(s.ljust(5,'0'),2) for s in rows.split('/')]
    for chars,base in [('ÁÉÍÓÚ','AEIOU'),('áéíóú','aeiou'),('Üü','Uu'),('Ññ','Nn')]:
        for ch,b in zip(chars,base):
            rows = result[b].copy()
            if b.isupper():
                body = rows[1:]
                del body[2]  # remove a repeated interior scanline, preserve the cap/crossbar
                center = max((x for r in body for x in range(5) if r & (16 >> x)), default=4) // 2
                mark = [16 >> (center+1),16 >> center]
                if ch=='Ü': mark=[0b01010,0]
                if ch=='Ñ': mark=[0b01010,0b10100]
                rows = mark+body
            else:
                center = max((x for r in rows for x in range(5) if r & (16 >> x)), default=4) // 2
                rows[0],rows[1] = (16 >> (center+1),16 >> center)
                if ch=='ü': rows[0],rows[1]=(0b01010,0)
                if ch=='ñ': rows[0],rows[1]=(0b01010,0b10100)
                if b=='i': rows[2] = 0
            result[ch] = rows
    result[' '] = [0]*8
    return result


def png(path,pixels,width,height,palette=RGB,indexed=False):
    def chunk(kind,data):
        return struct.pack('>I',len(data))+kind+data+struct.pack('>I',zlib.crc32(kind+data)&0xffffffff)
    raw=bytearray()
    for y in range(height):
        raw.append(0)
        for index in pixels[y*width:(y+1)*width]:
            raw.extend([index] if indexed else [round(c*255/31) for c in palette[index]]+[255 if index else 0])
    path.parent.mkdir(parents=True,exist_ok=True)
    colors=(chunk(b'PLTE',bytes(round(c*255/31) for color in palette for c in color))+chunk(b'tRNS',bytes([0]+[255]*15))) if indexed else b''
    path.write_bytes(b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',width,height,8,3 if indexed else 6,0,0,0))+colors+chunk(b'IDAT',zlib.compress(raw,9))+chunk(b'IEND',b''))


def tiles(pixels,width,height):
    return bytes(pixels[(ty+y)*width+tx+x] | pixels[(ty+y)*width+tx+x+1]<<4
                 for ty in range(0,height,8) for tx in range(0,width,8)
                 for y in range(8) for x in range(0,8,2))


def words(name,data):
    data=struct.unpack('<'+'I'*(len(data)//4),data)
    return [f'static const u32 {name}[{len(data)}] = {{']+[
        '    '+', '.join(f'0x{x:08X}' for x in data[i:i+8])+',' for i in range(0,len(data),8)]+['};']


def main():
    OUT.mkdir(parents=True,exist_ok=True)
    art={name:drawing(rows) for name,rows in ART.items()}
    art.update({name:arrow(direction) for name,direction in [('left','left'),('right','right'),('cursor','right'),('up','up'),('down','down')]})
    art['return']=return_arrow(16)
    palette=struct.pack('<16H',*[r | g<<5 | b<<10 for r,g,b in RGB])
    (OUT/'kanto.gbapal').write_bytes(palette)
    for theme in ('day','night'):
        colors=RGB.copy()
        colors[1]=(31,29,24)
        if theme=='night':
            for index,color in {0:(1,5,6),1:(2,9,10),2:(30,31,28),3:(0,3,4),4:(1,5,6),5:(18,25,23)}.items(): colors[index]=color
        (OUT/('menu_'+theme+'.gbapal')).write_bytes(struct.pack('<16H',*[r|g<<5|b<<10 for r,g,b in colors]))
    header=['// Generated by make_kanto_ui_art.py; authored at native resolution.']
    for name,pixels in art.items():
        png(OUT/(name+'.png'),pixels,16,16)
        png(OUT/(name+'.indexed.png'),pixels,16,16,indexed=True)
        data=tiles(pixels,16,16)
        (OUT/(name+'.4bpp')).write_bytes(data)
        header+=words('sKanto_'+name,data)
    for letter in 'ABRL':
        # Independent 3x5 button legends, not cropped from larger glyphs.
        p=[0]*64
        for y in range(8):
            for x in range(8):
                if x in (0,7) and y in (0,7): continue
                p[y*8+x]=15 if x in (0,7) or y in (0,7) else 14
        legend={'A':['010','101','111','101','101'],'B':['110','101','110','101','110'],'R':['110','101','110','101','101'],'L':['100','100','100','100','111']}[letter]
        for y,row in enumerate(legend):
            for x,c in enumerate(row):
                if c=='1': p[(y+1)*8+x+2]=15
        png(OUT/('button_'+letter+'.png'),p,8,8)
        png(OUT/('button_'+letter+'.indexed.png'),p,8,8,indexed=True)
        data=tiles(p,8,8)
        (OUT/('button_'+letter+'.4bpp')).write_bytes(data)
        header+=words('sKanto_button_'+letter,data)
    p=[0]*64
    for y in range(8):
        for x in range(8):
            if (2<=x<=4) or (2<=y<=4): p[y*8+x]=15 if x in (0,7) or y in (0,7) or (x not in (3,) and y not in (3,)) else 14
    png(OUT/'dpad.png',p,8,8)
    png(OUT/'dpad.indexed.png',p,8,8,indexed=True)
    data=tiles(p,8,8)
    (OUT/'dpad.4bpp').write_bytes(data)
    header+=words('sKanto_dpad',data)
    (ROOT/'src/data/rf_kanto_art.h').write_text('\n'.join(header)+'\n',encoding='utf-8')
    shared=['// Native outline variants generated by make_kanto_ui_art.py.',
            'static const u16 sRFMenuIconMasks[RF_ICON_COUNT][16] = {']
    for enum,name in [('POKEDEX','pokedex'),('TEAM','equipo'),('BAG','mochila'),('DEXNAV','dexnav'),('PROFILE','perfil'),('SAVE','guardar'),('OPTIONS','ajustes'),('EXIT','return'),('KEY','claves'),('QUESTS','misiones')]:
        masks=[sum(1<<(15-x) for x in range(16) if art[name][y*16+x]==15) for y in range(16)]
        shared.append('    [RF_ICON_'+enum+'] = {'+', '.join(f'0x{m:04X}' for m in masks)+'},')
    shared+=['};']
    (ROOT/'src/data/rf_menu_icons.h').write_text('\n'.join(shared)+'\n',encoding='utf-8')
    # Preserve existing bag sprite allocation/tags and its three palette banks.
    names=['mochila','medicine','equipo','evolution','claves','treasure','other','bundle','left','right','return','cursor','up','down']
    bag=['// Generated from native drawings by make_kanto_ui_art.py.', 'static const u16 sRFBagUserPalettes[1][16] = {']
    bag+=['    {'+', '.join(f'0x{x:04X}' for x in struct.unpack('<16H',palette))+'},']
    bag+=['};','static const u8 sRFBagUserPaletteGroups[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};']
    bag+=words('sRFBagUserTiles',b''.join(tiles(art[n],16,16) for n in names))
    big=return_arrow(32)
    png(OUT/'return_32.png',big,32,32)
    png(OUT/'return_32.indexed.png',big,32,32,indexed=True)
    data=tiles(big,32,32)
    (OUT/'return_32.4bpp').write_bytes(data)
    bag+=words('sRFBagUserReturnTiles',data)
    (ROOT/'src/data/rf_bag_user_art.h').write_text('\n'.join(bag)+'\n',encoding='utf-8')
    glyphs=font()
    mapping={m[1]:int(m[2],16) for m in re.finditer(r"^'(.)'\s*=\s*([0-9A-F]{2})\b",(ROOT/'charmap.txt').read_text(encoding='utf-8'),re.M)}
    output=['// Kanto Compact Pro: local 5x7/6x8 masks, game-encoded indices.', 'static const u8 sKantoGlyphs[256][8] = {']
    audit=[]
    atlas=[0]*(96*64)
    for i,(ch,rows) in enumerate(glyphs.items()):
        if ch in mapping: output.append('    [0x%02X] = {%s}, // %s'%(mapping[ch],','.join(f'0x{r:02X}' for r in rows),ch))
        width=max((x+1 for row in rows for x in range(5) if row & (16>>x)),default=2)
        audit.append({'character':ch,'rows':rows,'advance':width+1,'engineCode':mapping.get(ch),'review':'native hand-drawn; accent/sign distinct'})
        for y,row in enumerate(rows):
            for x in range(5):
                if row & (16>>x): atlas[((i//16)*8+y)*96+(i%16)*6+x]=14
    output+=['};','static const u8 sKantoAdvances[256] = {']
    output+=['    [0x%02X] = %d,'%(a['engineCode'],a['advance']) for a in audit if a['engineCode'] is not None]
    output+=['};']
    (ROOT/'src/data/rf_kanto_font.h').write_text('\n'.join(output)+'\n',encoding='utf-8')
    png(OUT/'kanto_compact_pro_6x8.png',atlas,96,64)
    (OUT/'font-audit.json').write_text(json.dumps(audit,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    (OUT/'manifest.json').write_text(json.dumps({'paletteRGB555':RGB,'nativeIcons':list(art),'bagTilesBytes':2304,'bagPaletteBytes':32,'menuOBJ':0,'fontGlyphs':len(glyphs),'fontFormat':'one byte per scanline; proportional advance; window BG 4bpp'},indent=2)+'\n',encoding='utf-8')
    legacy=ROOT/'graphics/rojo_fuego/bag_user/manifest.json'
    if legacy.exists():
        previous=legacy.parent/'legacy-import-manifest.json'
        if not previous.exists() and 'sprites' in json.loads(legacy.read_text(encoding='utf-8')):
            previous.write_bytes(legacy.read_bytes())
        legacy.write_text(json.dumps({'generator':'tools/rojo_fuego/make_kanto_ui_art.py','nativeArtwork':'graphics/rojo_fuego/kanto_ui','historicalImport':'legacy-import-manifest.json','frames':names,'smallSheetBytes':1792,'returnSheetBytes':512,'paletteBytes':32,'resampling':False},indent=2)+'\n',encoding='utf-8')
    print('Generated native art and',len(glyphs),'reviewed glyphs. No scaling or color quantization.')


if __name__=='__main__': main()
