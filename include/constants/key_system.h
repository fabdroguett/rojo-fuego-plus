#ifndef GUARD_CONSTANTS_KEY_SYSTEM_H
#define GUARD_CONSTANTS_KEY_SYSTEM_H

// Rojo Fuego+ Key System: game settings the player can switch on and off in game.
// Every key is on by default; the order here is the order of the in-game menu.
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
    KEY_DIFFICULTY, // Not an on/off key: cycles through enum KeyDifficulty
    KEY_COUNT
};

enum KeyDifficulty
{
    KEY_DIFFICULTY_NORMAL, // 0 so that older saves and new games start on Normal
    KEY_DIFFICULTY_EASY,
    KEY_DIFFICULTY_CHALLENGE,
    KEY_DIFFICULTY_COUNT
};

#endif // GUARD_CONSTANTS_KEY_SYSTEM_H
