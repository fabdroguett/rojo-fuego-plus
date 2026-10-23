#include "global.h"
#include "key_system.h"
#include "rf_fonts.h"

// Selectable Latin font styles, built by tools/rojo_fuego/make_rf_fonts.py (credits in CREDITS.md).
// Rows follow enum KeyFontStyle up to KEY_FONT_CLASSIC, which uses graphics/fonts/ as before.
#include "data/rf_font_widths.h"

ALIGNED(4) static const u16 sM5x7Normal[] = INCGFX_U16("graphics/rojo_fuego/fonts/m5x7/latin_normal.png", ".latfont");
ALIGNED(4) static const u16 sM5x7Narrow[] = INCGFX_U16("graphics/rojo_fuego/fonts/m5x7/latin_narrow.png", ".latfont");
ALIGNED(4) static const u16 sM5x7Short[] = INCGFX_U16("graphics/rojo_fuego/fonts/m5x7/latin_short.png", ".latfont");
ALIGNED(4) static const u16 sM5x7ShortNarrow[] = INCGFX_U16("graphics/rojo_fuego/fonts/m5x7/latin_short_narrow.png", ".latfont");
ALIGNED(4) static const u16 sM5x7Small[] = INCGFX_U16("graphics/rojo_fuego/fonts/m5x7/latin_small.png", ".latfont");
ALIGNED(4) static const u16 sM5x7SmallNarrow[] = INCGFX_U16("graphics/rojo_fuego/fonts/m5x7/latin_small_narrow.png", ".latfont");
ALIGNED(4) static const u16 sM6x11Normal[] = INCGFX_U16("graphics/rojo_fuego/fonts/m6x11plus/latin_normal.png", ".latfont");
ALIGNED(4) static const u16 sM6x11Narrow[] = INCGFX_U16("graphics/rojo_fuego/fonts/m6x11plus/latin_narrow.png", ".latfont");
ALIGNED(4) static const u16 sM6x11Short[] = INCGFX_U16("graphics/rojo_fuego/fonts/m6x11plus/latin_short.png", ".latfont");
ALIGNED(4) static const u16 sM6x11ShortNarrow[] = INCGFX_U16("graphics/rojo_fuego/fonts/m6x11plus/latin_short_narrow.png", ".latfont");
ALIGNED(4) static const u16 sM6x11Small[] = INCGFX_U16("graphics/rojo_fuego/fonts/m6x11plus/latin_small.png", ".latfont");
ALIGNED(4) static const u16 sM6x11SmallNarrow[] = INCGFX_U16("graphics/rojo_fuego/fonts/m6x11plus/latin_small_narrow.png", ".latfont");
ALIGNED(4) static const u16 sMonogramNormal[] = INCGFX_U16("graphics/rojo_fuego/fonts/monogram/latin_normal.png", ".latfont");
ALIGNED(4) static const u16 sMonogramNarrow[] = INCGFX_U16("graphics/rojo_fuego/fonts/monogram/latin_narrow.png", ".latfont");
ALIGNED(4) static const u16 sMonogramShort[] = INCGFX_U16("graphics/rojo_fuego/fonts/monogram/latin_short.png", ".latfont");
ALIGNED(4) static const u16 sMonogramShortNarrow[] = INCGFX_U16("graphics/rojo_fuego/fonts/monogram/latin_short_narrow.png", ".latfont");
ALIGNED(4) static const u16 sMonogramSmall[] = INCGFX_U16("graphics/rojo_fuego/fonts/monogram/latin_small.png", ".latfont");
ALIGNED(4) static const u16 sMonogramSmallNarrow[] = INCGFX_U16("graphics/rojo_fuego/fonts/monogram/latin_small_narrow.png", ".latfont");

static const u16 *const sRFFontGlyphs[KEY_FONT_CLASSIC][RF_FONT_SHEET_COUNT] =
{
    [KEY_FONT_M5X7] = {sM5x7Normal, sM5x7Narrow, sM5x7Short, sM5x7ShortNarrow, sM5x7Small, sM5x7SmallNarrow},
    [KEY_FONT_M6X11] = {sM6x11Normal, sM6x11Narrow, sM6x11Short, sM6x11ShortNarrow, sM6x11Small, sM6x11SmallNarrow},
    [KEY_FONT_MONOGRAM] = {sMonogramNormal, sMonogramNarrow, sMonogramShort, sMonogramShortNarrow, sMonogramSmall, sMonogramSmallNarrow},
};

STATIC_ASSERT(ARRAY_COUNT(sRFFontWidths) == KEY_FONT_CLASSIC, RFFontWidthsMatchStyles);

const u16 *RFFont_Glyphs(enum RFFontSheet sheet, const u16 *classic)
{
    enum KeyFontStyle style = KeySystem_GetFontStyle();

    if (style >= KEY_FONT_CLASSIC || sheet >= RF_FONT_SHEET_COUNT)
        return classic;
    return sRFFontGlyphs[style][sheet];
}

const u8 *RFFont_Widths(enum RFFontSheet sheet, const u8 *classic)
{
    enum KeyFontStyle style = KeySystem_GetFontStyle();

    if (style >= KEY_FONT_CLASSIC || sheet >= RF_FONT_SHEET_COUNT)
        return classic;
    return sRFFontWidths[style][sheet];
}
