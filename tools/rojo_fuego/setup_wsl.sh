#!/usr/bin/env bash
# Run once as root in Ubuntu 24.04; normal work runs as the unprivileged dev user.
set -euo pipefail
[[ $(id -u) == 0 ]] || { echo 'Run this setup as root.' >&2; exit 1; }
source /etc/os-release
[[ "$ID" == ubuntu ]] || { echo 'This setup expects Ubuntu.' >&2; exit 1; }
export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y --no-install-recommends build-essential binutils-arm-none-eabi \
    gcc-arm-none-eabi libnewlib-arm-none-eabi libpng-dev python3 python3-pil \
    git rsync ca-certificates libmgba-dev
id dev >/dev/null 2>&1 || useradd --create-home --shell /bin/bash dev
install -d -o dev -g dev /work
python3 - <<'PY'
import configparser
from pathlib import Path
path = Path('/etc/wsl.conf')
config = configparser.ConfigParser()
if path.exists():
    config.read(path)
if not config.has_section('user'):
    config.add_section('user')
config.set('user', 'default', 'dev')
if not config.has_section('boot'):
    config.add_section('boot')
config.set('boot', 'systemd', 'false')
with path.open('w') as output:
    config.write(output)
PY
echo 'Ubuntu tools installed. Restart this distribution to use the dev user.'
