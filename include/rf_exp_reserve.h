#ifndef GUARD_RF_EXP_RESERVE_H
#define GUARD_RF_EXP_RESERVE_H

u32 RFExpReserve_GetBalance(void);
void RFExpReserve_Store(u32 amount);
u32 RFExpReserve_ApplyCap(u32 currentExp, u32 capExp, u32 reward);
u32 RFExpReserve_Claim(void);

#endif
