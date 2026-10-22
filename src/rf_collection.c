#include "global.h"
#include "rf_collection.h"
#include "pokedex.h"
#include "pokemon.h"

#define COLLECTION_DEX_BYTES ((NATIONAL_DEX_COUNT + 8) / 8)

static const struct WildPokemonInfo *GetMethod(const struct WildEncounterTypes *types, u32 method)
{
    switch (method)
    {
    case 0: return types->landMonsInfo;
    case 1: return types->waterMonsInfo;
    case 2: return types->rockSmashMonsInfo;
    case 3: return types->fishingMonsInfo;
    case 4: return types->hiddenMonsInfo;
    }
    return NULL;
}

static void AddSpecies(u8 *seen, enum Species species, struct RFCollectionCount *count)
{
    u32 dex;
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return;
    dex = SpeciesToNationalPokedexNum(species);
    if (dex == 0 || dex > NATIONAL_DEX_COUNT || (seen[dex / 8] & (1u << (dex % 8))))
        return;
    seen[dex / 8] |= 1u << (dex % 8);
    count->total++;
    if (GetSetPokedexFlag(dex, FLAG_GET_CAUGHT))
        count->caught++;
}

void RFCollection_Count(const struct WildPokemonHeader *header, u32 time, struct RFCollectionSummary *summary)
{
    static const u8 slots[RF_COLLECTION_METHODS] =
    {
        NUM_LAND_MONS_ENCOUNTER_SLOTS, NUM_WATER_MONS_ENCOUNTER_SLOTS,
        NUM_ROCK_SMASH_MONS_ENCOUNTER_SLOTS, NUM_FISHING_MONS_ENCOUNTER_SLOTS,
        NUM_HIDDEN_MONS_ENCOUNTER_SLOTS,
    };
    u8 totalSeen[COLLECTION_DEX_BYTES] = {0};
    u8 methodSeen[COLLECTION_DEX_BYTES];
    u32 method, t, i;
    memset(summary, 0, sizeof(*summary));
    if (header == NULL || time > TIMES_OF_DAY_COUNT)
        return;
    for (method = 0; method < RF_COLLECTION_METHODS; method++)
    {
        memset(methodSeen, 0, sizeof(methodSeen));
        for (t = 0; t < TIMES_OF_DAY_COUNT; t++)
        {
            const struct WildPokemonInfo *info;
            u32 effective = OW_TIME_OF_DAY_ENCOUNTERS ? t : TIME_OF_DAY_DEFAULT;
            if (time != TIMES_OF_DAY_COUNT && t != time)
                continue;
            info = GetMethod(&header->encounterTypes[effective], method);
            if (info == NULL && OW_TIME_OF_DAY_ENCOUNTERS && !OW_TIME_OF_DAY_DISABLE_FALLBACK)
                info = GetMethod(&header->encounterTypes[OW_TIME_OF_DAY_FALLBACK], method);
            if (info == NULL || info->wildPokemon == NULL || info->encounterRate == 0)
                continue;
            for (i = 0; i < slots[method]; i++)
            {
                enum Species species = info->wildPokemon[i].species;
                AddSpecies(methodSeen, species, &summary->methods[method]);
                AddSpecies(totalSeen, species, &summary->total);
            }
        }
    }
}
