"""Run the existing emulator checks on Ubuntu WSL (default) or Docker.

RF_TEST_BACKEND=docker retains the previous container workflow.
RF_WSL_DISTRO selects a different WSL distribution.
"""
import os
from pathlib import Path
import subprocess


def run(*args, **kwargs):
    """Translate the checks' exec/cp operations without changing their assertions."""
    backend = os.environ.get('RF_TEST_BACKEND', 'wsl')
    if backend == 'docker':
        return subprocess.run(['docker', *args], check=True, **kwargs)
    if backend != 'wsl':
        raise ValueError(f'Unknown RF_TEST_BACKEND: {backend}')
    prefix = ['wsl.exe', '-d', os.environ.get('RF_WSL_DISTRO', 'Ubuntu-24.04')]
    env = dict(os.environ, WSL_UTF8='1')

    def host_path(value):
        return subprocess.check_output(
            prefix + ['--exec', 'wslpath', '-a', '-u', str(Path(value).resolve())],
            text=True, encoding='utf-8', env=env).strip()

    if args[0] == 'exec':
        position, cwd = 1, '/work'
        if args[position] == '-w':
            cwd, position = args[position + 1], position + 2
        if args[position] != 'rojo-fuego-check':
            raise ValueError('Unexpected build environment')
        command = prefix + ['--cd', cwd, '--exec', *args[position + 1:]]
    elif args[0] == 'cp':
        source, target = args[1:]
        remote = 'rojo-fuego-check:'
        if source.startswith(remote) == target.startswith(remote):
            raise ValueError('Expected one Linux path and one host path')
        source = source[len(remote):] if source.startswith(remote) else host_path(source)
        target = target[len(remote):] if target.startswith(remote) else host_path(target)
        command = prefix + ['--exec', 'cp', '--', source, target]
    else:
        raise ValueError(f'Unsupported environment operation: {args[0]}')
    return subprocess.run(command, check=True, env=env, **kwargs)


def output(*args):
    return run(*args, capture_output=True, text=True).stdout
