"""Execute the actual mission engine on a host compiler with mocked game services.

Run: python3 -m unittest discover -s tools/rojo_fuego/tests -v
Requires cc (the verification container already has it). No ROM is used or produced.
"""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]


PROGRESSION_MAIN = r'''
int main(void)
{
    memset(&save, 0xA5, sizeof(save));
    room = TRUE;
    assert(!RFQuests_IsUnlocked(RF_QUEST_MUSEUM));
    assert(!RFQuests_Accept(RF_QUEST_MUSEUM));
    assert(!RFQuests_Accept(UINT32_MAX));
    dex = TRUE;
    assert(RFQuests_GetState(RF_QUEST_MUSEUM) == RF_QUEST_AVAILABLE);
    for (u32 i = 0; i < sizeof(save.previousData); i++) assert(save.previousData[i] == 0xA5);
    assert(save.rfQuestTracked == 0);
    assert(!RFQuests_Accept(RF_QUEST_MT_MOON));
    assert(RFQuests_Accept(RF_QUEST_MUSEUM));
    assert(!RFQuests_Accept(RF_QUEST_MUSEUM));
    assert(!RFQuests_Claim(RF_QUEST_MUSEUM));
    Visit(MAP_PEWTER_CITY_MUSEUM_1F);
    assert(RFQuests_GetState(RF_QUEST_MUSEUM) == RF_QUEST_READY);
    room = FALSE;
    assert(!RFQuests_Claim(RF_QUEST_MUSEUM));
    assert(RFQuests_GetState(RF_QUEST_MUSEUM) == RF_QUEST_READY);
    assert(bag[ITEM_POKE_BALL] == 0);
    room = TRUE;
    assert(RFQuests_Claim(RF_QUEST_MUSEUM));
    assert(bag[ITEM_POKE_BALL] == 5);
    assert(!RFQuests_Claim(RF_QUEST_MUSEUM));
    assert(bag[ITEM_POKE_BALL] == 5);
    assert(save.rfQuestTracked == 0);
    assert(RFQuests_IsUnlocked(RF_QUEST_MT_MOON));
    assert(RFQuests_Accept(RF_QUEST_MT_MOON));
    Visit(MAP_MT_MOON_B2F);
    assert(RFQuests_Claim(RF_QUEST_MT_MOON));
    SetQuestState(RF_QUEST_LAB, RF_QUEST_CLAIMED);
    bag[ITEM_HELIX_FOSSIL] = 1;
    assert(RFQuests_Accept(RF_QUEST_REVIVAL));
    assert(RFQuests_GetState(RF_QUEST_REVIVAL) == RF_QUEST_ACTIVE);
    gParties[0][0] = (struct Pokemon){SPECIES_OMASTAR, TRUE};
    RFQuests_Refresh();
    assert(RFQuests_GetState(RF_QUEST_REVIVAL) == RF_QUEST_ACTIVE);
    gParties[0][0].egg = FALSE;
    RFQuests_Refresh();
    assert(RFQuests_GetState(RF_QUEST_REVIVAL) == RF_QUEST_READY);
    SetQuestState(RF_QUEST_AMBER, RF_QUEST_CLAIMED);
    assert(!RFQuests_IsUnlocked(RF_QUEST_TANOBY));
    champion = TRUE;
    assert(RFQuests_IsUnlocked(RF_QUEST_TANOBY));
    SetQuestState(RF_QUEST_TANOBY, RF_QUEST_CLAIMED);
    assert(RFQuests_Accept(RF_QUEST_UNOWN));
    gParties[0][0] = (struct Pokemon){10000, FALSE};
    RFQuests_Refresh();
    assert(RFQuests_GetState(RF_QUEST_UNOWN) == RF_QUEST_READY);
    // New chapters are independently accessible and preserve old completion states.
    assert(RFQuests_IsUnlocked(RF_QUEST_ROCK_SURVEY));
    assert(!RFQuests_IsUnlocked(RF_QUEST_TOWER_SURVEY));
    assert(RFQuests_Accept(RF_QUEST_ROCK_SURVEY));
    Visit(MAP_ROCK_TUNNEL_B1F);
    assert(RFQuests_Claim(RF_QUEST_ROCK_SURVEY));
    assert(RFQuests_IsUnlocked(RF_QUEST_TOWER_SURVEY));
    assert(RFQuests_IsUnlocked(RF_QUEST_GROWTH_FISH));
    assert(RFQuests_Accept(RF_QUEST_GROWTH_FISH));
    gParties[0][0] = (struct Pokemon){SPECIES_MILOTIC, TRUE};
    RFQuests_Refresh();
    assert(RFQuests_GetState(RF_QUEST_GROWTH_FISH) == RF_QUEST_ACTIVE);
    gParties[0][0].egg = FALSE;
    RFQuests_Refresh();
    assert(RFQuests_Claim(RF_QUEST_GROWTH_FISH));
    assert(RFQuests_GetState(RF_QUEST_MUSEUM) == RF_QUEST_CLAIMED);
    // An existing save unlocks the appended expedition without resetting old IDs.
    SetQuestState(RF_QUEST_VIAPOIS, RF_QUEST_CLAIMED);
    const u16 expedition[] = {RF_QUEST_LIPTOO, RF_QUEST_WEEPTH, RF_QUEST_DILFORD,
                             RF_QUEST_SCUFIB, RF_QUEST_RIXY};
    for (u32 step = 0; step < ARRAY_COUNT(expedition); step++) {
        u32 id = expedition[step];
        const struct RFQuest *quest = &sRFQuests[id];
        assert(RFQuests_IsUnlocked(id));
        if (step + 1 < ARRAY_COUNT(expedition))
            assert(!RFQuests_IsUnlocked(expedition[step + 1]));
        Visit(quest->argument); // Visits before accepting are not recorded.
        world.location.mapGroup = 0;
        world.location.mapNum = 0;
        assert(RFQuests_Accept(id));
        assert(RFQuests_GetState(id) == RF_QUEST_ACTIVE);
        Visit(MAP_MT_MOON_B2F);
        assert(RFQuests_GetState(id) == RF_QUEST_ACTIVE);
        Visit(quest->argument);
        assert(RFQuests_GetState(id) == RF_QUEST_READY);
        u16 before = bag[quest->rewardItem];
        room = FALSE;
        assert(!RFQuests_Claim(id));
        assert(bag[quest->rewardItem] == before);
        assert(RFQuests_GetState(id) == RF_QUEST_READY);
        room = TRUE;
        assert(RFQuests_Claim(id));
        assert(bag[quest->rewardItem] == before + quest->rewardCount);
        assert(!RFQuests_Claim(id));
        assert(bag[quest->rewardItem] == before + quest->rewardCount);
        for (u32 previous = 0; previous <= step; previous++)
            assert(RFQuests_GetState(expedition[previous]) == RF_QUEST_CLAIMED);
        assert(RFQuests_GetState(RF_QUEST_GROWTH_REPORT) == RF_QUEST_AVAILABLE);
        assert(RFQuests_GetState(RF_QUEST_VIAPOIS) == RF_QUEST_CLAIMED);
    }
    // Events apply only after acceptance; all stages persist using the existing state bits.
    assert(RFQuests_NotifyEvent(RF_QUEST_EVENT_OAK_REPORT, 0) == 0);
    assert(RFQuests_Accept(RF_QUEST_FIELD_BRIEFING));
    assert(RFQuests_NotifyEvent(RF_QUEST_EVENT_NOA_WIN, 0) == 0);
    assert(RFQuests_NotifyEvent(RF_QUEST_EVENT_OAK_REPORT, 0) == 1);
    assert(RFQuests_NotifyEvent(RF_QUEST_EVENT_OAK_REPORT, 0) == 0);
    assert(RFQuests_Claim(RF_QUEST_FIELD_BRIEFING));
    assert(bag[ITEM_POTION] == 2);
    assert(!RFQuests_DeliverItem(RF_QUEST_FIELD_SUPPLIES));
    assert(!RFQuests_DeliverItem(UINT32_MAX));
    assert(RFQuests_Accept(RF_QUEST_FIELD_SUPPLIES));
    RFQuests_Refresh(); // Possession alone must never remove supplies or finish a delivery.
    assert(RFQuests_GetState(RF_QUEST_FIELD_SUPPLIES) == RF_QUEST_ACTIVE);
    bag[ITEM_POTION] = 1;
    assert(!RFQuests_DeliverItem(RF_QUEST_FIELD_SUPPLIES));
    assert(bag[ITEM_POTION] == 1);
    bag[ITEM_POTION] = 3;
    assert(RFQuests_DeliverItem(RF_QUEST_FIELD_SUPPLIES));
    assert(bag[ITEM_POTION] == 1);
    assert(!RFQuests_DeliverItem(RF_QUEST_FIELD_SUPPLIES));
    assert(bag[ITEM_POTION] == 1);
    room = FALSE;
    assert(!RFQuests_Claim(RF_QUEST_FIELD_SUPPLIES));
    RFQuests_Refresh();
    assert(RFQuests_GetState(RF_QUEST_FIELD_SUPPLIES) == RF_QUEST_READY);
    room = TRUE;
    assert(RFQuests_Claim(RF_QUEST_FIELD_SUPPLIES));
    assert(RFQuests_Accept(RF_QUEST_FIELD_CHALLENGE));
    assert(RFQuests_NotifyEvent(RF_QUEST_EVENT_TRAINER_WIN, TRAINER_RF_REMATCH_BROCK) == 0);
    assert(RFQuests_NotifyEvent(RF_QUEST_EVENT_NOA_WIN, 0) == 1);
    assert(RFQuests_Claim(RF_QUEST_FIELD_CHALLENGE));
    assert(RFQuests_Accept(RF_QUEST_FIELD_REPORT));
    assert(RFQuests_NotifyEvent(RF_QUEST_EVENT_OAK_REPORT, 0) == 1);
    assert(RFQuests_Claim(RF_QUEST_FIELD_REPORT));
    const u16 rematches[] = {RF_QUEST_REMATCH_BROCK, RF_QUEST_REMATCH_MISTY, RF_QUEST_REMATCH_SURGE};
    const u16 trainers[] = {TRAINER_RF_REMATCH_BROCK, TRAINER_RF_REMATCH_MISTY, TRAINER_RF_REMATCH_LT_SURGE};
    for (u32 step = 0; step < ARRAY_COUNT(rematches); step++) {
        assert(RFQuests_Accept(rematches[step]));
        assert(RFQuests_NotifyEvent(RF_QUEST_EVENT_TRAINER_WIN, 0xFFFF) == 0);
        assert(RFQuests_NotifyEvent(RF_QUEST_EVENT_TRAINER_WIN, trainers[step]) == 1);
        assert(RFQuests_Claim(rematches[step]));
    }
    assert(RFQuests_Accept(RF_QUEST_REMATCH_REPORT));
    assert(RFQuests_NotifyEvent(RF_QUEST_EVENT_OAK_REPORT, 0) == 1);
    assert(RFQuests_Claim(RF_QUEST_REMATCH_REPORT));
    assert(RFQuests_GetState(RF_QUEST_MUSEUM) == RF_QUEST_CLAIMED);
    for (u32 i = 0; i < RF_QUEST_COUNT; i++) {
        assert(sRFQuests[i].name != NULL && sRFQuests[i].description != NULL);
        const struct RFQuest *quest = &sRFQuests[i];
        assert(quest->chapter < RF_QUEST_CHAPTER_COUNT);
        assert(quest->rewardItem != ITEM_NONE && quest->rewardCount > 0);
        u32 ancestor = quest->prerequisite, steps = 0;
        while (ancestor != RF_QUEST_NONE) {
            assert(ancestor < RF_QUEST_COUNT && ++steps < RF_QUEST_COUNT);
            assert(ancestor != i);
            ancestor = sRFQuests[ancestor].prerequisite;
        }
    }
    // Legendary sagas: locked by their flag, ready only once the saga is caught, then claimable.
    champion = FALSE;
    assert(!RFQuests_IsUnlocked(RF_QUEST_LEGEND_00));
    champion = TRUE;
    assert(RFQuests_Accept(RF_QUEST_LEGEND_00));
    RFQuests_Refresh();
    assert(RFQuests_GetState(RF_QUEST_LEGEND_00) == RF_QUEST_ACTIVE);
    sagaCaught = TRUE;
    RFQuests_Refresh();
    assert(RFQuests_GetState(RF_QUEST_LEGEND_00) == RF_QUEST_READY);
    assert(RFQuests_Claim(RF_QUEST_LEGEND_00));
    sagaCaught = FALSE;
    struct SaveBlock3 snapshot = save;
    RFQuests_Refresh();
    assert(memcmp(&snapshot, &save, sizeof(save)) == 0);
    for (u32 i = 0; i < RF_QUEST_CAPACITY; i++) SetQuestState(i, RF_QUEST_ACTIVE);
    for (u32 i = 0; i < RF_QUEST_STATE_BYTES; i++) assert(save.rfQuestStates[i] == 0x55);
    assert(save.rfQuestSaveMagic == QUEST_SAVE_MAGIC);
    return 0;
}
'''

COUNTER_MAIN = r"""
int main(void)
{
    (void)Visit; // shared harness helper; this scenario does not move the player
    // A save from before the extension: shipped states survive, extension fields start empty.
    memset(&save, 0xA5, sizeof(save));
    save.rfQuestSaveMagic = QUEST_SAVE_MAGIC;
    dex = champion = room = TRUE;
    u8 shipped[RF_QUEST_STATE_BYTES];
    memcpy(shipped, save.rfQuestStates, sizeof(shipped));
    (void)RFQuests_GetState(0);
    assert(memcmp(shipped, save.rfQuestStates, sizeof(shipped)) == 0);
    for (u32 i = 0; i < sizeof(save.unrelated); i++) assert(save.unrelated[i] == 0xA5);
    assert(save.rfQuestExtMagic == QUEST_EXT_MAGIC && save.rfQuestChoices == 0);
    for (u32 i = 0; i < RF_QUEST_COUNTER_SLOTS; i++) assert(save.rfQuestCounters[i] == 0);
    for (u32 i = 0; i < RF_QUEST_EXT_STATE_BYTES; i++) assert(save.rfQuestStatesExt[i] == 0);
    (void)RFQuests_GetState(1);
    assert(save.rfQuestExtMagic == QUEST_EXT_MAGIC);

    // IDs 500-649 use their own bytes and never touch the shipped array.
    memset(save.rfQuestStates, 0, sizeof(save.rfQuestStates));
    for (u32 id = RF_QUEST_CAPACITY; id < RF_QUEST_MAX; id++) SetQuestState(id, RF_QUEST_CLAIMED);
    for (u32 i = 0; i < RF_QUEST_STATE_BYTES; i++) assert(save.rfQuestStates[i] == 0);
    for (u32 i = 0; i + 1 < RF_QUEST_EXT_STATE_BYTES; i++) assert(save.rfQuestStatesExt[i] == 0xFF);
    assert(save.rfQuestStatesExt[RF_QUEST_EXT_STATE_BYTES - 1] == 0x0F);
    memset(save.rfQuestStatesExt, 0, sizeof(save.rfQuestStatesExt));

    // Counted objectives: a slot per accepted quest, filters, persistence and completion.
    const u32 any = RF_QUEST_EXTRA_143, only77 = RF_QUEST_EXTRA_144;
    const u32 fill[3] = {RF_QUEST_EXTRA_145, RF_QUEST_EXTRA_149, RF_QUEST_EXTRA_150};
    const u32 extra = RF_QUEST_EXTRA_242;
    SetQuestState(RF_QUEST_REMATCH_REPORT, RF_QUEST_CLAIMED);
    assert(sRFQuests[extra].prerequisite == RF_QUEST_REMATCH_REPORT);
    assert(sRFQuests[any].condition == QUEST_COUNT && sRFQuests[any].target == 3);
    assert(!RFQuests_CounterSlotsFull());
    assert(RFQuests_Accept(any) && RFQuests_GetProgress(any) == 0);
    assert(RFQuests_NotifyEvent(RF_QUEST_EVENT_TRAINER_WIN, 5) == 0);
    assert(RFQuests_GetProgress(any) == 1);
    RFQuests_Refresh(); // polling never counts an event
    assert(RFQuests_GetProgress(any) == 1);
    assert(RFQuests_Accept(only77));
    assert(RFQuests_NotifyEvent(RF_QUEST_EVENT_TRAINER_WIN, 5) == 0);
    assert(RFQuests_NotifyEvent(RF_QUEST_EVENT_OAK_REPORT, 77) == 0);
    assert(RFQuests_GetProgress(any) == 2 && RFQuests_GetProgress(only77) == 0);
    struct SaveBlock3 persisted = save;
    memset(&save, 0, sizeof(save));
    save = persisted;
    u32 noticesBefore = notices;
    assert(RFQuests_NotifyEvent(RF_QUEST_EVENT_TRAINER_WIN, 77) == 1);
    assert(RFQuests_GetState(any) == RF_QUEST_READY && RFQuests_GetProgress(any) == 3);
    assert(notices == noticesBefore + 1 && lastNotice == any && !lastNoticeHere);
    assert(RFQuests_GetProgress(only77) == 1);
    // The finished quest released its slot: three more fill all four.
    for (u32 i = 0; i < 3; i++) assert(RFQuests_Accept(fill[i]));
    assert(RFQuests_CounterSlotsFull());
    assert(!RFQuests_Accept(extra) && RFQuests_GetState(extra) == RF_QUEST_AVAILABLE);
    assert(RFQuests_NotifyEvent(RF_QUEST_EVENT_TRAINER_WIN, 77) == 1); // only77 completes
    assert(!RFQuests_CounterSlotsFull() && RFQuests_Accept(extra));
    assert(RFQuests_Claim(any) && !RFQuests_Claim(any));
    assert(RFQuests_GetProgress(RF_QUEST_MUSEUM) == 0 && RFQuests_GetProgress(UINT32_MAX) == 0);

    // Polled conditions: Pokédex sightings, the number of badges and unlocked achievements.
    dex = TRUE; champion = FALSE; achievementFive = FALSE;
    assert(RFQuests_Accept(RF_QUEST_EXTRA_243) && RFQuests_GetState(RF_QUEST_EXTRA_243) == RF_QUEST_READY);
    assert(RFQuests_Accept(RF_QUEST_EXTRA_244) && RFQuests_GetState(RF_QUEST_EXTRA_244) == RF_QUEST_ACTIVE);
    assert(RFQuests_Accept(RF_QUEST_EXTRA_248) && RFQuests_GetState(RF_QUEST_EXTRA_248) == RF_QUEST_ACTIVE);
    champion = TRUE; // the mock answers every badge flag with this value
    achievementFive = TRUE;
    RFQuests_Refresh();
    assert(RFQuests_GetState(RF_QUEST_EXTRA_244) == RF_QUEST_READY);
    assert(RFQuests_GetState(RF_QUEST_EXTRA_248) == RF_QUEST_READY);

    // Composite rewards: every part or nothing; a full bag keeps the quest ready.
    assert(sRFQuests[RF_QUEST_EXTRA_244].rewardItem == ITEM_POKE_BALL);
    u32 balls = bag[ITEM_POKE_BALL];
    capacity = balls + sRFQuests[RF_QUEST_EXTRA_244].rewardCount + 3; // one short of reward + bonus
    assert(!RFQuests_Claim(RF_QUEST_EXTRA_244) && bag[ITEM_POKE_BALL] == balls);
    assert(RFQuests_GetState(RF_QUEST_EXTRA_244) == RF_QUEST_READY);
    capacity = 999;
    assert(RFQuests_Claim(RF_QUEST_EXTRA_244));
    assert(bag[ITEM_POKE_BALL] == balls + sRFQuests[RF_QUEST_EXTRA_244].rewardCount + 4);
    world.money = 100; bag[ITEM_POTION] = 0;
    assert(RFQuests_Claim(RF_QUEST_EXTRA_243));
    assert(bag[ITEM_POTION] == 3 && world.money == 600 && lastFlagSet == 9 && unlockedAchievement == 5);
    assert(!RFQuests_Claim(RF_QUEST_EXTRA_243) && world.money == 600);

    // Grouped requirements: (243 or 244) and 248 must be claimed.
    SetQuestState(RF_QUEST_EXTRA_243, RF_QUEST_AVAILABLE);
    SetQuestState(RF_QUEST_EXTRA_244, RF_QUEST_AVAILABLE);
    SetQuestState(RF_QUEST_EXTRA_248, RF_QUEST_READY);
    assert(!RFQuests_IsUnlocked(RF_QUEST_EXTRA_249));
    SetQuestState(RF_QUEST_EXTRA_244, RF_QUEST_CLAIMED);
    assert(!RFQuests_IsUnlocked(RF_QUEST_EXTRA_249));
    assert(RFQuests_Claim(RF_QUEST_EXTRA_248));
    assert(RFQuests_IsUnlocked(RF_QUEST_EXTRA_249));
    SetQuestState(RF_QUEST_EXTRA_244, RF_QUEST_AVAILABLE);
    SetQuestState(RF_QUEST_EXTRA_243, RF_QUEST_CLAIMED);
    assert(RFQuests_IsUnlocked(RF_QUEST_EXTRA_249));

    // World changes follow quest states and need no saved data.
    assert(RFQuests_IsObjectHidden(RFTestScript_Guide) && !RFQuests_IsObjectHidden(RFTestScript_Gone));
    assert(!RFQuests_IsObjectHidden(RFTestScript_Other) && !RFQuests_IsObjectHidden(NULL));
    world.location.mapGroup = MAP_GROUP(MAP_PEWTER_CITY_MUSEUM_1F);
    world.location.mapNum = MAP_NUM(MAP_PEWTER_CITY_MUSEUM_1F);
    RFQuests_ApplyWorldChanges(); assert(tileCalls == 0);
    SetQuestState(RF_QUEST_EXTRA_249, RF_QUEST_ACTIVE); // Accept would complete it: the mock dex has it
    assert(!RFQuests_IsObjectHidden(RFTestScript_Guide) && !RFQuests_IsObjectHidden(RFTestScript_Gone));
    RFQuests_ApplyWorldChanges(); assert(tileCalls == 0);
    SetQuestState(RF_QUEST_EXTRA_249, RF_QUEST_READY);
    RFQuests_ApplyWorldChanges();
    assert(tileCalls == 2 && tileX[0] == 4 + MAP_OFFSET && tileY[0] == 9 + MAP_OFFSET && tileId[0] == 0x281);
    assert(tileX[1] == 5 + MAP_OFFSET && tileId[1] == (0x282 | MAPGRID_IMPASSABLE));
    SetQuestState(RF_QUEST_EXTRA_249, RF_QUEST_CLAIMED);
    assert(!RFQuests_IsObjectHidden(RFTestScript_Guide) && RFQuests_IsObjectHidden(RFTestScript_Gone));
    world.location.mapNum ^= 1; // another map: no tiles
    tileCalls = 0; RFQuests_ApplyWorldChanges(); assert(tileCalls == 0);

    // Notices: only real progress announces READY; the tracked scene's map announces itself.
    noticesBefore = notices;
    SetQuestState(RF_QUEST_MUSEUM, RF_QUEST_READY);
    assert(notices == noticesBefore);
    storyTargetMap = MAP_PEWTER_CITY_MUSEUM_1F;
    world.location.mapGroup = MAP_GROUP(MAP_PEWTER_CITY_MUSEUM_1F);
    world.location.mapNum = MAP_NUM(MAP_PEWTER_CITY_MUSEUM_1F);
    SetQuestState(RF_QUEST_STORY_000, RF_QUEST_ACTIVE);
    save.rfQuestTracked = RF_QUEST_STORY_000 + 1;
    tileCalls = 0;
    RFQuests_OnMapLoad();
    assert(notices == noticesBefore + 1 && lastNotice == RF_QUEST_STORY_000 && lastNoticeHere);
    storyTargetMap = MAP_MT_MOON_B2F; // elsewhere: no reminder
    RFQuests_OnMapLoad(); assert(notices == noticesBefore + 1);
    storyTargetMap = MAP_PEWTER_CITY_MUSEUM_1F;
    SetQuestState(RF_QUEST_STORY_000, RF_QUEST_READY);
    RFQuests_OnMapLoad(); assert(notices == noticesBefore + 1);
    save.rfQuestTracked = RF_QUEST_MUSEUM + 1; // generic quests have no meaningful target map
    RFQuests_OnMapLoad(); assert(notices == noticesBefore + 1);
    save.rfQuestTracked = 0;

    // Story decisions are permanent bits; out-of-range choices are ignored.
    assert(!RFQuests_GetChoice(3));
    RFQuests_SetChoice(3);
    RFQuests_SetChoice(RF_QUEST_CHOICE_COUNT);
    assert(RFQuests_GetChoice(3) && !RFQuests_GetChoice(4) && !RFQuests_GetChoice(99));
    assert(save.rfQuestChoices == (1u << 3));
    return 0;
}
"""


class QuestEngineTests(unittest.TestCase):
    def _run_engine(self, main_body, mutate=None):
        """Compile the production engine and catalogue with mocked services, then run main_body.

        mutate maps a src/data file name to a function that rewrites a temporary copy, so engine
        features without a shipped user yet run on real catalogue entries, never on game data.
        """
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler, "Run in the verification container with a host C compiler")
        engine = ROOT / "src/rf_quests.c"
        source = (engine.read_text(encoding="utf-8") + (ROOT / "src/data/rf_quests.h").read_text(encoding="utf-8")
                  + (ROOT / "src/data/rf_quests_legends.h").read_text(encoding="utf-8")
                  + (ROOT / "src/data/rf_quests_extra.h").read_text(encoding="utf-8")
                  + (ROOT / "src/data/rf_quests_story.h").read_text(encoding="utf-8"))
        species = sorted(set(re.findall(r"\bSPECIES_[A-Z0-9_]+", source)) - {"SPECIES_NONE"})
        items = sorted(set(re.findall(r"\bITEM_[A-Z0-9_]+", source)) - {"ITEM_NONE"})
        maps = sorted(set(re.findall(r"\bMAP_[A-Z0-9_]+", source)) - {"MAP_GROUP", "MAP_NUM", "MAP_OFFSET"})
        declarations = "enum Species {SPECIES_NONE, " + ", ".join(species) + "};\n"
        declarations += "enum Item {ITEM_NONE, " + ", ".join(items) + "};\n"
        declarations += "\n".join(f"#define {name} {1000 + i}" for i, name in enumerate(maps))
        global_header = r'''
#ifndef MOCK_GLOBAL_H
#define MOCK_GLOBAL_H
#include <stdint.h>
#include <string.h>
#include <assert.h>
typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef int32_t bool32;
typedef int16_t bool16;
#define TRUE 1
#define FALSE 0
#define IS_FRLG 1
#define PARTY_SIZE 6
#define B_TRAINER_PLAYER 0
#define min(a, b) ((a) < (b) ? (a) : (b))
#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))
#define STATIC_ASSERT(x, name) _Static_assert(x, #name)
#define ALIGNED(n) __attribute__((aligned(n)))
#define COMPOUND_STRING(x) ((const u8 *)(x))
#define MAP_GROUP(x) ((x) >> 8)
#define MAP_NUM(x) ((x) & 255)
#define FLAG_GET_SEEN 0
#define FLAG_GET_CAUGHT 1
#define GUARD_POKEDEX_H
#define FLAG_SYS_POKEDEX_GET 1
#define FLAG_IS_CHAMPION 2
#define FLAG_BADGE01_GET 3
#define FLAG_BADGE02_GET 4
#define FLAG_BADGE03_GET 5
#define FLAG_BADGE04_GET 6
#define FLAG_BADGE05_GET 7
#define FLAG_BADGE06_GET 8
#define FLAG_BADGE07_GET 9
#define FLAG_BADGE08_GET 10
#define FLAG_SYS_GAME_CLEAR 11
#define FLAG_RESCUED_MR_FUJI 12
#define FLAG_RESCUED_LOSTELLE 13
#include "constants/rf_quests.h"
DECLARATIONS
enum NationalDexOrder {NATIONAL_DEX_ORDER_TEST};
u8 GetSetPokedexFlag(enum NationalDexOrder species, u8 caseID);
enum NationalDexOrder SpeciesToNationalPokedexNum(enum Species species);
enum {MON_DATA_SPECIES, MON_DATA_IS_EGG};
struct Pokemon {u16 species; u8 egg;};
struct SaveBlock3 {
    u8 previousData[64];
    u32 rfQuestSaveMagic;
    u8 rfQuestStates[RF_QUEST_STATE_BYTES];
    u16 rfQuestTracked;
    u8 unrelated[40];
    u32 rfQuestExtMagic;
    u32 rfQuestChoices;
    u16 rfQuestCounters[RF_QUEST_COUNTER_SLOTS];
    u8 rfQuestStatesExt[RF_QUEST_EXT_STATE_BYTES];
};
struct SaveBlock1 {struct {u8 mapGroup; u8 mapNum;} location; u32 money;};
extern struct SaveBlock3 *gSaveBlock3Ptr;
extern struct SaveBlock1 *gSaveBlock1Ptr;
extern struct Pokemon gParties[1][PARTY_SIZE];
bool32 FlagGet(u16 flag);
u32 GetMonData(struct Pokemon *mon, u32 field);
u32 MockBaseSpecies(u32 species);
#define GET_BASE_SPECIES_ID(x) MockBaseSpecies(x)
bool32 CheckBagHasItem(enum Item item, u16 count);
bool32 AddBagItem(enum Item item, u16 count);
bool32 RemoveBagItem(enum Item item, u16 count);
bool32 Achievement_IsUnlocked(u32 id);
void Achievement_Unlock(u32 id);
bool32 CheckBagHasSpace(enum Item item, u16 count);
void AddMoney(u32 *money, u32 amount);
void FlagSet(u16 flag);
#define MAP_OFFSET 7
#define MAPGRID_IMPASSABLE 0xC00
void MapGridSetMetatileIdAt(s32 x, s32 y, u16 metatile);
void Achievements_QueueQuestNotice(u32 quest, bool32 here);
u32 RFStory_TargetMap(u32 quest);
#endif
'''.replace("DECLARATIONS", declarations)
        harness = r'''
#include "global.h"
#include "rf_quests.h"
static struct SaveBlock3 save;
static struct SaveBlock1 world;
struct SaveBlock3 *gSaveBlock3Ptr = &save;
struct SaveBlock1 *gSaveBlock1Ptr = &world;
struct Pokemon gParties[1][PARTY_SIZE];
static bool32 dex, champion, room, sagaCaught;
bool32 RFLegends_IsSagaCaught(u32 saga) {(void)saga; return sagaCaught;}
void RFStory_OnClaim(u32 quest) {(void)quest;}
static u16 bag[256];
static bool32 achievementFive;
bool32 Achievement_IsUnlocked(u32 id) {return achievementFive && id == 5;}
static u32 unlockedAchievement = UINT32_MAX, lastFlagSet;
static u16 capacity = 999;
void Achievement_Unlock(u32 id) {unlockedAchievement = id;}
bool32 CheckBagHasSpace(enum Item item, u16 count) {return room && bag[item] + count <= capacity;}
void AddMoney(u32 *money, u32 amount) {*money += amount;}
void FlagSet(u16 flag) {lastFlagSet = flag;}
static s32 tileX[8], tileY[8];
static u16 tileId[8];
static u32 tileCalls;
void MapGridSetMetatileIdAt(s32 x, s32 y, u16 metatile) {
    assert(tileCalls < 8); tileX[tileCalls] = x; tileY[tileCalls] = y; tileId[tileCalls++] = metatile;
}
const u8 RFTestScript_Guide[] = {0}, RFTestScript_Gone[] = {0}, RFTestScript_Other[] = {0};
static u32 notices, lastNotice = UINT32_MAX, lastNoticeHere, storyTargetMap;
void Achievements_QueueQuestNotice(u32 quest, bool32 here) {notices++; lastNotice = quest; lastNoticeHere = here;}
u32 RFStory_TargetMap(u32 quest) {(void)quest; return storyTargetMap;}
bool32 FlagGet(u16 flag) {return flag == FLAG_SYS_POKEDEX_GET ? dex : champion;}
u32 GetMonData(struct Pokemon *mon, u32 field) {return field == MON_DATA_SPECIES ? mon->species : mon->egg;}
u8 GetSetPokedexFlag(enum NationalDexOrder species, u8 caseID) {(void)species; (void)caseID; return dex;}
enum NationalDexOrder SpeciesToNationalPokedexNum(enum Species species) {return (enum NationalDexOrder)species;}
u32 MockBaseSpecies(u32 species) {return species == 10000 ? SPECIES_UNOWN : species;}
bool32 CheckBagHasItem(enum Item item, u16 count) {return bag[item] >= count;}
bool32 AddBagItem(enum Item item, u16 count) {if (!room || bag[item] + count > capacity) return FALSE; bag[item] += count; return TRUE;}
bool32 RemoveBagItem(enum Item item, u16 count) {if (bag[item] < count) return FALSE; bag[item] -= count; return TRUE;}
#include "ENGINE_PATH"
static void Visit(u16 map) {world.location.mapGroup = MAP_GROUP(map); world.location.mapNum = MAP_NUM(map); RFQuests_Refresh();}
MAIN_BODY
'''.replace("MAIN_BODY", main_body)
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            engine_path = engine
            if mutate:
                (work / "data").mkdir()
                for name in ("rf_quests.h", "rf_quests_legends.h", "rf_quests_extra.h", "rf_quests_story.h",
                             "rf_quest_links.h"):
                    text = (ROOT / "src/data" / name).read_text(encoding="utf-8")
                    (work / "data" / name).write_text(mutate.get(name, str)(text), encoding="utf-8")
                engine_path = work / "rf_quests.c"
                engine_path.write_text(engine.read_text(encoding="utf-8"), encoding="utf-8")
            harness = harness.replace("ENGINE_PATH", engine_path.as_posix())
            (work / "global.h").write_text(global_header, encoding="utf-8")
            for name in ("achievements.h", "event_data.h", "fieldmap.h", "item.h", "money.h", "pokemon.h"):
                (work / name).write_text('#include "global.h"\n', encoding="utf-8")
            (work / "test.c").write_text(harness, encoding="utf-8")
            binary = work / "test-quests"
            built = subprocess.run([compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
                                    "-fsanitize=address,undefined", "-I", str(work), "-I", str(ROOT / "include"),
                                    str(work / "test.c"), "-o", str(binary)], capture_output=True, text=True)
            self.assertEqual(built.returncode, 0, built.stderr[-6000:])
            result = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_progression_rewards_migration_and_forms(self):
        self._run_engine(PROGRESSION_MAIN)

    def test_counters_choices_and_extended_states(self):
        def counted(text):
            # Six legendary registrations (prerequisite: the Champion report only) become counted
            # trainer objectives for this scenario only.
            for index, target, trainer in ((143, 3, 0), (144, 2, 77), (145, 2, 0), (149, 2, 0), (150, 2, 0), (242, 2, 0)):
                text, n = re.subn(
                    rf"(\[RF_QUEST_EXTRA_{index:03d}\] = \{{.*?)\.condition = QUEST_DEX_CAUGHT,\s*\.argument = SPECIES_\w+,",
                    rf"\1.condition = QUEST_COUNT, .argument = RF_QUEST_EVENT_TRAINER_WIN, "
                    rf".alternative = {trainer}, .target = {target},", text, count=1, flags=re.S)
                self.assertEqual(n, 1)
            # Three more become the polled conditions added with the gameplay events.
            for index, condition in ((243, "QUEST_DEX_SEEN, .argument = SPECIES_PIKACHU"),
                                     (244, "QUEST_BADGES, .argument = 8"),
                                     (248, "QUEST_ACHIEVEMENT, .argument = 5")):
                text, n = re.subn(
                    rf"(\[RF_QUEST_EXTRA_{index:03d}\] = \{{.*?)\.condition = QUEST_DEX_CAUGHT,\s*\.argument = SPECIES_\w+,",
                    rf"\1.condition = {condition},", text, count=1, flags=re.S)
                self.assertEqual(n, 1)
            return text
        def links(text):
            # 249 needs (243 or 244) and 248; 243 and 244 grant composite rewards.
            text = text.replace("    {RF_QUEST_NONE, RF_QUEST_NONE, 0},",
                                "    {RF_QUEST_EXTRA_249, RF_QUEST_EXTRA_243, 0},\n"
                                "    {RF_QUEST_EXTRA_249, RF_QUEST_EXTRA_244, 0},\n"
                                "    {RF_QUEST_EXTRA_249, RF_QUEST_EXTRA_248, 1},")
            text = text.replace("    {.quest = RF_QUEST_NONE},",
                                "    {.quest = RF_QUEST_EXTRA_243, .item = ITEM_POTION, .itemCount = 3, .flag = 9,"
                                " .achievement = 6, .money = 500},\n"
                                "    {.quest = RF_QUEST_EXTRA_244, .item = ITEM_POKE_BALL, .itemCount = 4},", 1)
            # A guide appears once 249 is active; another object leaves when 249 is claimed;
            # two tiles of the museum change once 249 is ready.
            text = text.replace("    {.script = NULL, .quest = RF_QUEST_NONE},",
                                "    {.script = RFTestScript_Guide, .quest = RF_QUEST_EXTRA_249, .minState = 1, .maxState = 3},\n"
                                "    {.script = RFTestScript_Gone, .quest = RF_QUEST_EXTRA_249, .minState = 0, .maxState = 2},")
            return text.replace("    {.quest = RF_QUEST_NONE},",
                                "    {.map = MAP_PEWTER_CITY_MUSEUM_1F, .quest = RF_QUEST_EXTRA_249, .minState = 2,"
                                " .x = 4, .y = 9, .metatile = 0x281},\n"
                                "    {.map = MAP_PEWTER_CITY_MUSEUM_1F, .quest = RF_QUEST_EXTRA_249, .minState = 2,"
                                " .x = 5, .y = 9, .metatile = 0x282 | MAPGRID_IMPASSABLE},")
        self._run_engine(COUNTER_MAIN, {"rf_quests_extra.h": counted, "rf_quest_links.h": links})

    def test_field_registrations_use_short_chains_and_game_names(self):
        import sys
        sys.path.insert(0, str(ROOT / "tools/rojo_fuego"))
        from extra_quests import species_details
        details = species_details()
        source = (ROOT / "src/data/rf_quests_extra.h").read_text(encoding="utf-8")
        entries = re.findall(r'\[(RF_QUEST_EXTRA_\d+)\] = \{\s*\.name = COMPOUND_STRING\("([^"]*)"\),.*?'
                             r'\.prerequisite = (\w+), \.condition = (\w+),\s*\.argument = (\w+)', source, re.S)
        self.assertEqual(len(entries), 279)
        prerequisite = {quest: previous for quest, _, previous, _, _ in entries}
        legendary = {quest for quest, _, _, condition, argument in entries
                     if condition == "QUEST_DEX_CAUGHT" and details[argument][1]}
        self.assertIn("Registro: Nidoran♀", {name for _, name, _, _, _ in entries})
        self.assertIn("Registro: Mr. Mime", {name for _, name, _, _, _ in entries})
        for quest, name, previous, condition, argument in entries:
            self.assertNotIn("_", name, quest)
            if condition == "QUEST_DEX_CAUGHT":
                self.assertEqual(name, "Registro: " + details[argument][0])
            # A legendary registration never blocks another one, and is never blocked.
            self.assertNotIn(previous, legendary, quest)
            if quest in legendary:
                self.assertEqual(previous, "RF_QUEST_REMATCH_REPORT", quest)
            depth, current = 0, quest
            while current.startswith("RF_QUEST_EXTRA_"):
                current, depth = prerequisite[current], depth + 1
            self.assertEqual(current, "RF_QUEST_REMATCH_REPORT")
            self.assertLessEqual(depth, 40, quest)
        self.assertEqual(len(legendary), 10)


if __name__ == "__main__":
    unittest.main()
