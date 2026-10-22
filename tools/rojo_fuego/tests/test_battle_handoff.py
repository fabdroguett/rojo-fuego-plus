"""Exercise the production battle callback across task/controller menu handoffs."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]


class BattleHandoffTests(unittest.TestCase):
    def test_fast_tick_respects_callback_ownership_and_restores_input(self):
        source = (ROOT / "src/battle_main.c").read_text(encoding="utf-8")
        callback = re.search(r"void BattleMainCB2\(void\)\n\{.*?\n\}", source, re.S).group()
        stubs = r'''
#include <assert.h>
#include <stdint.h>
typedef uint16_t u16;
struct {void (*callback1)(void), (*callback2)(void); u16 newKeys, newAndRepeatedKeys;} gMain;
struct {unsigned active;} gPaletteFade;
unsigned gBattleTypeFlags, gSpecialVar_Result, gBattleOutcome;
static unsigned fast, tasks, ticks, handoff, fade;
#define KEY_BATTLE_SPEED 0
#define BATTLE_TYPE_LINK 1
#define BATTLE_TYPE_RECORDED 2
#define B_BUTTON 2
#define B_OUTCOME_PLAYER_TELEPORTED 3
#define PALETTES_ALL 0xffffffff
#define RGB_BLACK 0
#define JOY_HELD(x) 0
void BattleMainCB2(void);
static void Menu(void) {}
static unsigned KeySystem_IsOn(unsigned key) {return fast;}
static void AnimateSprites(void) {}
static void BuildOamBuffer(void) {}
static void RunTextPrinters(void) {}
static void UpdatePaletteFade(void) {if (fade) gPaletteFade.active = 1;}
static void RunTasks(void) {tasks++; if (handoff == 1) gMain.callback2 = Menu;}
static void BattleMainCB1(void) {
    assert(!gMain.newKeys && !gMain.newAndRepeatedKeys);
    ticks++;
    if (handoff == 2) gMain.callback2 = Menu;
    if (handoff == 3) gMain.callback1 = Menu;
}
static unsigned RecordedBattle_CanStopPlayback(void) {return 0;}
static void ResetPaletteFadeControl(void) {}
static void BeginNormalPaletteFade(unsigned a, int b, int c, int d, int e) {}
static void CB2_QuitRecordedBattle(void) {}
static void SetMainCallback2(void (*cb)(void)) {gMain.callback2 = cb;}
'''
        checks = r'''
static void reset(void) {
    tasks = ticks = handoff = fade = gPaletteFade.active = gBattleTypeFlags = 0;
    fast = 1;
    gMain.callback1 = BattleMainCB1; gMain.callback2 = BattleMainCB2;
    gMain.newKeys = 0x101; gMain.newAndRepeatedKeys = 0x205;
}
static void check(unsigned expectedTasks, unsigned expectedTicks) {
    BattleMainCB2();
    assert(tasks == expectedTasks && ticks == expectedTicks);
    assert(gMain.newKeys == 0x101 && gMain.newAndRepeatedKeys == 0x205);
}
int main(void) {
    reset(); check(2,1);
    reset(); handoff=1; check(1,0);
    reset(); handoff=2; check(1,1);
    reset(); handoff=3; check(1,1);
    reset(); fade=1; check(1,0);
    reset(); fast=0; check(1,0);
    reset(); gBattleTypeFlags=BATTLE_TYPE_LINK; check(1,0);
    reset(); gBattleTypeFlags=BATTLE_TYPE_RECORDED; check(1,0);
    reset(); gMain.callback2=Menu; check(1,0);
    return 0;
}
'''
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as directory:
            c = Path(directory) / "handoff.c"
            exe = Path(directory) / "handoff"
            c.write_text(stubs + callback + checks)
            subprocess.run([compiler, "-fsanitize=address,undefined", "-g", str(c), "-o", str(exe)], check=True)
            subprocess.run([str(exe)], check=True)


if __name__ == "__main__":
    unittest.main()
