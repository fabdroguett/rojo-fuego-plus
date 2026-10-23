#ifndef GUARD_RF_UI_THEME_H
#define GUARD_RF_UI_THEME_H

#include "constants/rgb.h"
#include "constants/rf_menu_icons.h"

// Colour roles of the selectable UI theme (Key System "Tema de color"). Every theme has a day
// and a night variant; the macros below read the active theme, so they are not constant
// expressions and cannot initialise static data.
enum RFUIRole
{
    RF_ROLE_GROUND,
    RF_ROLE_PANEL,
    RF_ROLE_EDGE,
    RF_ROLE_TEXT,
    RF_ROLE_MUTED,
    RF_ROLE_SHADOW,
    RF_ROLE_SELECTED,
    RF_ROLE_GOLD,
    RF_ROLE_ACCENT,
    RF_ROLE_BAR,       // Pokédex tab bar and its three companion tones
    RF_ROLE_BAR_LIGHT,
    RF_ROLE_BAR_DARK,
    RF_ROLE_BAR_EDGE,
    RF_ROLE_COUNT
};

u16 RFUI_Color(bool32 dark, enum RFUIRole role);

#define RF_UI_NIGHT_GROUND RFUI_Color(TRUE, RF_ROLE_GROUND)
#define RF_UI_NIGHT_PANEL RFUI_Color(TRUE, RF_ROLE_PANEL)
#define RF_UI_NIGHT_EDGE RFUI_Color(TRUE, RF_ROLE_EDGE)
#define RF_UI_NIGHT_TEXT RFUI_Color(TRUE, RF_ROLE_TEXT)
#define RF_UI_NIGHT_MUTED RFUI_Color(TRUE, RF_ROLE_MUTED)
#define RF_UI_NIGHT_SHADOW RFUI_Color(TRUE, RF_ROLE_SHADOW)
#define RF_UI_NIGHT_ACCENT RFUI_Color(TRUE, RF_ROLE_ACCENT)
#define RF_UI_SELECTED RFUI_Color(TRUE, RF_ROLE_SELECTED)
#define RF_UI_GOLD RFUI_Color(TRUE, RF_ROLE_GOLD)
#define RF_UI_DAY_GROUND RFUI_Color(FALSE, RF_ROLE_GROUND)
#define RF_UI_DAY_PANEL RFUI_Color(FALSE, RF_ROLE_PANEL)
#define RF_UI_DAY_EDGE RFUI_Color(FALSE, RF_ROLE_EDGE)
#define RF_UI_DAY_TEXT RFUI_Color(FALSE, RF_ROLE_TEXT)
#define RF_UI_DAY_MUTED RFUI_Color(FALSE, RF_ROLE_MUTED)
#define RF_UI_DAY_SHADOW RFUI_Color(FALSE, RF_ROLE_SHADOW)
#define RF_UI_DAY_SELECTED RFUI_Color(FALSE, RF_ROLE_SELECTED)
#define RF_UI_DAY_GOLD RFUI_Color(FALSE, RF_ROLE_GOLD)
#define RF_UI_DAY_ACCENT RFUI_Color(FALSE, RF_ROLE_ACCENT)
#define RF_UI_BATTLE_SELECTED_PAL 11 // BG bank for the gold battle menu cell (free in battle)
void RFUI_LoadPalette(u32 bank, bool32 dark);
void RFUI_LoadNightRole(enum RFUIRole role, u32 offset);
void RFUI_LoadWindowPalette(const u16 *palette, u32 offset);
void RFUI_ThemeHealthboxPalette(u32 paletteNum);
void RFUI_BeginFieldOverlay(void);
void RFUI_EndFieldOverlay(void);
void RFUI_FillRect(u8 window, u8 fill, u32 x, u32 y, u32 width, u32 height);
void RFUI_DrawRing(u8 window, u32 x, u32 y, u32 width, u32 height, u8 edge);
void RFUI_DrawPanel(u8 window, u32 x, u32 y, u32 width, u32 height, u8 fill, u8 edge);
#define RFUI_NO_SHADOW 0xFF
void RFUI_DrawIcon(u8 window, enum RFMenuIcon icon, u32 x, u32 y, u8 ink, u8 shadow);
void RFUI_PrintFitted(u8 window, const u8 *text, u32 x, u32 y, u32 width, const u8 *colors);

#endif
