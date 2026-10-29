#ifndef GUARD_RF_TITLE_SCREEN_H
#define GUARD_RF_TITLE_SCREEN_H

void CB2_InitTitleScreenRF(void);
void CB2_RFTitleToClearSave(void);
void CB2_RFTitleToResetRtc(void);
void CB2_RFTitleToBerryFix(void);
void RFTitle_LoadMenuBackdrop(u8 skyBg, u8 rangeBg, u8 skyPalette, u8 rangePalette);

#endif // GUARD_RF_TITLE_SCREEN_H
