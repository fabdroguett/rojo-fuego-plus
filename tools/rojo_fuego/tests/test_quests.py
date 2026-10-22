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


class QuestEngineTests(unittest.TestCase):
    def test_progression_rewards_migration_and_forms(self):
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler, "Run in the verification container with a host C compiler")
        engine = ROOT / "src/rf_quests.c"
        source = engine.read_text(encoding="utf-8") + (ROOT / "src/data/rf_quests.h").read_text(encoding="utf-8")
        species = sorted(set(re.findall(r"\bSPECIES_[A-Z0-9_]+", source)) - {"SPECIES_NONE"})
        items = sorted(set(re.findall(r"\bITEM_[A-Z0-9_]+", source)) - {"ITEM_NONE"})
        maps = sorted(set(re.findall(r"\bMAP_[A-Z0-9_]+", source)) - {"MAP_GROUP", "MAP_NUM"})
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
typedef uint16_t u16;
typedef uint32_t u32;
typedef int32_t bool32;
#define TRUE 1
#define FALSE 0
#define IS_FRLG 1
#define PARTY_SIZE 6
#define B_TRAINER_PLAYER 0
#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))
#define STATIC_ASSERT(x, name) _Static_assert(x, #name)
#define COMPOUND_STRING(x) ((const u8 *)(x))
#define MAP_GROUP(x) ((x) >> 8)
#define MAP_NUM(x) ((x) & 255)
#define FLAG_SYS_POKEDEX_GET 1
#define FLAG_IS_CHAMPION 2
#include "constants/rf_quests.h"
DECLARATIONS
enum {MON_DATA_SPECIES, MON_DATA_IS_EGG};
struct Pokemon {u16 species; u8 egg;};
struct SaveBlock3 {
    u8 previousData[64];
    u32 rfQuestSaveMagic;
    u8 rfQuestStates[RF_QUEST_STATE_BYTES];
    u16 rfQuestTracked;
};
struct SaveBlock1 {struct {u8 mapGroup; u8 mapNum;} location;};
extern struct SaveBlock3 *gSaveBlock3Ptr;
extern struct SaveBlock1 *gSaveBlock1Ptr;
extern struct Pokemon gParties[1][PARTY_SIZE];
bool32 FlagGet(u16 flag);
u32 GetMonData(struct Pokemon *mon, u32 field);
u32 MockBaseSpecies(u32 species);
#define GET_BASE_SPECIES_ID(x) MockBaseSpecies(x)
bool32 CheckBagHasItem(enum Item item, u16 count);
bool32 AddBagItem(enum Item item, u16 count);
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
static bool32 dex, champion, room;
static u16 bag[256];
bool32 FlagGet(u16 flag) {return flag == FLAG_SYS_POKEDEX_GET ? dex : champion;}
u32 GetMonData(struct Pokemon *mon, u32 field) {return field == MON_DATA_SPECIES ? mon->species : mon->egg;}
u32 MockBaseSpecies(u32 species) {return species == 10000 ? SPECIES_UNOWN : species;}
bool32 CheckBagHasItem(enum Item item, u16 count) {return bag[item] >= count;}
bool32 AddBagItem(enum Item item, u16 count) {if (!room) return FALSE; bag[item] += count; return TRUE;}
#include "ENGINE_PATH"
static void Visit(u16 map) {world.location.mapGroup = MAP_GROUP(map); world.location.mapNum = MAP_NUM(map); RFQuests_Refresh();}
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
    struct SaveBlock3 snapshot = save;
    RFQuests_Refresh();
    assert(memcmp(&snapshot, &save, sizeof(save)) == 0);
    for (u32 i = 0; i < RF_QUEST_CAPACITY; i++) SetQuestState(i, RF_QUEST_ACTIVE);
    for (u32 i = 0; i < RF_QUEST_STATE_BYTES; i++) assert(save.rfQuestStates[i] == 0x55);
    assert(save.rfQuestSaveMagic == QUEST_SAVE_MAGIC);
    return 0;
}
'''.replace("ENGINE_PATH", engine.as_posix())
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            (work / "global.h").write_text(global_header, encoding="utf-8")
            for name in ("event_data.h", "item.h", "pokemon.h"):
                (work / name).write_text('#include "global.h"\n', encoding="utf-8")
            (work / "test.c").write_text(harness, encoding="utf-8")
            binary = work / "test-quests"
            subprocess.run([compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
                            "-fsanitize=address,undefined", "-I", str(work), "-I", str(ROOT / "include"),
                            str(work / "test.c"), "-o", str(binary)], check=True, capture_output=True, text=True)
            subprocess.run([str(binary)], check=True, capture_output=True, text=True)


if __name__ == "__main__":
    unittest.main()
