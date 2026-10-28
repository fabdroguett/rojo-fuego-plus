// Extra unlock requirements and composite rewards, kept in ROM (no save space).
// Content generators append rows; the RF_QUEST_NONE rows only keep the arrays non-empty.
// BEGIN STORY EXTERNS (story_campaigns.py)
extern const u8 RFStory_Actor_59[];
// END STORY EXTERNS

// A quest also needs, for every group it lists, at least one of that group's quests
// claimed: groups are ANDed together, entries inside a group are alternatives.
static const struct RFQuestRequirement sRFQuestRequirements[] =
{
    {RF_QUEST_NONE, RF_QUEST_NONE, 0},
};

// Granted together with the catalogue reward when the quest is claimed.
static const struct RFQuestBonus sRFQuestBonuses[] =
{
    {.quest = RF_QUEST_NONE},
    // BEGIN STORY BONUSES (story_campaigns.py)
    {.quest = RF_QUEST_STORY_071, .item = ITEM_LUCKY_EGG, .itemCount = 1, .money = 5000},
    {.quest = RF_QUEST_STORY_079, .money = 3000},
    // END STORY BONUSES
};

// Objects that appear or leave with a quest's progress. Spawned objects update on the next
// map load; a script that changes the state mid-scene removes or adds the object itself.
static const struct RFQuestObject sRFQuestObjects[] =
{
    {.script = NULL, .quest = RF_QUEST_NONE},
    // BEGIN STORY OBJECTS (story_campaigns.py)
    {.script = RFStory_Actor_59, .quest = RF_QUEST_STORY_071, .minState = 3, .maxState = 3},
    // END STORY OBJECTS
};

// Map tiles changed by quest progress (a cleared path, a repaired bridge).
static const struct RFQuestTile sRFQuestTiles[] =
{
    {.quest = RF_QUEST_NONE},
};
