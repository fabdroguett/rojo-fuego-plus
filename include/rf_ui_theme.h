#ifndef GUARD_RF_UI_THEME_H
#define GUARD_RF_UI_THEME_H

#include "constants/rgb.h"

#define RF_UI_NIGHT_GROUND RGB(1, 5, 6)
#define RF_UI_NIGHT_PANEL RGB(2, 9, 10)
#define RF_UI_NIGHT_EDGE RGB(8, 18, 18)
#define RF_UI_NIGHT_TEXT RGB(30, 31, 28)
#define RF_UI_NIGHT_MUTED RGB(18, 25, 23)
#define RF_UI_NIGHT_SHADOW RGB(0, 3, 4)
#define RF_UI_SELECTED RGB(4, 14, 14)
#define RF_UI_GOLD RGB(31, 25, 6)
void RFUI_LoadPalette(u32 bank, bool32 dark);
void RFUI_LoadWindowPalette(const u16 *palette, u32 offset);
void RFUI_ThemeHealthboxPalette(u32 paletteNum);
void RFUI_DrawPanel(u8 window, u32 x, u32 y, u32 width, u32 height, u8 fill, u8 edge);
void RFUI_PrintFitted(u8 window, const u8 *text, u32 x, u32 y, u32 width, const u8 *colors);

#endif
