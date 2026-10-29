#ifndef GUARD_RF_UI_THEME_H
#define GUARD_RF_UI_THEME_H

#include "constants/rgb.h"
#include "constants/rf_menu_icons.h"

// Night tokens: "Kanto al atardecer" (DESIGN.md), shared by every dark screen.
#define RF_UI_NIGHT_GROUND RGB(2, 2, 3)
#define RF_UI_NIGHT_PANEL RGB(4, 3, 5)
#define RF_UI_NIGHT_PANEL_HI RGB(6, 5, 7) // top inner line of framed windows
#define RF_UI_NIGHT_EDGE RGB(13, 10, 14)
#define RF_UI_NIGHT_TEXT RGB(30, 29, 27)
#define RF_UI_NIGHT_MUTED RGB(22, 20, 20)
#define RF_UI_NIGHT_SHADOW RGB(1, 1, 2)
#define RF_UI_SELECTED RGB(11, 6, 6)
#define RF_UI_GOLD RGB(31, 25, 6)
#define RF_UI_FIRE_ACCENT RGB(30, 12, 10)
#define RF_UI_BATTLE_SELECTED RGB(31, 22, 9) // ember: the selection colour of the whole game
#define RF_UI_BATTLE_FRAME_DAY RGB(5, 14, 14) // battle textbox ground around the menu cards
#define RF_UI_DAY_GROUND RGB(23, 28, 25)
#define RF_UI_DAY_PANEL RGB(27, 31, 27)
#define RF_UI_DAY_EDGE RGB(11, 21, 19)
#define RF_UI_DAY_TEXT RGB(2, 7, 8)
#define RF_UI_DAY_MUTED RGB(8, 15, 14)
#define RF_UI_DAY_SHADOW RGB(22, 27, 24)
#define RF_UI_DAY_SELECTED RGB(30, 26, 11)
#define RF_UI_DAY_INDICATOR RGB(17, 11, 2)
#define RF_UI_BATTLE_SELECTED_PAL 11 // BG bank for the mint battle menu cell (free in battle)
void RFUI_LoadPalette(u32 bank, bool32 dark);
void RFUI_LoadBagPalette(u32 bank, bool32 dark);
void RFUI_LoadMainMenuPalette(u32 bank, bool32 dark);
#define RF_UI_BAG_DAY_PAPER RGB(31, 29, 24)
#define RF_UI_BAG_MINT RGB(31, 22, 9) // selection (was mint; now ember like the rest)
#define RF_UI_BAG_INK RGB(2, 7, 8)
void RFUI_LoadWindowPalette(const u16 *palette, u32 offset);
void RFUI_DuskTint(u16 *colors, u32 count); // night theme: pull artwork towards the dusk plum
void RFUI_ThemeHealthboxPalette(u32 paletteNum);
void RFUI_BeginFieldOverlay(void);
void RFUI_EndFieldOverlay(void);
void RFUI_FillRect(u8 window, u8 fill, u32 x, u32 y, u32 width, u32 height);
void RFUI_DrawRing(u8 window, u32 x, u32 y, u32 width, u32 height, u8 edge);
void RFUI_DrawPanel(u8 window, u32 x, u32 y, u32 width, u32 height, u8 fill, u8 edge);
#define RFUI_NO_SHADOW 0xFF
void RFUI_DrawIcon(u8 window, enum RFMenuIcon icon, u32 x, u32 y, u8 ink, u8 shadow);
void RFUI_PrintFitted(u8 window, const u8 *text, u32 x, u32 y, u32 width, const u8 *colors);
bool32 RFUI_PrintParagraph(u8 window, const u8 *text, u32 x, u32 y, u32 width, u32 lines, const u8 *colors);

#endif
