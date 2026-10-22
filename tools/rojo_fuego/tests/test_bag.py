"""Exercise production bag access/add/remove code, including overflow and re-encryption."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]


class BagTests(unittest.TestCase):
    def test_overflow_encryption_atomicity_and_multistack_removal(self):
        source = (ROOT / "src/item.c").read_text(encoding="utf-8")
        accessors = source[source.index("static inline struct ItemSlot NONNULL BagPocket_GetSlotDataGeneric"):
                           source.index("void SetBagItemsPointers")]
        add = source[source.index("static inline bool32 NONNULL CheckSlotAndUpdateCount"):
                     source.index("bool32 AddBagItem(")]
        remove = source[source.index("static bool32 NONNULL BagPocket_RemoveItem"):
                        source.index("bool32 RemoveBagItem(")]
        pc_remove = source[source.index("void RemovePCItem("):source.index("void CompactPCItems(")]
        pc_sort = source[source.index("void SortPCItems("):source.index("void SwapRegisteredBike(")]
        slot_remove = source[source.index("void RemoveBagItemFromSlot("):source.index("static u8 NONNULL BagPocket_CountUsedItemSlots(")]
        header = r'''
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int bool32;
#define NONNULL
#define IS_FRLG 1
#define TRUE 1
#define FALSE 0
#define BAG_ITEMS_COUNT 30
#define RF_BAG_EXTRA_ITEMS_COUNT 30
#define MAX_BAG_ITEM_CAPACITY 999
#define min(a,b) ((a) < (b) ? (a) : (b))
#define Free free
#define VAR_SECRET_BASE_LOW_TV_FLAGS 1
#define VAR_SECRET_BASE_LAST_ITEM_USED 2
#define SECRET_BASE_USED_BAG 1
enum Pocket {POCKET_ITEMS, POCKET_KEY_ITEMS, POCKET_POKE_BALLS, POCKET_TM_HM, POCKET_BERRIES, POCKET_DUMMY};
#define POCKETS_COUNT 5
enum Item {ITEM_NONE, ITEM_TEST = 1};
struct ItemSlot {u16 itemId; u16 quantity;};
struct BagPocket {struct ItemSlot *itemSlots; u16 capacity; enum Pocket id;};
struct SaveBlock1 {struct {struct ItemSlot items[30]; struct ItemSlot keys[30];} bag;};
struct SaveBlock2 {u32 encryptionKey;};
struct SaveBlock3 {u32 rfBagSaveMagic; struct ItemSlot rfBagExtraItems[30];};
static struct SaveBlock1 save1;
static struct SaveBlock2 save2;
static struct SaveBlock3 save3;
static struct SaveBlock1 *gSaveBlock1Ptr = &save1;
static struct SaveBlock2 *gSaveBlock2Ptr = &save2;
static struct SaveBlock3 *gSaveBlock3Ptr = &save3;
static struct BagPocket gBagPockets[POCKETS_COUNT];
static struct ItemSlot pcSlots[50];
#define DUMMY_PC_BAG_POCKET ((struct BagPocket){pcSlots, 50, POCKET_DUMMY})
static int allocationFails;
static void *AllocZeroed(size_t size) {return allocationFails ? NULL : calloc(1, size);}
static bool32 CurMapIsSecretBase(void) {return FALSE;}
static u16 VarGet(u32 var) {(void)var; return 0;}
static void VarSet(u32 var, u16 value) {(void)var; (void)value;}
static void ApplyNewEncryptionKeyToHword(u16 *value, u32 key) {*value ^= (u16)save2.encryptionKey ^ (u16)key;}
'''
        helper = r'''
static void BagPocket_SetSlotItemIdAndCount(struct BagPocket *p, u32 slot, enum Item item, u16 count)
{BagPocket_SetSlotData(p, slot, (struct ItemSlot){item, count});}
static void BagPocket_CompactItems(struct BagPocket *p)
{
    u32 next = 0;
    for (u32 i = 0; i < p->capacity; i++) {
        struct ItemSlot value = BagPocket_GetSlotData(p, i);
        if (value.itemId != ITEM_NONE) BagPocket_SetSlotData(p, next++, value);
    }
    while (next < p->capacity) BagPocket_SetSlotData(p, next++, (struct ItemSlot){0});
}
'''
        harness = r'''
static u32 Total(struct BagPocket *p, u16 item)
{
    u32 total = 0;
    for (u32 i = 0; i < p->capacity; i++) {
        struct ItemSlot slot = BagPocket_GetSlotData(p, i);
        if (slot.itemId == item) total += slot.quantity;
    }
    return total;
}
int main(void)
{
    struct BagPocket *p = &gBagPockets[POCKET_ITEMS];
    *p = (struct BagPocket){save1.bag.items, 60, POCKET_ITEMS};
    save2.encryptionKey = 0x1234;
    for (u32 i = 0; i < 30; i++) save1.bag.items[i].quantity = 0x1234;
    memset(save1.bag.keys, 0x5A, sizeof(save1.bag.keys));
    struct ItemSlot keySnapshot[30];
    memcpy(keySnapshot, save1.bag.keys, sizeof(keySnapshot));
    memset(&save3, 0xA5, sizeof(save3));
    assert(BagPocket_GetSlotData(p, 30).itemId == ITEM_NONE);
    assert(save3.rfBagSaveMagic == 0x42414731);
    BagPocket_SetSlotData(p, 29, (struct ItemSlot){1, 500});
    BagPocket_SetSlotData(p, 30, (struct ItemSlot){1, 500});
    BagPocket_SetSlotData(p, 59, (struct ItemSlot){2, 999});
    assert(Total(p, 1) == 1000);
    ApplyNewEncryptionKeyToBagItems(0xABCD);
    save2.encryptionKey = 0xABCD;
    assert(Total(p, 1) == 1000);
    assert(Total(p, 2) == 999);
    assert(memcmp(keySnapshot, save1.bag.keys, sizeof(keySnapshot)) == 0);
    assert(BagPocket_RemoveItem(p, 1, 750));
    assert(Total(p, 1) == 250);
    assert(Total(p, 2) == 999);
    assert(!BagPocket_RemoveItem(p, 1, 251));
    assert(Total(p, 1) == 250);
    struct SaveBlock3 snapshot = save3;
    BagPocket_SetSlotData(p, 60, (struct ItemSlot){2, 1});
    assert(BagPocket_GetSlotData(p, 60).itemId == ITEM_NONE);
    assert(memcmp(&snapshot, &save3, sizeof(save3)) == 0);
    for (u32 i = 0; i < 60; i++) BagPocket_SetSlotData(p, i, (struct ItemSlot){0});
    for (u32 i = 1; i <= 60; i++) assert(BagPocket_AddItem(p, i, 1));
    assert(!BagPocket_AddItem(p, 61, 1));
    for (u32 i = 1; i <= 60; i++) assert(Total(p, i) == 1);
    allocationFails = TRUE;
    assert(!BagPocket_AddItem(p, 1, 1));
    assert(!BagPocket_RemoveItem(p, 1, 1));
    assert(Total(p, 1) == 1);
    assert(BagPocket_AddItem(p, 1, 0));
    assert(memcmp(keySnapshot, save1.bag.keys, sizeof(keySnapshot)) == 0);
    pcSlots[0] = (struct ItemSlot){1, 2};
    pcSlots[1] = (struct ItemSlot){2, 7};
    RemovePCItem(0, 3);
    assert(pcSlots[0].quantity == 2);
    RemovePCItem(50, 1);
    assert(pcSlots[1].quantity == 7);
    RemovePCItem(0, 2);
    assert(pcSlots[0].itemId == 2 && pcSlots[0].quantity == 7);
    assert(pcSlots[1].itemId == ITEM_NONE);
    pcSlots[1] = (struct ItemSlot){1, 999};
    pcSlots[3] = (struct ItemSlot){1, 3};
    SortPCItems();
    assert(pcSlots[0].itemId == 1 && pcSlots[0].quantity == 999);
    assert(pcSlots[1].itemId == 1 && pcSlots[1].quantity == 3);
    assert(pcSlots[2].itemId == 2 && pcSlots[2].quantity == 7);
    for (u32 i = 3; i < 50; i++) assert(pcSlots[i].itemId == ITEM_NONE);
    struct ItemSlot sortedPC[50];
    memcpy(sortedPC, pcSlots, sizeof(sortedPC));
    SortPCItems();
    assert(memcmp(sortedPC, pcSlots, sizeof(sortedPC)) == 0);
    struct ItemSlot first = BagPocket_GetSlotData(p, 0);
    RemoveBagItemFromSlot(p, 0, first.quantity + 1);
    assert(BagPocket_GetSlotData(p, 0).quantity == first.quantity);
    RemoveBagItemFromSlot(p, 60, 1);
    return 0;
}
'''
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            test = work / "bag.c"
            test.write_text(header + accessors + helper + add + remove + pc_remove + pc_sort + slot_remove + harness, encoding="utf-8")
            binary = work / "test-bag"
            command = [compiler, "-std=c11", "-Wall", "-Wextra", "-Werror", "-fsanitize=address,undefined",
                       str(test), "-o", str(binary)]
            compiled = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
