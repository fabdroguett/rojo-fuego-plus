#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "event_data.h"
#include "rf_quests.h"
#include "rf_story.h"

void RFQuests_OnBattleEnd(void)
{
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

void Script_RF_QuestDeliver(void)
{
    u32 id = gSpecialVar_0x8004;

    gSpecialVar_Result = 0; // inactive, invalid, or not a delivery objective
    if (id >= RF_QUEST_COUNT || RFQuests_GetCatalog()[id].condition != QUEST_DELIVER_ITEM
     || RFQuests_GetState(id) != RF_QUEST_ACTIVE || !RFQuests_IsUnlocked(id))
        return;
    gSpecialVar_Result = RFQuests_DeliverItem(id) ? 1 : 2;
}
