#ifndef GUARD_RF_QUESTS_H
#define GUARD_RF_QUESTS_H

#include "constants/rf_quests.h"
#include "constants/rf_quests_extra.h"
#include "constants/rf_story.h"

enum RFQuestState
{
    RF_QUEST_AVAILABLE,
    RF_QUEST_ACTIVE,
    RF_QUEST_READY,
    RF_QUEST_CLAIMED,
};

enum QuestCondition
{
    QUEST_VISIT,
    QUEST_PARTY_EITHER,
    QUEST_PARTY_BOTH,
    QUEST_ITEM_OR_FOSSIL,
    QUEST_REVIVED_FOSSIL,
    QUEST_EVENT,
    QUEST_DELIVER_ITEM,
    QUEST_LEGENDS, // argument: legend saga; ready once every legend of the saga is caught
    QUEST_DEX_CAUGHT, // argument: species; ready once it has been caught at least once
    QUEST_COLLECT_ITEM, // argument: item; ready while the item is in the bag
    QUEST_USE_ITEM, // argument: item; completed by a successful item-use path
};

struct RFQuest
{
    const u8 *name;
    const u8 *description;
    u16 prerequisite;
    u16 requiredFlag;
    u16 argument;
    u16 alternative;
    u16 rewardItem;
    u8 rewardCount;
    u8 condition;
    u8 chapter;
    u8 objectiveCount; // 0/1: primary only; 2: primary AND secondary
    u8 secondaryCondition;
    u16 secondaryArgument;
    u16 secondaryAlternative;
};

const struct RFQuest *RFQuests_GetCatalog(void);
void RFQuests_Refresh(void);
bool32 RFQuests_Accept(u32 id);
bool32 RFQuests_Claim(u32 id);
void RFQuests_Update(void);
void RFQuests_OpenMenu(void);
enum RFQuestState RFQuests_GetState(u32 id);
bool32 RFQuests_IsUnlocked(u32 id);
bool32 RFQuests_HasRequirements(u32 id);
u32 RFQuests_NotifyEvent(u32 event, u32 argument);
bool32 RFQuests_DeliverItem(u32 id);
u32 RFQuests_NotifyItemCollected(u32 item);
u32 RFQuests_NotifyItemUsed(u32 item);
void RFQuests_OnBattleEnd(void);
void Script_RF_QuestNotify(void);
void Script_RF_QuestDeliver(void);
void Script_RF_QuestGetState(void);

#endif
