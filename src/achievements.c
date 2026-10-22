#include "global.h"
#include "achievements.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "item.h"
#include "key_system.h"
#include "list_menu.h"
#include "main.h"
#include "menu.h"
#include "money.h"
#include "omni_ring.h"
#include "overworld.h"
#include "palette.h"
#include "pokedex.h"
#include "pokemon.h"
#include "rtc.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "wild_encounter.h"
#include "constants/opponents.h"
#include "constants/songs.h"

// Rojo Fuego+ achievements engine.
// Most achievements are checked by looking at the save (flags, Pokédex, game stats, counters) every
// couple of seconds in the overworld; a few are unlocked by hooks (Hall of Fame, battle results).
// Unlocking queues a notification shown at the top of the screen when the player is free to move.
// Rewards are claimed from the achievements menu, opened with the Logros key item.

#include "data/rf_achievements.h"

#define CHECK_INTERVAL_FRAMES   120
#define NOTIFICATION_FRAMES     180
#define NOTIFICATION_QUEUE_SIZE 8
#define COUNTER_MAX             0xFFFF

static const u8 sTierPoints[ACH_TIER_COUNT] = {10, 25, 50, 100};
static const u8 *const sTierNames[ACH_TIER_COUNT] =
{
    [ACH_TIER_BRONZE]   = COMPOUND_STRING("BRONCE"),
    [ACH_TIER_SILVER]   = COMPOUND_STRING("PLATA"),
    [ACH_TIER_GOLD]     = COMPOUND_STRING("ORO"),
    [ACH_TIER_PLATINUM] = COMPOUND_STRING("PLATINO"),
};

// Read by RetroAchievements: a copy of the unlocked bits at an address that does not move
// (the save blocks move around in RAM). tools/rojo_fuego/achievements.py --ra writes the set.
// The last byte is MIRROR_SYNCED once the copy matches the save, so a fresh load does not look
// like every achievement unlocking at once.
#define MIRROR_SYNCED 0xA5
EWRAM_DATA u8 gRfAchievementMirror[ACH_BYTES + 1] = {0};

// Set by the DexNav when its battle starts; read and cleared when the battle ends
EWRAM_DATA bool8 gRfDexNavBattle = FALSE;

static EWRAM_DATA u16 sCheckTimer = 0;
static EWRAM_DATA u16 sNotificationQueue[NOTIFICATION_QUEUE_SIZE] = {0};
static EWRAM_DATA u8 sNotificationCount = 0;
static EWRAM_DATA u8 sNotificationWindowId = 0;
static EWRAM_DATA u16 sNotificationTimer = 0;
static EWRAM_DATA bool8 sNotificationShown = FALSE;

static void SyncMirror(void)
{
    memcpy(gRfAchievementMirror, gSaveBlock3Ptr->achievementsUnlocked, ACH_BYTES);
    gRfAchievementMirror[ACH_BYTES] = MIRROR_SYNCED;
}

static bool32 IsBitSet(const u8 *bits, u32 id)
{
    return (bits[id / 8] >> (id % 8)) & 1;
}

static void SetBit(u8 *bits, u32 id)
{
    bits[id / 8] |= 1 << (id % 8);
}

bool32 Achievement_IsUnlocked(u32 id)
{
    return IsBitSet(gSaveBlock3Ptr->achievementsUnlocked, id);
}

static bool32 Achievement_IsClaimed(u32 id)
{
    return IsBitSet(gSaveBlock3Ptr->achievementsClaimed, id);
}

u32 Achievement_GetCounter(enum AchievementCounter counter)
{
    return gSaveBlock3Ptr->achievementCounters[counter];
}

void Achievement_AddToCounter(enum AchievementCounter counter, u32 amount)
{
    u32 value = gSaveBlock3Ptr->achievementCounters[counter] + amount;
    gSaveBlock3Ptr->achievementCounters[counter] = min(value, COUNTER_MAX);
}

void Achievement_SetCounterMax(enum AchievementCounter counter, u32 value)
{
    if (value > gSaveBlock3Ptr->achievementCounters[counter])
        gSaveBlock3Ptr->achievementCounters[counter] = min(value, COUNTER_MAX);
}

void Achievement_Unlock(u32 id)
{
    if (id >= ACH_COUNT || Achievement_IsUnlocked(id))
        return;
    SetBit(gSaveBlock3Ptr->achievementsUnlocked, id);
    SyncMirror();
    if (sNotificationCount < NOTIFICATION_QUEUE_SIZE)
        sNotificationQueue[sNotificationCount++] = id;
}

//// Condition checks ////

struct CheckCache
{
    bool8 haveDex;
    bool8 haveTypes;
    u16 caught;
    u16 seen;
    u16 typeCaught[NUMBER_OF_MON_TYPES];
};

static void CacheDexCounts(struct CheckCache *cache)
{
    if (cache->haveDex)
        return;
    cache->caught = GetNationalPokedexCount(FLAG_GET_CAUGHT);
    cache->seen = GetNationalPokedexCount(FLAG_GET_SEEN);
    cache->haveDex = TRUE;
}

// One pass over the species table (NationalPokedexNumToSpecies is a linear search, too slow here).
// Only base forms count, so each Pokédex entry is counted once.
static void CacheTypeCounts(struct CheckCache *cache)
{
    enum Species species;

    if (cache->haveTypes)
        return;
    memset(cache->typeCaught, 0, sizeof(cache->typeCaught));
    for (species = 1; species < NUM_SPECIES; species++)
    {
        const struct SpeciesInfo *info = &gSpeciesInfo[species];
        if (info->natDexNum == 0 || GET_BASE_SPECIES_ID(species) != species)
            continue;
        if (!GetSetPokedexFlag(info->natDexNum, FLAG_GET_CAUGHT))
            continue;
        cache->typeCaught[info->types[0]]++;
        if (info->types[1] != info->types[0])
            cache->typeCaught[info->types[1]]++;
    }
    cache->haveTypes = TRUE;
}

static u32 CountPartyMatching(bool32 (*test)(struct Pokemon *mon, u32 value), u32 value)
{
    u32 i, count = 0;
    for (i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (!GetMonData(mon, MON_DATA_IS_EGG) && test(mon, value))
            count++;
    }
    return count;
}

static bool32 TestLevelAtLeast(struct Pokemon *mon, u32 value) { return GetMonData(mon, MON_DATA_LEVEL) >= value; }
static bool32 TestMaxFriendship(struct Pokemon *mon, u32 value) { return GetMonData(mon, MON_DATA_FRIENDSHIP) == MAX_FRIENDSHIP; }
static bool32 TestPokerus(struct Pokemon *mon, u32 value) { return GetMonData(mon, MON_DATA_POKERUS) != 0; }
static bool32 TestMagikarp100(struct Pokemon *mon, u32 value)
{
    return GetMonData(mon, MON_DATA_SPECIES) == SPECIES_MAGIKARP && GetMonData(mon, MON_DATA_LEVEL) == MAX_LEVEL;
}

static bool32 TestHiddenAbility(struct Pokemon *mon, u32 value)
{
    return GetMonData(mon, MON_DATA_ABILITY_NUM) == 2
        && GetSpeciesAbility(GetMonData(mon, MON_DATA_SPECIES), 2) != ABILITY_NONE;
}

static bool32 TestEvTotal(struct Pokemon *mon, u32 value)
{
    u32 stat, total = 0;
    for (stat = 0; stat < NUM_STATS; stat++)
        total += GetMonData(mon, MON_DATA_HP_EV + stat);
    return total >= value;
}

static bool32 TestEvInOneStat(struct Pokemon *mon, u32 value)
{
    u32 stat;
    for (stat = 0; stat < NUM_STATS; stat++)
    {
        if (GetMonData(mon, MON_DATA_HP_EV + stat) >= value)
            return TRUE;
    }
    return FALSE;
}

static bool32 TestPerfectIvs(struct Pokemon *mon, u32 value)
{
    u32 stat;
    for (stat = 0; stat < NUM_STATS; stat++)
    {
        if (GetMonData(mon, MON_DATA_HP_IV + stat) != MAX_PER_STAT_IVS)
            return FALSE;
    }
    return TRUE;
}

static u32 CountFlagsSet(const u16 *flags, u32 count)
{
    u32 i, set = 0;
    for (i = 0; i < count; i++)
        set += FlagGet(flags[i]) ? 1 : 0;
    return set;
}

static const u16 sKantoTownFlags[] =
{
    FLAG_WORLD_MAP_PALLET_TOWN, FLAG_WORLD_MAP_VIRIDIAN_CITY, FLAG_WORLD_MAP_PEWTER_CITY, FLAG_WORLD_MAP_CERULEAN_CITY,
    FLAG_WORLD_MAP_LAVENDER_TOWN, FLAG_WORLD_MAP_VERMILION_CITY, FLAG_WORLD_MAP_CELADON_CITY, FLAG_WORLD_MAP_FUCHSIA_CITY,
    FLAG_WORLD_MAP_SAFFRON_CITY, FLAG_WORLD_MAP_CINNABAR_ISLAND,
};

static const u16 sSeviiIslandFlags[] =
{
    FLAG_WORLD_MAP_ONE_ISLAND, FLAG_WORLD_MAP_TWO_ISLAND, FLAG_WORLD_MAP_THREE_ISLAND, FLAG_WORLD_MAP_FOUR_ISLAND,
    FLAG_WORLD_MAP_FIVE_ISLAND, FLAG_WORLD_MAP_SIX_ISLAND, FLAG_WORLD_MAP_SEVEN_ISLAND,
};

static const u16 sRematchTrainers[] =
{
    TRAINER_RF_REMATCH_BROCK, TRAINER_RF_REMATCH_MISTY, TRAINER_RF_REMATCH_LT_SURGE, TRAINER_RF_REMATCH_ERIKA,
    TRAINER_RF_REMATCH_KOGA, TRAINER_RF_REMATCH_SABRINA, TRAINER_RF_REMATCH_BLAINE,
};

static const u16 sDexRewardFlags[] =
{
    FLAG_RF_DEX_REWARD_0, FLAG_RF_DEX_REWARD_1, FLAG_RF_DEX_REWARD_2, FLAG_RF_DEX_REWARD_3,
    FLAG_RF_DEX_REWARD_4, FLAG_RF_DEX_REWARD_5, FLAG_RF_DEX_REWARD_6, FLAG_RF_DEX_REWARD_7,
};

#define NUM_HIDDEN_ITEM_FLAGS 302

static u32 GetSpecialValue(enum AchievementSpecial special, u32 target)
{
    u32 i, count = 0;

    switch (special)
    {
    case ACH_SPECIAL_ALL_GIMMICKS:
        return gSaveBlock3Ptr->achievementGimmicks == 0xF;
    case ACH_SPECIAL_ALL_KANTO_TOWNS:
        return CountFlagsSet(sKantoTownFlags, ARRAY_COUNT(sKantoTownFlags)) == ARRAY_COUNT(sKantoTownFlags);
    case ACH_SPECIAL_ALL_SEVII_ISLANDS:
        return CountFlagsSet(sSeviiIslandFlags, ARRAY_COUNT(sSeviiIslandFlags)) == ARRAY_COUNT(sSeviiIslandFlags);
    case ACH_SPECIAL_CHALLENGE_MODE:
        return KeySystem_GetDifficulty() == KEY_DIFFICULTY_CHALLENGE;
    case ACH_SPECIAL_DEXNAV_CHAIN:
        return gSaveBlock3Ptr->dexNavChain;
    case ACH_SPECIAL_DEXNAV_LEVEL:
        for (i = 0; i <= NATIONAL_DEX_COUNT; i++)
            count = max(count, gSaveBlock3Ptr->dexNavSearchLevels[i]);
        return count;
    case ACH_SPECIAL_DEX_REWARDS:
        return CountFlagsSet(sDexRewardFlags, ARRAY_COUNT(sDexRewardFlags));
    case ACH_SPECIAL_EV_252:
        return CountPartyMatching(TestEvInOneStat, MAX_PER_STAT_EVS) > 0;
    case ACH_SPECIAL_EV_510:
        return CountPartyMatching(TestEvTotal, MAX_TOTAL_EVS) > 0;
    case ACH_SPECIAL_FRIENDSHIP_ALL:
        return CountPartyMatching(TestMaxFriendship, 0) == PARTY_SIZE;
    case ACH_SPECIAL_FRIENDSHIP_ONE:
        return CountPartyMatching(TestMaxFriendship, 0) > 0;
    case ACH_SPECIAL_HIDDEN_ABILITY:
        return CountPartyMatching(TestHiddenAbility, 0) > 0;
    case ACH_SPECIAL_HIDDEN_ITEMS:
        for (i = 0; i < NUM_HIDDEN_ITEM_FLAGS; i++)
            count += FlagGet(FLAG_HIDDEN_ITEMS_START + i) ? 1 : 0;
        return count;
    case ACH_SPECIAL_IV_PERFECT:
        return CountPartyMatching(TestPerfectIvs, 0) > 0;
    case ACH_SPECIAL_MAGIKARP_100:
        return CountPartyMatching(TestMagikarp100, 0) > 0;
    case ACH_SPECIAL_OMNI_CHARGES:
        return gSaveBlock3Ptr->omniRingMaxCharges;
    case ACH_SPECIAL_OMNI_MODULES:
        for (i = 0; i < OMNI_MECHANIC_COUNT; i++)
            count += (gSaveBlock3Ptr->omniRingMechanics >> i) & 1;
        return count;
    case ACH_SPECIAL_POKERUS:
        return CountPartyMatching(TestPokerus, 0) > 0;
    case ACH_SPECIAL_REMATCHES:
        for (i = 0; i < ARRAY_COUNT(sRematchTrainers); i++)
            count += HasTrainerBeenFought(sRematchTrainers[i]) ? 1 : 0;
        return count;
    case ACH_SPECIAL_STATUS_TOTAL:
        return Achievement_GetCounter(ACH_COUNTER_PARALYZED) + Achievement_GetCounter(ACH_COUNTER_BURNED)
             + Achievement_GetCounter(ACH_COUNTER_SLEPT) + Achievement_GetCounter(ACH_COUNTER_FROZEN)
             + Achievement_GetCounter(ACH_COUNTER_POISONED);
    case ACH_SPECIAL_Z_TRIALS:
        for (i = 0; i < 18; i++)
            count += FlagGet(FLAG_RF_Z_TRIAL_00 + i) ? 1 : 0;
        return count;
    default:
        return 0;
    }
}

static u32 CountUnlocked(void)
{
    u32 i, count = 0;
    for (i = 0; i < ACH_COUNT; i++)
        count += Achievement_IsUnlocked(i) ? 1 : 0;
    return count;
}

static bool32 AllOthersUnlocked(u32 self, bool32 (*inGroup)(const struct Achievement *, u32), u32 arg)
{
    u32 i;
    for (i = 0; i < ACH_COUNT; i++)
    {
        if (i != self && inGroup(&gAchievements[i], arg) && !Achievement_IsUnlocked(i))
            return FALSE;
    }
    return TRUE;
}

static bool32 IsOfTier(const struct Achievement *a, u32 tier) { return a->tier == tier; }
static bool32 IsHidden(const struct Achievement *a, u32 unused) { return a->hidden; }

// Current progress towards condValue, for conditions that count something; used by the menu too.
static u32 GetProgress(u32 id, struct CheckCache *cache)
{
    const struct Achievement *a = &gAchievements[id];
    u32 i, count = 0;

    switch (a->condType)
    {
    case ACH_COND_FLAG:
        return FlagGet(a->condArg) ? 1 : 0;
    case ACH_COND_DEX_CAUGHT:
        CacheDexCounts(cache);
        return cache->caught;
    case ACH_COND_DEX_SEEN:
        CacheDexCounts(cache);
        return cache->seen;
    case ACH_COND_SPECIES_SET:
        for (i = 0; i < a->setSize; i++)
            count += GetSetPokedexFlag(SpeciesToNationalPokedexNum(a->set[i]), FLAG_GET_CAUGHT) ? 1 : 0;
        return count;
    case ACH_COND_GAME_STAT:
        return GetGameStat(a->condArg);
    case ACH_COND_COUNTER:
        return Achievement_GetCounter(a->condArg);
    case ACH_COND_MONEY:
        return GetMoney(&gSaveBlock1Ptr->money);
    case ACH_COND_PARTY_LEVEL:
        return CountPartyMatching(TestLevelAtLeast, a->condValue) > 0 ? a->condValue : 0;
    case ACH_COND_PARTY_ALL_LEVEL:
        return CountPartyMatching(TestLevelAtLeast, a->condValue) == PARTY_SIZE ? a->condValue : 0;
    case ACH_COND_ITEM_SET:
        for (i = 0; i < a->setSize; i++)
            count += (CheckBagHasItem(a->set[i], 1) || CheckPCHasItem(a->set[i], 1)) ? 1 : 0;
        return count;
    case ACH_COND_TYPE_CAUGHT:
        CacheTypeCounts(cache);
        return cache->typeCaught[a->condArg];
    case ACH_COND_SPECIAL:
        return GetSpecialValue(a->condArg, a->condValue);
    case ACH_COND_PERCENT:
        return CountUnlocked() * 100 / ACH_COUNT;
    default:
        return 0;
    }
}

// The value GetProgress has to reach. Flag and yes/no specials count as reaching 1.
static u32 GetGoal(u32 id)
{
    const struct Achievement *a = &gAchievements[id];

    switch (a->condType)
    {
    case ACH_COND_FLAG:
        return 1;
    case ACH_COND_SPECIES_SET:
        return a->setSize;
    case ACH_COND_SPECIAL:
        return a->condValue ? a->condValue : 1;
    default:
        return a->condValue;
    }
}

static bool32 IsConditionMet(u32 id, struct CheckCache *cache)
{
    const struct Achievement *a = &gAchievements[id];

    switch (a->condType)
    {
    case ACH_COND_EVENT:
        return FALSE;
    case ACH_COND_TIER_ALL:
        return AllOthersUnlocked(id, IsOfTier, a->condArg);
    case ACH_COND_HIDDEN_ALL:
        return AllOthersUnlocked(id, IsHidden, 0);
    default:
        return GetProgress(id, cache) >= GetGoal(id);
    }
}

void Achievements_CheckAll(void)
{
    struct CheckCache cache = {0};
    u32 id, pass;
    bool32 unlockedAny;

    // Meta achievements depend on the others, so repeat while new ones unlock
    for (pass = 0; pass < 3; pass++)
    {
        unlockedAny = FALSE;
        for (id = 0; id < ACH_COUNT; id++)
        {
            if (!Achievement_IsUnlocked(id) && IsConditionMet(id, &cache))
            {
                Achievement_Unlock(id);
                unlockedAny = TRUE;
            }
        }
        if (!unlockedAny)
            break;
    }
    SyncMirror();
}

//// Notification ////

static const struct WindowTemplate sNotificationWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 28,
    .height = 2,
    .paletteNum = 15,
    .baseBlock = 0x100,
};

void Achievements_HideNotification(void)
{
    if (!sNotificationShown)
        return;
    ClearStdWindowAndFrame(sNotificationWindowId, TRUE);
    RemoveWindow(sNotificationWindowId);
    sNotificationShown = FALSE;
    sNotificationTimer = 0;
}

static void ShowNextNotification(void)
{
    u32 id = sNotificationQueue[0];
    u32 i;

    for (i = 1; i < sNotificationCount; i++)
        sNotificationQueue[i - 1] = sNotificationQueue[i];
    sNotificationCount--;

    LoadMessageBoxAndBorderGfx();
    sNotificationWindowId = AddWindow(&sNotificationWindowTemplate);
    FillWindowPixelBuffer(sNotificationWindowId, PIXEL_FILL(1));
    StringCopy(gStringVar4, COMPOUND_STRING("¡LOGRO! "));
    StringAppend(gStringVar4, gAchievements[id].name);
    StringAppend(gStringVar4, COMPOUND_STRING(" · "));
    StringAppend(gStringVar4, sTierNames[gAchievements[id].tier]);
    AddTextPrinterParameterized(sNotificationWindowId, FONT_NORMAL, gStringVar4, 0, 1, TEXT_SKIP_DRAW, NULL);
    PutWindowTilemap(sNotificationWindowId);
    DrawStdWindowFrame(sNotificationWindowId, FALSE);
    CopyWindowToVram(sNotificationWindowId, COPYWIN_FULL);
    PlaySE(SE_EXP_MAX);
    sNotificationShown = TRUE;
    sNotificationTimer = NOTIFICATION_FRAMES;
}

// Called every overworld frame from CB1_Overworld
void Achievements_OverworldUpdate(void)
{
    bool32 playerIsFree = !ArePlayerFieldControlsLocked() && !gPaletteFade.active;

    if (gRfAchievementMirror[ACH_BYTES] != MIRROR_SYNCED)
        SyncMirror();

    if (++sCheckTimer >= CHECK_INTERVAL_FRAMES)
    {
        sCheckTimer = 0;
        Achievements_CheckAll();
    }

    if (sNotificationShown)
    {
        // Menus and scripts share this part of the screen, so get out of their way
        if (!playerIsFree || --sNotificationTimer == 0)
            Achievements_HideNotification();
    }
    else if (sNotificationCount != 0 && playerIsFree)
    {
        ShowNextNotification();
    }
}

//// Hooks ////

void Achievements_OnGimmickUsed(enum AchievementCounter counter)
{
    Achievement_AddToCounter(counter, 1);
    switch (counter)
    {
    case ACH_COUNTER_MEGA:    gSaveBlock3Ptr->achievementGimmicks |= 1 << 0; break;
    case ACH_COUNTER_TERA:    gSaveBlock3Ptr->achievementGimmicks |= 1 << 1; break;
    case ACH_COUNTER_DYNAMAX: gSaveBlock3Ptr->achievementGimmicks |= 1 << 2; break;
    case ACH_COUNTER_ZMOVE:   gSaveBlock3Ptr->achievementGimmicks |= 1 << 3; break;
    default: break;
    }
}

static bool32 IsLegendary(enum Species species)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    return info->isSubLegendary || info->isRestrictedLegendary || info->isMythical || info->isUltraBeast;
}

// Called when a wild or trainer battle ends, before returning to the field
void Achievements_OnBattleEnd(void)
{
    bool32 isTrainer = (gBattleTypeFlags & BATTLE_TYPE_TRAINER) != 0;
    bool32 wasDexNavBattle = gRfDexNavBattle;

    gRfDexNavBattle = FALSE;
    if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED))
        return;

    Achievement_AddToCounter(ACH_COUNTER_SWITCHES, gBattleResults.playerSwitchesCounter);
    Achievement_AddToCounter(ACH_COUNTER_FAINTED_FOES, gBattleResults.opponentFaintCounter);
    if (isTrainer)
    {
        if (TRAINER_BATTLE_PARAM.opponentA == TRAINER_ELITE_FOUR_LORELEI || TRAINER_BATTLE_PARAM.opponentA == TRAINER_ELITE_FOUR_LORELEI_2)
            gSaveBlock3Ptr->achievementCounters[ACH_COUNTER_LEAGUE_FAINTS] = 0;
        Achievement_AddToCounter(ACH_COUNTER_LEAGUE_FAINTS, gBattleResults.playerFaintCounter);
    }

    switch (gBattleOutcome)
    {
    case B_OUTCOME_WON:
        if (isTrainer)
        {
            u32 i, alive = 0, lastMon = PARTY_SIZE;

            Achievement_AddToCounter(ACH_COUNTER_TRAINER_WINS, 1);
            Achievement_AddToCounter(ACH_COUNTER_WIN_STREAK, 1);
            Achievement_SetCounterMax(ACH_COUNTER_BEST_WIN_STREAK, Achievement_GetCounter(ACH_COUNTER_WIN_STREAK));

            // A clutch win: the only Pokémon still standing has under 10% HP
            for (i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
            {
                struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
                if (!GetMonData(mon, MON_DATA_IS_EGG) && GetMonData(mon, MON_DATA_HP) != 0)
                {
                    alive++;
                    lastMon = i;
                }
            }
            if (alive == 1)
            {
                struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][lastMon];
                if (GetMonData(mon, MON_DATA_HP) * 10 < GetMonData(mon, MON_DATA_MAX_HP))
                    Achievement_AddToCounter(ACH_COUNTER_CLUTCH_WINS, 1);
            }
        }
        break;
    case B_OUTCOME_LOST:
    case B_OUTCOME_DREW:
        if (isTrainer)
            gSaveBlock3Ptr->achievementCounters[ACH_COUNTER_WIN_STREAK] = 0;
        break;
    case B_OUTCOME_RAN:
        Achievement_AddToCounter(ACH_COUNTER_FLED, 1);
        break;
    case B_OUTCOME_CAUGHT:
    {
        enum Species species = gBattleResults.caughtMonSpecies;
        u32 throws = 0, i;

        for (i = 0; i < POKEBALL_COUNT; i++)
            throws += gBattleResults.catchAttempts[i];
        if (throws <= 1)
            Achievement_AddToCounter(ACH_COUNTER_FIRST_BALL_CATCHES, 1);
        if (gBattleResults.shinyWildMon)
            Achievement_AddToCounter(ACH_COUNTER_SHINY_CAUGHT, 1);
        if (GetTimeOfDay() == TIME_NIGHT)
            Achievement_AddToCounter(ACH_COUNTER_NIGHT_CATCHES, 1);
        if (IsLegendary(species))
            Achievement_AddToCounter(ACH_COUNTER_LEGEND_CATCHES, 1);
        if (gBattleTypeFlags & BATTLE_TYPE_SAFARI)
            Achievement_AddToCounter(ACH_COUNTER_SAFARI_CATCHES, 1);
        if (gIsFishingEncounter)
            Achievement_AddToCounter(ACH_COUNTER_FISHING_CATCHES, 1);
        if (wasDexNavBattle)
            Achievement_AddToCounter(ACH_COUNTER_DEXNAV_CATCHES, 1);
        if (species == gSaveBlock1Ptr->outbreakPokemonSpecies && gSaveBlock1Ptr->outbreakDaysLeft != 0
         && gSaveBlock1Ptr->location.mapNum == gSaveBlock1Ptr->outbreakLocationMapNum
         && gSaveBlock1Ptr->location.mapGroup == gSaveBlock1Ptr->outbreakLocationMapGroup)
            Achievement_AddToCounter(ACH_COUNTER_OUTBREAK_CATCHES, 1);
        if (gBattleResults.caughtMonBall < 32 && !(gSaveBlock3Ptr->achievementBallKinds & (1u << gBattleResults.caughtMonBall)))
        {
            gSaveBlock3Ptr->achievementBallKinds |= 1u << gBattleResults.caughtMonBall;
            Achievement_AddToCounter(ACH_COUNTER_BALL_KINDS, 1);
        }
        break;
    }
    default:
        break;
    }
}

// Called when the player enters the Hall of Fame; checks the challenge achievements
void Achievements_OnHallOfFame(void)
{
    u32 i, count = 0, highestLevel = 0, sharedTypes = 0xFFFFFFFF, newGenOnly = TRUE, sameSpecies = TRUE, magikarp = FALSE;
    enum Species firstSpecies = SPECIES_NONE;
    u32 hours = gSaveBlock2Ptr->playTimeHours;

    for (i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        enum Species species = GetMonData(mon, MON_DATA_SPECIES);
        u32 types;

        if (species == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        count++;
        highestLevel = max(highestLevel, GetMonData(mon, MON_DATA_LEVEL));
        types = (1u << gSpeciesInfo[species].types[0]) | (1u << gSpeciesInfo[species].types[1]);
        sharedTypes &= types;
        if (SpeciesToNationalPokedexNum(species) < NATIONAL_DEX_VICTINI)
            newGenOnly = FALSE;
        if (firstSpecies == SPECIES_NONE)
            firstSpecies = species;
        else if (GET_BASE_SPECIES_ID(species) != GET_BASE_SPECIES_ID(firstSpecies))
            sameSpecies = FALSE;
        if (species == SPECIES_MAGIKARP)
            magikarp = TRUE;
    }

    if (KeySystem_GetDifficulty() == KEY_DIFFICULTY_CHALLENGE)
        Achievement_Unlock(ACH_R01);
    if (count == 1)
        Achievement_Unlock(ACH_R02);
    if (hours < 8)
        Achievement_Unlock(ACH_R03);
    if (hours < 4)
        Achievement_Unlock(ACH_R04);
    if (count != 0 && sharedTypes != 0)
        Achievement_Unlock(ACH_R05);
    if (count != 0 && highestLevel <= 55)
        Achievement_Unlock(ACH_R06);
    if (GetGameStat(GAME_STAT_EVOLVED_POKEMON) == 0)
        Achievement_Unlock(ACH_R07);
    if (GetGameStat(GAME_STAT_POKEMON_CAPTURES) <= 6)
        Achievement_Unlock(ACH_R08);
    if (count == PARTY_SIZE && sameSpecies)
        Achievement_Unlock(ACH_R09);
    if (count != 0 && newGenOnly)
        Achievement_Unlock(ACH_R10);
    if (GetGameStat(GAME_STAT_USED_POKECENTER) == 0)
        Achievement_Unlock(ACH_R11);
    if (magikarp)
        Achievement_Unlock(ACH_R12);
    // LEAGUE_FAINTS restarts at the Lorelei battle, so it covers this League run only
    if (Achievement_GetCounter(ACH_COUNTER_LEAGUE_FAINTS) == 0)
        Achievement_Unlock(ACH_B34);
}

//// Menu ////

#define MENU_WIDTH       28
#define MENU_LIST_ROWS   5
#define MENU_DESC_HEIGHT 6

enum
{
    MENU_CATEGORIES,
    MENU_ACHIEVEMENTS,
};

static const struct WindowTemplate sMenuListWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = MENU_WIDTH,
    .height = MENU_LIST_ROWS * 2,
    .paletteNum = 15,
    .baseBlock = 1,
};

static const struct WindowTemplate sMenuDescWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 13,
    .width = MENU_WIDTH,
    .height = MENU_DESC_HEIGHT,
    .paletteNum = 15,
    .baseBlock = 1 + MENU_WIDTH * MENU_LIST_ROWS * 2,
};

static EWRAM_DATA struct ListMenuItem sMenuItems[ACH_COUNT] = {0};
static EWRAM_DATA u8 sMenuState = 0;
static EWRAM_DATA u8 sMenuCategory = 0;
static EWRAM_DATA u16 sMenuCategoryRow = 0;

#define tListTaskId   data[0]
#define tListWindowId data[1]
#define tDescWindowId data[2]

static void Task_AchievementsMenu(u8 taskId);

static bool32 IsShownAsHidden(u32 id)
{
    return gAchievements[id].hidden && !Achievement_IsUnlocked(id);
}

static u8 GetMenuDescWindowId(void)
{
    u8 taskId = FindTaskIdByFunc(Task_AchievementsMenu);
    return (taskId == TASK_NONE) ? WINDOW_NONE : gTasks[taskId].tDescWindowId;
}

static void PrintRightAligned(u8 windowId, const u8 *text, u8 y)
{
    u32 x = MENU_WIDTH * 8 - GetStringWidth(FONT_NORMAL, text, 0) - 8;
    AddTextPrinterParameterized(windowId, FONT_NORMAL, text, x, y, TEXT_SKIP_DRAW, NULL);
}

static void PrintCategoryRow(u8 windowId, u32 category, u8 y)
{
    u32 id, unlocked = 0, total = 0;

    for (id = 0; id < ACH_COUNT; id++)
    {
        if (gAchievements[id].category != category)
            continue;
        total++;
        unlocked += Achievement_IsUnlocked(id) ? 1 : 0;
    }
    ConvertIntToDecimalStringN(gStringVar1, unlocked, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(gStringVar1, COMPOUND_STRING("/"));
    ConvertIntToDecimalStringN(gStringVar1 + StringLength(gStringVar1), total, STR_CONV_MODE_LEFT_ALIGN, 3);
    PrintRightAligned(windowId, gStringVar1, y);
}

static void PrintAchievementRow(u8 windowId, u32 id, u8 y)
{
    u32 type = gAchievements[id].condType;

    if (Achievement_IsClaimed(id))
    {
        PrintRightAligned(windowId, COMPOUND_STRING("HECHO"), y);
    }
    else if (Achievement_IsUnlocked(id))
    {
        PrintRightAligned(windowId, COMPOUND_STRING("¡PREMIO!"), y);
    }
    else if (IsShownAsHidden(id) || type == ACH_COND_EVENT || type == ACH_COND_TIER_ALL
          || type == ACH_COND_HIDDEN_ALL || GetGoal(id) == 1)
    {
        PrintRightAligned(windowId, COMPOUND_STRING("-"), y);
    }
    else
    {
        struct CheckCache cache = {0};
        u32 progress = min(GetProgress(id, &cache), GetGoal(id));

        ConvertIntToDecimalStringN(gStringVar1, progress, STR_CONV_MODE_LEFT_ALIGN, 6);
        StringAppend(gStringVar1, COMPOUND_STRING("/"));
        ConvertIntToDecimalStringN(gStringVar1 + StringLength(gStringVar1), GetGoal(id), STR_CONV_MODE_LEFT_ALIGN, 6);
        PrintRightAligned(windowId, gStringVar1, y);
    }
}

static void AchMenu_PrintRow(u8 windowId, u32 itemId, u8 y)
{
    if (sMenuState == MENU_CATEGORIES)
        PrintCategoryRow(windowId, itemId, y);
    else
        PrintAchievementRow(windowId, itemId, y);
}

static void PrintDescLines(u8 windowId, const u8 *lines, const u8 *lastLine)
{
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(windowId, FONT_NORMAL, lines, 0, 1, TEXT_SKIP_DRAW, NULL);
    if (lastLine != NULL)
        AddTextPrinterParameterized(windowId, FONT_NORMAL, lastLine, 0, 33, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(windowId, COPYWIN_GFX);
}

static void BufferRewardText(u8 *dest, u32 id)
{
    const struct Achievement *a = &gAchievements[id];

    StringCopy(dest, sTierNames[a->tier]);
    StringAppend(dest, COMPOUND_STRING(" · "));
    ConvertIntToDecimalStringN(dest + StringLength(dest), sTierPoints[a->tier], STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(dest, COMPOUND_STRING(" pts · "));
    if (IsShownAsHidden(id))
    {
        StringAppend(dest, COMPOUND_STRING("???"));
    }
    else if (a->rewardTitle != NULL)
    {
        StringAppend(dest, COMPOUND_STRING("Título: "));
        StringAppend(dest, a->rewardTitle);
    }
    else
    {
        CopyItemName(a->rewardItem, dest + StringLength(dest));
        StringAppend(dest, COMPOUND_STRING(" ×"));
        ConvertIntToDecimalStringN(dest + StringLength(dest), a->rewardQuantity, STR_CONV_MODE_LEFT_ALIGN, 2);
    }
}

static void PrintSummary(u8 windowId)
{
    u32 id, unlocked = 0, points = 0, pending = 0;

    for (id = 0; id < ACH_COUNT; id++)
    {
        if (!Achievement_IsUnlocked(id))
            continue;
        unlocked++;
        points += sTierPoints[gAchievements[id].tier];
        pending += Achievement_IsClaimed(id) ? 0 : 1;
    }
    StringCopy(gStringVar4, COMPOUND_STRING("Logros: "));
    ConvertIntToDecimalStringN(gStringVar4 + StringLength(gStringVar4), unlocked, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(gStringVar4, COMPOUND_STRING("/"));
    ConvertIntToDecimalStringN(gStringVar4 + StringLength(gStringVar4), ACH_COUNT, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(gStringVar4, COMPOUND_STRING(" · Puntos: "));
    ConvertIntToDecimalStringN(gStringVar4 + StringLength(gStringVar4), points, STR_CONV_MODE_LEFT_ALIGN, 5);
    StringAppend(gStringVar4, COMPOUND_STRING("\nPremios por reclamar: "));
    ConvertIntToDecimalStringN(gStringVar4 + StringLength(gStringVar4), pending, STR_CONV_MODE_LEFT_ALIGN, 3);
    PrintDescLines(windowId, gStringVar4, COMPOUND_STRING("A: abrir · B: salir"));
}

static void PrintAchievementDesc(u8 windowId, u32 id)
{
    BufferRewardText(gStringVar3, id);
    if (IsShownAsHidden(id))
        PrintDescLines(windowId, COMPOUND_STRING("Logro oculto. Sigue jugando\npara descubrirlo."), gStringVar3);
    else
        PrintDescLines(windowId, gAchievements[id].description, gStringVar3);
}

static void AchMenu_MoveCursor(s32 itemId, bool8 onInit, struct ListMenu *list)
{
    u8 windowId = GetMenuDescWindowId();

    if (!onInit)
        PlaySE(SE_SELECT);
    if (windowId == WINDOW_NONE)
        return;
    if (sMenuState == MENU_CATEGORIES)
        PrintSummary(windowId);
    else
        PrintAchievementDesc(windowId, itemId);
}

static u8 InitMenuList(u8 taskId, u16 selectedRow)
{
    struct ListMenuTemplate template = {0};
    u32 id, count = 0;

    if (sMenuState == MENU_CATEGORIES)
    {
        for (id = 0; id < ACH_CAT_COUNT; id++)
        {
            sMenuItems[count].name = sAchCategoryNames[id];
            sMenuItems[count].id = id;
            count++;
        }
    }
    else
    {
        for (id = 0; id < ACH_COUNT; id++)
        {
            if (gAchievements[id].category != sMenuCategory)
                continue;
            sMenuItems[count].name = IsShownAsHidden(id) ? COMPOUND_STRING("???") : gAchievements[id].name;
            sMenuItems[count].id = id;
            count++;
        }
    }

    FillWindowPixelBuffer(gTasks[taskId].tListWindowId, PIXEL_FILL(1));
    template.items = sMenuItems;
    template.moveCursorFunc = AchMenu_MoveCursor;
    template.itemPrintFunc = AchMenu_PrintRow;
    template.totalItems = count;
    template.maxShowed = MENU_LIST_ROWS;
    template.windowId = gTasks[taskId].tListWindowId;
    template.item_X = 8;
    template.upText_Y = 1;
    template.cursorPal = 2;
    template.fillValue = 1;
    template.cursorShadowPal = 3;
    template.lettersSpacing = 1;
    template.scrollMultiple = LIST_MULTIPLE_SCROLL_DPAD;
    template.fontId = FONT_NORMAL;
    return ListMenuInit(&template, 0, min(selectedRow, count - 1));
}

void Achievements_OpenMenu(void)
{
    u8 taskId;

    Achievements_HideNotification();
    Achievements_CheckAll();
    taskId = CreateTask(Task_AchievementsMenu, 80);
    sMenuState = MENU_CATEGORIES;
    LoadMessageBoxAndBorderGfx();
    gTasks[taskId].tListWindowId = AddWindow(&sMenuListWindowTemplate);
    gTasks[taskId].tDescWindowId = AddWindow(&sMenuDescWindowTemplate);
    DrawStdWindowFrame(gTasks[taskId].tListWindowId, FALSE);
    DrawStdWindowFrame(gTasks[taskId].tDescWindowId, FALSE);
    gTasks[taskId].tListTaskId = InitMenuList(taskId, sMenuCategoryRow);
    CopyWindowToVram(gTasks[taskId].tListWindowId, COPYWIN_FULL);
    CopyWindowToVram(gTasks[taskId].tDescWindowId, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
}

static void CloseMenu(u8 taskId)
{
    DestroyListMenuTask(gTasks[taskId].tListTaskId, NULL, NULL);
    ClearStdWindowAndFrame(gTasks[taskId].tListWindowId, TRUE);
    ClearStdWindowAndFrame(gTasks[taskId].tDescWindowId, TRUE);
    RemoveWindow(gTasks[taskId].tListWindowId);
    RemoveWindow(gTasks[taskId].tDescWindowId);
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
    DestroyTask(taskId);
}

static void SwitchList(u8 taskId, u8 state, u16 selectedRow)
{
    DestroyListMenuTask(gTasks[taskId].tListTaskId, NULL, NULL);
    sMenuState = state;
    gTasks[taskId].tListTaskId = InitMenuList(taskId, selectedRow);
    CopyWindowToVram(gTasks[taskId].tListWindowId, COPYWIN_GFX);
}

static void TryClaimReward(u8 taskId, u32 id)
{
    const struct Achievement *a = &gAchievements[id];
    u8 windowId = gTasks[taskId].tDescWindowId;

    if (!Achievement_IsUnlocked(id) || Achievement_IsClaimed(id))
        return;

    if (a->rewardTitle != NULL)
    {
        StringCopy(gStringVar4, COMPOUND_STRING("¡Título obtenido!\n"));
        StringAppend(gStringVar4, a->rewardTitle);
    }
    else if (AddBagItem(a->rewardItem, a->rewardQuantity))
    {
        StringCopy(gStringVar4, COMPOUND_STRING("¡Recibiste "));
        CopyItemName(a->rewardItem, gStringVar4 + StringLength(gStringVar4));
        StringAppend(gStringVar4, COMPOUND_STRING(" ×"));
        ConvertIntToDecimalStringN(gStringVar4 + StringLength(gStringVar4), a->rewardQuantity, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringAppend(gStringVar4, COMPOUND_STRING("!"));
    }
    else
    {
        PlaySE(SE_FAILURE);
        PrintDescLines(windowId, COMPOUND_STRING("No hay espacio en la mochila.\nHaz sitio y vuelve a intentarlo."), NULL);
        return;
    }
    SetBit(gSaveBlock3Ptr->achievementsClaimed, id);
    PlaySE(SE_EXP_MAX);
    PrintDescLines(windowId, gStringVar4, NULL);
    RedrawListMenu(gTasks[taskId].tListTaskId);
}

static void Task_AchievementsMenu(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    switch (input)
    {
    case LIST_NOTHING_CHOSEN:
        break;
    case LIST_CANCEL:
        PlaySE(SE_SELECT);
        if (sMenuState == MENU_ACHIEVEMENTS)
            SwitchList(taskId, MENU_CATEGORIES, sMenuCategory);
        else
            CloseMenu(taskId);
        break;
    default:
        if (sMenuState == MENU_CATEGORIES)
        {
            PlaySE(SE_SELECT);
            sMenuCategory = input;
            sMenuCategoryRow = input;
            SwitchList(taskId, MENU_ACHIEVEMENTS, 0);
        }
        else
        {
            TryClaimReward(taskId, input);
        }
        break;
    }
}
