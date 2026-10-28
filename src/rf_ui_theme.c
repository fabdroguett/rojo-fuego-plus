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
        RF_UI_DAY_GROUND, RF_UI_DAY_PANEL, RF_UI_DAY_TEXT, RF_UI_DAY_SHADOW,
        RF_UI_DAY_GROUND, RF_UI_DAY_MUTED, RF_UI_DAY_EDGE, RGB(4, 15, 10),
        RF_UI_DAY_SELECTED, RF_UI_DAY_INDICATOR, RF_UI_DAY_TEXT, RF_UI_DAY_PANEL,
        RF_UI_DAY_EDGE, RF_UI_DAY_MUTED, RF_UI_DAY_GROUND, RF_UI_DAY_TEXT,
    };
    if (bank < 16)
        LoadPalette(dark ? night : day, BG_PLTT_ID(bank), PLTT_SIZE_4BPP);
}

// Main-menu illustrations reserve stable shades; UI roles use the existing day/night tokens.
void RFUI_LoadMainMenuPalette(u32 bank, bool32 dark)
{
    u16 colors[16] = {
        RF_UI_DAY_GROUND, RF_UI_BAG_DAY_PAPER, RF_UI_DAY_TEXT, RF_UI_DAY_SHADOW,
        RF_UI_DAY_GROUND, RF_UI_DAY_MUTED, RF_UI_NIGHT_EDGE, RF_UI_BAG_MINT,
        RF_UI_BAG_MINT, RF_UI_GOLD, RGB(28, 9, 8), RGB(7, 14, 22),
        RGB(14, 23, 28), RGB(22, 15, 5), RGB(31, 30, 25), RF_UI_BAG_INK,
    };
    if (dark)
    {
        colors[0] = colors[4] = RF_UI_NIGHT_GROUND;
        colors[1] = RF_UI_NIGHT_PANEL;
        colors[2] = RF_UI_NIGHT_TEXT;
        colors[3] = RF_UI_NIGHT_SHADOW;
        colors[5] = RF_UI_NIGHT_MUTED;
    }
    if (bank < 16)
        LoadPalette(colors, BG_PLTT_ID(bank), PLTT_SIZE_4BPP);
}

// Bag-specific roles keep its indexed illustrations and mint selection identical in both themes.
void RFUI_LoadBagPalette(u32 bank, bool32 dark)
{
    static const u16 night[16] = {
        RF_UI_NIGHT_GROUND, RF_UI_NIGHT_PANEL, RF_UI_NIGHT_TEXT, RF_UI_NIGHT_SHADOW,
        RF_UI_NIGHT_GROUND, RF_UI_NIGHT_MUTED, RF_UI_NIGHT_EDGE, RF_UI_BAG_MINT,
        RF_UI_BAG_MINT, RGB(31, 25, 6), RGB(20, 10, 28), RGB(29, 10, 8),
        RGB(12, 22, 8), RGB(8, 17, 26), RF_UI_BAG_DAY_PAPER, RF_UI_BAG_INK,
    };
    static const u16 day[16] = {
        RGB(1, 9, 10), RF_UI_BAG_DAY_PAPER, RF_UI_BAG_INK, RGB(27, 25, 20),
        RGB(1, 9, 10), RF_UI_DAY_MUTED, RGB(10, 18, 16), RF_UI_BAG_MINT,
        RF_UI_BAG_MINT, RGB(31, 25, 6), RGB(20, 10, 28), RGB(29, 10, 8),
        RGB(12, 22, 8), RGB(8, 17, 26), RF_UI_BAG_DAY_PAPER, RF_UI_BAG_INK,
    };
    if (bank < 16)
        LoadPalette(dark ? night : day, BG_PLTT_ID(bank), PLTT_SIZE_4BPP);
}

// Full-screen field menus (quests, Key System): the map layers are hidden and the backdrop takes
// the theme's ground, so the gaps between framed panels read as the page, not as map slivers.
static EWRAM_DATA u16 sSavedBackdrop = 0;
static EWRAM_DATA bool8 sFieldOverlayActive = FALSE;

void RFUI_BeginFieldOverlay(void)
{
    u16 ground = KeySystem_IsOn(KEY_DARK_MODE) ? RF_UI_NIGHT_GROUND : RF_UI_DAY_GROUND;

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

// Bounded, word-wrapped native text. Returns FALSE when an ellipsis replaces overflow.
bool32 RFUI_PrintParagraph(u8 window, const u8 *text, u32 x, u32 y, u32 width, u32 lines, const u8 *colors)
{
    u8 flat[244], line[96];
    u32 i, position = 0, row;
    bool32 complete;
    const u32 font = FONT_SMALL_NARROWER;
    const u32 inputLimit = sizeof(flat) - 5;
    if (text == NULL || text[0] == EOS)
        text = COMPOUND_STRING("Sin descripción disponible.");
    complete = StringLength(text) < inputLimit;
    // Native format commands can skip four bytes; keep EOS padding if cut mid-command.
    memset(flat, EOS, sizeof(flat));
    StringCopyN(flat, text, inputLimit);
    StripExtCtrlCodes(flat);
    for (i = 0; flat[i] != EOS; i++)
        if (flat[i] == CHAR_NEWLINE || flat[i] == CHAR_PROMPT_SCROLL || flat[i] == CHAR_PROMPT_CLEAR)
            flat[i] = CHAR_SPACE;
    for (row = 0; row < lines; row++)
    {
        u32 start, length = 0, lastSpace = 0;
        while (flat[position] == CHAR_SPACE)
            position++;
        if (flat[position] == EOS)
            break;
        start = position;
        line[0] = EOS;
        while (flat[position] != EOS && length < sizeof(line) - 4)
        {
            line[length] = flat[position];
            line[length + 1] = EOS;
            if (GetStringWidth(font, line, 0) > width)
                break;
            if (flat[position] == CHAR_SPACE)
                lastSpace = length + 1;
            length++;
            position++;
        }
        if (flat[position] != EOS && lastSpace)
        {
            position = start + lastSpace;
            length = lastSpace - 1;
        }
        if (!length && flat[position] != EOS)
        {
            position++;
            complete = FALSE;
        }
        line[length] = EOS;
        if ((row + 1 == lines && flat[position] != EOS) || (!complete && flat[position] == EOS))
        {
            u32 dots = GetStringWidth(font, COMPOUND_STRING("..."), 0);
            while (length && GetStringWidth(font, line, 0) + dots > width)
                line[--length] = EOS;
            if (dots <= width)
                StringAppend(line, COMPOUND_STRING("..."));
            complete = FALSE;
        }
        AddTextPrinterParameterized3(window, font, x, y + row * 9, colors, TEXT_SKIP_DRAW, line);
    }
    return complete && flat[position] == EOS;
}

// Battle healthbox frame (ball_status_bar.pal): 1 ink, 2 paper, 3 shading and ink shadow,
// 4 EXP strip, 6 rim, 8 rim shadow. HP/EXP bars, gender and status colours keep their slots.
void RFUI_ThemeHealthboxPalette(u32 paletteNum)
{
    static const u8 slots[] = {1, 2, 3, 4, 6, 8};
    static const u16 night[] = {
        RF_UI_NIGHT_TEXT, RF_UI_NIGHT_PANEL, RF_UI_NIGHT_SHADOW,
        RF_UI_GOLD, RF_UI_NIGHT_EDGE, RF_UI_SELECTED,
    };
    static const u16 day[] = {
        RF_UI_DAY_TEXT, RF_UI_DAY_PANEL, RF_UI_DAY_SHADOW,
        RF_UI_DAY_INDICATOR, RF_UI_DAY_EDGE, RF_UI_DAY_SHADOW,
    };
    const u16 *colors;
    u32 i;

    if (paletteNum >= 16)
        return;
    colors = KeySystem_IsOn(KEY_DARK_MODE) ? night : day;
    for (i = 0; i < ARRAY_COUNT(slots); i++)
        LoadPalette(&colors[i], OBJ_PLTT_ID(paletteNum) + slots[i], PLTT_SIZEOF(1));
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
