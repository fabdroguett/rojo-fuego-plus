"""Compile the production bulk-release predicate against representative records."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]


class StorageTests(unittest.TestCase):
    def test_sort_keeps_records_and_equal_species_order(self):
        source = (ROOT / "src/pokemon_storage_system.c").read_text(encoding="utf-8")
        start = source.index("        for (i = 1; i < IN_BOX_COUNT; i++)")
        end = source.index("        for (i = 0; i < IN_BOX_COUNT; i++)", start)
        loop = source[start:end]
        header = """
#include <assert.h>
#include <string.h>
typedef unsigned u32;
#define IN_BOX_COUNT 30
#define SPECIES_NONE 0
#define MON_DATA_SPECIES 0
enum Species { NORMAL = 1 };
struct BoxPokemon { unsigned species, uniqueId, item, marks, personality; };
static struct BoxPokemon boxMons[IN_BOX_COUNT];
static struct BoxPokemon *GetBoxedMonPtr(unsigned box, unsigned pos) { (void)box; return &boxMons[pos]; }
static unsigned GetBoxMonData(struct BoxPokemon *mon, unsigned field) { (void)field; return mon->species; }
static unsigned SpeciesToNationalPokedexNum(unsigned species) { return species == 501 ? 1 : species; }
static void Sort(void) { unsigned i, j, box = 0;
"""
        harness = """
}
int main(void) {
    struct BoxPokemon original[IN_BOX_COUNT];
    for (unsigned i = 0; i < IN_BOX_COUNT; i++) {
        boxMons[i] = (struct BoxPokemon){i % 5 ? (29 - i) % 8 + 1 : 0,
                                        i, 1000 + i, i % 16, 123456 + i};
    }
    boxMons[1].species = 501; // Alternate form shares national index with species 1.
    memcpy(original, boxMons, sizeof(original));
    Sort();
    unsigned seen[IN_BOX_COUNT] = {0}, previous = 0;
    for (unsigned i = 0; i < IN_BOX_COUNT; i++) {
        struct BoxPokemon *mon = &boxMons[i];
        unsigned id = mon->uniqueId;
        assert(id < IN_BOX_COUNT && !seen[id]++);
        assert(memcmp(mon, &original[id], sizeof(*mon)) == 0);
        unsigned order = mon->species ? SpeciesToNationalPokedexNum(mon->species) : 65535;
        assert(order >= previous);
        if (i && order == previous) assert(boxMons[i - 1].uniqueId < id);
        previous = order;
    }
    memcpy(original, boxMons, sizeof(original));
    Sort();
    assert(memcmp(original, boxMons, sizeof(original)) == 0);
    return 0;
}
"""
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            test = work / "sort.c"
            test.write_text(header + loop + harness, encoding="utf-8")
            binary = work / "sort"
            compiled = subprocess.run([compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
                                       "-fsanitize=address,undefined", str(test), "-o", str(binary)],
                                      capture_output=True, text=True)
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)

    def test_bulk_release_protection(self):
        source = (ROOT / "src/pokemon_storage_system.c").read_text(encoding="utf-8")
        start = source.index("static bool32 IsProtectedFromBulkRelease(")
        end = source.index("static void Task_SortBox", start)
        function = source[start:end]
        constants = ["MON_DATA_SPECIES", "MON_DATA_SANITY_IS_BAD_EGG", "MON_DATA_IS_EGG",
                     "MON_DATA_MARKINGS", "MON_DATA_IS_SHINY", "MON_DATA_HELD_ITEM"]
        moves = ["CUT", "FLY", "SURF", "STRENGTH", "FLASH", "ROCK_SMASH", "WATERFALL", "DIVE", "ROCK_CLIMB"]
        header = """
#include <assert.h>
#include <string.h>
typedef unsigned u32;
typedef int bool32;
#define TRUE 1
#define FALSE 0
#define SPECIES_NONE 0
#define NUM_SPECIES 4
#define ITEM_NONE 0
#define MAX_MON_MOVES 4
enum Species { NORMAL = 1, LEGEND = 2, MYTHICAL = 3 };
struct BoxPokemon { u32 data[10]; };
struct SpeciesInfo { int isSubLegendary, isRestrictedLegendary, isMythical, isUltraBeast; };
static struct SpeciesInfo gSpeciesInfo[4];
static u32 GetBoxMonData(struct BoxPokemon *mon, unsigned key) { return mon->data[key]; }
"""
        header += "\n".join(f"#define {key} {i}" for i, key in enumerate(constants))
        header += "\n#define MON_DATA_MOVE1 6\n"
        header += "\n".join(f"#define MOVE_{move} {i + 1}" for i, move in enumerate(moves))
        harness = """
int main(void) {
    struct BoxPokemon mon = {{NORMAL}};
    assert(!IsProtectedFromBulkRelease(&mon));
    for (unsigned field = 1; field <= 5; field++) {
        mon.data[field] = 1;
        assert(IsProtectedFromBulkRelease(&mon));
        mon.data[field] = 0;
    }
    for (unsigned move = 1; move <= 9; move++) {
        for (unsigned slot = 0; slot < 4; slot++) {
            mon.data[6 + slot] = move;
            assert(IsProtectedFromBulkRelease(&mon));
            mon.data[6 + slot] = 0;
        }
    }
    for (unsigned flag = 0; flag < 4; flag++) {
        gSpeciesInfo[LEGEND] = (struct SpeciesInfo){0};
        if (flag == 0) gSpeciesInfo[LEGEND].isSubLegendary = 1;
        if (flag == 1) gSpeciesInfo[LEGEND].isRestrictedLegendary = 1;
        if (flag == 2) gSpeciesInfo[LEGEND].isMythical = 1;
        if (flag == 3) gSpeciesInfo[LEGEND].isUltraBeast = 1;
        mon.data[0] = LEGEND;
        assert(IsProtectedFromBulkRelease(&mon));
    }
    mon.data[0] = NUM_SPECIES;
    assert(IsProtectedFromBulkRelease(&mon));
    mon.data[0] = SPECIES_NONE;
    assert(IsProtectedFromBulkRelease(&mon));
    return 0;
}
"""
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            test = work / "storage.c"
            test.write_text(header + "\n" + function + harness, encoding="utf-8")
            binary = work / "storage"
            compiled = subprocess.run([compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
                                       "-fsanitize=address,undefined", str(test), "-o", str(binary)],
                                      capture_output=True, text=True)
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
