"""Lint the shipped quest catalogue before compiling or expanding its content."""
from pathlib import Path
import ast
import re

ROOT = Path(__file__).resolve().parents[2]


def validate():
    constants = (ROOT / "include/constants/rf_quests.h").read_text(encoding="utf-8")
    values = {name: int(value) for name, value in
              re.findall(r"^#define (RF_QUEST_\w+)\s+(\d+)\s*$", constants, re.M)}
    extra_constants = (ROOT / "include/constants/rf_quests_extra.h").read_text(encoding="utf-8")
    extra_constants += (ROOT / "include/constants/rf_story.h").read_text(encoding="utf-8")
    values.update({name: int(value) for name, value in
                   re.findall(r"^#define (RF_QUEST_\w+)\s+(\d+)\s*$", extra_constants, re.M)})
    count, chapters = values["RF_QUEST_COUNT"], values["RF_QUEST_CHAPTER_COUNT"]
    source = "\n".join((ROOT / "src/data" / name).read_text(encoding="utf-8")
                       for name in ("rf_quests.h", "rf_quests_legends.h", "rf_quests_extra.h", "rf_quests_story.h"))
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
        if fields.get("condition") == "QUEST_COUNT":
            if not fields.get("argument", "").startswith("RF_QUEST_EVENT_"):
                fail(name, "counted objective requires an RF_QUEST_EVENT_* argument")
            if not fields.get("target", "").isdigit() or not 1 <= int(fields["target"]) <= 63:
                fail(name, "counted objective requires target 1..63")
            # A dual-type catch reports both types, and map 0 is a real map: these need a filter.
            if fields.get("argument") in ("RF_QUEST_EVENT_CATCH_MAP", "RF_QUEST_EVENT_CATCH_TYPE") \
                    and fields.get("alternative", "0") == "0":
                fail(name, "catch-by-map/type objectives require a filter")
        elif "target" in fields:
            fail(name, "target is only used by QUEST_COUNT")
        if fields.get("condition") == "QUEST_BADGES" \
                and (not fields.get("argument", "").isdigit() or not 1 <= int(fields["argument"]) <= 8):
            fail(name, "badge objective requires 1..8 badges")
        if fields.get("condition") in ("QUEST_COLLECT_ITEM", "QUEST_USE_ITEM") \
                and not fields.get("argument", "").startswith("ITEM_"):
            fail(name, "item objective requires an item ID")
        if fields.get("objectiveCount") == "2":
            secondary = fields.get("secondaryCondition")
            if secondary not in {"QUEST_VISIT", "QUEST_PARTY_EITHER", "QUEST_PARTY_BOTH",
                                 "QUEST_ITEM_OR_FOSSIL", "QUEST_REVIVED_FOSSIL",
                                 "QUEST_DEX_CAUGHT", "QUEST_COLLECT_ITEM", "QUEST_LEGENDS",
                                 "QUEST_DEX_SEEN", "QUEST_BADGES", "QUEST_ACHIEVEMENT"}:
                fail(name, "secondary objective must be checkable without a transient event")
            if "secondaryArgument" not in fields:
                fail(name, "missing secondary objective argument")
        elif any(field in fields for field in ("secondaryCondition", "secondaryArgument", "secondaryAlternative")):
            fail(name, "secondary objective fields require objectiveCount = 2")
        for symbol in re.findall(r"\b(?:ITEM|SPECIES|MAP|TRAINER|RF_QUEST_EVENT)_\w+", body):
            if symbol not in known:
                fail(name, f"unknown reference {symbol}")
        for field, text in re.findall(r'\.(name|description)\s*=\s*COMPOUND_STRING\(("(?:[^"\\]|\\.)*")\)', body):
            decoded = ast.literal_eval(text)
            # A coarse text lint; actual font width and panel fit still require native validation.
            if not decoded or any(len(line) > 40 for line in decoded.split("\n")):
                fail(name, f"empty or unusually long {field}")

    links = (ROOT / "src/data/rf_quest_links.h").read_text(encoding="utf-8")
    requirements = {}
    for quest, needs, group in re.findall(r"\{(RF_QUEST_\w+), (RF_QUEST_\w+), (\d+)\}", links):
        if quest == "RF_QUEST_NONE":
            continue
        for symbol in (quest, needs):
            if symbol not in catalogue:
                errors.append(f"requirement {quest}: unknown quest {symbol}")
        if int(group) > 7:
            errors.append(f"requirement {quest}: group must be 0..7")
        requirements.setdefault(quest, []).append(needs)
    for quest, body in re.findall(r"\{\.quest = (RF_QUEST_\w+)(.*?)\}", links, re.S):
        if quest == "RF_QUEST_NONE":
            continue
        fields = dict(re.findall(r"\.(\w+)\s*=\s*([A-Z][A-Z0-9_]*|\d+)", body))
        if quest not in catalogue:
            errors.append(f"bonus {quest}: unknown quest")
        if fields.get("item", "ITEM_NONE") != "ITEM_NONE":
            if fields["item"] not in known:
                errors.append(f"bonus {quest}: unknown item {fields['item']}")
            if not fields.get("itemCount", "").isdigit() or not 1 <= int(fields["itemCount"]) <= 999:
                errors.append(f"bonus {quest}: extra item needs itemCount 1..999")
    for body in re.findall(r"\{(\.script = \w+, \.quest = RF_QUEST_\w+[^}]*)\}", links):
        fields = dict(re.findall(r"\.(\w+)\s*=\s*(\w+)", body))
        if fields["quest"] == "RF_QUEST_NONE":
            continue
        if fields["quest"] not in catalogue:
            errors.append(f"object {fields['script']}: unknown quest {fields['quest']}")
        if not fields.get("minState", "x").isdigit() or not fields.get("maxState", "x").isdigit() \
                or not int(fields["minState"]) <= int(fields["maxState"]) <= 3:
            errors.append(f"object {fields['script']}: needs 0 <= minState <= maxState <= 3")
    for body in re.findall(r"\{(\.map = MAP_\w+[^}]*)\}", links):
        fields = dict(re.findall(r"\.(\w+)\s*=\s*(\w+)", body))
        if fields.get("map") not in known or fields.get("quest") not in catalogue:
            errors.append(f"tile {fields.get('map')}: unknown map or quest")
        if not all(fields.get(key, "x").isdigit() for key in ("minState", "x", "y")) or int(fields["minState"]) > 3:
            errors.append(f"tile {fields.get('map')}: needs numeric minState (0..3), x and y")
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
    # Extra requirements join the prerequisite graph: no quest may (indirectly) require itself.
    graph = {name: {fields.get("prerequisite", "RF_QUEST_NONE")} | set(requirements.get(name, []))
             for name, fields in catalogue.items()}
    state = {}

    def visit(node):
        if node not in graph or state.get(node) == 2:
            return
        if state.get(node) == 1:
            errors.append(f"{node}: requirement cycle")
            return
        state[node] = 1
        for parent in graph[node]:
            visit(parent)
        state[node] = 2
    for name in graph:
        visit(name)
    return count, errors


if __name__ == "__main__":
    count, errors = validate()
    if errors:
        raise SystemExit("\n".join(errors))
    print(f"Validated {count} quests: IDs, references, prerequisites, rewards and text lint.")
