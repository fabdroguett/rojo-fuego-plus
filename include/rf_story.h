#ifndef GUARD_RF_STORY_H
#define GUARD_RF_STORY_H

#include "constants/rf_story.h"
enum RFQuestFilter
{
    RF_QUEST_FILTER_DISCOVERED, RF_QUEST_FILTER_AVAILABLE, RF_QUEST_FILTER_ACTIVE,
    RF_QUEST_FILTER_READY, RF_QUEST_FILTER_CLAIMED, RF_QUEST_FILTER_HERE,
    RF_QUEST_FILTER_EASY, RF_QUEST_FILTER_MEDIUM, RF_QUEST_FILTER_HARD,
    RF_QUEST_FILTER_COUNT,
};

bool32 RFStory_IsObjectHidden(const u8 *script);
u32 RFStory_TargetMap(u32 quest);
u32 RFStory_Difficulty(u32 quest);
bool32 RFStory_MatchesFilter(u32 quest, u32 filter);
bool32 RFStory_ReadEnding(u32 quest, u32 page, u8 *dest, u32 capacity);
const u8 *RFStory_Hint(u32 quest);
const u8 *RFStory_Resolution(u32 quest);
bool32 RFStory_RecordClue(u32 actor);
u32 RFStory_ClueCount(u32 quest);
u32 RFStory_DeliverContract(u32 contract, u32 day);
void RFStory_EnsureSave(void);
void RFStory_OnClaim(u32 quest);
u32 RFStory_NotifyTrainerWin(u32 trainer);
void Script_RFStoryPrepare(void);
void Script_RFStoryAccept(void);
void Script_RFStoryConfirm(void);
void Script_RFStoryPuzzle(void);
void Script_RFStoryBattleWon(void);
void Script_RFStoryAfterText(void);

#endif
