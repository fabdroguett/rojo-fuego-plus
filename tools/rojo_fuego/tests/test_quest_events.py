"""Check real battle/script adapters exclude losses and simulated trainer IDs."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]


class QuestEventTests(unittest.TestCase):
    def test_battle_filters_and_script_delivery(self):
        source = (ROOT / "src/rf_quest_events.c").read_text(encoding="utf-8")
        source = re.sub(r'^#include .*$', '', source, flags=re.M)
        flags = sorted(set(re.findall(r"\bBATTLE_TYPE_[A-Z_]+", source)))
        header = r'''
#include <stdint.h>
#include <assert.h>
#include <string.h>
typedef uint32_t u32;
#define B_OUTCOME_WON 1
#define B_OUTCOME_CAUGHT 7
#define RF_QUEST_EVENT_TRAINER_WIN 3
#define RF_QUEST_EVENT_CATCH 5
#define RF_QUEST_EVENT_CATCH_MAP 6
#define RF_QUEST_EVENT_CATCH_TYPE 7
#define RF_QUEST_EVENT_WILD_WIN 10
#define GET_BASE_SPECIES_ID(x) ((x) == 900 ? 25u : (x)) // species 900 is a form of 25
static struct {u32 caughtMonSpecies, lastOpponentSpecies;} gBattleResults;
struct World {struct {u32 mapGroup, mapNum;} location;};
static struct World world;
static struct World *gSaveBlock1Ptr = &world;
// Species 25 has a single type (13); every other test species is dual-typed (12/4).
static u32 GetSpeciesType(u32 species, u32 slot) {return species == 25 ? 13 : slot ? 4 : 12;}
#define RF_QUEST_COUNT 43
#define RF_QUEST_ACTIVE 1
#define QUEST_DELIVER_ITEM 6
typedef uint8_t u8;
struct RFQuest {u32 condition; const u8 *name;};
#define RF_QUEST_CLAIMED 3
struct Save3 {u32 rfQuestTracked;};
static struct Save3 save3;
static struct Save3 *gSaveBlock3Ptr = &save3;
static u8 gStringVar1[64], gStringVar4[64];
static u8 *StringCopy(u8 *dest, const u8 *src) {strcpy((char *)dest, (const char *)src); return dest;}
static const u8 *RFStory_Hint(u32 quest) {return quest == 36 ? (const u8 *)"Pista" : (const u8 *)"?";}
static struct RFQuest quests[RF_QUEST_COUNT];
static u32 gBattleOutcome, gBattleTypeFlags;
static struct {u32 opponentA;} trainerParams;
#define TRAINER_BATTLE_PARAM trainerParams
static u32 gSpecialVar_Result, gSpecialVar_0x8004, gSpecialVar_0x8005;
static u32 eventCalls, lastEvent, lastArgument, state, unlocked, delivered, deliveryCalls;
static u32 eventLog[64][2];
static u32 RFQuests_NotifyEvent(u32 event, u32 argument) {
    assert(eventCalls < 64);
    eventLog[eventCalls][0] = event; eventLog[eventCalls][1] = argument;
    eventCalls++; lastEvent = event; lastArgument = argument; return 7;
}
static const struct RFQuest *RFQuests_GetCatalog(void) {return quests;}
static u32 RFStory_NotifyTrainerWin(u32 trainer) {(void)trainer; return 0;}
static u32 RFQuests_GetState(u32 id) {(void)id; return state;}
static u32 RFQuests_IsUnlocked(u32 id) {(void)id; return unlocked;}
static u32 RFQuests_DeliverItem(u32 id) {assert(id == 36); deliveryCalls++; return delivered;}
static u32 choices;
static u32 RFQuests_GetChoice(u32 choice) {return choice < 32 && ((choices >> choice) & 1);}
static void RFQuests_SetChoice(u32 choice) {if (choice < 32) choices |= 1u << choice;}
'''
        header += "\n".join(f"#define {flag} (1u << {i})" for i, flag in enumerate(flags)) + "\n"
        harness = r'''
int main(void) {
    TRAINER_BATTLE_PARAM.opponentA = 123;
    RFQuests_OnBattleEnd(); assert(eventCalls == 0);
    gBattleOutcome = B_OUTCOME_WON;
    gBattleResults.lastOpponentSpecies = 900;
    RFQuests_OnBattleEnd(); // a wild win reports the base species, never a trainer win
    assert(eventCalls == 1 && lastEvent == RF_QUEST_EVENT_WILD_WIN && lastArgument == 25);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    gBattleOutcome = 2;
    RFQuests_OnBattleEnd(); assert(eventCalls == 1);
    gBattleOutcome = B_OUTCOME_WON;
    RFQuests_OnBattleEnd();
    assert(eventCalls == 2 && lastEvent == RF_QUEST_EVENT_TRAINER_WIN && lastArgument == 123);
    for (u32 flag = 1; flag <= (1u << LAST_FLAG); flag <<= 1) {
        if (flag == BATTLE_TYPE_TRAINER) continue;
        gBattleTypeFlags = BATTLE_TYPE_TRAINER | flag;
        RFQuests_OnBattleEnd(); assert(eventCalls == 2);
    }
    // A wild catch reports species, map and each distinct type exactly once.
    gBattleTypeFlags = 0; gBattleOutcome = B_OUTCOME_CAUGHT;
    world.location.mapGroup = 3; world.location.mapNum = 7;
    gBattleResults.caughtMonSpecies = 900;
    u32 before = eventCalls;
    RFQuests_OnBattleEnd();
    assert(eventCalls == before + 3);
    assert(eventLog[before][0] == RF_QUEST_EVENT_CATCH && eventLog[before][1] == 25);
    assert(eventLog[before + 1][0] == RF_QUEST_EVENT_CATCH_MAP && eventLog[before + 1][1] == ((3u << 8) | 7));
    assert(eventLog[before + 2][0] == RF_QUEST_EVENT_CATCH_TYPE && eventLog[before + 2][1] == 13);
    gBattleResults.caughtMonSpecies = 1;
    before = eventCalls;
    RFQuests_OnBattleEnd();
    assert(eventCalls == before + 4 && eventLog[before + 2][1] == 12 && eventLog[before + 3][1] == 4);
    // Catches and wins inside link, recorded or facility battles never count; nor do losses.
    for (u32 flag = 1; flag <= (1u << LAST_FLAG); flag <<= 1) {
        gBattleTypeFlags = flag;
        before = eventCalls; RFQuests_OnBattleEnd(); assert(eventCalls == before);
    }
    gBattleTypeFlags = 0; gBattleOutcome = 2;
    before = eventCalls; RFQuests_OnBattleEnd(); assert(eventCalls == before);
    gSpecialVar_0x8004 = 99; gSpecialVar_0x8005 = 88;
    Script_RF_QuestNotify();
    assert(gSpecialVar_Result == 7 && lastEvent == 99 && lastArgument == 88);
    Script_RF_QuestGetState(); assert(gSpecialVar_Result == 0);
    gSpecialVar_0x8004 = UINT32_MAX;
    Script_RF_QuestDeliver(); assert(!gSpecialVar_Result && !deliveryCalls);
    gSpecialVar_0x8004 = 36;
    Script_RF_QuestDeliver(); assert(!gSpecialVar_Result && !deliveryCalls);
    quests[36].condition = QUEST_DELIVER_ITEM;
    state = RF_QUEST_ACTIVE;
    Script_RF_QuestDeliver(); assert(!gSpecialVar_Result && !deliveryCalls);
    unlocked = 1;
    Script_RF_QuestDeliver(); assert(gSpecialVar_Result == 2 && deliveryCalls == 1);
    delivered = 1;
    Script_RF_QuestDeliver(); assert(gSpecialVar_Result == 1 && deliveryCalls == 2);
    gSpecialVar_0x8004 = 7; Script_RF_QuestGetChoice(); assert(gSpecialVar_Result == 0);
    Script_RF_QuestSetChoice(); Script_RF_QuestGetChoice(); assert(gSpecialVar_Result == 1 && choices == (1u << 7));
    gSpecialVar_0x8004 = 8; Script_RF_QuestGetChoice(); assert(gSpecialVar_Result == 0);
    // Guide hint: nothing tracked, out of range, claimed, then a tracked unfinished quest.
    Script_RF_QuestTrackedHint(); assert(gSpecialVar_Result == 0);
    save3.rfQuestTracked = RF_QUEST_COUNT + 1; Script_RF_QuestTrackedHint(); assert(gSpecialVar_Result == 0);
    quests[36].name = (const u8 *)"Encargo";
    save3.rfQuestTracked = 37; state = RF_QUEST_CLAIMED;
    Script_RF_QuestTrackedHint(); assert(gSpecialVar_Result == 0);
    state = RF_QUEST_ACTIVE;
    Script_RF_QuestTrackedHint();
    assert(gSpecialVar_Result == 1 && !strcmp((char *)gStringVar1, "Encargo") && !strcmp((char *)gStringVar4, "Pista"));
    return 0;
}
'''.replace("LAST_FLAG", str(len(flags) - 1))
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            test = work / "events.c"
            test.write_text(header + source + harness, encoding="utf-8")
            result = subprocess.run([compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
                                     "-fsanitize=address,undefined", str(test), "-o", str(work / "test")],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(work / "test")], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
