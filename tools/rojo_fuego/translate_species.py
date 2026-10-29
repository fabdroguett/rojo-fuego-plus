#!/usr/bin/env python3
"""Spanish Pokédex category and entry text for every species (PokéAPI data), wrapped to the game's font.

usage: translate_species.py CSV_DIR

SPECIES_BULBASAUR is matched to the PokéAPI identifier "bulbasaur"; forms without their own identifier
keep their text. The category is the Spanish genus without "Pokémon" (the game prints "Pokémon <category>").
Entries use the newest Spanish flavour text that wraps inside the widest English line and the most lines
an English entry uses. Idempotent.
"""
import csv
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from translate_descriptions import CODES, width, wrap  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
FILES = sorted((ROOT / 'src/data/pokemon/species_info').glob('gen_*_families.h'))


def load(csv_dir):
    ident = {}
    with open(Path(csv_dir) / 'pokemon_species.csv', encoding='utf-8') as f:
        for row in csv.DictReader(f):
            ident[row['id']] = row['identifier']
    genus = {}
    with open(Path(csv_dir) / 'pokemon_species_names.csv', encoding='utf-8') as f:
        for row in csv.DictReader(f):
            if row['local_language_id'] == '7' and row['genus']:
                g = re.sub(r'^Pokémon\s+', '', row['genus']).strip()
                genus[ident[row['pokemon_species_id']]] = g
    texts = {}
    with open(Path(csv_dir) / 'pokemon_species_flavor_text.csv', encoding='utf-8') as f:
        for row in csv.DictReader(f):
            if row['language_id'] != '7':
                continue
            text = re.sub(r'\s+', ' ', row['flavor_text'].replace('\xad', '')).strip().replace("'", '’').replace('"', '')
            texts.setdefault(ident[row['species_id']], []).append((int(row['version_id']), text))
    texts = {k: [t for _, t in sorted(v, reverse=True)] for k, v in texts.items()}
    return genus, texts


ENTRY = re.compile(r'\[(?P<const>SPECIES_\w+)\]\s*=\s*\{(?P<body>.*?)\n    \},', re.S)
DESC = re.compile(r'(\.description = COMPOUND_STRING\()((?:\s*"(?:[^"\\]|\\.)*")+)(\))', re.S)
CAT = re.compile(r'(\.categoryName = _\(")([^"]*)("\))')


def lines_of(strs):
    return ''.join(re.findall(r'"((?:[^"\\]|\\.)*)"', strs)).split('\\n')


def main():
    genus, texts = load(sys.argv[1])
    sources = {p: p.read_bytes().decode('utf-8').replace('\r\n', '\n') for p in FILES}
    english = [lines_of(m.group(2)) for s in sources.values() for m in DESC.finditer(s)]
    english = [d for d in english if all(all(c in CODES for c in line) for line in d) and d != ['']]
    max_w = max(width(line) for d in english for line in d)
    max_lines = max(len(d) for d in english)
    cats = descs = 0
    for path, src in sources.items():
        def per_entry(m):
            nonlocal cats, descs
            key = m.group('const')[len('SPECIES_'):].lower().replace('_', '-')
            body = m.group('body')
            g = genus.get(key)
            if g and len(g) <= 12 and all(c in CODES for c in g):
                body, n = CAT.subn(lambda c: c.group(1) + g + c.group(3), body, count=1)
                cats += n
            dm = DESC.search(body)
            if dm and not any(c in ''.join(lines_of(dm.group(2))) for c in 'áéíóúñ¿¡'):
                for cand in texts.get(key, []):
                    out = wrap(cand, max_w, max_lines)
                    if out:
                        literal = ''.join(f'\n            "{line}\\n"' for line in out[:-1]) + f'\n            "{out[-1]}"'
                        body = body[:dm.start()] + dm.group(1) + literal + dm.group(3) + body[dm.end():]
                        descs += 1
                        break
            return m.group(0).replace(m.group('body'), body)
        new = ENTRY.sub(per_entry, src)
        raw = path.read_bytes().decode('utf-8')
        nl = '\r\n' if '\r\n' in raw else '\n'
        path.write_bytes(new.replace('\n', nl).encode('utf-8'))
    print(f'species: {cats} categories, {descs} entries (budget {max_w}px x {max_lines} lines)')


if __name__ == '__main__':
    main()
