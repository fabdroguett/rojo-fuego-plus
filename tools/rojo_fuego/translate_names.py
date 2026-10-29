#!/usr/bin/env python3
"""Translate move, item, ability, nature and Kanto location names to their official Spanish names.

usage: translate_names.py CSV_DIR   (PokéAPI data/v2/csv files: *_names.csv, location_names.csv)

The English name already in the source is matched (case and punctuation insensitive) to PokéAPI's English
name, then replaced by the Spanish one when every character exists in charmap.txt and it fits the game's
length limit; otherwise the English name stays. The script is idempotent: Spanish names are left alone.
"""
import csv
import json
import re
import sys
import unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ES, EN = '7', '9'


def charset():
    chars = set()
    for line in (ROOT / 'charmap.txt').read_text(encoding='utf-8').splitlines():
        m = re.match(r"^'(.)'\s*=", line)
        if m:
            chars.add(m.group(1))
    return chars


CHARS = charset()


def norm(name):
    name = unicodedata.normalize('NFKD', name).encode('ascii', 'ignore').decode().lower()
    return re.sub(r'[^a-z0-9]', '', name)


def table(csv_dir, file, key):
    by_id = {}
    with open(Path(csv_dir) / file, encoding='utf-8') as f:
        for row in csv.DictReader(f):
            by_id.setdefault(row[key], {})[row['local_language_id']] = row['name']
    out = {}
    for names in by_id.values():
        if EN in names and ES in names:
            out.setdefault(norm(names[EN]), names[ES])
    return out


def usable(text, limit):
    text = text.replace("'", '’')
    if len(text) > limit or any(c not in CHARS for c in text):
        return None
    return text


def translate_source(rel, pattern, names, limit, upper=False):
    path = ROOT / rel
    src = path.read_text(encoding='utf-8')
    done = kept = 0

    def swap(m):
        nonlocal done, kept
        english = m.group(2)
        spanish = names.get(norm(english))
        if spanish:
            spanish = usable(spanish.upper() if upper else spanish, limit)
        if not spanish or spanish == english:
            kept += 1
            return m.group(0)
        done += 1
        return m.group(1) + spanish + m.group(3)

    src = re.sub(pattern, swap, src)
    path.write_text(src, encoding='utf-8')
    print(f'{rel}: {done} translated, {kept} kept')


def translate_locations(csv_dir):
    names = table(csv_dir, 'location_names.csv', 'location_id')
    path = ROOT / 'src/data/region_map/region_map_sections.json'
    data = json.loads(path.read_text(encoding='utf-8'))
    done = 0
    for section in data['map_sections']:
        english = section.get('name', '')
        m = re.fullmatch(r'ROUTE (\d+)', english)
        if m:
            spanish = f'RUTA {m.group(1)}'
        else:
            spanish = names.get(norm(english))
            spanish = usable(spanish.upper(), 16) if spanish else None
        if spanish and spanish != english:
            section['name'] = spanish
            done += 1
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
    print(f'map sections: {done} translated')


def main():
    csv_dir = sys.argv[1]
    moves = table(csv_dir, 'move_names.csv', 'move_id')
    items = table(csv_dir, 'item_names.csv', 'item_id')
    abilities = table(csv_dir, 'ability_names.csv', 'ability_id')
    natures = table(csv_dir, 'nature_names.csv', 'nature_id')
    translate_source('src/data/moves_info.h', r'(\.name = COMPOUND_STRING\(")([^"]+)("\))', moves, 16)
    translate_source('src/data/items.h', r'(\.name = ITEM_NAME\(")([^"]+)("\))', items, 19)  # ITEM_NAME_LENGTH counts the terminator
    translate_source('src/data/abilities.h', r'(\.name = _\(")([^"]+)("\))', abilities, 16)
    translate_source('src/pokemon.c', r'(\.name = COMPOUND_STRING\(")([^"]+)("\))', natures, 12)
    translate_locations(csv_dir)


if __name__ == '__main__':
    main()
