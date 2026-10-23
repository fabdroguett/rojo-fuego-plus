"""Run the production restoration path, including optional trainer berries."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]


class HeldItemTests(unittest.TestCase):
    def test_berries_theft_replacements_and_wild_items(self):
        source = (ROOT / "src/battle_util.c").read_text(encoding="utf-8")
        helper = re.search(r"static bool32 ShouldRestoreTrainerBerry\([^\n]*\)\n\{.*?\n\}", source, re.S).group()
        restore = re.search(r"void TryRestoreHeldItems\(void\)\n\{.*?\n\}", source, re.S).group()
        header = r'''
#include <stdint.h>
#include <string.h>
#include <assert.h>
typedef uint32_t u32; typedef int bool32;
#define PARTY_SIZE 6
#define TRUE 1
#define FALSE 0
#define MON_DATA_HELD_ITEM 1
#define POCKET_BERRIES 3
#define KEY_RESTORE_TRAINER_BERRIES 23
#define BATTLE_TYPE_TRAINER 1
#define BATTLE_TYPE_LINK 2
#define BATTLE_TYPE_RECORDED 4
enum BattleTrainer {B_TRAINER_PLAYER, B_TRAINER_OPPONENT_A, B_TRAINER_PARTNER, B_TRAINER_OPPONENT_B};
enum Item {ITEM_NONE, ITEM_BERRY, ITEM_OTHER_BERRY, ITEM_SASH};
struct Pokemon {enum Item item;};
struct LostItem {enum Item originalItem; unsigned stolen, restoreAfterBattle, wildItemPending;};
struct PartyState {enum Item usedHeldItem;};
struct Battle {struct PartyState partyState[4][6];};
static struct Battle battle, *gBattleStruct = &battle;
static struct Pokemon gParties[4][6];
static struct LostItem lost[4][6];
static u32 gBattleTypeFlags, gBattlersCount = 2, bag;
static bool32 enabled;
static u32 GetItemPocket(enum Item item) {return item == ITEM_BERRY || item == ITEM_OTHER_BERRY ? POCKET_BERRIES : 0;}
static bool32 KeySystem_IsOn(u32 key) {assert(key == KEY_RESTORE_TRAINER_BERRIES); return enabled;}
static struct LostItem *GetLostItemState(enum BattleTrainer trainer, u32 slot) {return &lost[trainer][slot];}
static u32 GetMonData(struct Pokemon *mon, u32 field) {assert(field == MON_DATA_HELD_ITEM); return mon->item;}
static void SetMonData(struct Pokemon *mon, u32 field, const enum Item *item) {assert(field == MON_DATA_HELD_ITEM); mon->item = *item;}
static bool32 AddBagItem(enum Item item, u32 count) {assert(item != ITEM_NONE); bag += count; return TRUE;}
'''
        harness = r'''
static void Reset(void) {
    memset(gParties, 0, sizeof(gParties)); memset(lost, 0, sizeof(lost));
    memset(&battle, 0, sizeof(battle)); enabled = TRUE; gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    lost[0][0] = (struct LostItem){ITEM_BERRY, FALSE, TRUE, FALSE};
    battle.partyState[0][0].usedHeldItem = ITEM_BERRY;
}
int main(void) {
    Reset(); TryRestoreHeldItems(); assert(gParties[0][0].item == ITEM_BERRY);
    TryRestoreHeldItems(); assert(gParties[0][0].item == ITEM_BERRY && bag == 0);
    Reset(); enabled = FALSE; TryRestoreHeldItems(); assert(gParties[0][0].item == ITEM_NONE);
    Reset(); gBattleTypeFlags = 0; TryRestoreHeldItems(); assert(gParties[0][0].item == ITEM_NONE);
    Reset(); gBattleTypeFlags |= BATTLE_TYPE_LINK; TryRestoreHeldItems(); assert(gParties[0][0].item == ITEM_NONE);
    Reset(); gBattleTypeFlags |= BATTLE_TYPE_RECORDED; TryRestoreHeldItems(); assert(gParties[0][0].item == ITEM_NONE);
    Reset(); lost[0][0].stolen = TRUE; TryRestoreHeldItems(); assert(gParties[0][0].item == ITEM_NONE);
    Reset(); battle.partyState[0][0].usedHeldItem = ITEM_NONE;
    TryRestoreHeldItems(); assert(gParties[0][0].item == ITEM_NONE); // knocked off, not consumed
    Reset(); gParties[0][0].item = ITEM_OTHER_BERRY;
    TryRestoreHeldItems(); assert(gParties[0][0].item == ITEM_OTHER_BERRY);
    Reset(); lost[1][0] = lost[0][0]; battle.partyState[1][0] = battle.partyState[0][0];
    TryRestoreHeldItems(); assert(gParties[1][0].item == ITEM_NONE);
    Reset(); lost[0][5] = lost[0][0]; battle.partyState[0][5] = battle.partyState[0][0];
    TryRestoreHeldItems(); assert(gParties[0][5].item == ITEM_BERRY);
    Reset(); lost[0][0].originalItem = ITEM_SASH; enabled = FALSE;
    TryRestoreHeldItems(); assert(gParties[0][0].item == ITEM_SASH); // existing modern restoration
    Reset(); lost[0][0].wildItemPending = TRUE;
    TryRestoreHeldItems(); assert(bag == 1 && gParties[0][0].item == ITEM_NONE);
    return 0;
}
'''
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            test = work / "held.c"
            test.write_text(header + helper + restore + harness, encoding="utf-8")
            result = subprocess.run([compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
                                     "-fsanitize=address,undefined", str(test), "-o", str(work / "test")],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(work / "test")], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
