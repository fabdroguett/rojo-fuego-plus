// Extra unlock requirements and composite rewards, kept in ROM (no save space).
// Content generators append rows; the RF_QUEST_NONE rows only keep the arrays non-empty.

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
};

// Objects that appear or leave with a quest's progress. Spawned objects update on the next
// map load; a script that changes the state mid-scene removes or adds the object itself.
static const struct RFQuestObject sRFQuestObjects[] =
{
    {.script = NULL, .quest = RF_QUEST_NONE},
};

// Map tiles changed by quest progress (a cleared path, a repaired bridge).
static const struct RFQuestTile sRFQuestTiles[] =
{
    {.quest = RF_QUEST_NONE},
};
