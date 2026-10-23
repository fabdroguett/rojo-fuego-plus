#include "global.h"
#include "event_data.h"
#include "pokemon.h"
#include "item.h"
#include "rf_traveler.h"
#include "rf_quests.h"
#include "string_util.h"
#include "text.h"
#include "constants/flags.h"
#include "constants/vars.h"

// Rojo Fuego+ traveling challenger (daily challenges, from Crystal Clear): Noa waits in a different
// Pokemon Center each day and offers one battle with special rules. Her team depends on the
// player's progress; she can be beaten once a day (FLAG_RF_DAILY_TRAVELER).

#if IS_FRLG

enum
{
    RULE_INVERSE = 1 << 0,
    RULE_NO_BAG  = 1 << 1,
    RULE_DOUBLE  = 1 << 2,
};

#define TRAVELER_CITIES 7
#define TRAVELER_SAVE_MAGIC 0x4E4F4131

static void EnsureTravelerSave(void)
{
    if (gSaveBlock3Ptr->rfTravelerSaveMagic != TRAVELER_SAVE_MAGIC)
    {
        gSaveBlock3Ptr->rfTravelerPendingPrize = 0;
        gSaveBlock3Ptr->rfTravelerSaveMagic = TRAVELER_SAVE_MAGIC;
    }
}

// Index order matches tools/rojo_fuego/traveler.py
static const u8 *const sCityNames[TRAVELER_CITIES] =
{
    COMPOUND_STRING("Ciudad Verde"),
    COMPOUND_STRING("Ciudad Plateada"),
    COMPOUND_STRING("Ciudad Celeste"),
    COMPOUND_STRING("Ciudad Carmín"),
    COMPOUND_STRING("Ciudad Azulona"),
    COMPOUND_STRING("Ciudad Fucsia"),
    COMPOUND_STRING("Ciudad Azafrán"),
};

static const u8 sRules[TRAVELER_CITIES] =
{
    RULE_INVERSE,
    RULE_NO_BAG,
    RULE_DOUBLE,
    RULE_INVERSE | RULE_NO_BAG,
    RULE_DOUBLE | RULE_INVERSE,
    RULE_DOUBLE | RULE_NO_BAG,
    RULE_DOUBLE | RULE_INVERSE | RULE_NO_BAG,
};

static u32 GetTravelerDay(void)
{
    return VarGet(VAR_DAYS_FRLG);
}

static u32 GetTodaysRules(void)
{
    u32 day = GetTravelerDay();
    // The rule set shifts every week, so a city does not always get the same rules
    return sRules[(day + day / TRAVELER_CITIES) % TRAVELER_CITIES];
}

static u32 CountUsableMons(void)
{
    u32 i, count = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES) != SPECIES_NONE
         && !GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_IS_EGG)
         && GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_HP) != 0)
            count++;
    }
    return count;
}

static u32 GetProgressTier(void)
{
    u32 i, badges = 0;

    if (FlagGet(FLAG_SYS_GAME_CLEAR))
        return 3;
    for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; i++)
    {
        if (FlagGet(i))
            badges++;
    }
    if (badges >= 6)
        return 2;
    if (badges >= 3)
        return 1;
    return 0;
}

static void AppendRule(u8 *dest, const u8 *rule, u32 index, u32 count)
{
    if (index != 0)
        StringAppend(dest, index == count - 1 ? COMPOUND_STRING(" y ") : COMPOUND_STRING(", "));
    StringAppend(dest, rule);
}

// Called on every map load: only today's Pokemon Center shows her, and rules never leak into
// other battles (a lost battle ends with a whiteout warp, which also lands here).
void RF_UpdateTraveler(void)
{
    u32 i, today = GetTravelerDay() % TRAVELER_CITIES;

    for (i = 0; i < TRAVELER_CITIES; i++)
    {
        if (i == today)
            FlagClear(FLAG_RF_TRAVELER_HIDE_0 + i);
        else
            FlagSet(FLAG_RF_TRAVELER_HIDE_0 + i);
    }
    FlagClear(FLAG_RF_INVERSE_BATTLE);
    VarSet(VAR_RF_NO_BAG_USE, 0);
}

// gStringVar1 = today's rules, gStringVar2 = tomorrow's city, VAR_0x8004 = progress tier,
// VAR_0x8005 = TRUE for a double battle, VAR_0x8006 = TRUE if the party can take the battle,
// VAR_0x8007 = TRUE if types are inverted today
void Script_RF_TravelerSetup(void)
{
    u32 rules = GetTodaysRules(), count = 0, index = 0;

    if (rules & RULE_DOUBLE)
        count++;
    if (rules & RULE_INVERSE)
        count++;
    if (rules & RULE_NO_BAG)
        count++;

    gStringVar1[0] = EOS;
    if (rules & RULE_DOUBLE)
        AppendRule(gStringVar1, COMPOUND_STRING("combate doble"), index++, count);
    if (rules & RULE_INVERSE)
        AppendRule(gStringVar1, COMPOUND_STRING("inversa"), index++, count);
    if (rules & RULE_NO_BAG)
        AppendRule(gStringVar1, COMPOUND_STRING("sin mochila"), index++, count);

    StringCopy(gStringVar2, sCityNames[(GetTravelerDay() + 1) % TRAVELER_CITIES]);
    gSpecialVar_0x8004 = GetProgressTier();
    gSpecialVar_0x8005 = (rules & RULE_DOUBLE) != 0;
    gSpecialVar_0x8006 = CountUsableMons() >= ((rules & RULE_DOUBLE) ? 2 : 1);
    gSpecialVar_0x8007 = (rules & RULE_INVERSE) != 0;
}

void Script_RF_TravelerApplyRules(void)
{
    u32 rules = GetTodaysRules();

    if (rules & RULE_INVERSE)
        FlagSet(FLAG_RF_INVERSE_BATTLE);
    if (rules & RULE_NO_BAG)
        VarSet(VAR_RF_NO_BAG_USE, 1);
}

void Script_RF_TravelerClearRules(void)
{
    FlagClear(FLAG_RF_INVERSE_BATTLE);
    VarSet(VAR_RF_NO_BAG_USE, 0);
}

void Script_RF_TravelerRecordWin(void)
{
    EnsureTravelerSave();
    if (!FlagGet(FLAG_RF_DAILY_TRAVELER))
    {
        // Never overwrite a reward waiting from a previous day.
        if (gSaveBlock3Ptr->rfTravelerPendingPrize == 0)
            gSaveBlock3Ptr->rfTravelerPendingPrize = GetProgressTier() + 1;
        FlagSet(FLAG_RF_DAILY_TRAVELER);
        RFQuests_NotifyEvent(RF_QUEST_EVENT_NOA_WIN, 0);
    }
}

void Script_RF_TravelerClaimPrize(void)
{
    static const u16 items[] = {ITEM_GREAT_BALL, ITEM_RARE_CANDY, ITEM_PP_UP, ITEM_BOTTLE_CAP};
    static const u8 counts[] = {5, 2, 2, 1};
    u32 pending;

    EnsureTravelerSave();
    pending = gSaveBlock3Ptr->rfTravelerPendingPrize;
    gSpecialVar_Result = 0; // no outstanding reward
    if (pending == 0 || pending > ARRAY_COUNT(items))
        return;
    gSpecialVar_Result = 2; // bag full; preserve the complete reward
    if (!AddBagItem(items[pending - 1], counts[pending - 1]))
        return;
    gSpecialVar_0x8000 = items[pending - 1];
    gSpecialVar_0x8001 = counts[pending - 1];
    gSaveBlock3Ptr->rfTravelerPendingPrize = 0;
    gSpecialVar_Result = 1;
}

#else

void RF_UpdateTraveler(void)
{
}

#endif
