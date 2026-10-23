#ifndef GUARD_CONSTANTS_KEY_SYSTEM_H
#define GUARD_CONSTANTS_KEY_SYSTEM_H

// Rojo Fuego+ Key System: game settings the player can switch on and off in game.
// Boolean keys are on by default; player overrides are saved.
// the order here is the order of the in-game menu.
enum KeySystemKey
{
    KEY_CAPTURE_EXP,
    KEY_EXP_ALL,
    KEY_SCALED_EXP,
    KEY_LEVEL_CAP,
    KEY_PHYSICAL_SPECIAL_SPLIT,
    KEY_MODERN_CRITS,
    KEY_MODERN_STATUS,
    KEY_ABILITY_WEATHER_TURNS,
    KEY_FOLLOWERS,
    KEY_OW_ENCOUNTERS,
    KEY_HM_FREE,
    KEY_AUTORUN,
    KEY_AUTOSAVE,
    KEY_BATTLE_SPEED,
    KEY_HIDDEN_SPARKLE,
    KEY_DIFFICULTY, // Not an on/off key: cycles through enum KeyDifficulty
    KEY_DARK_MODE,
    KEY_SHINY_ODDS, // Not an on/off key: cycles through enum KeyShinyOdds
    KEY_INFINITE_REPEL,
    KEY_WILD_LOOT, // Optional wild held-item drops.
    KEY_EXP_RESERVE,
    KEY_R_TEXT_FAST,
    KEY_DEXNAV_COLLECTION,
    KEY_RESTORE_TRAINER_BERRIES,
    KEY_COLOR_THEME, // Not an on/off key: cycles through enum KeyColorTheme. Shown after KEY_DARK_MODE.
    KEY_FONT_STYLE,  // Not an on/off key: cycles through enum KeyFontStyle. Shown after KEY_COLOR_THEME.
    KEY_COUNT
};

enum KeyDifficulty
{
    KEY_DIFFICULTY_NORMAL, // 0 so that older saves and new games start on Normal
    KEY_DIFFICULTY_EASY,
    KEY_DIFFICULTY_CHALLENGE,
    KEY_DIFFICULTY_COUNT
};

enum KeyShinyOdds
{
    KEY_SHINY_ODDS_NORMAL, // 1/4096
    KEY_SHINY_ODDS_1024,
    KEY_SHINY_ODDS_512,
    KEY_SHINY_ODDS_256,
    KEY_SHINY_ODDS_COUNT
};

// Colour themes for the Rojo Fuego+ menus (rf_ui_theme.c). Each has a day and a night variant.
enum KeyColorTheme
{
    KEY_COLOR_THEME_MINT, // 0: the original look, so older saves keep it
    KEY_COLOR_THEME_CRIMSON,
    KEY_COLOR_THEME_AZURE,
    KEY_COLOR_THEME_VIOLET,
    KEY_COLOR_THEME_AMBER,
    KEY_COLOR_THEME_GRAPHITE,
    KEY_COLOR_THEME_COUNT
};

// Latin text fonts (rf_fonts.c). Every style but the classic one is built by make_rf_fonts.py.
enum KeyFontStyle
{
    KEY_FONT_M5X7, // 0: the default for saves without the setting
    KEY_FONT_M6X11,
    KEY_FONT_MONOGRAM,
    KEY_FONT_CLASSIC,
    KEY_FONT_STYLE_COUNT
};

#endif // GUARD_CONSTANTS_KEY_SYSTEM_H
