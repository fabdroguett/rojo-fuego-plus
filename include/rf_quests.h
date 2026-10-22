#ifndef GUARD_RF_QUESTS_H
#define GUARD_RF_QUESTS_H

#include "constants/rf_quests.h"

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
};

const struct RFQuest *RFQuests_GetCatalog(void);
void RFQuests_Refresh(void);
bool32 RFQuests_Accept(u32 id);
bool32 RFQuests_Claim(u32 id);
void RFQuests_Update(void);
void RFQuests_OpenMenu(void);
enum RFQuestState RFQuests_GetState(u32 id);
bool32 RFQuests_IsUnlocked(u32 id);

#endif
