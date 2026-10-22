#include "global.h"
#include "palette.h"
#include "rf_ui_theme.h"
#include "key_system.h"
#include "window.h"
#include "text.h"
#include "string_util.h"
#include "menu.h"

void RFUI_LoadPalette(u32 bank, bool32 dark)
{
    static const u16 night[16] =
    {
        RF_UI_NIGHT_GROUND, RF_UI_NIGHT_PANEL, RF_UI_NIGHT_TEXT, RF_UI_NIGHT_SHADOW,
        RF_UI_NIGHT_GROUND, RF_UI_NIGHT_MUTED, RF_UI_NIGHT_EDGE, RGB(14, 27, 21),
        RF_UI_SELECTED, RF_UI_GOLD, RF_UI_NIGHT_TEXT, RF_UI_NIGHT_PANEL,
        RF_UI_NIGHT_EDGE, RF_UI_NIGHT_MUTED, RF_UI_NIGHT_GROUND, RF_UI_NIGHT_TEXT,
    };
    static const u16 day[16] =
    {
        RGB(23,28,25), RGB(27,31,27), RGB(2,7,8), RGB(22,27,24),
        RGB(23,28,25), RGB(8,15,14), RGB(11,21,19), RGB(4,15,10),
        RGB(30,26,11), RGB(17,11,2), RGB(2,7,8), RGB(27,31,27),
        RGB(11,21,19), RGB(8,15,14), RGB(23,28,25), RGB(2,7,8),
    };
    if (bank < 16)
        LoadPalette(dark ? night : day, BG_PLTT_ID(bank), PLTT_SIZE_4BPP);
}

// Three-tone, two-pixel stepped corners. All dimensions are native pixels.
void RFUI_DrawPanel(u8 window, u32 x, u32 y, u32 width, u32 height, u8 fill, u8 edge)
{
    if (width < 6 || height < 6)
        return;
    if (x == 0 && y == 0
        && width == GetWindowAttribute(window, WINDOW_WIDTH) * 8
        && height == GetWindowAttribute(window, WINDOW_HEIGHT) * 8)
        FillWindowPixelBuffer(window, PIXEL_FILL(fill));
    else
        FillWindowPixelRect(window, PIXEL_FILL(fill), x + 1, y + 1, width - 2, height - 2);
    FillWindowPixelRect(window, PIXEL_FILL(edge), x + 2, y, width - 4, 1);
    FillWindowPixelRect(window, PIXEL_FILL(edge), x + 2, y + height - 1, width - 4, 1);
    FillWindowPixelRect(window, PIXEL_FILL(edge), x, y + 2, 1, height - 4);
    FillWindowPixelRect(window, PIXEL_FILL(edge), x + width - 1, y + 2, 1, height - 4);
    FillWindowPixelRect(window, PIXEL_FILL(edge), x + 1, y + 1, 1, 1);
    FillWindowPixelRect(window, PIXEL_FILL(edge), x + width - 2, y + 1, 1, 1);
    FillWindowPixelRect(window, PIXEL_FILL(edge), x + 1, y + height - 2, 1, 1);
    FillWindowPixelRect(window, PIXEL_FILL(edge), x + width - 2, y + height - 2, 1, 1);
}

void RFUI_PrintFitted(u8 window, const u8 *text, u32 x, u32 y, u32 width, const u8 *colors)
{
    u8 buffer[96];
    u32 font = FONT_SMALL, length;
    if (GetStringWidth(font, text, 0) > width)
        font = FONT_SMALL_NARROWER;
    if (GetStringWidth(font, text, 0) > width)
    {
        StringCopyN(buffer, text, sizeof(buffer) - 4);
        buffer[sizeof(buffer) - 4] = EOS;
        length = StringLength(buffer);
        while (length && GetStringWidth(font, buffer, 0) + 12 > width)
            buffer[--length] = EOS;
        StringAppend(buffer, COMPOUND_STRING("..."));
        text = buffer;
    }
    AddTextPrinterParameterized3(window, font, x, y, colors, TEXT_SKIP_DRAW, text);
}

// Battle healthbox frame (ball_status_bar.pal): 1 ink, 2 paper, 3 shading and ink shadow,
// 4 EXP strip, 6 rim, 8 rim shadow. HP/EXP bars, gender and status colours keep their slots.
void RFUI_ThemeHealthboxPalette(u32 paletteNum)
{
    static const u8 slots[] = {1, 2, 3, 4, 6, 8};
    static const u16 night[] = {
        RF_UI_NIGHT_TEXT, RF_UI_NIGHT_PANEL, RF_UI_NIGHT_SHADOW,
        RF_UI_SELECTED, RF_UI_NIGHT_EDGE, RF_UI_SELECTED,
    };
    u32 i;

    if (paletteNum >= 16 || !KeySystem_IsOn(KEY_DARK_MODE))
        return;
    for (i = 0; i < ARRAY_COUNT(slots); i++)
        LoadPalette(&night[i], OBJ_PLTT_ID(paletteNum) + slots[i], PLTT_SIZEOF(1));
}

// Standard text windows share indices 1 (paper), 2 (ink), 3 (shadow).
// Preserve coloured words (4..9); border indices (10..15) share the night surfaces.
void RFUI_LoadWindowPalette(const u16 *palette, u32 offset)
{
    u16 colors[16];
    u32 i;
    for (i = 0; i < ARRAY_COUNT(colors); i++)
        colors[i] = palette[i];
    if (KeySystem_IsOn(KEY_DARK_MODE))
    {
        colors[1] = RF_UI_NIGHT_PANEL;
        colors[2] = RF_UI_NIGHT_TEXT;
        colors[3] = RF_UI_NIGHT_SHADOW;
        // Border-only indices: avoid white rails and fluorescent message-box trim.
        // Keep ink (2) and the script's semantic word colours (4..9) intact.
        colors[10] = RF_UI_NIGHT_EDGE;
        colors[11] = RF_UI_NIGHT_PANEL;
        colors[12] = RF_UI_NIGHT_EDGE;
        colors[13] = RF_UI_NIGHT_EDGE;
        colors[14] = RF_UI_NIGHT_EDGE;
        colors[15] = RF_UI_NIGHT_SHADOW;
    }
    LoadPalette(colors, offset, sizeof(colors));
}
