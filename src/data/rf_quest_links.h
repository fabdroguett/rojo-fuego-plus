// Extra unlock requirements and composite rewards, kept in ROM (no save space).
// Content generators append rows; the RF_QUEST_NONE rows only keep the arrays non-empty.
// BEGIN STORY EXTERNS (story_campaigns.py)
extern const u8 RFStory_Actor_59[];
extern const u8 RFStory_Actor_66[];
// END STORY EXTERNS

// A quest also needs, for every group it lists, at least one of that group's quests
// claimed: groups are ANDed together, entries inside a group are alternatives.
static const struct RFQuestRequirement sRFQuestRequirements[] =
{
    {RF_QUEST_NONE, RF_QUEST_NONE, 0},
    // BEGIN STORY REQUIREMENTS (story_campaigns.py)
    // END STORY REQUIREMENTS
};

// Granted together with the catalogue reward when the quest is claimed.
static const struct RFQuestBonus sRFQuestBonuses[] =
{
    {.quest = RF_QUEST_NONE},
    // BEGIN STORY BONUSES (story_campaigns.py)
    {.quest = RF_QUEST_STORY_071, .item = ITEM_LUCKY_EGG, .itemCount = 1, .money = 5000},
    {.quest = RF_QUEST_STORY_079, .money = 3000},
    {.quest = RF_QUEST_STORY_087, .money = 2500},
    {.quest = RF_QUEST_STORY_095, .money = 4000},
    {.quest = RF_QUEST_STORY_103, .money = 1000},
    {.quest = RF_QUEST_STORY_111, .money = 1500},
    {.quest = RF_QUEST_STORY_119, .money = 2000},
    {.quest = RF_QUEST_STORY_127, .money = 1500},
    {.quest = RF_QUEST_STORY_135, .money = 2000},
    {.quest = RF_QUEST_STORY_143, .money = 1500},
    {.quest = RF_QUEST_STORY_151, .money = 2500},
    {.quest = RF_QUEST_STORY_159, .money = 1500},
    {.quest = RF_QUEST_STORY_167, .money = 2000},
    {.quest = RF_QUEST_STORY_175, .money = 2500},
    {.quest = RF_QUEST_STORY_183, .money = 2500},
    {.quest = RF_QUEST_STORY_191, .money = 2000},
    {.quest = RF_QUEST_STORY_199, .money = 2500},
    {.quest = RF_QUEST_STORY_207, .money = 2000},
    {.quest = RF_QUEST_STORY_215, .money = 2500},
    {.quest = RF_QUEST_STORY_223, .money = 3000},
    {.quest = RF_QUEST_STORY_231, .money = 1500},
    {.quest = RF_QUEST_STORY_239, .money = 3000},
    {.quest = RF_QUEST_STORY_247, .money = 2500},
    {.quest = RF_QUEST_STORY_255, .money = 2500},
    {.quest = RF_QUEST_STORY_263, .money = 3000},
    {.quest = RF_QUEST_STORY_271, .money = 2500},
    {.quest = RF_QUEST_STORY_279, .money = 3000},
    {.quest = RF_QUEST_STORY_287, .money = 2000},
    {.quest = RF_QUEST_STORY_295, .money = 2500},
    {.quest = RF_QUEST_STORY_303, .money = 3000},
    {.quest = RF_QUEST_STORY_311, .money = 2500},
    {.quest = RF_QUEST_STORY_319, .money = 4000},
    {.quest = RF_QUEST_STORY_327, .money = 2500},
    {.quest = RF_QUEST_STORY_335, .money = 3500},
    {.quest = RF_QUEST_STORY_343, .money = 4000},
    {.quest = RF_QUEST_STORY_351, .money = 3500},
    {.quest = RF_QUEST_STORY_358, .money = 2500},
    // END STORY BONUSES
};

// Objects that appear or leave with a quest's progress. Spawned objects update on the next
// map load; a script that changes the state mid-scene removes or adds the object itself.
static const struct RFQuestObject sRFQuestObjects[] =
{
    {.script = NULL, .quest = RF_QUEST_NONE},
    // BEGIN STORY OBJECTS (story_campaigns.py)
    {.script = RFStory_Actor_59, .quest = RF_QUEST_STORY_071, .minState = 3, .maxState = 3},
    {.script = RFStory_Actor_66, .quest = RF_QUEST_STORY_087, .minState = 3, .maxState = 3},
    // END STORY OBJECTS
};

// Quest IDs whose meaning changed (reconverted generic slots): a save written before
// `version` resets them to available, frees their counters and stops tracking them.
static const struct RFQuestMigration sRFQuestMigrations[] =
{
    {0, RF_QUEST_NONE, RF_QUEST_NONE},
    // BEGIN STORY MIGRATIONS (story_campaigns.py)
    {0x33, 334, 341},
    {0x33, 342, 349},
    {0x34, 71, 78},
    {0x34, 79, 86},
    {0x34, 87, 94},
    {0x34, 95, 102},
    {0x34, 103, 110},
    {0x34, 111, 118},
    {0x34, 119, 126},
    {0x34, 127, 134},
    {0x34, 135, 142},
    {0x34, 143, 150},
    {0x34, 151, 158},
    {0x34, 159, 166},
    {0x34, 167, 174},
    {0x34, 175, 182},
    {0x34, 183, 190},
    {0x34, 191, 198},
    {0x34, 199, 206},
    {0x34, 207, 214},
    {0x34, 215, 222},
    {0x34, 223, 230},
    {0x34, 231, 238},
    {0x34, 239, 246},
    {0x34, 247, 254},
    {0x34, 255, 262},
    {0x34, 263, 270},
    {0x34, 271, 278},
    {0x34, 279, 286},
    {0x34, 287, 294},
    {0x34, 295, 302},
    {0x34, 303, 310},
    {0x34, 311, 318},
    {0x34, 319, 326},
    {0x34, 327, 333},
    // END STORY MIGRATIONS
};

// Map tiles changed by quest progress (a cleared path, a repaired bridge).
static const struct RFQuestTile sRFQuestTiles[] =
{
    {.quest = RF_QUEST_NONE},
};
