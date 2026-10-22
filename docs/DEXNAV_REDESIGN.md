# DexNav and dark theme redesign

Mode: Operate. Question: which local Pokémon can I search for, and what do I know about it?
The user granted design freedom; priority is quick searching, retaining collection.

## Direction contract

THESIS: a compact field instrument for Kanto; preserve Pokémon sprites and native navigation while
replacing the striped, label-heavy backdrop and overlapping route/control header.
OWN-WORLD: ink surfaces, warm white text, restrained FireRed accent, mint capture marks; light mode
uses pale slate surfaces with dark ink. Native bitmap type, no glow or gradients.
STORY: locate encounter group, choose Pokémon, read search readiness and search; collection is one
START press away, with time navigation visible in that view.
FIRST VIEWPORT: 240x160; title/route at top, encounters on the left, data on the right, a continuous
control footer. A name never shares a line with the route. Empty slots and unseen Pokémon differ.
FORM: code-first native GBA interface, preserving actual sprites; seed: none (native platform, no
applicable web concept catalog). Signature interaction: selection updates panel and group emphasis.
FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict,
DESIGN.md, and every shipping raster carrying its provenance.

## Validation scope

Light/dark, known/unseen species, empty area, collection, return, registration/filter changes;
wild battle party opening/cancellation/switch with speed mode on, then off. All fixture-only code
must be absent from the delivered ROM. Existing asset sprites retain project provenance.

## Result

Independent finish review: `ship`, no material findings, 44 native captures in both themes.
The fixed GBA viewport is captured at integer 2x scale (480x320). The scope covers DexNav and
its collection/registration/filter/empty/unseen states, summary pages, bag, quests, Key System,
PC and party flows. This is not a claim that every minigame or menu was rebuilt.
Additional runtime checks pass opening/cancelling/switching at normal and fast speed and replacing
a fainted Pokémon in both themes. The active party index is checked after each switch.
The original user's freeze did not reproduce; the callback guard prevents an identified unsafe
handoff, without claiming that it establishes the original root cause.
