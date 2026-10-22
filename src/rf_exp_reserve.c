#include "global.h"
#include "item.h"
#include "key_system.h"
#include "rf_exp_reserve.h"

#define RF_EXP_SAVE_MAGIC 0x45585031
#define RF_EXP_RESERVE_MAX 9999999u

static void EnsureExpSave(void)
{
    if (gSaveBlock3Ptr->rfExpReserveMagic != RF_EXP_SAVE_MAGIC)
    {
        gSaveBlock3Ptr->rfExpReserve = 0;
        gSaveBlock3Ptr->rfExpReserveMagic = RF_EXP_SAVE_MAGIC;
    }
}

u32 RFExpReserve_GetBalance(void)
{
    if (!IS_FRLG)
        return 0;
    EnsureExpSave();
    return gSaveBlock3Ptr->rfExpReserve;
}

void RFExpReserve_Store(u32 amount)
{
    if (!IS_FRLG || !KeySystem_IsOn(KEY_EXP_RESERVE))
        return;
    EnsureExpSave();
    if (gSaveBlock3Ptr->rfExpReserve > RF_EXP_RESERVE_MAX)
        gSaveBlock3Ptr->rfExpReserve = RF_EXP_RESERVE_MAX;
    gSaveBlock3Ptr->rfExpReserve += min(amount, RF_EXP_RESERVE_MAX - gSaveBlock3Ptr->rfExpReserve);
}

u32 RFExpReserve_ApplyCap(u32 currentExp, u32 capExp, u32 reward)
{
    u32 allowed = currentExp >= capExp ? 0 : min(reward, capExp - currentExp);
    RFExpReserve_Store(reward - allowed);
    return allowed;
}

u32 RFExpReserve_Claim(void)
{
    static const struct { enum Item item; u32 exp; } candies[] =
    {
        {ITEM_EXP_CANDY_XL, 30000}, {ITEM_EXP_CANDY_L, 10000},
        {ITEM_EXP_CANDY_M, 3000}, {ITEM_EXP_CANDY_S, 800}, {ITEM_EXP_CANDY_XS, 100},
    };
    u32 i, claimed = 0;
    if (!IS_FRLG)
        return 0;
    EnsureExpSave();
    // Five bounded attempts; never loop once per stored EXP point or candy.
    for (i = 0; i < ARRAY_COUNT(candies); i++)
    {
        u32 count = min(gSaveBlock3Ptr->rfExpReserve / candies[i].exp, 999);
        if (count && AddBagItem(candies[i].item, count))
        {
            gSaveBlock3Ptr->rfExpReserve -= count * candies[i].exp;
            claimed += count;
        }
    }
    return claimed;
}
