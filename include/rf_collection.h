#ifndef GUARD_RF_COLLECTION_H
#define GUARD_RF_COLLECTION_H

#include "wild_encounter.h"

#define RF_COLLECTION_METHODS 5
struct RFCollectionCount { u16 caught; u16 total; };
struct RFCollectionSummary
{
    struct RFCollectionCount methods[RF_COLLECTION_METHODS];
    struct RFCollectionCount total;
};
// TIMES_OF_DAY_COUNT requests the union of all effective time tables.
void RFCollection_Count(const struct WildPokemonHeader *header, u32 time, struct RFCollectionSummary *summary);

#endif
