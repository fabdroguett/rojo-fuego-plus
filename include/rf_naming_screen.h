#ifndef GUARD_RF_NAMING_SCREEN_H
#define GUARD_RF_NAMING_SCREEN_H

#include "main.h"

// Pokémon Ultimate Kanto naming screen: three keyboard pages of 9x4 keys, BORRAR/ESPACIO/LISTO and a
// letter counter. Returns FALSE when it could not allocate, so the caller falls back to the old screen.
bool32 RFNaming_Start(u8 templateNum, u8 *destBuffer, u16 monSpeciesOrPlayerGender, u16 monGender,
                      u32 monPersonality, MainCallback returnCallback);

#endif // GUARD_RF_NAMING_SCREEN_H
