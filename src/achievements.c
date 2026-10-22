#include "global.h"
#include "achievements.h"
#include "battle.h"
#include "battle_setup.h"
#include "bg.h"
#include "data.h"
#include "dexnav.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "gpu_regs.h"
#include "item.h"
#include "key_system.h"
#include "list_menu.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "money.h"
#include "omni_ring.h"
#include "overworld.h"
#include "palette.h"
#include "pokedex.h"
#include "pokemon.h"
#include "rf_ui_theme.h"
#include "rtc.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "wild_encounter.h"
#include "constants/opponents.h"
#include "constants/rgb.h"
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

// Title shown on the Trainer Card, or NULL
const u8 *Achievements_GetEquippedTitle(void)
{
    u32 title = gSaveBlock3Ptr->achievementTitle;

    if (title == 0 || title > ACH_COUNT || !Achievement_IsClaimed(title - 1))
        return NULL;
    return gAchievements[title - 1].rewardTitle;
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
    // Past the message box (0x194-0x1FF), the frame tiles (0x200-0x23A) and the map preview (0x259-0x284)
    .baseBlock = 0x2A0,
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
    // The frame call clears the window, so it goes before the text
    DrawStdWindowFrame(sNotificationWindowId, FALSE);
    StringCopy(gStringVar4, COMPOUND_STRING("¡LOGRO! "));
    StringAppend(gStringVar4, gAchievements[id].name);
    StringAppend(gStringVar4, COMPOUND_STRING(" · "));
    StringAppend(gStringVar4, sTierNames[gAchievements[id].tier]);
    AddTextPrinterParameterized(sNotificationWindowId, FONT_NORMAL, gStringVar4, 0, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sNotificationWindowId, COPYWIN_FULL);
    PlaySE(SE_EXP_MAX);
    sNotificationShown = TRUE;
    sNotificationTimer = NOTIFICATION_FRAMES;
}

// Called every overworld frame from CB1_Overworld
void Achievements_OverworldUpdate(void)
{
    // The map name popup and the DexNav search windows share that part of the screen
    bool32 playerIsFree = !ArePlayerFieldControlsLocked() && !gPaletteFade.active
                       && GetMapNamePopUpWindowId() == WINDOW_NONE && !FlagGet(DN_FLAG_SEARCHING);

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

//// Achievements screen ////
// Full-screen list styled after the trophy screen in Pokémon SoulGold: a header with the category
// (L/R or left/right to change it), the achievements with a trophy in their tier's color and their
// status, and a box at the bottom with the description and the reward.

#include "data/rf_achievements_gfx.h"

#define SCREEN_ROWS       5
#define ROW_HEIGHT        16
#define LIST_WIDTH        18
#define STATUS_WIDTH      7
#define NAME_X            19
#define TAG_SCROLL_ARROWS 0x5A10

enum
{
    WIN_HEADER,
    WIN_LIST,
    WIN_STATUS,
    WIN_DESC,
};

// Text palette slots (sAchScreenTextPal)
#define COL_TRANSPARENT 0
#define COL_WHITE       1
#define COL_DARK        2
#define COL_SHADOW      3
#define COL_DARK_SHADOW 4
#define COL_SELECTED    5
#define COL_GREEN       6
#define COL_GOLD        11
#define COL_LOCKED      15

static const struct BgTemplate sAchScreenBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .priority = 0,
    },
    {
        .bg = 1,
        .charBaseIndex = 3,
        .mapBaseIndex = 30,
        .priority = 1,
    },
};

static const struct WindowTemplate sAchScreenWindowTemplates[] =
{
    [WIN_HEADER] = {.bg = 0, .tilemapLeft = 0,  .tilemapTop = 0,  .width = 30,           .height = 2,  .paletteNum = 1, .baseBlock = 1},
    [WIN_LIST]   = {.bg = 0, .tilemapLeft = 2,  .tilemapTop = 3,  .width = LIST_WIDTH,   .height = 10, .paletteNum = 1, .baseBlock = 61},
    [WIN_STATUS] = {.bg = 0, .tilemapLeft = 22, .tilemapTop = 3,  .width = STATUS_WIDTH, .height = 10, .paletteNum = 1, .baseBlock = 241},
    [WIN_DESC]   = {.bg = 0, .tilemapLeft = 1,  .tilemapTop = 14, .width = 28,           .height = 6,  .paletteNum = 1, .baseBlock = 311},
    DUMMY_WIN_TEMPLATE,
};

struct AchScreen
{
    MainCallback exitCallback;
    u16 ids[ACH_COUNT];
    u16 count;
    u16 scroll;
    u16 cursor;
    u8 category;
    u8 arrowsTaskId;
    bool8 showingMessage;
    struct CheckCache cache;
    u16 bg1Tilemap[32 * 32];
};

// Rojo Fuego+ dark theme: same layout, night surfaces. Trophy colours (7..14) are kept;
// the locked trophy (3 body, 15 rim) becomes an outline.
static void LoadAchScreenNightPalettes(void)
{
    static const u16 bg[12] =
    {
        RF_UI_NIGHT_GROUND, RGB(2, 7, 8), RF_UI_SELECTED, RF_UI_NIGHT_SHADOW,
        RF_UI_NIGHT_EDGE, RF_UI_NIGHT_PANEL, RF_UI_NIGHT_EDGE, RF_UI_NIGHT_SHADOW,
        RF_UI_NIGHT_PANEL, RF_UI_NIGHT_EDGE, RF_UI_NIGHT_MUTED, RGB(2, 7, 8),
    };
    static const u16 shadow = RF_UI_NIGHT_SHADOW, row = RF_UI_SELECTED;
    static const u16 green = RGB(12, 28, 14), locked = RGB(12, 17, 16);

    LoadPalette(bg, BG_PLTT_ID(0), sizeof(bg));
    LoadPalette(&shadow, BG_PLTT_ID(1) + COL_SHADOW, PLTT_SIZEOF(1));
    LoadPalette(&row, BG_PLTT_ID(1) + COL_SELECTED, PLTT_SIZEOF(1));
    LoadPalette(&green, BG_PLTT_ID(1) + COL_GREEN, PLTT_SIZEOF(1));
    LoadPalette(&locked, BG_PLTT_ID(1) + COL_LOCKED, PLTT_SIZEOF(1));
}

// COL_DARK also draws the L/R button glyphs, so night rows use COL_WHITE as ink instead.
static u32 AchInk(void)
{
    return KeySystem_IsOn(KEY_DARK_MODE) ? COL_WHITE : COL_DARK;
}

static EWRAM_DATA struct AchScreen *sAchScreen = NULL;
static EWRAM_DATA u8 sLastCategory = 0;

static bool32 IsShownAsHidden(u32 id)
{
    return gAchievements[id].hidden && !Achievement_IsUnlocked(id);
}

static void PrintColored(u32 windowId, u32 font, const u8 *text, u32 x, u32 y, u32 bg, u32 fg, u32 shadow)
{
    const u8 colors[3] = {bg, fg, shadow};
    AddTextPrinterParameterized4(windowId, font, x, y, 0, 0, colors, TEXT_SKIP_DRAW, text);
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

// "x/y" for counted achievements, a percentage when that does not fit, NULL when there is no count to show
static const u8 *BufferProgress(u32 id)
{
    u32 type = gAchievements[id].condType;
    u32 goal = GetGoal(id);
    u32 progress;

    if (IsShownAsHidden(id) || type == ACH_COND_EVENT || type == ACH_COND_TIER_ALL
     || type == ACH_COND_HIDDEN_ALL || goal <= 1)
        return NULL;

    progress = min(GetProgress(id, &sAchScreen->cache), goal);
    ConvertIntToDecimalStringN(gStringVar1, progress, STR_CONV_MODE_LEFT_ALIGN, 7);
    StringAppend(gStringVar1, COMPOUND_STRING("/"));
    ConvertIntToDecimalStringN(gStringVar1 + StringLength(gStringVar1), goal, STR_CONV_MODE_LEFT_ALIGN, 7);
    if (GetStringWidth(FONT_SMALL, gStringVar1, 0) > STATUS_WIDTH * 8 - 4)
    {
        ConvertIntToDecimalStringN(gStringVar1, progress * 100 / goal, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(gStringVar1, COMPOUND_STRING("%"));
    }
    return gStringVar1;
}

static void BuildCategoryList(void)
{
    u32 id;

    sAchScreen->count = 0;
    for (id = 0; id < ACH_COUNT; id++)
    {
        if (gAchievements[id].category == sAchScreen->category)
            sAchScreen->ids[sAchScreen->count++] = id;
    }
    sAchScreen->cursor = 0;
    sAchScreen->scroll = 0;
}

static void DrawHeader(void)
{
    u32 i, unlocked = 0, x;

    for (i = 0; i < sAchScreen->count; i++)
        unlocked += Achievement_IsUnlocked(sAchScreen->ids[i]) ? 1 : 0;

    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(COL_TRANSPARENT));
    PrintColored(WIN_HEADER, FONT_NORMAL, COMPOUND_STRING("LOGROS"), 6, 1, COL_TRANSPARENT, COL_WHITE, COL_DARK_SHADOW);

    StringCopy(gStringVar4, COMPOUND_STRING("{L_BUTTON} "));
    StringAppend(gStringVar4, sAchCategoryNames[sAchScreen->category]);
    StringAppend(gStringVar4, COMPOUND_STRING(" {R_BUTTON}"));
    x = (240 - GetStringWidth(FONT_NORMAL, gStringVar4, 0)) / 2;
    PrintColored(WIN_HEADER, FONT_NORMAL, gStringVar4, x, 1, COL_TRANSPARENT, COL_WHITE, COL_DARK_SHADOW);

    ConvertIntToDecimalStringN(gStringVar1, unlocked, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(gStringVar1, COMPOUND_STRING("/"));
    ConvertIntToDecimalStringN(gStringVar1 + StringLength(gStringVar1), sAchScreen->count, STR_CONV_MODE_LEFT_ALIGN, 3);
    x = 240 - 6 - GetStringWidth(FONT_NORMAL, gStringVar1, 0);
    PrintColored(WIN_HEADER, FONT_NORMAL, gStringVar1, x, 1, COL_TRANSPARENT, COL_WHITE, COL_DARK_SHADOW);
    CopyWindowToVram(WIN_HEADER, COPYWIN_GFX);
}

static void DrawRow(u32 row)
{
    u32 id = sAchScreen->ids[sAchScreen->scroll + row];
    u32 y = row * ROW_HEIGHT;
    u32 bg = (sAchScreen->scroll + row == sAchScreen->cursor) ? COL_SELECTED : COL_TRANSPARENT;
    u32 icon = Achievement_IsUnlocked(id) ? gAchievements[id].tier : ACH_TIER_COUNT;
    const u8 *name = IsShownAsHidden(id) ? COMPOUND_STRING("???") : gAchievements[id].name;
    u32 font = (GetStringWidth(FONT_NORMAL, name, 0) > LIST_WIDTH * 8 - NAME_X) ? FONT_NARROW : FONT_NORMAL;
    const u8 *status;
    u32 statusColor = AchInk(), statusFont = FONT_NORMAL;

    FillWindowPixelRect(WIN_LIST, PIXEL_FILL(bg), 0, y, LIST_WIDTH * 8, ROW_HEIGHT);
    FillWindowPixelRect(WIN_STATUS, PIXEL_FILL(bg), 0, y, STATUS_WIDTH * 8, ROW_HEIGHT);
    BlitBitmapToWindow(WIN_LIST, sAchTrophyIcons[icon], 1, y, 16, 16);
    PrintColored(WIN_LIST, font, name, NAME_X, y + 1, bg, AchInk(), COL_SHADOW);

    if (Achievement_IsClaimed(id))
    {
        status = COMPOUND_STRING("Hecho");
    }
    else if (Achievement_IsUnlocked(id))
    {
        status = COMPOUND_STRING("¡Premio!");
        statusColor = COL_GREEN;
    }
    else
    {
        status = BufferProgress(id);
        statusFont = FONT_SMALL;
        if (status == NULL)
        {
            status = COMPOUND_STRING("Bloq.");
            statusFont = FONT_NORMAL;
        }
        statusColor = COL_LOCKED;
        if (status == gStringVar1)
            statusColor = AchInk();
    }
    PrintColored(WIN_STATUS, statusFont, status, STATUS_WIDTH * 8 - 3 - GetStringWidth(statusFont, status, 0),
                 y + (statusFont == FONT_SMALL ? 3 : 1), bg, statusColor, COL_SHADOW);
}

static void DrawList(void)
{
    u32 row;

    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(COL_TRANSPARENT));
    FillWindowPixelBuffer(WIN_STATUS, PIXEL_FILL(COL_TRANSPARENT));
    for (row = 0; row < SCREEN_ROWS && sAchScreen->scroll + row < sAchScreen->count; row++)
        DrawRow(row);
    CopyWindowToVram(WIN_LIST, COPYWIN_GFX);
    CopyWindowToVram(WIN_STATUS, COPYWIN_GFX);
}

static void DrawDescription(const u8 *text, const u8 *lastLine)
{
    FillWindowPixelBuffer(WIN_DESC, PIXEL_FILL(COL_TRANSPARENT));
    PrintColored(WIN_DESC, FONT_NORMAL, text, 2, 3, COL_TRANSPARENT, COL_WHITE, COL_DARK_SHADOW);
    if (lastLine != NULL)
        PrintColored(WIN_DESC, FONT_NORMAL, lastLine, 2, 33, COL_TRANSPARENT, COL_GOLD, COL_DARK_SHADOW);
    CopyWindowToVram(WIN_DESC, COPYWIN_GFX);
}

static void DrawSelectedDescription(void)
{
    u32 id = sAchScreen->ids[sAchScreen->cursor];

    sAchScreen->showingMessage = FALSE;
    BufferRewardText(gStringVar3, id);
    if (IsShownAsHidden(id))
        DrawDescription(COMPOUND_STRING("Logro oculto. Sigue jugando\npara descubrirlo."), gStringVar3);
    else
        DrawDescription(gAchievements[id].description, gStringVar3);
}

static void CreateScrollArrows(void)
{
    struct ScrollArrowsTemplate template =
    {
        .firstArrowType = SCROLL_ARROW_UP,
        .firstX = 86,
        .firstY = 20,
        .secondArrowType = SCROLL_ARROW_DOWN,
        .secondX = 86,
        .secondY = 108,
        .fullyUpThreshold = 0,
        .tileTag = TAG_SCROLL_ARROWS,
        .palTag = TAG_SCROLL_ARROWS,
        .palNum = 0,
    };

    if (sAchScreen->arrowsTaskId != TASK_NONE)
        RemoveScrollIndicatorArrowPair(sAchScreen->arrowsTaskId);
    sAchScreen->arrowsTaskId = TASK_NONE;
    if (sAchScreen->count <= SCREEN_ROWS)
        return;
    template.fullyDownThreshold = sAchScreen->count - SCREEN_ROWS;
    sAchScreen->arrowsTaskId = AddScrollIndicatorArrowPair(&template, &sAchScreen->scroll);
}

static void ShowCategory(u32 category)
{
    sAchScreen->category = category;
    sLastCategory = category;
    BuildCategoryList();
    DrawHeader();
    DrawList();
    DrawSelectedDescription();
    CreateScrollArrows();
}

static void MoveCursor(s32 delta)
{
    s32 cursor = sAchScreen->cursor + delta;

    if (cursor < 0 || cursor >= sAchScreen->count)
        return;
    PlaySE(SE_SELECT);
    sAchScreen->cursor = cursor;
    if (cursor < sAchScreen->scroll)
        sAchScreen->scroll = cursor;
    else if (cursor >= sAchScreen->scroll + SCREEN_ROWS)
        sAchScreen->scroll = cursor - SCREEN_ROWS + 1;
    DrawList();
    DrawSelectedDescription();
}

static void TryClaimReward(u32 id)
{
    const struct Achievement *a = &gAchievements[id];

    if (!Achievement_IsUnlocked(id))
    {
        PlaySE(SE_FAILURE);
        return;
    }

    // A claimed title can be equipped again
    if (Achievement_IsClaimed(id))
    {
        if (a->rewardTitle == NULL)
            return;
        gSaveBlock3Ptr->achievementTitle = id + 1;
        PlaySE(SE_SELECT);
        StringCopy(gStringVar4, COMPOUND_STRING("Título equipado en tu Ficha:\n"));
        StringAppend(gStringVar4, a->rewardTitle);
        DrawDescription(gStringVar4, NULL);
        sAchScreen->showingMessage = TRUE;
        return;
    }

    if (a->rewardTitle != NULL)
    {
        gSaveBlock3Ptr->achievementTitle = id + 1;
        StringCopy(gStringVar4, COMPOUND_STRING("¡Título obtenido y equipado!\n"));
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
        DrawDescription(COMPOUND_STRING("No hay espacio en la mochila.\nHaz sitio y vuelve a intentarlo."), NULL);
        sAchScreen->showingMessage = TRUE;
        return;
    }
    SetBit(gSaveBlock3Ptr->achievementsClaimed, id);
    PlaySE(SE_EXP_MAX);
    DrawList();
    DrawDescription(gStringVar4, NULL);
    sAchScreen->showingMessage = TRUE;
}

static void AchScreen_VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void AchScreen_MainCB(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void Task_AchScreenExit(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    SetMainCallback2(sAchScreen->exitCallback);
    if (sAchScreen->arrowsTaskId != TASK_NONE)
        RemoveScrollIndicatorArrowPair(sAchScreen->arrowsTaskId);
    FreeAllWindowBuffers();
    FREE_AND_SET_NULL(sAchScreen);
    DestroyTask(taskId);
}

static void Task_AchScreenInput(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    if (JOY_REPEAT(DPAD_UP))
    {
        MoveCursor(-1);
    }
    else if (JOY_REPEAT(DPAD_DOWN))
    {
        MoveCursor(1);
    }
    else if (JOY_NEW(DPAD_LEFT | L_BUTTON))
    {
        PlaySE(SE_SELECT);
        ShowCategory(sAchScreen->category == 0 ? ACH_CAT_COUNT - 1 : sAchScreen->category - 1);
    }
    else if (JOY_NEW(DPAD_RIGHT | R_BUTTON))
    {
        PlaySE(SE_SELECT);
        ShowCategory(sAchScreen->category == ACH_CAT_COUNT - 1 ? 0 : sAchScreen->category + 1);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        if (sAchScreen->showingMessage)
            DrawSelectedDescription();
        else
            TryClaimReward(sAchScreen->ids[sAchScreen->cursor]);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_AchScreenExit;
    }
}

static void CB2_AchScreenSetup(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankHBlankCallbacksToNull();
        ClearScheduledBgCopiesToVram();
        ScanlineEffect_Stop();
        FreeAllSpritePalettes();
        ResetPaletteFade();
        ResetSpriteData();
        ResetTasks();
        gMain.state++;
        break;
    case 1:
        ResetVramOamAndBgCntRegs();
        ResetAllBgsCoordinates();
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sAchScreenBgTemplates, NELEMS(sAchScreenBgTemplates));
        memcpy(sAchScreen->bg1Tilemap, sAchScreenTilemap, sizeof(sAchScreenTilemap));
        SetBgTilemapBuffer(1, sAchScreen->bg1Tilemap);
        LoadBgTiles(1, sAchScreenTiles, sizeof(sAchScreenTiles), 0);
        ScheduleBgCopyTilemapToVram(1);
        gMain.state++;
        break;
    case 2:
        InitWindows(sAchScreenWindowTemplates);
        DeactivateAllTextPrinters();
        LoadPalette(sAchScreenBgPal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
        LoadPalette(sAchScreenTextPal, BG_PLTT_ID(1), PLTT_SIZE_4BPP);
        if (KeySystem_IsOn(KEY_DARK_MODE))
            LoadAchScreenNightPalettes();
        gMain.state++;
        break;
    case 3:
        PutWindowTilemap(WIN_HEADER);
        PutWindowTilemap(WIN_LIST);
        PutWindowTilemap(WIN_STATUS);
        PutWindowTilemap(WIN_DESC);
        ShowCategory(sLastCategory);
        ScheduleBgCopyTilemapToVram(0);
        gMain.state++;
        break;
    default:
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        ShowBg(0);
        ShowBg(1);
        BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        CreateTask(Task_AchScreenInput, 0);
        SetVBlankCallback(AchScreen_VBlankCB);
        SetMainCallback2(AchScreen_MainCB);
        break;
    }
}

// Opens the screen; exitCallback runs when it closes
void Achievements_OpenScreen(MainCallback exitCallback)
{
    Achievements_HideNotification();
    Achievements_CheckAll();
    sAchScreen = AllocZeroed(sizeof(*sAchScreen));
    if (sAchScreen == NULL)
    {
        SetMainCallback2(exitCallback);
        return;
    }
    sAchScreen->exitCallback = exitCallback;
    sAchScreen->arrowsTaskId = TASK_NONE;
    gMain.state = 0;
    SetMainCallback2(CB2_AchScreenSetup);
}

// From the bag
void CB2_OpenAchievementsScreenFromBag(void)
{
    gFieldCallback = FieldCB_ReturnToFieldNoScript;
    Achievements_OpenScreen(CB2_ReturnToField);
}

static void Task_OpenAchievementsScreenFromField(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    CleanupOverworldWindowsAndTilemaps();
    gFieldCallback = FieldCB_ReturnToFieldNoScript;
    Achievements_OpenScreen(CB2_ReturnToField);
    DestroyTask(taskId);
}

// From the field (registered item): fades out first
void Achievements_OpenMenu(void)
{
    Achievements_HideNotification();
    FadeScreen(FADE_TO_BLACK, 0);
    CreateTask(Task_OpenAchievementsScreenFromField, 10);
}
