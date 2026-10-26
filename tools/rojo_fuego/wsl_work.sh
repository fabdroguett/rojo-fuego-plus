#!/usr/bin/env bash
# Windows remains the source of truth; compilation happens on the Linux filesystem.
set -euo pipefail
action="${1:-check}"
source_root="${2:?Windows repository path required}"
jobs="${3:-2}"
[[ "$jobs" =~ ^[1-9][0-9]*$ ]] || exit 2
[[ -d "$source_root/.git" && -f "$source_root/Makefile" ]] || exit 2
[[ -d /work && -w /work ]] || { echo 'Missing writable /work; see docs/ENTORNO_WSL.md.' >&2; exit 2; }
case "$action" in sync|check|test|build) ;; *) exit 2 ;; esac

archive="${4:?Source archive required; use wsl.ps1}"
echo 'Extracting Windows sources on the Linux filesystem...'
tar --extract --file="$archive" --directory=/work --no-same-owner
# Remove only previously synchronized source files that have disappeared from Windows.
python3 - <<'PY'
import json
from pathlib import Path
root = Path('/work').resolve()
state = root / '.rf-wsl-sources.json'
incoming = root / '.rf-wsl-incoming.json'
data = json.loads(incoming.read_text())
current = set(data)
previous = set(json.loads(state.read_text())) if state.exists() else set()
for name in previous - current:
    target = (root / name).resolve()
    if not target.is_relative_to(root):
        raise ValueError(f'Invalid source path: {name}')
    if target.is_file():
        target.unlink()
state.write_text(json.dumps(data))
incoming.unlink()
PY
cd /work
# /work is a build snapshot; the real Git history stays in the Windows repository.
touch .histignore
cc -O2 tools/rojo_fuego/test/harness.c -o /tmp/rf-mgba-harness -lmgba
echo "Sources synchronized to /work ($jobs build jobs)."
case "$action" in
    sync) ;;
    check) RF_VERIFY_JOBS="$jobs" bash tools/rojo_fuego/verify_code.sh ;;
    test)
        python3 tools/rojo_fuego/validate_quests.py
        python3 -m unittest discover -s tools/rojo_fuego/tests -v
        ;;
    build)
        if find src -maxdepth 1 -name '*fixture*.c' -print -quit | grep -q .; then
            echo 'Remove test fixtures before a production build.' >&2
            exit 1
        fi
        make SHELL='bash -o pipefail' BUILD=firered -j"$jobs" -O pokefirered.gba
        if arm-none-eabi-nm pokefirered.elf | grep -E 'RFProgress|RFUiFixture|RFTrainingFixture|gRFSaveLayout'; then
            echo 'Fixture symbols in production ELF.' >&2
            exit 1
        fi
        ;;
esac
