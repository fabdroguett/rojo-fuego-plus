#include "global.h"
#include "achievements.h"
#include "event_data.h"
#include "fieldmap.h"
#include "item.h"
#include "money.h"
#include "pokemon.h"
#include "pokedex.h"
#include "rf_legends.h"
#include "rf_quests.h"
#include "rf_story.h"
#include "constants/opponents_frlg.h"

#define QUEST_SAVE_MAGIC 0x51535431
#define QUEST_EXT_MAGIC 0x51535832
#define COUNTER_ID_BITS 10
#define COUNTER_ID_MASK ((1 << COUNTER_ID_BITS) - 1)
#define FREE_COUNTER ((u32)-1) // a free slot stores 0, which FindCounter matches as ID + 1

#include "data/rf_quests.h"
#include "data/rf_quest_links.h"

STATIC_ASSERT(RF_QUEST_COUNT <= RF_QUEST_MAX, RFQuestSaveCapacity);
STATIC_ASSERT(RF_QUEST_MAX < COUNTER_ID_MASK, RFQuestCounterIdFits);
STATIC_ASSERT(RF_QUEST_COUNTER_MAX == (0xFFFF >> COUNTER_ID_BITS), RFQuestCounterValueFits);
STATIC_ASSERT(ARRAY_COUNT(sRFQuests) == RF_QUEST_COUNT, RFQuestCatalogSize);

static void EnsureQuestSave(void)
{
    // The previous save layout ended before these fields. Old padding is not assumed zero.
    if (gSaveBlock3Ptr->rfQuestSaveMagic != QUEST_SAVE_MAGIC)
    {
        memset(gSaveBlock3Ptr->rfQuestStates, 0, sizeof(gSaveBlock3Ptr->rfQuestStates));
        gSaveBlock3Ptr->rfQuestTracked = 0;
        gSaveBlock3Ptr->rfQuestSaveMagic = QUEST_SAVE_MAGIC;
    }
    // The extension has its own magic: older saves keep every shipped state untouched.
    if (gSaveBlock3Ptr->rfQuestExtMagic != QUEST_EXT_MAGIC)
    {
        memset(gSaveBlock3Ptr->rfQuestStatesExt, 0, sizeof(gSaveBlock3Ptr->rfQuestStatesExt));
        memset(gSaveBlock3Ptr->rfQuestCounters, 0, sizeof(gSaveBlock3Ptr->rfQuestCounters));
        gSaveBlock3Ptr->rfQuestChoices = 0;
        gSaveBlock3Ptr->rfQuestExtMagic = QUEST_EXT_MAGIC;
    }
}

static u8 *StateByte(u32 id)
{
    return id < RF_QUEST_CAPACITY ? &gSaveBlock3Ptr->rfQuestStates[id / 4]
                                  : &gSaveBlock3Ptr->rfQuestStatesExt[(id - RF_QUEST_CAPACITY) / 4];
}

enum RFQuestState RFQuests_GetState(u32 id)
{
    if (id >= RF_QUEST_COUNT)
        return RF_QUEST_AVAILABLE;
    EnsureQuestSave();
    return (*StateByte(id) >> ((id % 4) * 2)) & 3;
}

static void SetQuestState(u32 id, enum RFQuestState state)
{
    u32 shift = (id % 4) * 2;
    u8 *states = StateByte(id);
    *states = (*states & ~(3 << shift)) | (state << shift);
}

// Objective met: the field shows "¡MISIÓN LISTA!" once the player is free again.
static void MarkReady(u32 id)
{
    SetQuestState(id, RF_QUEST_READY);
    Achievements_QueueQuestNotice(id, FALSE);
}

// Counter slots: index of the slot holding `id`, or RF_QUEST_COUNTER_SLOTS.
static u32 FindCounter(u32 id)
{
    u32 i;
    for (i = 0; i < RF_QUEST_COUNTER_SLOTS; i++)
        if ((gSaveBlock3Ptr->rfQuestCounters[i] & COUNTER_ID_MASK) == id + 1)
            return i;
    return RF_QUEST_COUNTER_SLOTS;
}

static void ReleaseCounter(u32 id)
{
    u32 slot = FindCounter(id);
    if (slot < RF_QUEST_COUNTER_SLOTS)
        gSaveBlock3Ptr->rfQuestCounters[slot] = 0;
}

bool32 RFQuests_CounterSlotsFull(void)
{
    EnsureQuestSave();
    return FindCounter(FREE_COUNTER) == RF_QUEST_COUNTER_SLOTS;
}

u32 RFQuests_GetProgress(u32 id)
{
    u32 slot;
    if (id >= RF_QUEST_COUNT || sRFQuests[id].condition != QUEST_COUNT)
        return 0;
    if (RFQuests_GetState(id) >= RF_QUEST_READY)
        return sRFQuests[id].target;
    slot = FindCounter(id);
    return slot < RF_QUEST_COUNTER_SLOTS ? gSaveBlock3Ptr->rfQuestCounters[slot] >> COUNTER_ID_BITS : 0;
}

bool32 RFQuests_GetChoice(u32 choice)
{
    EnsureQuestSave();
    return choice < RF_QUEST_CHOICE_COUNT && ((gSaveBlock3Ptr->rfQuestChoices >> choice) & 1);
}

void RFQuests_SetChoice(u32 choice)
{
    EnsureQuestSave();
    if (choice < RF_QUEST_CHOICE_COUNT)
        gSaveBlock3Ptr->rfQuestChoices |= 1u << choice;
}

static bool32 ExtraRequirementsMet(u32 id)
{
    u32 i, needed = 0, met = 0;

    for (i = 0; i < ARRAY_COUNT(sRFQuestRequirements); i++)
    {
        const struct RFQuestRequirement *req = &sRFQuestRequirements[i];
        if (req->quest != id)
            continue;
        needed |= 1u << req->group;
        if (RFQuests_GetState(req->needs) == RF_QUEST_CLAIMED)
            met |= 1u << req->group;
    }
    return needed == met;
}

bool32 RFQuests_IsUnlocked(u32 id)
{
    const struct RFQuest *quest;

    if (!IS_FRLG || id >= RF_QUEST_COUNT || !FlagGet(FLAG_SYS_POKEDEX_GET))
        return FALSE;
    quest = &sRFQuests[id];
    return (quest->requiredFlag == 0 || FlagGet(quest->requiredFlag))
        && (quest->prerequisite == RF_QUEST_NONE
         || RFQuests_GetState(quest->prerequisite) == RF_QUEST_CLAIMED)
        && ExtraRequirementsMet(id);
}

static const struct RFQuestBonus *FindBonus(u32 id)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sRFQuestBonuses); i++)
        if (sRFQuestBonuses[i].quest == id)
            return &sRFQuestBonuses[i];
    return NULL;
}

static bool32 PartyHasSpecies(u32 species)
{
    u32 i;

    if (species == SPECIES_NONE)
        return FALSE;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GET_BASE_SPECIES_ID(GetMonData(mon, MON_DATA_SPECIES)) == species && !GetMonData(mon, MON_DATA_IS_EGG))
            return TRUE;
    }
    return FALSE;
}

static u32 CountBadges(void)
{
    static const u16 badges[] = {
        FLAG_BADGE01_GET, FLAG_BADGE02_GET, FLAG_BADGE03_GET, FLAG_BADGE04_GET,
        FLAG_BADGE05_GET, FLAG_BADGE06_GET, FLAG_BADGE07_GET, FLAG_BADGE08_GET,
    };
    u32 i, count = 0;
    for (i = 0; i < ARRAY_COUNT(badges); i++)
        count += FlagGet(badges[i]) ? 1 : 0;
    return count;
}

static bool32 QuestConditionMet(u32 condition, u32 argument, u32 alternative)
{
    switch (condition)
    {
    case QUEST_VISIT:
        return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(argument)
            && gSaveBlock1Ptr->location.mapNum == MAP_NUM(argument);
    case QUEST_PARTY_EITHER:
        return PartyHasSpecies(argument) || PartyHasSpecies(alternative);
    case QUEST_PARTY_BOTH:
        return PartyHasSpecies(argument) && PartyHasSpecies(alternative);
    case QUEST_ITEM_OR_FOSSIL:
    case QUEST_REVIVED_FOSSIL:
        // Fossils already revived must remain eligible; nothing is consumed or removed.
        if (condition != QUEST_REVIVED_FOSSIL
         && (CheckBagHasItem(argument, 1)
          || (alternative != ITEM_NONE && CheckBagHasItem(alternative, 1))))
            return TRUE;
        if (argument == ITEM_HELIX_FOSSIL)
            return PartyHasSpecies(SPECIES_OMANYTE) || PartyHasSpecies(SPECIES_OMASTAR)
                || PartyHasSpecies(SPECIES_KABUTO) || PartyHasSpecies(SPECIES_KABUTOPS);
        return FALSE;
    case QUEST_LEGENDS:
        return RFLegends_IsSagaCaught(argument);
    case QUEST_DEX_CAUGHT:
        return GetSetPokedexFlag(SpeciesToNationalPokedexNum(argument), FLAG_GET_CAUGHT);
    case QUEST_COLLECT_ITEM:
        return CheckBagHasItem(argument, 1);
    case QUEST_DEX_SEEN:
        return GetSetPokedexFlag(SpeciesToNationalPokedexNum(argument), FLAG_GET_SEEN);
    case QUEST_BADGES:
        return CountBadges() >= argument;
    case QUEST_ACHIEVEMENT:
        return Achievement_IsUnlocked(argument);
    }
    return FALSE;
}

static bool32 QuestObjectiveMet(const struct RFQuest *quest)
{
    if (!QuestConditionMet(quest->condition, quest->argument, quest->alternative))
        return FALSE;
    return quest->objectiveCount < 2
        || QuestConditionMet(quest->secondaryCondition, quest->secondaryArgument, quest->secondaryAlternative);
}

bool32 RFQuests_HasRequirements(u32 id)
{
    const struct RFQuest *quest;
    if (id >= RF_QUEST_COUNT)
        return FALSE;
    quest = &sRFQuests[id];
    return quest->objectiveCount < 2
        || QuestConditionMet(quest->secondaryCondition, quest->secondaryArgument, quest->secondaryAlternative);
}

static void CheckQuestObjectives(void)
{
    u32 i;

    if (!IS_FRLG || !FlagGet(FLAG_SYS_POKEDEX_GET))
        return;
    EnsureQuestSave();
    for (i = 0; i < RF_QUEST_COUNT; i++)
    {
        if (RFQuests_GetState(i) == RF_QUEST_ACTIVE && QuestObjectiveMet(&sRFQuests[i]))
            MarkReady(i);
    }
}

void RFQuests_Update(void)
{
    static u8 timer;

    // Also runs during movement: short map visits should count, even before a menu is opened.
    if (++timer >= 30)
    {
        timer = 0;
        CheckQuestObjectives();
    }
}

bool32 RFQuests_IsObjectHidden(const u8 *script)
{
    u32 i;

    if (!IS_FRLG || script == NULL)
        return FALSE;
    for (i = 0; i < ARRAY_COUNT(sRFQuestObjects); i++)
    {
        const struct RFQuestObject *object = &sRFQuestObjects[i];
        u32 state;
        if (object->script != script || object->quest >= RF_QUEST_COUNT)
            continue;
        state = RFQuests_GetState(object->quest);
        if (state < object->minState || state > object->maxState)
            return TRUE;
    }
    return FALSE;
}

// Called after the map's own on-load script, so quest progress has the last word.
void RFQuests_ApplyWorldChanges(void)
{
    u32 i, map;

    if (!IS_FRLG)
        return;
    map = gSaveBlock1Ptr->location.mapNum | (gSaveBlock1Ptr->location.mapGroup << 8);
    for (i = 0; i < ARRAY_COUNT(sRFQuestTiles); i++)
    {
        const struct RFQuestTile *tile = &sRFQuestTiles[i];
        if (tile->quest < RF_QUEST_COUNT && tile->map == map
         && RFQuests_GetState(tile->quest) >= tile->minState)
            MapGridSetMetatileIdAt(tile->x + MAP_OFFSET, tile->y + MAP_OFFSET, tile->metatile);
    }
}

// Entering the map of the tracked campaign scene reminds the player that it continues here.
void RFQuests_OnMapLoad(void)
{
    u32 tracked, map;

    RFQuests_ApplyWorldChanges();
    if (!IS_FRLG || !FlagGet(FLAG_SYS_POKEDEX_GET))
        return;
    tracked = gSaveBlock3Ptr->rfQuestTracked;
    if (tracked == 0 || tracked > RF_QUEST_COUNT || tracked - 1 < RF_STORY_FIRST
     || tracked - 1 >= RF_STORY_FIRST + RF_STORY_COUNT || RFQuests_GetState(tracked - 1) != RF_QUEST_ACTIVE)
        return;
    map = gSaveBlock1Ptr->location.mapNum | (gSaveBlock1Ptr->location.mapGroup << 8);
    if (RFStory_TargetMap(tracked - 1) == map)
        Achievements_QueueQuestNotice(tracked - 1, TRUE);
}

const struct RFQuest *RFQuests_GetCatalog(void)
{
    return sRFQuests;
}

void RFQuests_Refresh(void)
{
    CheckQuestObjectives();
}

// Only active objectives receive events: a past conversation or victory is not retroactive.
// Ready/claimed states already persist, so one-shot events need no extra save fields.
u32 RFQuests_NotifyEvent(u32 event, u32 argument)
{
    u32 i, completed = 0;

    if (!IS_FRLG || !FlagGet(FLAG_SYS_POKEDEX_GET))
        return 0;
    EnsureQuestSave();
    for (i = 0; i < RF_QUEST_COUNT; i++)
    {
        const struct RFQuest *quest = &sRFQuests[i];
        if (quest->condition == QUEST_COUNT && quest->argument == event
         && (quest->alternative == 0 || quest->alternative == argument)
         && RFQuests_GetState(i) == RF_QUEST_ACTIVE)
        {
            u32 slot = FindCounter(i);
            u32 count;
            if (slot == RF_QUEST_COUNTER_SLOTS)
                continue; // never accepted with a slot; Accept guarantees one
            count = (gSaveBlock3Ptr->rfQuestCounters[slot] >> COUNTER_ID_BITS) + 1;
            if (count >= quest->target
             && (quest->objectiveCount < 2
              || QuestConditionMet(quest->secondaryCondition, quest->secondaryArgument, quest->secondaryAlternative)))
            {
                ReleaseCounter(i);
                MarkReady(i);
                completed++;
            }
            else
            {
                count = min(count, (u32)quest->target - 1);
                gSaveBlock3Ptr->rfQuestCounters[slot] = (i + 1) | (count << COUNTER_ID_BITS);
            }
            continue;
        }
        if (quest->condition == QUEST_EVENT && quest->argument == event
         && (quest->alternative == 0 || quest->alternative == argument)
         && (quest->objectiveCount < 2
          || QuestConditionMet(quest->secondaryCondition, quest->secondaryArgument, quest->secondaryAlternative))
         && RFQuests_GetState(i) == RF_QUEST_ACTIVE)
        {
            MarkReady(i);
            completed++;
        }
    }
    return completed;
}

bool32 RFQuests_DeliverItem(u32 id)
{
    const struct RFQuest *quest;

    if (!RFQuests_IsUnlocked(id) || RFQuests_GetState(id) != RF_QUEST_ACTIVE)
        return FALSE;
    quest = &sRFQuests[id];
    if (quest->condition != QUEST_DELIVER_ITEM || quest->alternative == 0
     || !CheckBagHasItem(quest->argument, quest->alternative)
     || !RemoveBagItem(quest->argument, quest->alternative))
        return FALSE;
    MarkReady(id);
    return TRUE;
}

u32 RFQuests_NotifyItemCollected(u32 item)
{
    u32 i, completed = 0;

    if (!IS_FRLG || !FlagGet(FLAG_SYS_POKEDEX_GET))
        return 0;
    EnsureQuestSave();
    for (i = 0; i < RF_QUEST_COUNT; i++)
    {
        const struct RFQuest *quest = &sRFQuests[i];
        if (quest->condition == QUEST_COLLECT_ITEM && quest->argument == item
         && RFQuests_GetState(i) == RF_QUEST_ACTIVE)
        {
            MarkReady(i);
            completed++;
        }
    }
    return completed;
}

u32 RFQuests_NotifyItemUsed(u32 item)
{
    u32 i, completed = 0;

    if (!IS_FRLG || !FlagGet(FLAG_SYS_POKEDEX_GET))
        return 0;
    EnsureQuestSave();
    for (i = 0; i < RF_QUEST_COUNT; i++)
    {
        const struct RFQuest *quest = &sRFQuests[i];
        if (quest->condition == QUEST_USE_ITEM && quest->argument == item
         && RFQuests_GetState(i) == RF_QUEST_ACTIVE)
        {
            MarkReady(i);
            completed++;
        }
    }
    return completed;
}

bool32 RFQuests_Accept(u32 id)
{
    if (!RFQuests_IsUnlocked(id) || RFQuests_GetState(id) != RF_QUEST_AVAILABLE)
        return FALSE;
    if (sRFQuests[id].condition == QUEST_COUNT)
    {
        u32 slot = FindCounter(FREE_COUNTER);
        if (slot == RF_QUEST_COUNTER_SLOTS)
            return FALSE; // the player finishes another counted quest first
        gSaveBlock3Ptr->rfQuestCounters[slot] = id + 1;
    }
    SetQuestState(id, RF_QUEST_ACTIVE);
    gSaveBlock3Ptr->rfQuestTracked = id + 1;
    CheckQuestObjectives();
    return TRUE;
}

bool32 RFQuests_Claim(u32 id)
{
    const struct RFQuest *quest;
    const struct RFQuestBonus *bonus;

    if (!RFQuests_IsUnlocked(id) || RFQuests_GetState(id) != RF_QUEST_READY)
        return FALSE;
    quest = &sRFQuests[id];
    bonus = FindBonus(id);
    if (bonus != NULL && bonus->item != ITEM_NONE)
    {
        // Both items must fit before anything is granted; a full bag keeps the quest ready.
        u32 sameItem = bonus->item == quest->rewardItem;
        if (!CheckBagHasSpace(quest->rewardItem, quest->rewardCount + (sameItem ? bonus->itemCount : 0))
         || (!sameItem && !CheckBagHasSpace(bonus->item, bonus->itemCount)))
            return FALSE;
    }
    if (!AddBagItem(quest->rewardItem, quest->rewardCount))
        return FALSE;
    if (bonus != NULL)
    {
        if (bonus->item != ITEM_NONE)
            AddBagItem(bonus->item, bonus->itemCount);
        if (bonus->money != 0)
            AddMoney(&gSaveBlock1Ptr->money, bonus->money);
        if (bonus->flag != 0)
            FlagSet(bonus->flag);
        if (bonus->achievement != 0)
            Achievement_Unlock(bonus->achievement - 1);
    }
    SetQuestState(id, RF_QUEST_CLAIMED);
    RFStory_OnClaim(id);
    if (gSaveBlock3Ptr->rfQuestTracked == id + 1)
    {
        gSaveBlock3Ptr->rfQuestTracked = 0;
        if (id >= RF_STORY_FIRST)
        {
            u32 next;
            for (next = id + 1; next < RF_QUEST_COUNT; next++)
                if (sRFQuests[next].prerequisite == id && RFQuests_IsUnlocked(next)
                 && RFQuests_GetState(next) == RF_QUEST_AVAILABLE)
                {
                    // Track the next lead; accepting it remains the player's decision.
                    gSaveBlock3Ptr->rfQuestTracked = next + 1;
                    break;
                }
        }
    }
    return TRUE;
}
