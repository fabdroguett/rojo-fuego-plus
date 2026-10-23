#include "global.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "rf_legends.h"
#include "rf_quests.h"
#include "constants/opponents_frlg.h"

#define QUEST_SAVE_MAGIC 0x51535431

#include "data/rf_quests.h"

STATIC_ASSERT(RF_QUEST_COUNT <= RF_QUEST_CAPACITY, RFQuestSaveCapacity);
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
}

enum RFQuestState RFQuests_GetState(u32 id)
{
    if (id >= RF_QUEST_COUNT)
        return RF_QUEST_AVAILABLE;
    EnsureQuestSave();
    return (gSaveBlock3Ptr->rfQuestStates[id / 4] >> ((id % 4) * 2)) & 3;
}

static void SetQuestState(u32 id, enum RFQuestState state)
{
    u32 shift = (id % 4) * 2;
    u8 *states = &gSaveBlock3Ptr->rfQuestStates[id / 4];
    *states = (*states & ~(3 << shift)) | (state << shift);
}

bool32 RFQuests_IsUnlocked(u32 id)
{
    const struct RFQuest *quest;

    if (!IS_FRLG || id >= RF_QUEST_COUNT || !FlagGet(FLAG_SYS_POKEDEX_GET))
        return FALSE;
    quest = &sRFQuests[id];
    return (quest->requiredFlag == 0 || FlagGet(quest->requiredFlag))
        && (quest->prerequisite == RF_QUEST_NONE
         || RFQuests_GetState(quest->prerequisite) == RF_QUEST_CLAIMED);
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

static bool32 QuestObjectiveMet(const struct RFQuest *quest)
{
    switch (quest->condition)
    {
    case QUEST_VISIT:
        return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(quest->argument)
            && gSaveBlock1Ptr->location.mapNum == MAP_NUM(quest->argument);
    case QUEST_PARTY_EITHER:
        return PartyHasSpecies(quest->argument) || PartyHasSpecies(quest->alternative);
    case QUEST_PARTY_BOTH:
        return PartyHasSpecies(quest->argument) && PartyHasSpecies(quest->alternative);
    case QUEST_ITEM_OR_FOSSIL:
    case QUEST_REVIVED_FOSSIL:
        // Fossils already revived must remain eligible; nothing is consumed or removed.
        if (quest->condition != QUEST_REVIVED_FOSSIL
         && (CheckBagHasItem(quest->argument, 1)
          || (quest->alternative != ITEM_NONE && CheckBagHasItem(quest->alternative, 1))))
            return TRUE;
        if (quest->argument == ITEM_HELIX_FOSSIL)
            return PartyHasSpecies(SPECIES_OMANYTE) || PartyHasSpecies(SPECIES_OMASTAR)
                || PartyHasSpecies(SPECIES_KABUTO) || PartyHasSpecies(SPECIES_KABUTOPS);
        return FALSE;
    case QUEST_LEGENDS:
        return RFLegends_IsSagaCaught(quest->argument);
    }
    return FALSE;
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
            SetQuestState(i, RF_QUEST_READY);
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
        if (quest->condition == QUEST_EVENT && quest->argument == event
         && (quest->alternative == 0 || quest->alternative == argument)
         && RFQuests_GetState(i) == RF_QUEST_ACTIVE)
        {
            SetQuestState(i, RF_QUEST_READY);
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
    SetQuestState(id, RF_QUEST_READY);
    return TRUE;
}

bool32 RFQuests_Accept(u32 id)
{
    if (!RFQuests_IsUnlocked(id) || RFQuests_GetState(id) != RF_QUEST_AVAILABLE)
        return FALSE;
    SetQuestState(id, RF_QUEST_ACTIVE);
    gSaveBlock3Ptr->rfQuestTracked = id + 1;
    CheckQuestObjectives();
    return TRUE;
}

bool32 RFQuests_Claim(u32 id)
{
    const struct RFQuest *quest;

    if (!RFQuests_IsUnlocked(id) || RFQuests_GetState(id) != RF_QUEST_READY)
        return FALSE;
    quest = &sRFQuests[id];
    if (!AddBagItem(quest->rewardItem, quest->rewardCount))
        return FALSE;
    SetQuestState(id, RF_QUEST_CLAIMED);
    if (gSaveBlock3Ptr->rfQuestTracked == id + 1)
        gSaveBlock3Ptr->rfQuestTracked = 0;
    return TRUE;
}
