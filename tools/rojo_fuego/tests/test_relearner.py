"""Exercise real relearn-list builders with ancestry, duplicates and long lists."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]


class RelearnerTests(unittest.TestCase):
    def test_ancestry_known_moves_and_bounded_lists(self):
        source = (ROOT / "src/move_relearner.c").read_text(encoding="utf-8")
        functions = []
        for name in ("AppendRelearnerMove", "GetRelearnerLevelUpMoves", "GetRelearnerEggMoves",
                     "GetRelearnerTMMoves", "GetRelearnerTutorMoves"):
            functions.append(re.search(r"static u32 " + name + r"\([^\n]*\)\n\{.*?\n\}", source, re.S).group())
        header = r'''
#include <stdint.h>
#include <assert.h>
#include <string.h>
typedef uint16_t u16; typedef uint32_t u32; typedef int bool32;
#define TRUE 1
#define FALSE 0
#define MAX_RELEARNER_MOVES 254
#define MAX_LEVEL 100
#define P_PRE_EVO_MOVES TRUE
#define P_ENABLE_ALL_LEVEL_UP_MOVES FALSE
#define MOVE_NONE 0
#define MOVE_UNAVAILABLE 65535
#define LEVEL_UP_MOVE_END 65535
#define NUM_ALL_MACHINES 400
#define MON_DATA_SPECIES 0
enum Species {SPECIES_NONE, SPECIES_BASE, SPECIES_MIDDLE, SPECIES_FINAL};
enum Move {MOVE_DUMMY}; enum Item {ITEM_DUMMY};
struct BoxPokemon {enum Species species;};
struct LevelUpMove {u16 move, level;};
static u16 gTutorMoves[405];
static const struct LevelUpMove baseMoves[] = {{10,1},{11,20},{12,60},{LEVEL_UP_MOVE_END,0}};
static const struct LevelUpMove middleMoves[] = {{21,1},{11,20},{LEVEL_UP_MOVE_END,0}};
static const struct LevelUpMove finalMoves[] = {{30,1},{LEVEL_UP_MOVE_END,0}};
static const u16 eggMoves[] = {41,42,41,MOVE_UNAVAILABLE};
static u32 known, checkAvailability;
static enum Species GetBoxMonData(struct BoxPokemon *mon, u32 field) {(void)field; return mon->species;}
static u32 GetLevelFromBoxMonExp(struct BoxPokemon *mon) {(void)mon; return 30;}
static enum Species GetSpeciesPreEvolution(enum Species species) {return species ? species - 1 : 0;}
static const struct LevelUpMove *GetSpeciesLevelUpLearnset(enum Species species) {
    return species == SPECIES_FINAL ? finalMoves : species == SPECIES_MIDDLE ? middleMoves : baseMoves;
}
static bool32 BoxMonKnowsMove(struct BoxPokemon *mon, u32 move) {(void)mon; return move == known;}
static const u16 *GetSpeciesEggMoves(enum Species species) {assert(species == SPECIES_BASE); return eggMoves;}
static enum Item GetTMHMItemId(u32 index) {return index;}
static enum Move GetTMHMMoveId(u32 index) {return 100 + index % 300;}
static bool32 IsTmAvailable(enum Item item) {return !checkAvailability || item == 1;}
static bool32 CanLearnTeachableMove(enum Species species, enum Move move) {
    assert(species == SPECIES_FINAL); return (u32)move != 200;
}
'''
        harness = r'''
int main(void) {
    struct BoxPokemon mon = {SPECIES_FINAL};
    struct {u16 before; u16 moves[MAX_RELEARNER_MOVES]; u16 after;} buffer;
    memset(&buffer, 0, sizeof(buffer)); buffer.before = 0xAAAA; buffer.after = 0xBBBB;
    known = 10;
    u32 count = GetRelearnerLevelUpMoves(&mon, buffer.moves);
    assert(count == 3 && buffer.moves[0] == 30 && buffer.moves[1] == 21 && buffer.moves[2] == 11);
    known = 41;
    count = GetRelearnerEggMoves(&mon, buffer.moves);
    assert(count == 1 && buffer.moves[0] == 42);
    known = 201;
    count = GetRelearnerTMMoves(&mon, buffer.moves);
    assert(count == MAX_RELEARNER_MOVES);
    for (u32 i = 0; i < count; i++) {
        assert(buffer.moves[i] != 200 && buffer.moves[i] != 201);
        for (u32 j = i + 1; j < count; j++) assert(buffer.moves[i] != buffer.moves[j]);
    }
    assert(buffer.before == 0xAAAA && buffer.after == 0xBBBB);
    checkAvailability = 1;
    count = GetRelearnerTMMoves(&mon, buffer.moves);
    assert(count == 1 && buffer.moves[0] == 101);
    for (u32 i = 0; i < 400; i++) gTutorMoves[i] = 100 + i % 300;
    gTutorMoves[400] = MOVE_UNAVAILABLE;
    count = GetRelearnerTutorMoves(&mon, buffer.moves);
    assert(count == MAX_RELEARNER_MOVES);
    assert(buffer.before == 0xAAAA && buffer.after == 0xBBBB);
    assert(AppendRelearnerMove(buffer.moves, 0, MOVE_NONE) == 0);
    assert(AppendRelearnerMove(buffer.moves, 0, MOVE_UNAVAILABLE) == 0);
    return 0;
}
'''
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            test = work / "relearner.c"
            test.write_text(header + "\n".join(functions) + harness, encoding="utf-8")
            result = subprocess.run([compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
                                     "-fsanitize=address,undefined", str(test), "-o", str(work / "test")],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(work / "test")], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
