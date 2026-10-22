"""Emerald-only new game setup must not write vars that FRLG uses for the trainer card."""
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[3]


def var_ids(name):
    text = (ROOT / "include/constants" / name).read_text(encoding="utf-8")
    return {m[0]: int(m[1], 16) for m in re.findall(r"#define (VAR_\w+)\s+(0x4[0-9A-Fa-f]{3})\b", text)}


class NewGameVarTests(unittest.TestCase):
    def test_emerald_setup_skipped_where_ids_collide_with_frlg(self):
        emerald, frlg = var_ids("vars.h"), {v: k for k, v in var_ids("vars_frlg.h").items()}
        # Each call writes a nonzero value into the Emerald var on a new game.
        calls = {"InitSeedotSizeRecord": "VAR_SEEDOT_SIZE_RECORD", "ResetLotteryCorner": "VAR_POKELOT_RND1"}
        source = (ROOT / "src/new_game.c").read_text(encoding="utf-8")
        body = re.search(r"void NewGameInitData\(void\)\n\{.*?\n\}", source, re.S).group()
        for call, var in calls.items():
            self.assertIn(emerald[var], frlg, f"{var} no longer collides; the guard may be dropped")
            self.assertRegex(body, rf"if \(!IS_FRLG\)\n\s+{call}\(\);", f"{call} clobbers {frlg[emerald[var]]}")


if __name__ == "__main__":
    unittest.main()
