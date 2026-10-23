"""Lint the shipped quest catalogue before compiling or expanding its content."""
from pathlib import Path
import ast
import re

ROOT = Path(__file__).resolve().parents[2]


def validate():
    constants = (ROOT / "include/constants/rf_quests.h").read_text(encoding="utf-8")
    values = {name: int(value) for name, value in
              re.findall(r"^#define (RF_QUEST_\w+)\s+(\d+)\s*$", constants, re.M)}
    count, chapters = values["RF_QUEST_COUNT"], values["RF_QUEST_CHAPTER_COUNT"]
    source = (ROOT / "src/data/rf_quests.h").read_text(encoding="utf-8")
    entries = re.findall(r"\[(RF_QUEST_\w+)\]\s*=\s*\{(.*?)\n    \},", source, re.S)
    errors, catalogue, ids = [], {}, set()
    symbols = "\n".join((ROOT / "include/constants" / path).read_text(encoding="utf-8")
                        for path in ("items.h", "species.h", "map_groups.h", "opponents_frlg.h"))
    known = set(re.findall(r"\b(?:ITEM|SPECIES|MAP|TRAINER)_\w+", symbols)) | set(values)
    known.update(re.findall(r"^#define (RF_QUEST_EVENT_\w+)", constants, re.M))
    conditions = set(re.findall(r"\bQUEST_[A-Z_]+", (ROOT / "include/rf_quests.h").read_text(encoding="utf-8")))

    def fail(name, message):
        errors.append(f"{name}: {message}")

    for name, body in entries:
        if name not in values or values[name] >= count or values[name] in ids:
            fail(name, "missing, duplicated or out-of-range ID")
        ids.add(values.get(name, -1))
        fields = dict(re.findall(r"\.(\w+)\s*=\s*([A-Z][A-Z0-9_]*|\d+)", body))
        if name in catalogue:
            fail(name, "duplicate catalogue entry")
        catalogue[name] = fields
        for field in ("prerequisite", "condition", "argument", "rewardItem", "rewardCount", "chapter"):
            if field not in fields:
                fail(name, f"missing {field}")
        if fields.get("condition") not in conditions:
            fail(name, "unknown objective condition")
        if not fields.get("chapter", "").isdigit() or int(fields["chapter"]) >= chapters:
            fail(name, "invalid chapter")
        if not fields.get("rewardCount", "").isdigit() or not 1 <= int(fields["rewardCount"]) <= 255:
            fail(name, "invalid reward quantity")
        if fields.get("rewardItem") == "ITEM_NONE":
            fail(name, "empty reward")
        if fields.get("condition") == "QUEST_DELIVER_ITEM":
            if not fields.get("argument", "").startswith("ITEM_") or not fields.get("alternative", "").isdigit():
                fail(name, "delivery requires an item and explicit quantity")
            elif not 1 <= int(fields["alternative"]) <= 999:
                fail(name, "invalid delivery quantity")
        for symbol in re.findall(r"\b(?:ITEM|SPECIES|MAP|TRAINER|RF_QUEST_EVENT)_\w+", body):
            if symbol not in known:
                fail(name, f"unknown reference {symbol}")
        for field, text in re.findall(r'\.(name|description)\s*=\s*COMPOUND_STRING\(("(?:[^"\\]|\\.)*")\)', body):
            decoded = ast.literal_eval(text)
            # A coarse text lint; actual font width and panel fit still require native validation.
            if not decoded or any(len(line) > 40 for line in decoded.split("\n")):
                fail(name, f"empty or unusually long {field}")

    if ids != set(range(count)) or len(entries) != count:
        errors.append(f"catalogue must contain exactly IDs 0..{count - 1}")
    for name in catalogue:
        seen, current = set(), name
        while current != "RF_QUEST_NONE":
            if current not in catalogue:
                fail(name, f"unknown prerequisite {current}")
                break
            if current in seen:
                fail(name, f"prerequisite cycle at {current}")
                break
            seen.add(current)
            current = catalogue[current].get("prerequisite", "RF_QUEST_NONE")
    return count, errors


if __name__ == "__main__":
    count, errors = validate()
    if errors:
        raise SystemExit("\n".join(errors))
    print(f"Validated {count} quests: IDs, references, prerequisites, rewards and text lint.")
