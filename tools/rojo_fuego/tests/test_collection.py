"""Verify species unions and encounter-time fallback using the production counter."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]


class CollectionTests(unittest.TestCase):
    def test_duplicates_forms_methods_time_fallback_and_empty_maps(self):
        source = (ROOT / "src/rf_collection.c").read_text(encoding="utf-8")
        source = re.sub(r'^#include .*$', '', source, flags=re.MULTILINE)
        header = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#define SPECIES_NONE 0
#define NUM_SPECIES 128
#define NATIONAL_DEX_COUNT 64
#define RF_COLLECTION_METHODS 5
#define TIMES_OF_DAY_COUNT 4
#define TIME_OF_DAY_DEFAULT 0
#define OW_TIME_OF_DAY_FALLBACK 0
#define FLAG_GET_CAUGHT 1
#define NUM_LAND_MONS_ENCOUNTER_SLOTS 12
#define NUM_WATER_MONS_ENCOUNTER_SLOTS 5
#define NUM_ROCK_SMASH_MONS_ENCOUNTER_SLOTS 5
#define NUM_FISHING_MONS_ENCOUNTER_SLOTS 10
#define NUM_HIDDEN_MONS_ENCOUNTER_SLOTS 3
enum Species {NORMAL = 1};
struct RFCollectionCount {u16 caught, total;};
struct RFCollectionSummary {struct RFCollectionCount methods[5], total;};
struct WildPokemon {u8 minLevel, maxLevel; enum Species species;};
struct WildPokemonInfo {u8 encounterRate; const struct WildPokemon *wildPokemon;};
struct WildEncounterTypes {
    const struct WildPokemonInfo *landMonsInfo, *waterMonsInfo, *rockSmashMonsInfo,
                                 *fishingMonsInfo, *hiddenMonsInfo;
};
struct WildPokemonHeader {struct WildEncounterTypes encounterTypes[4];};
static unsigned SpeciesToNationalPokedexNum(unsigned species) {return species == 99 ? 1 : species;}
static unsigned GetSetPokedexFlag(unsigned dex, unsigned action) {
    assert(dex > 0 && dex <= NATIONAL_DEX_COUNT && action == FLAG_GET_CAUGHT);
    return dex == 1 || dex == 4 || dex == 7;
}
'''
        harness = r'''
int main(void) {
    const struct WildPokemon land[12] = {{0,0,1}, {0,0,1}, {0,0,99}, {0,0,2}, {0,0,128}, {0,0,100}};
    const struct WildPokemon night[12] = {{0,0,2}, {0,0,3}};
    const struct WildPokemon water[5] = {{0,0,2}, {0,0,4}};
    const struct WildPokemon rocks[5] = {{0,0,4}, {0,0,5}};
    const struct WildPokemon fish[10] = {{0,0,5}, {0,0,6}};
    const struct WildPokemon hidden[3] = {{0,0,6}, {0,0,7}};
    const struct WildPokemonInfo infos[] = {{20,land}, {20,night}, {20,water}, {20,rocks}, {20,fish}, {20,hidden}, {0,water}};
    struct WildPokemonHeader map = {{{&infos[0], &infos[2], &infos[3], &infos[4], &infos[5]},
                                     {0}, {0}, {&infos[1], &infos[6], 0, 0, 0}}};
    struct RFCollectionSummary result;
    RFCollection_Count(&map, 0, &result);
    assert(result.total.total == 6 && result.total.caught == 3);
    assert(result.methods[0].total == 2 && result.methods[0].caught == 1);
    assert(result.methods[3].total == 2 && result.methods[3].caught == 0);
    RFCollection_Count(&map, 1, &result);
#if OW_TIME_OF_DAY_ENCOUNTERS && OW_TIME_OF_DAY_DISABLE_FALLBACK
    assert(result.total.total == 0);
#else
    assert(result.total.total == 6 && result.total.caught == 3);
#endif
    RFCollection_Count(&map, 3, &result);
#if !OW_TIME_OF_DAY_ENCOUNTERS
    assert(result.total.total == 6 && result.methods[1].total == 2);
#elif OW_TIME_OF_DAY_DISABLE_FALLBACK
    assert(result.total.total == 2 && result.total.caught == 0);
    assert(result.methods[1].total == 0);
#else
    assert(result.total.total == 6 && result.total.caught == 2);
    assert(result.methods[1].total == 0);
#endif
    RFCollection_Count(&map, TIMES_OF_DAY_COUNT, &result);
    assert(result.total.total == (OW_TIME_OF_DAY_ENCOUNTERS ? 7 : 6) && result.total.caught == 3);
    assert(result.methods[0].total == (OW_TIME_OF_DAY_ENCOUNTERS ? 3 : 2));
    RFCollection_Count(0, 0, &result);
    assert(result.total.total == 0);
    RFCollection_Count(&map, TIMES_OF_DAY_COUNT + 1, &result);
    assert(result.total.total == 0);
    memset(&map, 0, sizeof(map));
    RFCollection_Count(&map, TIMES_OF_DAY_COUNT, &result);
    assert(result.total.total == 0 && result.total.caught == 0);
    return 0;
}
'''
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            test = work / "collection.c"
            test.write_text(header + source + harness, encoding="utf-8")
            for time_enabled, no_fallback in ((1, 0), (1, 1), (0, 0)):
                binary = work / "collection"
                result = subprocess.run([compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
                                         "-fsanitize=address,undefined", f"-DOW_TIME_OF_DAY_ENCOUNTERS={time_enabled}",
                                         f"-DOW_TIME_OF_DAY_DISABLE_FALLBACK={no_fallback}", str(test), "-o", str(binary)],
                                        capture_output=True, text=True)
                self.assertEqual(result.returncode, 0, result.stderr)
                result = subprocess.run([str(binary)], capture_output=True, text=True)
                self.assertEqual(result.returncode, 0, result.stderr)
