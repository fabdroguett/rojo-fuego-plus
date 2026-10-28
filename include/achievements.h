#ifndef GUARD_ACHIEVEMENTS_H
#define GUARD_ACHIEVEMENTS_H

#include "constants/rf_achievements.h"

// Rojo Fuego+ achievements. The list lives in tools/rojo_fuego/achievements.py, which generates
// include/constants/rf_achievements.h and src/data/rf_achievements.h.

enum AchievementTier
{
    ACH_TIER_BRONZE,
    ACH_TIER_SILVER,
    ACH_TIER_GOLD,
    ACH_TIER_PLATINUM,
    ACH_TIER_COUNT
};

enum AchievementCondition
{
    ACH_COND_FLAG,            // condArg flag is set
    ACH_COND_DEX_CAUGHT,      // condValue species caught (National Pokédex)
    ACH_COND_DEX_SEEN,        // condValue species seen
    ACH_COND_SPECIES_SET,     // every species in set caught
    ACH_COND_GAME_STAT,       // game stat condArg >= condValue
    ACH_COND_COUNTER,         // achievement counter condArg >= condValue
    ACH_COND_MONEY,           // money >= condValue
    ACH_COND_PARTY_LEVEL,     // a party Pokémon at level >= condValue
    ACH_COND_PARTY_ALL_LEVEL, // six party Pokémon at level >= condValue
    ACH_COND_ITEM_SET,        // condValue items of set in the bag or PC
    ACH_COND_TYPE_CAUGHT,     // condValue species of type condArg caught
    ACH_COND_SPECIAL,         // special check condArg (enum AchievementSpecial) with condValue
    ACH_COND_EVENT,           // unlocked only by Achievement_Unlock from game code
    ACH_COND_PERCENT,         // condValue % of all achievements unlocked
    ACH_COND_TIER_ALL,        // every other achievement of tier condArg unlocked
    ACH_COND_HIDDEN_ALL,      // every other hidden achievement unlocked
};

struct Achievement
{
    const u8 *name;
    const u8 *description;
    u8 category;
    u8 tier;
    bool8 hidden;
    u8 condType;
    u16 condArg;
    const u16 *set;
    u16 setSize;
    u32 condValue;
    u16 rewardItem;
    u8 rewardQuantity;
    const u8 *rewardTitle;
};

extern const struct Achievement gAchievements[ACH_COUNT];
extern bool8 gRfDexNavBattle;

void Achievements_OverworldUpdate(void);
void Achievements_CheckAll(void);
void Achievement_Unlock(u32 id);
bool32 Achievement_IsUnlocked(u32 id);
void Achievement_AddToCounter(enum AchievementCounter counter, u32 amount);
void Achievement_SetCounterMax(enum AchievementCounter counter, u32 value);
u32 Achievement_GetCounter(enum AchievementCounter counter);
void Achievements_HideNotification(void);
void Achievements_QueueQuestNotice(u32 quest, bool32 here);
void Achievements_OpenMenu(void);
void Achievements_OpenScreen(void (*exitCallback)(void));
void CB2_OpenAchievementsScreenFromBag(void);
const u8 *Achievements_GetEquippedTitle(void);

// Hooks called from game code
void Achievements_OnBattleEnd(void);
void Achievements_OnHallOfFame(void);
void Achievements_OnGimmickUsed(enum AchievementCounter counter);

#endif // GUARD_ACHIEVEMENTS_H
