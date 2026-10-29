#ifndef GUARD_NAMING_SCREEN_H
#define GUARD_NAMING_SCREEN_H

#include "main.h"
#include "constants/species.h"

enum {
    NAMING_SCREEN_PLAYER,
    NAMING_SCREEN_BOX,
    NAMING_SCREEN_CAUGHT_MON,
    NAMING_SCREEN_NICKNAME,
    NAMING_SCREEN_WALDA,
    NAMING_SCREEN_CODE,
    NAMING_SCREEN_RIVAL,
};

extern void BattleMainCB2(void);

void DoNamingScreen(u8 templateNum, u8 *destBuffer, u16 monSpeciesOrPlayerGender, u16 monGender, u32 monPersonality, MainCallback returnCallback);
// Portrait of whoever is being named (player, rival or Pokémon); used by the Pokémon Ultimate Kanto screen.
u8 NamingScreen_CreatePortrait(u8 templateNum, u16 monSpeciesOrPlayerGender, u32 monPersonality, s16 x, s16 y);

#endif // GUARD_NAMING_SCREEN_H
