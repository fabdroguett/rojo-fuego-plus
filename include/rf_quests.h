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
    QUEST_COUNT, // argument: event, alternative: filter (0 = any); ready after `target` events
    QUEST_DEX_SEEN, // argument: species; ready once it has been seen (observed, not caught)
    QUEST_BADGES, // argument: number of Kanto badges held
    QUEST_ACHIEVEMENT, // argument: achievement ID; ready once it is unlocked
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
    u8 target; // QUEST_COUNT only: events needed, 2..RF_QUEST_COUNTER_MAX
};

struct RFQuestRequirement
{
    u16 quest;
    u16 needs; // must be claimed; alternatives share a group
    u8 group;  // 0..7
};

struct RFQuestBonus
{
    u16 quest;
    u16 item;        // ITEM_NONE for no extra item
    u16 itemCount;
    u16 flag;        // set on claim; 0 for none (unlocks world changes and dialogue)
    u16 achievement; // achievement ID + 1; 0 for none
    u32 money;
};

// World changes derived from quest states: no save space, reapplied on every map load.
struct RFQuestObject
{
    const u8 *script; // object event script that identifies the NPC/object
    u16 quest;
    u8 minState, maxState; // visible only while minState <= state <= maxState
};

struct RFQuestTile
{
    u16 map; // MAP_* value
    u16 quest;
    u8 minState; // applied once the quest reaches this state
    u8 x, y;
    u16 metatile; // may include MAPGRID_IMPASSABLE
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
u32 RFQuests_GetProgress(u32 id);
bool32 RFQuests_CounterSlotsFull(void);
bool32 RFQuests_GetChoice(u32 choice);
void RFQuests_SetChoice(u32 choice);
u32 RFQuests_NotifyEvent(u32 event, u32 argument);
bool32 RFQuests_DeliverItem(u32 id);
u32 RFQuests_NotifyItemCollected(u32 item);
u32 RFQuests_NotifyItemUsed(u32 item);
void RFQuests_OnBattleEnd(void);
bool32 RFQuests_IsObjectHidden(const u8 *script);
void RFQuests_ApplyWorldChanges(void);
void RFQuests_OnMapLoad(void);
void Script_RF_QuestNotify(void);
void Script_RF_QuestDeliver(void);
void Script_RF_QuestGetState(void);
void Script_RF_QuestGetChoice(void);
void Script_RF_QuestSetChoice(void);
void Script_RF_QuestTrackedHint(void);

#endif
