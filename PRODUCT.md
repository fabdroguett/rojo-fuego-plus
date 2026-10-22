# Product

<!-- impeccable:product-schema 1 -->

## Platform

Native Game Boy Advance, 240 x 160 pixels (outside the web/iOS/Android skill taxonomy).

## Users

Confirmed: the owner requests mGBA on Windows and grants design freedom across the main interfaces.
Searching is the chosen DexNav priority; collection and detailed species information remain available.
The emulator/save involved in the original reported freeze is still unknown.

## Product Purpose

Rojo Fuego Plus expands FireRed with optional conveniences, quests and existing-species forms.
The current request replaces the DexNav interface, improves the principal menu surfaces and dark mode,
and investigates a freeze when opening the party selector during a wild battle.

## Capabilities and Constraints

Native C, tilemaps, indexed palettes and sprites. No HTML, CSS, scalable web fonts or touch UI.
Preserve search, registration, hunter filters, species information, collection by method/time,
Pokédex visibility rules, saves and battle choices. All boolean features enabled by default,
with persistent opt-out in Key System. Difficulty selector remains Normal. Never fabricate data.
ROM generation is authorized; source goes to GitHub and ROMs stay local. Back up prior ROMs.

## Evidence on Hand

Source in src/dexnav.c, src/party_menu.c, src/item_menu.c and src/battle_bg.c.
Headless mGBA fixture and incumbent screenshot build_artifacts/dex-before.png.
Defensive battle callback-ownership fix tested with menu handoffs. Native mGBA opening, cancellation,
switching and fainted replacement pass; the original user freeze was not reproduced.

## Product Principles

- Readable at native resolution and on a scaled emulator screen.
- Selection, capture state and available actions are explicit.
- Theme changes UI surfaces, not Pokémon or HP/status semantics.
- Verify actual GBA frames, transitions and failure states.
