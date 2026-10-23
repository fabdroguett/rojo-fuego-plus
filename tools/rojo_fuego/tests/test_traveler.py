"""Run Noa's real rules, progression and persistent rewards with host services."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]


class TravelerTests(unittest.TestCase):
    def test_schedule_rules_tiers_and_pending_rewards(self):
        source = (ROOT / "src/rf_traveler.c").read_text(encoding="utf-8")
        source = re.sub(r'^#include .*$', '', source, flags=re.MULTILINE)
        header = r'''
#include <stdint.h>
#include <string.h>
#include <assert.h>
typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32; typedef int bool32;
#define IS_FRLG 1
#define TRUE 1
#define FALSE 0
#define EOS 0
#define PARTY_SIZE 6
#define B_TRAINER_PLAYER 0
#define RF_QUEST_EVENT_NOA_WIN 2
#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))
#define COMPOUND_STRING(x) ((const u8 *)(x))
enum {SPECIES_NONE, SPECIES_TEST};
enum {MON_DATA_SPECIES, MON_DATA_IS_EGG, MON_DATA_HP};
enum {FLAG_SYS_GAME_CLEAR, FLAG_BADGE01_GET, FLAG_BADGE08_GET = 8,
      FLAG_RF_TRAVELER_HIDE_0 = 16, FLAG_RF_INVERSE_BATTLE = 32, FLAG_RF_DAILY_TRAVELER};
enum {VAR_DAYS_FRLG, VAR_RF_NO_BAG_USE};
enum {ITEM_GREAT_BALL, ITEM_RARE_CANDY, ITEM_PP_UP, ITEM_BOTTLE_CAP};
struct Pokemon {u16 species, egg, hp;};
struct Save {u8 previous[32]; u32 rfTravelerSaveMagic; u8 rfTravelerPendingPrize;};
static struct Save save, *gSaveBlock3Ptr = &save;
static struct Pokemon gParties[1][6];
static u8 gStringVar1[256], gStringVar2[256];
static u16 gSpecialVar_0x8000, gSpecialVar_0x8001, gSpecialVar_0x8004,
           gSpecialVar_0x8005, gSpecialVar_0x8006, gSpecialVar_0x8007, gSpecialVar_Result;
static u16 vars[2], bag[4]; static bool32 flags[64], room;
static u16 VarGet(u16 id) {return vars[id];}
static void VarSet(u16 id, u16 value) {vars[id] = value;}
static bool32 FlagGet(u16 id) {return flags[id];}
static void FlagSet(u16 id) {flags[id] = TRUE;}
static void FlagClear(u16 id) {flags[id] = FALSE;}
static u32 GetMonData(struct Pokemon *mon, u32 field) {
    return field == MON_DATA_SPECIES ? mon->species : field == MON_DATA_IS_EGG ? mon->egg : mon->hp;
}
static void StringCopy(u8 *dest, const u8 *src) {strcpy((char *)dest, (const char *)src);}
static void StringAppend(u8 *dest, const u8 *src) {strcat((char *)dest, (const char *)src);}
static bool32 AddBagItem(u16 item, u16 count) {if (!room) return FALSE; bag[item] += count; return TRUE;}
static u32 questWins;
static u32 RFQuests_NotifyEvent(u32 event, u32 argument) {
    assert(event == RF_QUEST_EVENT_NOA_WIN && argument == 0); questWins++; return 0;
}
'''
        harness = r'''
int main(void) {
    memset(&save, 0xA5, sizeof(save));
    room = TRUE;
    Script_RF_TravelerClaimPrize();
    assert(gSpecialVar_Result == 0 && save.rfTravelerPendingPrize == 0);
    for (u32 i = 0; i < sizeof(save.previous); i++) assert(save.previous[i] == 0xA5);
    for (u32 day = 0; day < 49; day++) {
        vars[VAR_DAYS_FRLG] = day;
        flags[FLAG_RF_INVERSE_BATTLE] = TRUE; vars[VAR_RF_NO_BAG_USE] = 1;
        RF_UpdateTraveler();
        for (u32 i = 0; i < 7; i++) assert(flags[FLAG_RF_TRAVELER_HIDE_0 + i] == (i != day % 7));
        assert(!flags[FLAG_RF_INVERSE_BATTLE] && !vars[VAR_RF_NO_BAG_USE]);
        u32 rules = sRules[(day + day / 7) % 7];
        memset(gParties, 0, sizeof(gParties));
        Script_RF_TravelerSetup(); assert(!gSpecialVar_0x8006);
        gParties[0][0] = (struct Pokemon){SPECIES_TEST, FALSE, 1};
        gParties[0][1] = (struct Pokemon){SPECIES_TEST, TRUE, 1};
        Script_RF_TravelerSetup();
        assert(gSpecialVar_0x8006 == !(rules & RULE_DOUBLE));
        gParties[0][1].egg = FALSE; gParties[0][1].hp = 0;
        Script_RF_TravelerSetup(); assert(gSpecialVar_0x8006 == !(rules & RULE_DOUBLE));
        gParties[0][1].hp = 1;
        Script_RF_TravelerSetup(); assert(gSpecialVar_0x8006);
        assert(gSpecialVar_0x8005 == !!(rules & RULE_DOUBLE));
        assert(gSpecialVar_0x8007 == !!(rules & RULE_INVERSE));
        assert(strcmp((char *)gStringVar2, (char *)sCityNames[(day + 1) % 7]) == 0);
        Script_RF_TravelerApplyRules();
        assert(flags[FLAG_RF_INVERSE_BATTLE] == !!(rules & RULE_INVERSE));
        assert(vars[VAR_RF_NO_BAG_USE] == !!(rules & RULE_NO_BAG));
        Script_RF_TravelerClearRules();
        assert(!flags[FLAG_RF_INVERSE_BATTLE] && !vars[VAR_RF_NO_BAG_USE]);
    }
    const u16 rewards[] = {ITEM_GREAT_BALL, ITEM_RARE_CANDY, ITEM_PP_UP, ITEM_BOTTLE_CAP};
    const u16 quantities[] = {5, 2, 2, 1};
    for (u32 tier = 0; tier < 4; tier++) {
        memset(flags, 0, sizeof(flags));
        u32 badges = tier == 0 ? 2 : tier == 1 ? 5 : 8;
        for (u32 i = 0; i < badges; i++) flags[FLAG_BADGE01_GET + i] = TRUE;
        flags[FLAG_SYS_GAME_CLEAR] = tier == 3;
        Script_RF_TravelerSetup(); assert(gSpecialVar_0x8004 == tier);
        Script_RF_TravelerRecordWin();
        assert(flags[FLAG_RF_DAILY_TRAVELER] && save.rfTravelerPendingPrize == tier + 1);
        Script_RF_TravelerRecordWin(); // Repeated callbacks cannot duplicate the prize.
        assert(questWins == tier * 2 + 1);
        room = FALSE; Script_RF_TravelerClaimPrize();
        assert(gSpecialVar_Result == 2 && save.rfTravelerPendingPrize == tier + 1);
        struct Save snapshot = save;
        RF_UpdateTraveler(); assert(memcmp(&snapshot, &save, sizeof(save)) == 0);
        flags[FLAG_RF_DAILY_TRAVELER] = FALSE; // Daily reset must not discard an older prize.
        flags[FLAG_SYS_GAME_CLEAR] = TRUE;
        Script_RF_TravelerRecordWin(); assert(save.rfTravelerPendingPrize == tier + 1);
        room = TRUE; Script_RF_TravelerClaimPrize();
        assert(gSpecialVar_Result == 1 && save.rfTravelerPendingPrize == 0);
        assert(gSpecialVar_0x8000 == rewards[tier] && gSpecialVar_0x8001 == quantities[tier]);
        assert(bag[rewards[tier]] == quantities[tier]);
        Script_RF_TravelerClaimPrize(); assert(gSpecialVar_Result == 0);
        Script_RF_TravelerRecordWin(); assert(save.rfTravelerPendingPrize == 0);
    }
    return 0;
}
'''
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            test = work / "traveler.c"
            test.write_text(header + source + harness, encoding="utf-8")
            result = subprocess.run([compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
                                     "-fsanitize=address,undefined", str(test), "-o", str(work / "test")],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(work / "test")], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
