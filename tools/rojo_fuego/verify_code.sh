#!/usr/bin/env bash
# Run from the repository root. Explicit ELF target: never invoke the ROM/default target.
set -euo pipefail

jobs="${RF_VERIFY_JOBS:-8}"
log="${RF_VERIFY_LOG:-/tmp/rojo-fuego-code-check.log}"
before="absent"
if [[ -f pokefirered.gba ]]; then
    before="$(sha256sum pokefirered.gba)"
fi

make SHELL="bash -o pipefail" BUILD=firered -j"$jobs" -O pokefirered.elf > "$log" 2>&1 || {
    tail -n 80 "$log"
    exit 1
}

after="absent"
if [[ -f pokefirered.gba ]]; then
    after="$(sha256sum pokefirered.gba)"
fi
if [[ "$before" != "$after" ]]; then
    echo 'ERROR: verification changed or created a ROM.' >&2
    exit 1
fi
tail -n 12 "$log"
echo 'Code compiled and ELF linked; no ROM generated or changed.'
