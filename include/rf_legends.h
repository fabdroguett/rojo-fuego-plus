#ifndef GUARD_RF_LEGENDS_H
#define GUARD_RF_LEGENDS_H

#include "constants/rf_legends.h"

struct RFLegend
{
    u16 species;
    u16 quest; // saga mission; the legend is only on its map while this mission is active
    u16 map;   // MAP_* where it waits (shown by the Pokédex area screen)
    const u8 *script;
};

bool32 RFLegends_IsObjectHidden(const u8 *script);
bool32 RFLegends_IsSagaCaught(u32 saga);
void Script_RF_LegendCaught(void);
const struct RFLegend *RFLegends_FindSpecies(u16 species);

#endif // GUARD_RF_LEGENDS_H
