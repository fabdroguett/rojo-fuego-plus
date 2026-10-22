#include "global.h"
#include "palette.h"
#include "rf_ui_theme.h"
#include "key_system.h"

void RFUI_LoadPalette(u32 bank, bool32 dark)
{
    static const u16 night[16] =
    {
        RF_UI_NIGHT_GROUND, RF_UI_NIGHT_PANEL, RF_UI_NIGHT_TEXT, RF_UI_NIGHT_SHADOW,
        RF_UI_NIGHT_GROUND, RF_UI_NIGHT_MUTED, RF_UI_NIGHT_EDGE, RGB(14, 27, 21),
        RGB(30, 12, 10), RGB(30, 24, 13), RF_UI_NIGHT_TEXT, RF_UI_NIGHT_PANEL,
        RF_UI_NIGHT_EDGE, RF_UI_NIGHT_MUTED, RF_UI_NIGHT_GROUND, RF_UI_NIGHT_TEXT,
    };
    static const u16 day[16] =
    {
        RGB(28,29,28), RGB(31,31,30), RGB(4,5,6), RGB(27,28,27),
        RGB(28,29,28), RGB(11,12,13), RGB(20,21,21), RGB(4,15,10),
        RGB(23,5,4), RGB(17,11,2), RGB(4,5,6), RGB(31,31,30),
        RGB(20,21,21), RGB(11,12,13), RGB(28,29,28), RGB(4,5,6),
    };
    if (bank < 16)
        LoadPalette(dark ? night : day, BG_PLTT_ID(bank), PLTT_SIZE_4BPP);
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
