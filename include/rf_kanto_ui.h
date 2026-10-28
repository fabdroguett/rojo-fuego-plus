#ifndef GUARD_RF_KANTO_UI_H
#define GUARD_RF_KANTO_UI_H
#include "constants/rf_menu_icons.h"

// Local bitmap font and native indexed art. Never changes an engine font ID.
u32 RFKanto_TextWidth(const u8 *text);
void RFKanto_Print(u8 window, const u8 *text, u32 x, u32 y, u32 width, u8 ink);
void RFKanto_DrawIcon(u8 window, enum RFMenuIcon icon, u32 x, u32 y);
enum RFKantoControl { RF_KANTO_A, RF_KANTO_B, RF_KANTO_R, RF_KANTO_DPAD, RF_KANTO_L };
void RFKanto_DrawControl(u8 window, enum RFKantoControl control, u32 x, u32 y);
void RFKanto_DrawCursor(u8 window, u32 x, u32 y);

#endif
