"""Bounded native description rendering: fallback, wrapping, and extreme input."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]


class BagTextTests(unittest.TestCase):
    def test_native_paragraph_bounds_and_missing_text(self):
        source = (ROOT / "src/rf_ui_theme.c").read_text(encoding="utf-8")
        function = re.search(r"bool32 RFUI_PrintParagraph\([^\n]*\)\n\{.*?\n\}", source, re.S).group()
        strings = (ROOT / "src/string_util.c").read_text(encoding="utf-8")
        strip = re.search(r"void StripExtCtrlCodes\([^\n]*\)\n\{.*?\n\}", strings, re.S).group()
        header = r'''
#include <stdint.h>
#include <string.h>
#include <assert.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int bool32;
#define TRUE 1
#define FALSE 0
#define EOS 255
#define CHAR_SPACE 32
#define CHAR_NEWLINE 254
#define CHAR_PROMPT_SCROLL 250
#define CHAR_PROMPT_CLEAR 251
#define EXT_CTRL_CODE_BEGIN 252
#define FONT_SMALL_NARROWER 11
#define TEXT_SKIP_DRAW 255
#define COMPOUND_STRING(s) ((const u8 *)(s))
static u32 limit, printed, lastY;
static u8 lastLine[128];
static u32 StringLength(const u8 *s) {u32 n=0; while(s[n] && s[n]!=EOS) n++; return n;}
static void StringCopyN(u8 *d, const u8 *s, u32 n) {
    u32 i=0; while(i<n && s[i] && s[i]!=EOS) {d[i]=s[i]; i++;} d[i]=EOS;
}
static u8 GetExtCtrlCodeLength(u8 code) {return code==4 ? 4 : code==EOS ? 0 : 1;}
static u32 GetStringWidth(u32 font, const u8 *s, u32 spacing) {
    u32 width=0; (void)font; (void)spacing;
    while(*s && *s!=EOS) {width+=*s=='i' || *s==' ' ? 1 : *s=='W' ? 3 : 2; s++;} return width;
}
static void StringAppend(u8 *d, const u8 *s) {StringCopyN(d+StringLength(d),s,32);}
static void AddTextPrinterParameterized3(u8 window, u32 font, u32 x, u32 y, const u8 *colors, u8 speed, const u8 *s) {
    (void)window; (void)font; (void)x; (void)colors; (void)speed;
    assert(GetStringWidth(font,s,0)<=limit);
    if(printed) assert(y==lastY+9);
    lastY=y; printed++; StringCopyN(lastLine,s,127);
}
'''
        checks = r'''
static int draw(const u8 *text, u32 width, u32 lines) {
    limit=width; printed=0; lastY=0;
    int fits=RFUI_PrintParagraph(1,text,3,2,width,lines,(const u8[]){0,1,3});
    assert(printed<=lines); return fits;
}
int main(void) {
    u8 empty[]={EOS};
    assert(draw(NULL,60,4) && printed>0);
    assert(draw(empty,60,4) && printed>0);
    assert(draw((const u8 *)"uno dos tres cuatro",12,4) && printed==4);
    u8 newline[]={'u','n','o',CHAR_NEWLINE,'d','o','s',EOS};
    assert(draw(newline,16,4) && printed==1);
    assert(!draw((const u8 *)"palabra larguisima sin espacios suficientes para caber",12,4));
    u32 n=StringLength(lastLine); assert(n>=3 && lastLine[n-1]=='.');
    u8 longText[1024]; memset(longText,'W',1023); longText[1023]=EOS;
    assert(!draw(longText,98,4));
    assert(!draw(longText,1000,4)); // line buffer must reserve room for ellipsis
    u8 controlled[260]; memset(controlled,'W',259); controlled[259]=EOS;
    controlled[237]=EXT_CTRL_CODE_BEGIN; controlled[238]=4;
    controlled[239]=1; controlled[240]=2; controlled[241]=3;
    assert(!draw(controlled,98,4)); // truncation inside a native 4-byte format command
    assert(!draw((const u8 *)"texto",0,4));
    assert(!draw((const u8 *)"texto",20,0) && printed==0);
    return 0;
}
'''
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            cfile = work / "paragraph.c"
            cfile.write_text(header + strip + function + checks, encoding="utf-8")
            binary = work / "paragraph"
            compiled = subprocess.run([compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
                                       "-fsanitize=address,undefined", str(cfile), "-o", str(binary)],
                                      capture_output=True, text=True)
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
