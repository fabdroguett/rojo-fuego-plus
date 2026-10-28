"""Compare every supplied glyph and native resource. No image manipulation packages."""
import json
import hashlib
import re
from pathlib import Path
from make_menu_icons import read_rgba
from make_kanto_ui_art import ROOT, OUT, font, RGB, drawing, ART, png


def main():
    source=OUT/'reference_source'
    output=ROOT/'build_artifacts/ui/kanto-art'
    output.mkdir(parents=True,exist_ok=True)
    original=json.loads((source/'font/kanto_compact_6x8.json').read_text(encoding='utf-8'))
    w,rows=read_rgba(source/'font/kanto_compact_6x8.png')
    fnt={}
    for line in (source/'font/kanto_compact_6x8.fnt').read_text(encoding='utf-8').splitlines():
        if line.startswith('char '):
            fields={k:int(v) for k,v in re.findall(r'(\w+)=(-?\d+)',line)}
            fnt[chr(fields['id'])]=fields
    glyphs=font()
    audit=[]
    for g in original['characters']:
        ch=g['character']
        assert all(g[k]==fnt[ch][k] for k in ('x','y','width','height'))
        assert g['x']+6<=w and g['y']+8<=len(rows)
        mask=[sum(16>>x for x in range(5) if rows[y+g['y']][(x+g['x'])*4+3]>=128) for y in range(8)]
        audit.append({'character':ch,'originalRows':mask,'nativeRows':glyphs[ch],
                      'changed':mask!=glyphs[ch],'review':'redrawn at native 5x7 in 6x8 cell; proportional spacing'})
    names=['pokedex','dexnav','equipo','mochila','perfil','guardar','ajustes','claves','misiones']
    # Native comparison: top = supplied 16px source, bottom = hand-drawn final 16px.
    palette=[(0,0,0)]
    pixels=[0]*(216*48)
    for i,name in enumerate(names):
        width,old=read_rgba(source/'icons'/('icon_'+name+'.png'))
        assert (width,len(old))==(16,16)
        for y in range(16):
            for x in range(16):
                c=old[y][x*4:x*4+4]
                if c[3]:
                    color=tuple(round(v*31/255) for v in c[:3])
                    if color not in palette: palette.append(color)
                    pixels[(y+4)*216+i*24+x+4]=palette.index(color)
        for y in range(16):
            for x in range(16):
                index=drawing(ART[name])[y*16+x]
                if index:
                    color=RGB[index]
                    if color not in palette: palette.append(color)
                    pixels[(y+28)*216+i*24+x+4]=palette.index(color)
    png(output/'icons-before-after-native.png',pixels,216,48,palette)
    zoom=[pixels[(y//6)*216+x//6] for y in range(48*6) for x in range(216*6)]
    png(output/'icons-before-after-6x.png',zoom,1296,288,palette)
    legacy=json.loads((ROOT/'graphics/rojo_fuego/bag_user/legacy-import-manifest.json').read_text(encoding='utf-8'))
    report={'glyphsReviewed':len(audit),'sourceFNTValidated':True,'glyphs':audit,'previousBagImport':legacy['sprites'],
            'sourceSHA256':{str(p.relative_to(source)):hashlib.sha256(p.read_bytes()).hexdigest() for p in source.rglob('*') if p.is_file()},
            'diagnosis':['Bag UI: large raster downsampled into about 14x14; details and outlines lost.',
                         'Bag row OBJ: affine 512 matrix renders source at half scale; source pixel loss.',
                         'Source font: uppercase acute accents duplicate base letters; malformed 3-pixel stems.',
                         'No bilinear filter in headless mGBA; PNG screenshots are integer nearest-neighbor.',
                         'New UI drawings: direct final pixel masters, exact 4bpp index packing, no quantization.'],
            'sizeDecision':{'gridAndCategories':16,'itemCatalog':24,'returnPreview':32,'rejected':{'24pxGrid':'reduces label width below MISIONES','32pxGrid':'overlaps 27px cards','8x12Font':'more height with no room for wider columns'}},
            'originalGlyphsAllPresent':all(g['character'] in glyphs for g in original['characters'])}
    (output/'source-audit.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Reviewed',len(audit),'original glyphs and',len(names),'icon pairs; native/6x comparisons saved.')


if __name__=='__main__': main()
