"""Pack current tracked and non-ignored sources using the fast Windows filesystem."""
import io
import json
from pathlib import Path
import subprocess
import sys
import tarfile

root = Path(__file__).resolve().parents[2]
output = root / 'build_artifacts/wsl/sources.tar'
output.parent.mkdir(parents=True, exist_ok=True)
names = subprocess.check_output(['git', '-C', str(root), 'ls-files', '-co',
                                 '--exclude-standard', '-z']).split(b'\0')
previous_path = Path(sys.argv[1]) if len(sys.argv) > 1 else None
previous = json.loads(previous_path.read_text(encoding='utf-8-sig')) if previous_path and previous_path.exists() else {}
if not isinstance(previous, dict):
    previous = {}
included = {}
changed = 0
with tarfile.open(output, 'w') as archive:
    for raw in sorted(set(names)):
        if not raw:
            continue
        name = raw.decode('utf-8')
        path = root / name
        if not path.exists():
            continue
        if not path.is_file() or path.is_symlink() or not path.resolve().is_relative_to(root):
            raise ValueError(f'Unexpected source path: {name}')
        stat = path.stat()
        stamp = [stat.st_mtime_ns, stat.st_size]
        included[name] = stamp
        if previous.get(name) != stamp:
            archive.add(path, arcname=name, recursive=False)
            changed += 1
    data = json.dumps(included).encode('utf-8')
    info = tarfile.TarInfo('.rf-wsl-incoming.json')
    info.size = len(data)
    info.mode = 0o644
    archive.addfile(info, io.BytesIO(data))
print(f'Packed {changed} changed files ({len(included)} sources) for Ubuntu.')
