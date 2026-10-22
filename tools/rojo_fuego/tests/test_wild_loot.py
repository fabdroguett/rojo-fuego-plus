"""Run the production loot module with deterministic battle and inventory services."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]


class WildLootTests(unittest.TestCase):
    def test_ownership_filters_reserve_and_migration(self):
        source = (ROOT / "src/rf_wild_loot.c").read_text(encoding="utf-8")
        flags = sorted(set(re.findall(r"\bBATTLE_TYPE_[A-Z_]+", source)))
        header = r'''
#include <stdint.h>
#include <string.h>
#include <assert.h>
typedef uint32_t u32;
typedef uint16_t u16;
typedef int bool32;
#define TRUE 1
#define FALSE 0
#define IS_FRLG 1
#define PARTY_SIZE 6
#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))
#define B_OUTCOME_WON 1
#define B_TRAINER_OPPONENT_A 1
#define SPECIES_NONE 0
#define KEY_WILD_LOOT 1
#define POCKET_KEY_ITEMS 1
enum Item {ITEM_NONE, ITEM_TEST, ITEM_OTHER, ITEM_KEY = 7, ITEMS_COUNT = 10};
enum {MON_DATA_HELD_ITEM, MON_DATA_SPECIES, MON_DATA_HP};
struct Pokemon {u16 item, species, hp;};
struct LostItem {enum Item originalItem; unsigned stolen, wildItemPending, restoreAfterBattle;};
struct PartyState {unsigned sentOut, usedHeldItem;};
struct Battle {struct PartyState partyState[2][6];};
struct Save {unsigned char previous[32]; u32 rfLootSaveMagic; u16 rfPendingLoot[8];};
static struct Battle battle, *gBattleStruct = &battle;
static struct Save save, *gSaveBlock3Ptr = &save;
static struct Pokemon gParties[2][6];
static struct LostItem lost[2][6];
static u32 gBattleOutcome, gBattleTypeFlags, bag[10], roll, randomCalls;
static bool32 enabled, room;
static bool32 KeySystem_IsOn(unsigned key) {(void)key; return enabled;}
static unsigned GetItemImportance(enum Item item) {return item == ITEM_KEY;}
static unsigned GetItemPocket(enum Item item) {return item == ITEM_KEY ? POCKET_KEY_ITEMS : 0;}
static unsigned GetMonData(struct Pokemon *mon, unsigned field) {
    return field == MON_DATA_HELD_ITEM ? mon->item : field == MON_DATA_SPECIES ? mon->species : mon->hp;
}
static void SetMonData(struct Pokemon *mon, unsigned field, const enum Item *item) {
    assert(field == MON_DATA_HELD_ITEM); mon->item = *item;
}
static struct LostItem *GetLostItemState(unsigned trainer, unsigned slot) {return &lost[trainer][slot];}
static bool32 AddBagItem(enum Item item, unsigned count) {if (!room) return FALSE; bag[item] += count; return TRUE;}
static unsigned Random(void) {randomCalls++; return roll;}
'''
        header += "\n".join(f"#define {flag} (1u << {i})" for i, flag in enumerate(flags)) + "\n"
        header += (ROOT / "include/rf_wild_loot.h").read_text(encoding="utf-8")
        source = re.sub(r'^#include .*$', '', source, flags=re.MULTILINE)
        harness = r'''
static void Enemy(void) {
    memset(&battle, 0, sizeof(battle)); memset(lost, 0, sizeof(lost));
    memset(gParties, 0, sizeof(gParties));
    gParties[1][0] = (struct Pokemon){ITEM_TEST, 25, 0};
    lost[1][0].originalItem = ITEM_TEST; lost[1][0].restoreAfterBattle = TRUE;
    battle.partyState[1][0].sentOut = TRUE;
}
int main(void) {
    memset(&save, 0xA5, sizeof(save));
    enabled = room = TRUE; gBattleOutcome = B_OUTCOME_WON;
    Enemy();
    assert(RFWildLoot_Collect() == RF_LOOT_BAG && bag[ITEM_TEST] == 1);
    assert(gParties[1][0].item == ITEM_NONE && lost[1][0].originalItem == ITEM_NONE);
    assert(!lost[1][0].restoreAfterBattle);
    assert(RFWildLoot_Collect() == RF_LOOT_NONE && bag[ITEM_TEST] == 1);
    for (unsigned i = 0; i < sizeof(save.previous); i++) assert(save.previous[i] == 0xA5);
    Enemy(); room = FALSE;
    assert(RFWildLoot_Collect() == RF_LOOT_PENDING);
    assert(save.rfPendingLoot[0] == ITEM_TEST && gParties[1][0].item == ITEM_NONE);
    assert(!RFWildLoot_TryClaim() && save.rfPendingLoot[0] == ITEM_TEST);
    room = TRUE; enabled = FALSE;
    assert(RFWildLoot_TryClaim() == 1 && bag[ITEM_TEST] == 2);
    assert(!RFWildLoot_TryClaim());
    Enemy(); unsigned calls = randomCalls;
    assert(RFWildLoot_Collect() == RF_LOOT_NONE && randomCalls == calls);
    enabled = TRUE;
    for (unsigned flag = 1; flag <= (1u << FLAG_LAST); flag <<= 1) {
        gBattleTypeFlags = flag;
        assert(RFWildLoot_Collect() == RF_LOOT_NONE && gParties[1][0].item == ITEM_TEST);
    }
    gBattleTypeFlags = 0;
    for (unsigned outcome = 0; outcome < 8; outcome++) {
        if (outcome == B_OUTCOME_WON) continue;
        gBattleOutcome = outcome;
        assert(RFWildLoot_Collect() == RF_LOOT_NONE);
    }
    gBattleOutcome = B_OUTCOME_WON;
    for (unsigned filter = 0; filter < 7; filter++) {
        Enemy();
        if (filter == 0) lost[1][0].stolen = TRUE;
        if (filter == 1) lost[1][0].wildItemPending = TRUE;
        if (filter == 2) lost[1][0].originalItem = ITEM_OTHER;
        if (filter == 3) battle.partyState[1][0].usedHeldItem = ITEM_TEST;
        if (filter == 4) battle.partyState[1][0].sentOut = FALSE;
        if (filter == 5) gParties[1][0].hp = 1;
        if (filter == 6) gParties[1][0].item = lost[1][0].originalItem = ITEM_KEY;
        assert(RFWildLoot_Collect() == RF_LOOT_NONE);
    }
    Enemy(); roll = 1;
    assert(RFWildLoot_Collect() == RF_LOOT_NONE && gParties[1][0].item == ITEM_TEST);
    roll = 0; room = FALSE;
    for (unsigned i = 0; i < 8; i++) save.rfPendingLoot[i] = ITEM_OTHER;
    calls = randomCalls;
    assert(RFWildLoot_Collect() == RF_LOOT_NONE && randomCalls == calls);
    assert(gParties[1][0].item == ITEM_TEST);
    room = TRUE; save.rfPendingLoot[0] = 65535;
    assert(RFWildLoot_TryClaim() == 7 && bag[ITEM_OTHER] == 7);
    Enemy();
    gParties[1][1] = gParties[1][0];
    lost[1][1] = lost[1][0];
    battle.partyState[1][1] = battle.partyState[1][0];
    unsigned before = bag[ITEM_TEST];
    assert(RFWildLoot_Collect() == RF_LOOT_BAG && bag[ITEM_TEST] == before + 2);
    assert(RFWildLoot_Collect() == RF_LOOT_NONE && bag[ITEM_TEST] == before + 2);
    return 0;
}
'''.replace("FLAG_LAST", str(len(flags) - 1))
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            test = work / "loot.c"
            test.write_text(header + source + harness, encoding="utf-8")
            binary = work / "loot"
            result = subprocess.run([compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
                                     "-fsanitize=address,undefined", str(test), "-o", str(binary)],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
