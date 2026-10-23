#include "global.h"
#include "palette.h"
#include "rf_ui_theme.h"
#include "key_system.h"
#include "window.h"
#include "text.h"
#include "string_util.h"
#include "menu.h"
#include "bg.h"
#include "data/rf_menu_icons.h"

// One row per enum KeyColorTheme; Menta keeps the original Rojo Fuego+ colours exactly.
// Role order: ground, panel, edge, text, muted, shadow, selected, gold, accent,
// bar, bar light, bar dark, bar edge.
static const u16 sThemeColors[KEY_COLOR_THEME_COUNT][2][RF_ROLE_COUNT] =
{
    [KEY_COLOR_THEME_MINT] = {
        {RGB(23,28,25), RGB(27,31,27), RGB(11,21,19), RGB(2,7,8), RGB(8,15,14), RGB(22,27,24), RGB(30,26,11),
         RGB(17,11,2), RGB(4,15,10), RGB(3,12,11), RGB(8,18,15), RGB(2,9,8), RGB(10,20,17)},
        {RGB(1,5,6), RGB(2,9,10), RGB(8,18,18), RGB(30,31,28), RGB(18,25,23), RGB(0,3,4), RGB(4,14,14),
         RGB(31,25,6), RGB(14,27,21), RGB(4,14,14), RGB(2,9,10), RGB(1,5,6), RGB(8,18,18)},
    },
    [KEY_COLOR_THEME_CRIMSON] = {
        {RGB(29,24,25), RGB(31,29,29), RGB(22,8,12), RGB(8,2,4), RGB(15,7,9), RGB(28,22,23), RGB(31,25,11),
         RGB(17,11,2), RGB(18,3,8), RGB(17,3,7), RGB(23,6,11), RGB(10,1,4), RGB(21,7,11)},
        {RGB(5,1,2), RGB(9,2,4), RGB(20,6,10), RGB(31,29,29), RGB(25,18,20), RGB(3,0,1), RGB(16,3,7),
         RGB(31,25,6), RGB(29,12,16), RGB(16,3,7), RGB(9,2,4), RGB(5,1,2), RGB(20,6,10)},
    },
    [KEY_COLOR_THEME_AZURE] = {
        {RGB(24,27,31), RGB(29,30,31), RGB(8,15,27), RGB(2,4,10), RGB(7,11,19), RGB(22,25,30), RGB(31,26,11),
         RGB(17,11,2), RGB(3,10,24), RGB(3,10,23), RGB(6,14,27), RGB(1,6,15), RGB(7,14,26)},
        {RGB(1,3,7), RGB(2,5,11), RGB(7,14,25), RGB(28,30,31), RGB(18,22,28), RGB(0,1,4), RGB(4,11,22),
         RGB(31,25,6), RGB(12,21,31), RGB(4,11,22), RGB(2,5,11), RGB(1,3,7), RGB(7,14,25)},
    },
    [KEY_COLOR_THEME_VIOLET] = {
        {RGB(27,25,30), RGB(30,29,31), RGB(17,10,24), RGB(6,2,10), RGB(12,8,17), RGB(25,23,29), RGB(31,26,11),
         RGB(17,11,2), RGB(12,4,20), RGB(12,4,19), RGB(17,9,25), RGB(7,2,12), RGB(16,9,23)},
        {RGB(4,1,7), RGB(7,3,11), RGB(16,9,24), RGB(30,29,31), RGB(23,19,27), RGB(2,0,4), RGB(12,5,19),
         RGB(31,25,6), RGB(24,16,31), RGB(12,5,19), RGB(7,3,11), RGB(4,1,7), RGB(16,9,24)},
    },
    [KEY_COLOR_THEME_AMBER] = {
        {RGB(30,27,21), RGB(31,30,26), RGB(24,14,3), RGB(9,4,1), RGB(16,10,4), RGB(29,25,18), RGB(31,22,9),
         RGB(17,9,1), RGB(20,10,1), RGB(19,9,1), RGB(25,15,4), RGB(11,5,1), RGB(23,13,3)},
        {RGB(6,3,1), RGB(10,5,2), RGB(22,13,4), RGB(31,30,26), RGB(27,22,15), RGB(3,1,0), RGB(18,9,2),
         RGB(31,27,10), RGB(31,20,6), RGB(18,9,2), RGB(10,5,2), RGB(6,3,1), RGB(22,13,4)},
    },
    [KEY_COLOR_THEME_GRAPHITE] = {
        {RGB(25,25,26), RGB(30,30,30), RGB(11,11,13), RGB(3,3,4), RGB(9,9,10), RGB(23,23,24), RGB(31,26,11),
         RGB(17,11,2), RGB(6,6,8), RGB(6,6,8), RGB(12,12,14), RGB(3,3,4), RGB(10,10,12)},
        {RGB(2,2,3), RGB(4,4,5), RGB(12,12,14), RGB(30,30,30), RGB(20,20,22), RGB(1,1,1), RGB(8,8,10),
         RGB(31,25,6), RGB(22,22,25), RGB(8,8,10), RGB(4,4,5), RGB(2,2,3), RGB(12,12,14)},
    },
};

u16 RFUI_Color(bool32 dark, enum RFUIRole role)
{
    if (role >= RF_ROLE_COUNT)
        return 0;
    return sThemeColors[KeySystem_GetColorTheme()][dark != FALSE][role];
}

// Loads one night colour of the active theme into a palette slot (dark-mode recolour tables).
void RFUI_LoadNightRole(enum RFUIRole role, u32 offset)
{
    u16 color = RFUI_Color(TRUE, role);
    LoadPalette(&color, offset, PLTT_SIZEOF(1));
}

void RFUI_LoadPalette(u32 bank, bool32 dark)
{
    // Slot -> role. Slot 7 is the accent; the rest repeat the surfaces for the window chrome.
    static const u8 roles[16] =
    {
        RF_ROLE_GROUND, RF_ROLE_PANEL, RF_ROLE_TEXT, RF_ROLE_SHADOW,
        RF_ROLE_GROUND, RF_ROLE_MUTED, RF_ROLE_EDGE, RF_ROLE_ACCENT,
        RF_ROLE_SELECTED, RF_ROLE_GOLD, RF_ROLE_TEXT, RF_ROLE_PANEL,
        RF_ROLE_EDGE, RF_ROLE_MUTED, RF_ROLE_GROUND, RF_ROLE_TEXT,
    };
    u16 colors[16];
    u32 i;

    if (bank >= 16)
        return;
    for (i = 0; i < ARRAY_COUNT(colors); i++)
        colors[i] = RFUI_Color(dark, roles[i]);
    LoadPalette(colors, BG_PLTT_ID(bank), PLTT_SIZE_4BPP);
}

// Full-screen field menus (quests, Key System): the map layers are hidden and the backdrop takes
// the theme's ground, so the gaps between framed panels read as the page, not as map slivers.
static EWRAM_DATA u16 sSavedBackdrop = 0;
static EWRAM_DATA bool8 sFieldOverlayActive = FALSE;

void RFUI_BeginFieldOverlay(void)
{
    u16 ground = RFUI_Color(KeySystem_IsOn(KEY_DARK_MODE), RF_ROLE_GROUND);

    if (!sFieldOverlayActive)
    {
        sSavedBackdrop = gPlttBufferUnfaded[0];
        HideBg(1);
        HideBg(2);
        HideBg(3);
        sFieldOverlayActive = TRUE;
    }
    gPlttBufferUnfaded[0] = gPlttBufferFaded[0] = ground;
}

void RFUI_EndFieldOverlay(void)
{
    if (!sFieldOverlayActive)
        return;
    gPlttBufferUnfaded[0] = gPlttBufferFaded[0] = sSavedBackdrop;
    ShowBg(1);
    ShowBg(2);
    ShowBg(3);
    sFieldOverlayActive = FALSE;
}

// Three-tone, two-pixel stepped corners. All dimensions are native pixels.
// Fills a window rectangle a tile row at a time. FillWindowPixelRect writes one nibble per
// call from ROM code, which made full-screen RF menus take several frames to repaint and
// drop button presses; aligned 8-pixel spans here are a single 32-bit store.
void RFUI_FillRect(u8 window, u8 fill, u32 x, u32 y, u32 width, u32 height)
{
    u8 *tiles = gWindows[window].tileData;
    u32 pitch = gWindows[window].window.width;
    u32 xEnd = min(x + width, pitch * 8);
    u32 yEnd = min(y + height, gWindows[window].window.height * 8u);
    u32 word = (fill & 0xF) * 0x11111111u;
    u32 row, col;

    if (tiles == NULL)
        return;
    for (row = y; row < yEnd; row++)
    {
        u8 *line = tiles + (((row >> 3) * pitch) << 5) + ((row & 7) << 2);
        for (col = x; col < xEnd;)
        {
            if ((col & 7) == 0 && col + 8 <= xEnd)
            {
                *(u32 *)(line + ((col >> 3) << 5)) = word;
                col += 8;
            }
            else
            {
                u8 *pixel = line + ((col >> 3) << 5) + ((col & 7) >> 1);
                *pixel = (col & 1) ? ((*pixel & 0x0F) | (word & 0xF0)) : ((*pixel & 0xF0) | (word & 0x0F));
                col++;
            }
        }
    }
}

// Rounded single-pixel outline without touching the interior.
void RFUI_DrawRing(u8 window, u32 x, u32 y, u32 width, u32 height, u8 edge)
{
    if (width < 6 || height < 6)
        return;
    RFUI_FillRect(window, edge, x + 2, y, width - 4, 1);
    RFUI_FillRect(window, edge, x + 2, y + height - 1, width - 4, 1);
    RFUI_FillRect(window, edge, x, y + 2, 1, height - 4);
    RFUI_FillRect(window, edge, x + width - 1, y + 2, 1, height - 4);
    RFUI_FillRect(window, edge, x + 1, y + 1, 1, 1);
    RFUI_FillRect(window, edge, x + width - 2, y + 1, 1, 1);
    RFUI_FillRect(window, edge, x + 1, y + height - 2, 1, 1);
    RFUI_FillRect(window, edge, x + width - 2, y + height - 2, 1, 1);
}

void RFUI_DrawPanel(u8 window, u32 x, u32 y, u32 width, u32 height, u8 fill, u8 edge)
{
    if (width < 6 || height < 6)
        return;
    if (x == 0 && y == 0
        && width == GetWindowAttribute(window, WINDOW_WIDTH) * 8
        && height == GetWindowAttribute(window, WINDOW_HEIGHT) * 8)
        FillWindowPixelBuffer(window, PIXEL_FILL(fill));
    else
        RFUI_FillRect(window, fill, x + 1, y + 1, width - 2, height - 2);
    RFUI_DrawRing(window, x, y, width, height, edge);
}

// Paints a 1-bit menu icon with theme roles: an optional 1px drop shadow, then the ink.
void RFUI_DrawIcon(u8 window, enum RFMenuIcon icon, u32 x, u32 y, u8 ink, u8 shadow)
{
    u32 pass, row, col, start;
    if (icon >= RF_ICON_COUNT)
        return;
    for (pass = (shadow == RFUI_NO_SHADOW); pass < 2; pass++)
    {
        u32 offset = pass == 0;
        for (row = 0; row < 16; row++)
        {
            u32 bits = sRFMenuIconMasks[icon][row];
            for (col = 0; col < 16;)
            {
                if (!(bits & (0x8000 >> col)))
                {
                    col++;
                    continue;
                }
                for (start = col; col < 16 && (bits & (0x8000 >> col)); col++)
                    ;
                RFUI_FillRect(window, pass == 0 ? shadow : ink, x + start + offset, y + row + offset, col - start, 1);
            }
        }
    }
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
    const u16 night[] = {
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
