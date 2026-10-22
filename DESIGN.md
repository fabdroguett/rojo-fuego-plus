---
name: Rojo Fuego Plus
description: Native Kanto field instrument and shared dark UI palette
colors:
  fire-accent: "#f76352"
  night-ground: "#191921"
  night-panel: "#292931"
  night-edge: "#4a4a52"
  night-text: "#f7efde"
  night-muted: "#b5b5ad"
  night-shadow: "#101019"
  capture-mint: "#73dead"
  warm-indicator: "#f7c56b"
  day-ground: "#e6efe6"
  day-panel: "#fffff7"
  day-text: "#212931"
  day-shadow: "#dee6de"
  day-muted: "#5a636b"
  day-edge: "#a5adad"
  day-capture: "#217b52"
  day-accent: "#bd2921"
  day-indicator: "#8c5a10"
typography:
  label:
    fontFamily: "GBA FONT_SMALL bitmap"
rounded:
  square: "0px"
components:
  dexnav-canvas:
    backgroundColor: "{colors.night-ground}"
    textColor: "{colors.night-text}"
    width: "240px"
    height: "160px"
  dexnav-panel:
    backgroundColor: "{colors.night-panel}"
    textColor: "{colors.night-text}"
    rounded: "{rounded.square}"
---

# Design System: Rojo Fuego Plus

## Overview

**Creative North Star: "Kanto field instrument"**

A compact field instrument for Kanto: dense, readable native screens with ink surfaces, warm text, restrained FireRed selection accents and unchanged Pokémon imagery. The visual system belongs to a 240 × 160 Game Boy Advance framebuffer, not a browser.

This is a source-derived record of the implemented design. DexNav receives a new full-canvas composition; other principal interfaces receive targeted palette mappings. The bounded finish review accepted all 44 required dark/light native captures in `.impeccable/review` with a ship disposition, persistence pass, fidelity match and no remaining material findings. These 480 × 320 captures display the fixed 240 × 160 framebuffer at integer 2× scale. Dark mode is the product default with persistent Key System opt-out. This does not establish that every game interface was redesigned or verified.

**Key Characteristics:**
- Native bitmap type and indexed palettes.
- Selection and actions remain explicit at 240 × 160.
- Existing sprite, type, HP and status semantics survive theme changes.

## Colors

### Primary

Fire accent identifies selected UI details. Capture mint and warm indicator are supporting indexed UI colors; they must not replace existing gameplay color semantics.

### Neutral

Night ground anchors the canvas, night panel carries content, and night edge separates regions. Night text is warm and bright; night muted carries secondary information. Night shadow is the bitmap text shadow color, not a blurred surface effect.

The frontmatter hex values are documentation previews of the canonical `RGB(r,g,b)` 5-bit channels in `include/rf_ui_theme.h` and `src/rf_ui_theme.c`, expanded by round(channel × 255 / 31). The C palette definitions remain authoritative. The day tokens describe `RFUI_LoadPalette` used by DexNav; other screens retain their incumbent day palettes.

**The Semantic Palette Rule.** Change UI surfaces through known indexed entries; preserve sprite, type, HP and status palettes.

`RFUI_LoadWindowPalette` copies the incumbent palette and, only in dark mode, replaces indices 1, 2 and 3 with panel, text and shadow. It preserves other entries used for colored words and frames. Battle, bag, party, storage, summary and shop use dedicated indexed mappings, not a universal full-palette replacement.

## Typography

DexNav chrome and collection text use the engine's `FONT_SMALL` bitmap face; detail lines start with `FONT_SMALL`, switch to `FONT_SMALL_NARROWER` above 72 pixels, then truncate to 60 pixels before appending an ellipsis if still too wide; its field-search arrow uses `FONT_NORMAL`. Other menus retain incumbent engine font roles. These are fixed raster glyphs with engine metrics, not scalable font families or web typography. Use `GetStringRightAlignXOffset` and font metrics for alignment; do not infer a CSS font size or weight. DexNav detail and collection lines commonly advance by 12 native pixels. A bitmap shadow provides separation without glow.

## Layout

The fixed framebuffer is 240 × 160 pixels, equivalent to 30 × 20 tiles of 8 × 8 pixels. There are no responsive breakpoints: emulator scaling should preserve the native composition and crisp pixels.

DexNav's `WINDOW_CHROME` covers the full canvas. The left encounter surface occupies x=0..161 from y=16 for 132 pixels. A one-pixel divider is drawn at x=163, y=20 for 124 pixels. The detail window starts at tile (21,3), with 9 × 15 tiles. Header window starts at tile (1,0), with 29 × 2 tiles. Water, land and hidden group headers begin at y=16, 53 and 113; the active group receives edge-tone emphasis. The continuous footer begins its text at (4,148). These measurements describe DexNav only, not a requirement for other screens.

## Elevation & Depth

Flat indexed surfaces use tonal separation and single-pixel edges. There are no blurred shadows, gradients, glass layers or CSS elevation effects. Text shadow remains a glyph-rendering role. Preserve established engine fades and sprite animations; this documentation introduces no motion timing tokens.

## Shapes

DexNav chrome uses rectangular, pixel-aligned fills and a single-pixel divider. Existing menu frame tiles keep their incumbent silhouettes. Do not round or rescale pixel geometry through web conventions.

## Components

### DexNav encounter groups

Three labelled regions retain water, land and hidden encounter slots, existing Pokémon icons and collection counts. Active group emphasis follows cursor selection. An empty group prints `Sin encuentros`; unseen detail labels (`Sin avistar`, `Aun no visto`) remain distinct from absent encounters (`Sin datos`, `Sin encuentro`). The selector is hidden for an absent raw species. Icon centers use y=40 for water, y=77/101 for land and y=137 for hidden encounters; the authored 32 × 32 corner selector has visible local bounds 5..26, keeping its corners clear of group headers.

### DexNav detail panel

The right panel follows selection and shows species/search readiness, registration, available ability information and hunter filter. The filter line begins at local y=108 inside the 120-pixel-high detail window. Retain Pokédex visibility rules and real encounter data. Collection uses the same panel with method/time counts; START returns and L/R change time there.

### Header and footer

Header identifies DexNav or roamer mode and right-aligns the route. The footer exposes A search, R registration, SELECT filter, START collection/area and B exit. D-pad moves the selection. These are native hardware actions; there is no hover state or mouse focus model.

### Shared text windows and menus

Message, standard, sign and selected user frames use the shared palette loader. Battle text/action menus, bag, party, storage, summary and shop contain targeted night mappings. Party mappings also handle palette entries copied when the cursor moves. These changes share material and contrast roles while retaining each interface's established structure. Reviewed native captures cover DexNav, collection, empty/unseen species, registration/filter, summary/stats/moves, bag, quests, keys, PC and party opening/cancellation/switching in dark and light modes. Schematic HTML previews in the sidecar remain documentation aids; the native captures are the visual evidence. The defensive battle handoff fix passes mGBA party opening/cancellation/switching checks with fast mode, but the original user-reported freeze was not reproduced, so it is not proven resolved by reproduction.

## Do's and Don'ts

- Do use shared night tokens for UI surfaces and text.
- Do measure strings with the engine font helpers and check actual native frames.
- Do distinguish no encounters, unseen species and known species.
- Do retain search, registration, filters, collection, visibility rules and save behavior.
- Don't recolor Pokémon sprites, type icons, HP bars or status meanings with surface tokens.
- Don't introduce browser layout, scalable fonts, hover-only actions or touch controls.
- Don't claim every screen has been redesigned or visually verified.
- Don't create new generated raster assets for this theme.
