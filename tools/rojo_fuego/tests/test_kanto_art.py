"""Native art losslessness, Spanish glyph coverage, and real bounded C rendering."""
from pathlib import Path
import importlib.util
import re
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[3]
sys.path.insert(0,str(ROOT/'tools/rojo_fuego'))
import make_kanto_ui_art as art
from make_menu_icons import read_rgba


class KantoArtTests(unittest.TestCase):
    def test_native_pixels_round_trip_without_quantization(self):
        for name,rows in art.ART.items():
            original=art.drawing(rows)
            data=art.tiles(original,16,16)
            self.assertEqual(len(data),128)
            decoded=[0]*256
            for y in range(16):
                for x in range(16):
                    offset=((y//8)*2+x//8)*32+(y%8)*4+(x%8)//2
                    decoded[y*16+x]=(data[offset]>>(4*(x&1)))&15
            self.assertEqual(decoded,original,name)
            w,png=read_rgba(ROOT/'graphics/rojo_fuego/kanto_ui'/(name+'.png'))
            self.assertEqual((w,len(png)),(16,16))
            for i,index in enumerate(original):
                rgba=png[i//16][(i%16)*4:(i%16)*4+4]
                self.assertEqual(rgba[3],255 if index else 0)
                self.assertEqual(list(rgba[:3]),[round(c*255/31) for c in art.RGB[index]])

    def test_all_spanish_accents_and_distinguishable_numbers(self):
        font=art.font()
        for a,b in zip('ÁÉÍÓÚáéíóúÑñÜü','AEIOUaeiouNnUu'):
            self.assertNotEqual(font[a],font[b],a)
            self.assertTrue(any(font[a]))
        for a,b in [('0','O'),('1','I'),('6','8'),('¿','?'),('¡','!')]:
            self.assertNotEqual(font[a],font[b])
        for name in ['POKÉDEX','DEXNAV','EQUIPO','MOCHILA','PERFIL','GUARDAR','AJUSTES','CLAVES','MISIONES']:
            width=sum(max(x+1 for row in font[ch] for x in range(5) if row&(16>>x))+1 for ch in name)
            self.assertLessEqual(width,49,name)

    def test_production_c_font_bounds_and_unknown_characters(self):
        src=(ROOT/'src/rf_kanto_ui.c').read_text(encoding='utf-8')
        function=re.search(r'void RFKanto_Print\([^\n]+\)\n\{.*?\n\}',src,re.S).group()
        prefix='''
#include <assert.h>
#include <stdint.h>
typedef uint8_t u8; typedef uint32_t u32;
#define EOS 255
#define CHAR_NEWLINE 254
#define CHAR_QUESTION_MARK 172
#define min(a,b) ((a)<(b)?(a):(b))
#include <string.h>
static unsigned left,limit;
static u8 buffer[28*2*32];
struct Window { struct { u8 width,height; } window; u8 *tileData; };
static struct Window gWindows[1] = {{{28,2},buffer}};
static unsigned pixel(unsigned x,unsigned y) {
    unsigned offset=((y/8)*28+x/8)*32+(y%8)*4+(x%8)/2;
    return (buffer[offset]>>(4*(x&1)))&15;
}
'''
        checks='''
int main(void) {
    unsigned w,c,x,y; u8 text[3];
    for(c=0;c<254;c++) for(w=0;w<24;w++) {
        text[0]=c; text[1]=c; text[2]=EOS;
        left=7; limit=7+w; memset(buffer,0,sizeof buffer);
        RFKanto_Print(0,text,7,3,w,2);
        for(y=0;y<16;y++) for(x=0;x<224;x++) if(pixel(x,y)) {
            assert(x>=left && x<limit); assert(y>=3 && y<11); assert(pixel(x,y)==2);
        }
    }
    text[0]=0xBB; text[1]=EOS;
    RFKanto_Print(0,text,223,15,24,2); // clipped physical right/bottom border
    RFKanto_Print(0,text,999,999,24,2); // wholly outside the window
    return 0;
}
'''
        compiler=shutil.which('cc')
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as d:
            file=Path(d)/'font.c'
            binary=Path(d)/'font'
            file.write_text(prefix+(ROOT/'src/data/rf_kanto_font.h').read_text(encoding='utf-8')+function+checks,encoding='utf-8')
            subprocess.run([compiler,'-fsanitize=address,undefined','-g',str(file),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)

if __name__=='__main__': unittest.main()
