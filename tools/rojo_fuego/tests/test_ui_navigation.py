"""Check the production dashboard navigation against complete/incomplete grids."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]


class RemasteredNavigationTests(unittest.TestCase):
    def test_grid_bounds_direction_wrap_and_partial_rows(self):
        source = (ROOT / "src/start_menu.c").read_text(encoding="utf-8")
        function = re.search(r"static u8 RFStartMoveCursor\([^\n]*\)\n\{.*?\n\}", source, re.S).group()
        header = r'''
#include <assert.h>
#include <stdint.h>
typedef uint8_t u8; typedef uint16_t u16; typedef int32_t s32;
#define DPAD_RIGHT 16
#define DPAD_LEFT 32
#define DPAD_UP 64
#define DPAD_DOWN 128
'''
        checks = r'''
int main(void) {
    unsigned count, pos, key;
    const unsigned directions[] = {DPAD_RIGHT, DPAD_LEFT, DPAD_UP, DPAD_DOWN};
    assert(RFStartMoveCursor(0,0,DPAD_UP) == 0);
    assert(RFStartMoveCursor(0,9,DPAD_DOWN) == 3);
    assert(RFStartMoveCursor(0,9,DPAD_UP) == 6);
    assert(RFStartMoveCursor(8,9,DPAD_RIGHT) == 0);
    assert(RFStartMoveCursor(2,8,DPAD_UP) == 5); // no empty ninth cell
    assert(RFStartMoveCursor(5,8,DPAD_DOWN) == 2);
    for (count=1; count<=9; count++) for (pos=0; pos<count; pos++) {
        assert(RFStartMoveCursor(pos,count,0) == pos);
        for (key=0; key<4; key++) assert(RFStartMoveCursor(pos,count,directions[key]) < count);
        assert(RFStartMoveCursor(RFStartMoveCursor(pos,count,DPAD_RIGHT),count,DPAD_LEFT) == pos);
        assert(RFStartMoveCursor(RFStartMoveCursor(pos,count,DPAD_UP),count,DPAD_DOWN) == pos);
    }
    return 0;
}
'''
        compiler = shutil.which("cc")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as directory:
            c = Path(directory) / "navigation.c"
            exe = Path(directory) / "navigation"
            c.write_text(header + function + checks)
            subprocess.run([compiler, "-fsanitize=address,undefined", "-g", str(c), "-o", str(exe)], check=True)
            subprocess.run([str(exe)], check=True)


if __name__ == "__main__":
    unittest.main()
