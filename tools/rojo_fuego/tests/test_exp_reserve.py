"""Native checks of the production EXP reserve and key-default migration."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]


class ReserveTests(unittest.TestCase):
    def run_c(self, source):
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            test = work / "check.c"
            test.write_text(source, encoding="utf-8")
            binary = work / "check"
            result = subprocess.run([compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
                                     "-fsanitize=address,undefined", str(test), "-o", str(binary)],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)

    def test_medal_cap_toggle_and_pre_cap_reward(self):
        source = (ROOT / "src/caps.c").read_text(encoding="utf-8")
        source = source[source.index("u32 GetCurrentLevelCap("):source.index("u32 GetCurrentEVCap(")]
        flags = [f"FLAG_BADGE{i:02}_GET" for i in range(1, 9)] + ["FLAG_IS_CHAMPION"]
        declarations = "\n".join(f"#define {flag} {i}" for i, flag in enumerate(flags))
        self.run_c(r'''
#include <assert.h>
typedef unsigned u32;
#define IS_FRLG 1
#define GEN_5 5
#define MAX_LEVEL 100
#define KEY_LEVEL_CAP 1
#define KEY_EXP_RESERVE 2
#define EXP_CAP_NONE 0
#define EXP_CAP_HARD 1
#define EXP_CAP_SOFT 2
#define LEVEL_CAP_FLAG_LIST 1
#define LEVEL_CAP_VARIABLE 2
#define B_EXP_CAP_TYPE EXP_CAP_HARD
#define B_LEVEL_CAP_TYPE LEVEL_CAP_FLAG_LIST
#define B_LEVEL_CAP_VARIABLE 0
#define B_LEVEL_CAP_EXP_UP 0
#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))
static unsigned flags, capOn = 1, reserveOn = 1;
static unsigned FlagGet(unsigned flag) {return (flags >> flag) & 1;}
static unsigned VarGet(unsigned var) {(void)var; return 100;}
static unsigned KeySystem_IsOn(unsigned key) {return key == KEY_LEVEL_CAP ? capOn : reserveOn;}
''' + declarations + "\n" + source + r'''
int main(void) {
    assert(GetCurrentLevelCap() == 14);
    assert(GetSoftLevelCapExpValue(13, 123) == 123);
    assert(GetSoftLevelCapExpValue(14, 123) == 123);
    reserveOn = 0;
    assert(GetSoftLevelCapExpValue(14, 123) == 0);
    flags = 1;
    assert(GetCurrentLevelCap() == 21);
    capOn = 0;
    assert(GetCurrentLevelCap() == 100);
    assert(GetSoftLevelCapExpValue(21, 123) == 123);
    capOn = 1; flags = 511;
    assert(GetCurrentLevelCap() == 100);
    return 0;
}
''')

    def test_cap_conservation_full_bag_and_saturation(self):
        source = (ROOT / "src/rf_exp_reserve.c").read_text(encoding="utf-8")
        source = re.sub(r'^#include .*$', '', source, flags=re.MULTILINE)
        self.run_c(r'''
#include <stdint.h>
#include <assert.h>
#include <string.h>
typedef uint32_t u32;
typedef int bool32;
#define IS_FRLG 1
#define KEY_EXP_RESERVE 1
#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))
#define min(a,b) ((a) < (b) ? (a) : (b))
enum Item {ITEM_EXP_CANDY_XS, ITEM_EXP_CANDY_S, ITEM_EXP_CANDY_M, ITEM_EXP_CANDY_L, ITEM_EXP_CANDY_XL};
struct Save {unsigned char old[16]; u32 rfExpReserveMagic, rfExpReserve;};
static struct Save save, *gSaveBlock3Ptr = &save;
static int enabled = 1, room = 1, xsOnly;
static u32 bag[5];
static bool32 KeySystem_IsOn(unsigned key) {(void)key; return enabled;}
static bool32 AddBagItem(enum Item item, unsigned count) {
    if (!room || (xsOnly && item != ITEM_EXP_CANDY_XS)) return 0;
    bag[item] += count; return 1;
}
''' + source + r'''
int main(void) {
    memset(&save, 0xA5, sizeof(save));
    assert(RFExpReserve_GetBalance() == 0);
    for (unsigned i = 0; i < sizeof(save.old); i++) assert(save.old[i] == 0xA5);
    assert(RFExpReserve_ApplyCap(950, 1000, 100) == 50);
    assert(save.rfExpReserve == 50);
    assert(RFExpReserve_ApplyCap(1000, 1000, 77) == 0);
    assert(save.rfExpReserve == 127);
    assert(RFExpReserve_ApplyCap(2000, 1000, 73) == 0);
    assert(save.rfExpReserve == 200);
    enabled = 0;
    assert(RFExpReserve_ApplyCap(0, 1000, 2000) == 1000);
    assert(save.rfExpReserve == 200);
    room = 0;
    assert(RFExpReserve_Claim() == 0 && save.rfExpReserve == 200);
    room = 1;
    assert(RFExpReserve_Claim() == 2 && save.rfExpReserve == 0);
    assert(bag[ITEM_EXP_CANDY_XS] == 2);
    enabled = 1;
    RFExpReserve_Store(43999);
    assert(RFExpReserve_Claim() == 5); // XL + L + M + S + XS, remainder 99.
    assert(save.rfExpReserve == 99);
    assert(RFExpReserve_Claim() == 0);
    RFExpReserve_Store(1);
    assert(RFExpReserve_Claim() == 1 && save.rfExpReserve == 0);
    xsOnly = 1;
    RFExpReserve_Store(500000);
    assert(RFExpReserve_Claim() == 999);
    assert(save.rfExpReserve == 400100);
    assert(RFExpReserve_ApplyCap(UINT32_MAX - 2, UINT32_MAX, UINT32_MAX) == 2);
    assert(save.rfExpReserve == RF_EXP_RESERVE_MAX);
    RFExpReserve_Store(UINT32_MAX);
    assert(save.rfExpReserve == RF_EXP_RESERVE_MAX);
    return 0;
}
''')

    def test_all_boolean_defaults_and_persistent_overrides(self):
        source = (ROOT / "src/key_system.c").read_text(encoding="utf-8")
        source = source[source.index("static void EnsureKeyDefaults("):source.index("enum KeyDifficulty KeySystem_GetDifficulty(")]
        constants = (ROOT / "include/constants/key_system.h").read_text(encoding="utf-8")
        self.run_c(r'''
#include <stdint.h>
#include <assert.h>
typedef uint32_t u32;
typedef int bool32;
#define TRUE 1
#define FALSE 0
#define FLAG_EXP_ALL 1
struct Save {u32 keySystemOff, rfKeyDefaultsMagic;};
static struct Save save, *gSaveBlock3Ptr = &save;
static int expAll = 1;
static int FlagGet(unsigned flag) {(void)flag; return expAll;}
static void FlagSet(unsigned flag) {(void)flag; expAll = 1;}
static void FlagClear(unsigned flag) {(void)flag; expAll = 0;}
''' + constants + source + r'''
int main(void) {
    // Previous "on" overrides for old off-default keys must not become "off".
    save.keySystemOff = (1u << KEY_WILD_LOOT) | (1u << KEY_BATTLE_SPEED);
    assert(KeySystem_IsOn(KEY_WILD_LOOT));
    assert(KeySystem_IsOn(KEY_BATTLE_SPEED));
    assert(KeySystem_IsOn(KEY_DARK_MODE));
    assert(KeySystem_IsOn(KEY_INFINITE_REPEL));
    assert(!KeySystem_IsOn((enum KeySystemKey)-1));
    assert(!KeySystem_IsOn(KEY_COUNT));
    u32 before = save.keySystemOff;
    KeySystem_Set((enum KeySystemKey)-1, FALSE);
    assert(save.keySystemOff == before);
    for (unsigned key = 0; key < KEY_COUNT; key++) {
        if (key == KEY_DIFFICULTY || key == KEY_SHINY_ODDS) continue;
        assert(KeySystem_IsOn(key));
        KeySystem_Set(key, FALSE);
        assert(!KeySystem_IsOn(key));
        EnsureKeyDefaults();
        assert(!KeySystem_IsOn(key));
        KeySystem_Set(key, TRUE);
        assert(KeySystem_IsOn(key));
    }
    return 0;
}
''')
