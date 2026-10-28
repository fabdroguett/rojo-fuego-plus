#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "event_data.h"
#include "pokemon.h"
#include "rf_quests.h"
#include "rf_story.h"
#include "string_util.h"

// Counted objectives: a catch reports its species, map and each distinct type.
static void NotifyCatch(u32 species)
{
    u32 type1, type2;

    species = GET_BASE_SPECIES_ID(species);
    RFQuests_NotifyEvent(RF_QUEST_EVENT_CATCH, species);
    RFQuests_NotifyEvent(RF_QUEST_EVENT_CATCH_MAP,
                         gSaveBlock1Ptr->location.mapNum | (gSaveBlock1Ptr->location.mapGroup << 8));
    type1 = GetSpeciesType(species, 0);
    type2 = GetSpeciesType(species, 1);
    RFQuests_NotifyEvent(RF_QUEST_EVENT_CATCH_TYPE, type1);
    if (type2 != type1)
        RFQuests_NotifyEvent(RF_QUEST_EVENT_CATCH_TYPE, type2);
}

void RFQuests_OnBattleEnd(void)
{
    if (!(gBattleTypeFlags & (BATTLE_TYPE_TRAINER | BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED
                            | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_TRAINER_HILL | BATTLE_TYPE_SECRET_BASE)))
    {
        if (gBattleOutcome == B_OUTCOME_CAUGHT)
            NotifyCatch(gBattleResults.caughtMonSpecies);
        else if (gBattleOutcome == B_OUTCOME_WON)
            RFQuests_NotifyEvent(RF_QUEST_EVENT_WILD_WIN, GET_BASE_SPECIES_ID(gBattleResults.lastOpponentSpecies));
    }
    // Facility trainer IDs belong to different tables; simulated/link wins do not count.
    if (gBattleOutcome == B_OUTCOME_WON && (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
     && !(gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED
                          | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_TRAINER_HILL
                          | BATTLE_TYPE_SECRET_BASE)))
    {
        RFQuests_NotifyEvent(RF_QUEST_EVENT_TRAINER_WIN, TRAINER_BATTLE_PARAM.opponentA);
        RFStory_NotifyTrainerWin(TRAINER_BATTLE_PARAM.opponentA);
    }
}

void Script_RF_QuestNotify(void)
{
    gSpecialVar_Result = RFQuests_NotifyEvent(gSpecialVar_0x8004, gSpecialVar_0x8005);
}

void Script_RF_QuestGetState(void)
{
    gSpecialVar_Result = RFQuests_GetState(gSpecialVar_0x8004);
}

// Story decisions: VAR_0x8004 = decision bit. Setting is permanent and never cleared.
void Script_RF_QuestGetChoice(void)
{
    gSpecialVar_Result = RFQuests_GetChoice(gSpecialVar_0x8004);
}

void Script_RF_QuestSetChoice(void)
{
    RFQuests_SetChoice(gSpecialVar_0x8004);
}

// For guides (Bill, the PC): gStringVar1 = tracked quest name, gStringVar4 = its next hint.
// VAR_RESULT is FALSE when nothing unfinished is tracked.
void Script_RF_QuestTrackedHint(void)
{
    u32 tracked = gSaveBlock3Ptr->rfQuestTracked;

    gSpecialVar_Result = tracked != 0 && tracked <= RF_QUEST_COUNT
                      && RFQuests_GetState(tracked - 1) != RF_QUEST_CLAIMED;
    if (!gSpecialVar_Result)
        return;
    StringCopy(gStringVar1, RFQuests_GetCatalog()[tracked - 1].name);
    StringCopy(gStringVar4, RFStory_Hint(tracked - 1));
}

void Script_RF_QuestDeliver(void)
{
    u32 id = gSpecialVar_0x8004;

    gSpecialVar_Result = 0; // inactive, invalid, or not a delivery objective
    if (id >= RF_QUEST_COUNT || RFQuests_GetCatalog()[id].condition != QUEST_DELIVER_ITEM
     || RFQuests_GetState(id) != RF_QUEST_ACTIVE || !RFQuests_IsUnlocked(id))
        return;
    gSpecialVar_Result = RFQuests_DeliverItem(id) ? 1 : 2;
}
