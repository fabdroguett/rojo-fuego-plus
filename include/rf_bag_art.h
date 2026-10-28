#ifndef GUARD_RF_BAG_ART_H
#define GUARD_RF_BAG_ART_H

void RFBagArt_Init(void);
void RFBagArt_ShowCategories(bool32 show);
void RFBagArt_ShowPocketArrows(bool32 show);
void RFBagArt_SetCursor(u32 y, bool32 show);
void RFBagArt_SetScroll(u32 offset, u32 visible, u32 total);
void RFBagArt_HideTransient(void);
void RFBagArt_ShowReturn(bool32 show);
u8 RFBagArt_CreateReturnRow(s16 x, s16 y);

#endif
