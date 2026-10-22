#ifndef GUARD_RF_UI_THEME_H
#define GUARD_RF_UI_THEME_H

#include "constants/rgb.h"

#define RF_UI_NIGHT_GROUND RGB(3, 3, 4)
#define RF_UI_NIGHT_PANEL RGB(5, 5, 6)
#define RF_UI_NIGHT_EDGE RGB(9, 9, 10)
#define RF_UI_NIGHT_TEXT RGB(30, 29, 27)
#define RF_UI_NIGHT_MUTED RGB(22, 22, 21)
#define RF_UI_NIGHT_SHADOW RGB(2, 2, 3)
void RFUI_LoadPalette(u32 bank, bool32 dark);
void RFUI_LoadWindowPalette(const u16 *palette, u32 offset);

#endif
