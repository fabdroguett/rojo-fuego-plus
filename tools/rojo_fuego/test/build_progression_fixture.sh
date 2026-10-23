#!/usr/bin/env bash
# Disposable test ROM in the build container only. Never use as a delivery build.
set -euo pipefail
[[ "$PWD" == /work && -f Makefile && -f tools/rojo_fuego/test/progression_fixture.c ]] || {
    echo 'Run in /work in the disposable build container.' >&2
    exit 1
}
cp tools/rojo_fuego/test/progression_fixture.c src/rf_progression_fixture.c
roots=(RFProgress_Briefing RFProgress_Delivery RFProgress_Report RFProgress_CheckQuests
       RFProgress_Traveler RFProgress_CheckTraveler RFProgress_Relearner RFProgress_CheckRelearner
       RFProgress_DisableBerry RFProgress_Rematch RFProgress_CheckRematch gRFSaveLayout)
flags='-Map ../../pokefirered.map --print-memory-usage --gc-sections'
for symbol in "${roots[@]}"; do flags+=" -u $symbol"; done
# Relink even if a prior fixture was built with another set of roots.
rm -f pokefirered.elf
make SHELL='bash -o pipefail' BUILD=firered -j"${RF_VERIFY_JOBS:-8}" -O pokefirered.gba LDFLAGS="$flags"
echo 'TEST ROM ONLY. Remove src/rf_progression_fixture.c and the ELF before a production build.'
