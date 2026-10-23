#ifndef GUARD_RF_FONTS_H
#define GUARD_RF_FONTS_H

// Latin glyph sheets that follow the Key System "Fuente" setting (rf_fonts.c).
// The narrower sheets and the Japanese fonts always use the classic drawing.
enum RFFontSheet
{
    RF_FONT_SHEET_NORMAL,
    RF_FONT_SHEET_NARROW,
    RF_FONT_SHEET_SHORT,
    RF_FONT_SHEET_SHORT_NARROW,
    RF_FONT_SHEET_SMALL,
    RF_FONT_SHEET_SMALL_NARROW,
    RF_FONT_SHEET_COUNT
};

// Glyphs and widths of the selected style, or the classic arrays passed in.
const u16 *RFFont_Glyphs(enum RFFontSheet sheet, const u16 *classic);
const u8 *RFFont_Widths(enum RFFontSheet sheet, const u8 *classic);

#endif // GUARD_RF_FONTS_H
