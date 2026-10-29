#!/usr/bin/env python3
"""Spanish move, item and ability descriptions from PokéAPI flavour text, wrapped to the game's font.

usage: translate_descriptions.py CSV_DIR

Entries are matched by constant (MOVE_POUND -> pound). Every Spanish flavour text is a candidate, newest
version first; the first one that wraps (normal font pixel widths from src/fonts.c) inside the widest
English line and the most lines any English description of that file uses is kept. Otherwise the English
text stays. Idempotent: entries already in Spanish are skipped.
"""
import csv
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def font():
    codes = {}
    for line in (ROOT / 'charmap.txt').read_text(encoding='utf-8').splitlines():
        m = re.match(r"^'(.)'\s*=\s*([0-9A-F]{2})\b", line)
        if m:
            codes.setdefault(m.group(1), int(m.group(2), 16))
    src = (ROOT / 'src/fonts.c').read_text(encoding='utf-8')
    body = src[src.index('gFontNormalLatinGlyphWidths[] = {'):]
    body = body[body.index('{') + 1:body.index('};')]
    widths = [int(v) for v in re.findall(r'\d+', body)]
    return codes, widths


CODES, WIDTHS = font()


def width(text):
    return sum(WIDTHS[CODES[c]] for c in text)


def flavour(csv_dir, file, key, names_file):
    ident = {}
    with open(Path(csv_dir) / names_file, encoding='utf-8') as f:
        for row in csv.DictReader(f):
            ident[row['id']] = row['identifier']
    texts = {}
    with open(Path(csv_dir) / file, encoding='utf-8') as f:
        for row in csv.DictReader(f):
            if row['language_id'] != '7':
                continue
            text = re.sub(r'\s+', ' ', row['flavor_text'].replace('\xad', '').replace('­', '')).strip()
            text = text.replace("'", '’').replace('"', '')
            texts.setdefault(ident.get(row[key], ''), []).append((int(row['version_group_id']), text))
    return {k: [t for _, t in sorted(v, reverse=True)] for k, v in texts.items()}


def wrap(text, max_w, max_lines):
    if any(c not in CODES for c in text):
        return None
    lines, cur = [], ''
    for word in text.split(' '):
        trial = (cur + ' ' + word).strip()
        if width(trial) <= max_w:
            cur = trial
        else:
            if not cur or width(word) > max_w:
                return None
            lines.append(cur)
            cur = word
    lines.append(cur)
    return lines if len(lines) <= max_lines else None


ENTRY = re.compile(r'\[(?P<const>(?:MOVE|ITEM|ABILITY)_\w+)\]\s*=\s*\{(?P<body>.*?)\n    \},', re.S)
DESC = re.compile(r'(\.description = COMPOUND_STRING\()(?P<str>(?:\s*"(?:[^"\\]|\\.)*")+)(\))', re.S)


def english_lines(strs):
    parts = re.findall(r'"((?:[^"\\]|\\.)*)"', strs)
    return ''.join(parts).split('\\n')


def translate(rel, prefix, texts):
    path = ROOT / rel
    raw = path.read_bytes().decode('utf-8')
    nl = '\r\n' if '\r\n' in raw else '\n'
    src = raw.replace('\r\n', '\n')
    descs = [english_lines(m.group('str')) for m in DESC.finditer(src)]
    descs = [d for d in descs if all(all(c in CODES for c in line) for line in d)]
    max_w = max(width(line) for d in descs for line in d)
    max_lines = max(len(d) for d in descs)
    done = kept = 0

    def per_entry(m):
        nonlocal done, kept
        body = m.group('body')
        key = m.group('const')[len(prefix):].lower().replace('_', '-')
        dm = DESC.search(body)
        if not dm:
            return m.group(0)
        old = english_lines(dm.group('str'))
        if any(c in ''.join(old) for c in 'áéíóúñ¿¡'):
            return m.group(0)  # already Spanish
        for cand in texts.get(key, []):
            lines = wrap(cand, max_w, max_lines)
            if lines:
                literal = ''.join(f'\n            "{line}\\n"' for line in lines[:-1]) + f'\n            "{lines[-1]}"'
                body = body[:dm.start()] + dm.group(1) + literal + dm.group(3) + body[dm.end():]
                done += 1
                return m.group(0).replace(m.group('body'), body)
        kept += 1
        return m.group(0)

    src = ENTRY.sub(per_entry, src)
    path.write_bytes(src.replace('\n', nl).encode('utf-8'))
    print(f'{rel}: {done} translated, {kept} kept (budget {max_w}px x {max_lines} lines)')


def main():
    csv_dir = sys.argv[1]
    translate('src/data/moves_info.h', 'MOVE_', flavour(csv_dir, 'move_flavor_text.csv', 'move_id', 'moves.csv'))
    translate('src/data/items.h', 'ITEM_', flavour(csv_dir, 'item_flavor_text.csv', 'item_id', 'items.csv'))
    translate('src/data/abilities.h', 'ABILITY_', flavour(csv_dir, 'ability_flavor_text.csv', 'ability_id', 'abilities.csv'))


if __name__ == '__main__':
    main()
