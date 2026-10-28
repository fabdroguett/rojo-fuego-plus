#include "global.h"
#include "rf_ui_theme.h"
#include "rf_kanto_ui.h"
#include "window.h"
#include "constants/characters.h"
#include "data/rf_kanto_art.h"
#include "data/rf_kanto_font.h"

u32 RFKanto_TextWidth(const u8 *text)
{
    u32 width = 0;
    // All callers pass resolved, single-line game strings. Bound even malformed input.
    u32 i;
    for (i = 0; i < 256 && text[i] != EOS && text[i] != CHAR_NEWLINE; i++)
        width += sKantoAdvances[text[i]] ? sKantoAdvances[text[i]] : 6;
    return width;
}

void RFKanto_Print(u8 window, const u8 *text, u32 x, u32 y, u32 width, u8 ink)
{
    u32 i, row, col, drawn = 0;
    u32 pitch = gWindows[window].window.width;
    u32 height = gWindows[window].window.height * 8;
    u8 *tiles = gWindows[window].tileData;
    if (tiles == NULL || x >= pitch * 8 || y >= height)
        return;
    width = min(width, pitch * 8 - x);
    ink &= 15;
    for (i = 0; i < 256 && text[i] != EOS && text[i] != CHAR_NEWLINE; i++)
    {
        u32 ch = text[i];
        u32 advance = sKantoAdvances[ch];
        if (!advance)
        {
            ch = CHAR_QUESTION_MARK;
            advance = sKantoAdvances[ch];
        }
        if (advance > width - drawn)
            break;
        // Write directly to the engine's tiled 4bpp window buffer. Avoid a
        // rectangle-function call for every tiny stroke during cursor redraws.
        for (row = 0; row < 8 && y + row < height; row++)
            for (col = 0; col < 5; col++)
            {
                if (sKantoGlyphs[ch][row] & (16 >> col))
                {
                    u32 px = x + drawn + col;
                    u32 py = y + row;
                    u8 *pixel = tiles + (((py >> 3) * pitch + (px >> 3)) << 5)
                        + ((py & 7) << 2) + ((px & 7) >> 1);
                    *pixel = (px & 1) ? ((*pixel & 15) | (ink << 4)) : ((*pixel & 240) | ink);
                }
            }
        drawn += advance;
    }
}

void RFKanto_DrawIcon(u8 window, enum RFMenuIcon icon, u32 x, u32 y)
{
    const u32 *data;
    switch (icon)
    {
    case RF_ICON_POKEDEX: data = sKanto_pokedex; break;
    case RF_ICON_TEAM: data = sKanto_equipo; break;
    case RF_ICON_BAG: data = sKanto_mochila; break;
    case RF_ICON_DEXNAV: data = sKanto_dexnav; break;
    case RF_ICON_PROFILE: data = sKanto_perfil; break;
    case RF_ICON_SAVE: data = sKanto_guardar; break;
    case RF_ICON_OPTIONS: data = sKanto_ajustes; break;
    case RF_ICON_KEY: data = sKanto_claves; break;
    case RF_ICON_QUESTS: data = sKanto_misiones; break;
    default: RFUI_DrawIcon(window, icon, x, y, 7, RFUI_NO_SHADOW); return;
    }
    BlitBitmapToWindow(window, (const u8 *)data, x, y, 16, 16);
}

void RFKanto_DrawControl(u8 window, enum RFKantoControl control, u32 x, u32 y)
{
    static const u32 *const images[] = {sKanto_button_A, sKanto_button_B, sKanto_button_R, sKanto_dpad, sKanto_button_L};
    if (control < ARRAY_COUNT(images))
        BlitBitmapToWindow(window, (const u8 *)images[control], x, y, 8, 8);
}

void RFKanto_DrawCursor(u8 window, u32 x, u32 y)
{
    // Source rectangle is cropped, never scaled, from the native 16x16 arrow.
    BlitBitmapRectToWindow(window, (const u8 *)sKanto_cursor, 5, 3, 16, 16, x, y, 7, 10);
}
