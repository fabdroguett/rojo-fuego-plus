#!/usr/bin/env bash
set -euo pipefail
[[ "$PWD" == /work && -f Makefile && -f tools/rojo_fuego/test/training_fixture.c ]] || exit 1
cp tools/rojo_fuego/test/training_fixture.c src/rf_training_fixture.c
roots=(RFTrainingFixture_Begin RFTrainingFixture_IV RFTrainingFixture_CheckPreview
       RFTrainingFixture_CheckEV RFTrainingFixture_CheckIV)
flags='-Map ../../pokefirered.map --print-memory-usage --gc-sections'
for symbol in "${roots[@]}"; do flags+=" -u $symbol"; done
rm -f pokefirered.elf
make SHELL='bash -o pipefail' BUILD=firered -j"${RF_VERIFY_JOBS:-8}" -O pokefirered.gba LDFLAGS="$flags"
echo 'TEST ROM ONLY. Remove src/rf_training_fixture.c and the ELF before a production build.'
