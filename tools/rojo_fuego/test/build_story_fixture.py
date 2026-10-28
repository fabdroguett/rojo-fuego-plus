"""Link disposable story scenarios in a Linux snapshot. Never run in the source workspace."""
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

root=Path(__file__).resolve().parents[3]
if os.name!='posix' or os.environ.get('RF_DISPOSABLE_BUILD')!='1':
    raise SystemExit('Requires an isolated Linux snapshot and RF_DISPOSABLE_BUILD=1.')
source=root/'src/rf_story_fixture.c'
header=root/'src/data/rf_story_fixture_actors.h'
flags=[]
if '--release' in sys.argv:
    source.unlink(missing_ok=True)
    header.unlink(missing_ok=True)
else:
    shutil.copyfile(Path(__file__).with_name('story_fixture.c'),source)
    shutil.copyfile(Path(__file__).with_name('story_fixture_actors.h'),header)
    names=re.findall(r'^void (RFStoryFixture_\w+)\(',source.read_text(),re.M)
    flags=['LDFLAGS=-Map ../../pokefirered.map --print-memory-usage --gc-sections '+' '.join('-u '+n for n in names)]
(root/'pokefirered.elf').unlink(missing_ok=True)
subprocess.run(['make','BUILD=firered','-j2','pokefirered.gba',*flags],cwd=root,check=True)
