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
        source = (engine.read_text(encoding="utf-8") + (ROOT / "src/data/rf_quests.h").read_text(encoding="utf-8")
                  + (ROOT / "src/data/rf_quests_legends.h").read_text(encoding="utf-8")
                  + (ROOT / "src/data/rf_quests_extra.h").read_text(encoding="utf-8")
                  + (ROOT / "src/data/rf_quests_story.h").read_text(encoding="utf-8"))
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
#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))
#define STATIC_ASSERT(x, name) _Static_assert(x, #name)
#define ALIGNED(n) __attribute__((aligned(n)))
#define COMPOUND_STRING(x) ((const u8 *)(x))
#define MAP_GROUP(x) ((x) >> 8)
#define MAP_NUM(x) ((x) & 255)
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
bool32 RemoveBagItem(enum Item item, u16 count);
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
bool32 FlagGet(u16 flag) {return flag == FLAG_SYS_POKEDEX_GET ? dex : champion;}
u32 GetMonData(struct Pokemon *mon, u32 field) {return field == MON_DATA_SPECIES ? mon->species : mon->egg;}
u8 GetSetPokedexFlag(enum NationalDexOrder species, u8 caseID) {(void)species; (void)caseID; return dex;}
enum NationalDexOrder SpeciesToNationalPokedexNum(enum Species species) {return (enum NationalDexOrder)species;}
u32 MockBaseSpecies(u32 species) {return species == 10000 ? SPECIES_UNOWN : species;}
bool32 CheckBagHasItem(enum Item item, u16 count) {return bag[item] >= count;}
bool32 AddBagItem(enum Item item, u16 count) {if (!room) return FALSE; bag[item] += count; return TRUE;}
bool32 RemoveBagItem(enum Item item, u16 count) {if (bag[item] < count) return FALSE; bag[item] -= count; return TRUE;}
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
