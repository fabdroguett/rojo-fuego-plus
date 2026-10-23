"""Keep generated species compatibility in sync with shared gym tutor scripts."""
from pathlib import Path
import importlib.util
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[3]


class TutorDiscoveryTests(unittest.TestCase):
    def test_standard_legacy_and_shared_gym_declarations(self):
        spec = importlib.util.spec_from_file_location("rf_tutor_helper", ROOT / "tools/learnset_helpers/make_tutors.py")
        helper = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(helper)
        with tempfile.TemporaryDirectory() as directory:
            script = Path(directory) / "tutors.inc"
            script.write_text(
                "move_tutor MOVE_SWAGGER, text, text, end, end\n"
                "special ChooseMonForMoveTutor\nsetvar VAR_0x8005, MOVE_ROCK_SLIDE\n"
                "\trf_gym_tutor 4, FLAG_BADGE05_GET, MOVE_GUNK_SHOT\n"
                "  rf_gym_tutor 7 , FLAG_BADGE08_GET , MOVE_EARTH_POWER\n"
                "@ rf_gym_tutor 9, FLAG_OTHER, MOVE_NOT_A_TUTOR\n",
                encoding="utf-8")
            with patch.object(helper.glob, "glob", side_effect=lambda pattern: [str(script)] if "data/scripts" in pattern else []):
                self.assertEqual(set(helper.extract_repo_tutors()), {
                    "MOVE_SWAGGER", "MOVE_ROCK_SLIDE", "MOVE_GUNK_SHOT", "MOVE_EARTH_POWER"})
