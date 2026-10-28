"""Build UI fixtures inside an isolated Linux checkout; --release removes the fixtures.

Never run in the source workspace. Requires RF_DISPOSABLE_BUILD=1 explicitly.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[3]
if os.name!='posix' or os.environ.get('RF_DISPOSABLE_BUILD')!='1':
    raise SystemExit('Use only an isolated Linux snapshot with RF_DISPOSABLE_BUILD=1.')
files={'ui_fixture.c':'rf_ui_fixture.c','bag_reference_fixture.c':'rf_bag_fixture.c','main_menu_fixture.c':'rf_main_menu_fixture.c'}
release='--release' in sys.argv
if release:
    for name in files.values(): (ROOT/'src'/name).unlink(missing_ok=True)
    flags=[]
else:
    for source,target in files.items(): shutil.copyfile(Path(__file__).parent/source,ROOT/'src'/target)
    names=[]
    for name in ('bag_reference_fixture.c','main_menu_fixture.c'):
        names+=re.findall(r'^void (RF(?:Bag|MainMenu)Fixture_\w+)\(',Path(__file__).with_name(name).read_text(),re.M)
    names+=['RFUiFixture_Start','RFUiFixture_Bag','RFUiFixture_BagFemale','RFUiFixture_Day','RFUiFixture_CheckOrder','RFUiFixture_Voiceover']
    flags=['LDFLAGS=-Map ../../pokefirered.map --print-memory-usage --gc-sections '+' '.join('-u '+n for n in names)]
(ROOT/'pokefirered.elf').unlink(missing_ok=True)
subprocess.run(['make','BUILD=firered','-j2','pokefirered.gba',*flags],cwd=ROOT,check=True)
