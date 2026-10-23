"""Verify the real EV/IV service respects caps and previews without mutations."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]


class TrainingTests(unittest.TestCase):
    def test_limits_preview_hyper_training_and_invalid_inputs(self):
        source = re.sub(r'^#include .*$', '', (ROOT / "src/rf_training.c").read_text(encoding="utf-8"), flags=re.M)
        header = r'''
#include <stdint.h>
#include <assert.h>
#include <string.h>
typedef uint32_t u32; typedef int bool32;
#define TRUE 1
#define FALSE 0
#define IS_FRLG 1
#define PARTY_SIZE 6
#define NUM_STATS 6
#define B_TRAINER_PLAYER 0
#define MAX_PER_STAT_IVS 31
#define MAX_PER_STAT_EVS 252
#define MAX_TOTAL_EVS 510
#define min(a,b) ((a) < (b) ? (a) : (b))
enum {SPECIES_NONE, SPECIES_TEST};
enum {MON_DATA_SPECIES, MON_DATA_IS_EGG, MON_DATA_SANITY_IS_BAD_EGG,
      MON_DATA_HP_EV = 10, MON_DATA_HP_IV = 20, MON_DATA_HYPER_TRAINED_HP = 30};
struct Pokemon {u32 fields[40];};
static struct Pokemon gParties[1][6];
static struct {bool32 inBattle;} gMain;
static u32 gSpecialVar_0x8004, gSpecialVar_0x8005, gSpecialVar_0x8006, gSpecialVar_0x8007,
           gSpecialVar_0x8008, gSpecialVar_0x8009, gSpecialVar_0x800A, gSpecialVar_0x800B,
           gSpecialVar_Result, recalculations;
static u32 GetMonData(struct Pokemon *mon, u32 field) {assert(field < 40); return mon->fields[field];}
static void SetMonData(struct Pokemon *mon, u32 field, const u32 *value) {assert(field < 40); mon->fields[field] = *value;}
static void CalculateMonStats(struct Pokemon *mon) {(void)mon; recalculations++;}
'''
        header += (ROOT / "include/rf_training.h").read_text(encoding="utf-8")
        harness = r'''
int main(void) {
    struct Pokemon *mon = &gParties[0][0];
    assert(!RFTraining_SetValue(0,0,RF_TRAIN_EV,1));
    mon->fields[MON_DATA_SPECIES] = SPECIES_TEST;
    assert(!RFTraining_SetValue(UINT32_MAX,0,0,1));
    assert(!RFTraining_SetValue(0,NUM_STATS,0,1));
    assert(!RFTraining_SetValue(0,0,RF_TRAIN_MODE_COUNT,1));
    mon->fields[MON_DATA_IS_EGG] = 1; assert(!RFTraining_SetValue(0,0,0,1));
    mon->fields[MON_DATA_IS_EGG] = 0;
    mon->fields[MON_DATA_SANITY_IS_BAD_EGG] = 1; assert(!RFTraining_SetValue(0,0,0,1));
    mon->fields[MON_DATA_SANITY_IS_BAD_EGG] = 0;
    gMain.inBattle = 1; assert(!RFTraining_SetValue(0,0,0,1)); gMain.inBattle = 0;
    assert(!RFTraining_SetValue(0,0,RF_TRAIN_EV,253));
    assert(RFTraining_SetValue(0,0,RF_TRAIN_EV,252));
    assert(RFTraining_SetValue(0,1,RF_TRAIN_EV,252));
    assert(!RFTraining_SetValue(0,2,RF_TRAIN_EV,7));
    assert(GetEVTotal(mon) == 504);
    gSpecialVar_0x8005 = 2; gSpecialVar_0x8006 = RF_TRAIN_EV;
    Script_RF_TrainingRead();
    assert(gSpecialVar_Result && gSpecialVar_0x8007 == 0 && gSpecialVar_0x8008 == 6 && gSpecialVar_0x800B == 504);
    struct Pokemon snapshot = *mon; u32 before = recalculations;
    gSpecialVar_0x800A = RF_TRAIN_MAX; Script_RF_TrainingPreview();
    assert(gSpecialVar_Result && gSpecialVar_0x8007 == 6);
    assert(memcmp(&snapshot, mon, sizeof(snapshot)) == 0 && recalculations == before);
    Script_RF_TrainingEdit(); assert(gSpecialVar_Result && GetEVTotal(mon) == 510);
    gSpecialVar_0x8005 = 3; gSpecialVar_0x800A = RF_TRAIN_PLUS_ONE;
    Script_RF_TrainingEdit(); assert(!gSpecialVar_Result && GetEVTotal(mon) == 510);
    gSpecialVar_0x8005 = 2; gSpecialVar_0x800A = RF_TRAIN_MINUS_TEN;
    Script_RF_TrainingEdit(); assert(gSpecialVar_Result && GetEVTotal(mon) == 504);
    gSpecialVar_0x800A = RF_TRAIN_PLUS_TEN;
    Script_RF_TrainingEdit(); assert(gSpecialVar_Result && GetEVTotal(mon) == 510);
    gSpecialVar_0x800A = RF_TRAIN_MINUS_ONE;
    Script_RF_TrainingEdit(); assert(gSpecialVar_Result && GetEVTotal(mon) == 509);
    gSpecialVar_0x800A = RF_TRAIN_ZERO;
    Script_RF_TrainingEdit(); assert(gSpecialVar_Result && GetEVTotal(mon) == 504);
    assert(!RFTraining_SetValue(0,0,RF_TRAIN_IV,32));
    mon->fields[MON_DATA_HP_IV] = 15;
    mon->fields[MON_DATA_HYPER_TRAINED_HP] = 1;
    mon->fields[MON_DATA_HYPER_TRAINED_HP + 1] = 1;
    assert(RFTraining_SetValue(0,0,RF_TRAIN_IV,15)); // Clear only the chosen override.
    assert(!mon->fields[MON_DATA_HYPER_TRAINED_HP] && mon->fields[MON_DATA_HYPER_TRAINED_HP + 1]);
    assert(!RFTraining_SetValue(0,0,RF_TRAIN_IV,15));
    gSpecialVar_0x8005 = 0; gSpecialVar_0x8006 = RF_TRAIN_IV;
    gSpecialVar_0x800A = RF_TRAIN_MAX; Script_RF_TrainingEdit();
    assert(gSpecialVar_Result && mon->fields[MON_DATA_HP_IV] == 31);
    gSpecialVar_0x800A = RF_TRAIN_PLUS_TEN; Script_RF_TrainingEdit(); assert(!gSpecialVar_Result);
    gSpecialVar_0x800A = RF_TRAIN_MINUS_ONE; Script_RF_TrainingEdit();
    assert(gSpecialVar_Result && mon->fields[MON_DATA_HP_IV] == 30);
    snapshot = *mon; before = recalculations;
    gSpecialVar_0x800A = UINT32_MAX; Script_RF_TrainingEdit(); assert(!gSpecialVar_Result);
    assert(memcmp(&snapshot, mon, sizeof(snapshot)) == 0 && recalculations == before);
    gSpecialVar_0x8004 = PARTY_SIZE;
    Script_RF_TrainingCanEdit(); assert(!gSpecialVar_Result);
    Script_RF_TrainingRead(); assert(!gSpecialVar_Result);
    return 0;
}
'''
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            test = work / "training.c"
            test.write_text(header + source + harness, encoding="utf-8")
            result = subprocess.run([compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
                                     "-fsanitize=address,undefined", str(test), "-o", str(work / "test")],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(work / "test")], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
