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
typedef uint32_t u32;
#define B_OUTCOME_WON 1
#define RF_QUEST_EVENT_TRAINER_WIN 3
#define RF_QUEST_COUNT 43
#define RF_QUEST_ACTIVE 1
#define QUEST_DELIVER_ITEM 6
struct RFQuest {u32 condition;};
static struct RFQuest quests[RF_QUEST_COUNT];
static u32 gBattleOutcome, gBattleTypeFlags;
static struct {u32 opponentA;} trainerParams;
#define TRAINER_BATTLE_PARAM trainerParams
static u32 gSpecialVar_Result, gSpecialVar_0x8004, gSpecialVar_0x8005;
static u32 eventCalls, lastEvent, lastArgument, state, unlocked, delivered, deliveryCalls;
static u32 RFQuests_NotifyEvent(u32 event, u32 argument) {
    eventCalls++; lastEvent = event; lastArgument = argument; return 7;
}
static const struct RFQuest *RFQuests_GetCatalog(void) {return quests;}
static u32 RFQuests_GetState(u32 id) {(void)id; return state;}
static u32 RFQuests_IsUnlocked(u32 id) {(void)id; return unlocked;}
static u32 RFQuests_DeliverItem(u32 id) {assert(id == 36); deliveryCalls++; return delivered;}
'''
        header += "\n".join(f"#define {flag} (1u << {i})" for i, flag in enumerate(flags)) + "\n"
        harness = r'''
int main(void) {
    TRAINER_BATTLE_PARAM.opponentA = 123;
    RFQuests_OnBattleEnd(); assert(eventCalls == 0);
    gBattleOutcome = B_OUTCOME_WON;
    RFQuests_OnBattleEnd(); assert(eventCalls == 0); // wild wins have no trainer ID
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    gBattleOutcome = 2;
    RFQuests_OnBattleEnd(); assert(eventCalls == 0);
    gBattleOutcome = B_OUTCOME_WON;
    RFQuests_OnBattleEnd();
    assert(eventCalls == 1 && lastEvent == RF_QUEST_EVENT_TRAINER_WIN && lastArgument == 123);
    for (u32 flag = 1; flag <= (1u << LAST_FLAG); flag <<= 1) {
        if (flag == BATTLE_TYPE_TRAINER) continue;
        gBattleTypeFlags = BATTLE_TYPE_TRAINER | flag;
        RFQuests_OnBattleEnd(); assert(eventCalls == 1);
    }
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
