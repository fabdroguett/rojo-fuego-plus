#include "global.h"
#include "event_data.h"
#include "main.h"
#include "mass_outbreak.h"
#include "overworld.h"
#include "random.h"
#include "region_map.h"
#include "script.h"
#include "wild_encounter.h"

static const struct MassOutbreak sPokeOutbreakSpeciesList[OUTBREAK_COUNT] = {
    [OUTBREAK_ID_ROUTE102] = {
        .species = SPECIES_SEEDOT,
        .moves = {MOVE_BIDE, MOVE_HARDEN, MOVE_LEECH_SEED},
        .level = 3,
        .probability = 100,
        .duration = 1,
        .location = MAP_ROUTE102

    },
    [OUTBREAK_ID_ROUTE114] = {
        .species = SPECIES_NUZLEAF,
        .moves = {MOVE_HARDEN, MOVE_GROWTH, MOVE_NATURE_POWER, MOVE_LEECH_SEED},
        .level = 15,
        .probability = 50,
        .duration = 1,
        .location = MAP_ROUTE114,
    },
    [OUTBREAK_ID_ROUTE117] = {
        .species = SPECIES_SEEDOT,
        .moves = {MOVE_HARDEN, MOVE_GROWTH, MOVE_NATURE_POWER, MOVE_LEECH_SEED},
        .level = 13,
        .probability = 50,
        .duration = 1,
        .location = MAP_ROUTE117,
    },
    [OUTBREAK_ID_ROUTE120] = {
        .species = SPECIES_SEEDOT,
        .moves = {MOVE_GIGA_DRAIN, MOVE_FRUSTRATION, MOVE_SOLAR_BEAM, MOVE_LEECH_SEED},
        .level = 25,
        .probability = 50,
        .duration = 1,
        .location = MAP_ROUTE120,
    },
    [OUTBREAK_ID_ROUTE116] = {
        .species = SPECIES_SKITTY,
        .moves = {MOVE_GROWL, MOVE_TACKLE, MOVE_TAIL_WHIP, MOVE_ATTRACT},
        .level = 8,
        .probability = 50,
        .duration = 1,
        .location = MAP_ROUTE116,
    }
};

void ZeroMassOutbreak(void)
{
    gSaveBlock1Ptr->outbreakPokemonSpecies = 0;
    gSaveBlock1Ptr->outbreakPokemonSpecies = 0;
    gSaveBlock1Ptr->outbreakLocationMapNum = 0;
    gSaveBlock1Ptr->outbreakLocationMapGroup = 0;
    gSaveBlock1Ptr->outbreakPokemonLevel = 0;
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
        gSaveBlock1Ptr->outbreakPokemonMoves[i] = 0;
    gSaveBlock1Ptr->outbreakPokemonProbability = 0;
    gSaveBlock1Ptr->outbreakDaysLeft = 0;
}

struct MassOutbreak GetStaticOutbreak(enum MassOutbreakIndex outbreakIdx)
{
    return sPokeOutbreakSpeciesList[outbreakIdx];
}

enum Species GetStaticOutbreakSpecies(enum MassOutbreakIndex outbreakIdx)
{
    return sPokeOutbreakSpeciesList[outbreakIdx].species;
}

u8 *GetStaticOutbreakMapName(u8 *dest, enum MassOutbreakIndex outbreakIdx)
{
    u16 mapGroup = MAP_GROUP(sPokeOutbreakSpeciesList[outbreakIdx].location);
    u16 mapNum = MAP_NUM(sPokeOutbreakSpeciesList[outbreakIdx].location);
    return GetMapName(dest, Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum)->regionMapSectionId, 0);
}

void PrepareTvShowForRandomOutbreak(TVShow *show)
{
    u32 outbreakIdx = RandomUniform(RNG_NONE, 0, ARRAY_COUNT(sPokeOutbreakSpeciesList));
    show->massOutbreak.outbreakIndex = outbreakIdx + 1;
    show->massOutbreak.species = sPokeOutbreakSpeciesList[outbreakIdx].species;
    show->massOutbreak.locationMapNum = MAP_NUM(sPokeOutbreakSpeciesList[outbreakIdx].location);
    show->massOutbreak.locationMapGroup = MAP_GROUP(sPokeOutbreakSpeciesList[outbreakIdx].location);
}

void StartMassOutbreak(struct MassOutbreak outbreak)
{
    gSaveBlock1Ptr->outbreakPokemonSpecies = outbreak.species;
    gSaveBlock1Ptr->outbreakLocationMapNum = MAP_NUM(outbreak.location);
    gSaveBlock1Ptr->outbreakLocationMapGroup = MAP_GROUP(outbreak.location);
    gSaveBlock1Ptr->outbreakPokemonLevel = outbreak.level;
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        gSaveBlock1Ptr->outbreakPokemonMoves[i] = outbreak.moves[i];
    }
    gSaveBlock1Ptr->outbreakPokemonProbability = outbreak.probability;
    gSaveBlock1Ptr->outbreakDaysLeft = outbreak.duration;
}

void StartStaticMassOutbreak(enum MassOutbreakIndex outbreakIdx)
{
    StartMassOutbreak(sPokeOutbreakSpeciesList[outbreakIdx]);
}

void UpdateMassOutbreakDaysLeft(u16 days)
{
    if (gSaveBlock1Ptr->outbreakDaysLeft <= days)
        gSaveBlock1Ptr->outbreakDaysLeft = 0;
    else
        gSaveBlock1Ptr->outbreakDaysLeft -= days;
}

bool32 IsMassOutbreakActive(void)
{
    return (gSaveBlock1Ptr->outbreakDaysLeft > 0);
}

#if IS_FRLG
// Rojo Fuego+: one Kanto outbreak a day, each bringing a Pokémon from a later generation.
// Only areas the player has reached (required flag set) are picked.
struct KantoOutbreak
{
    u16 map;
    enum Species species;
    u8 level;
    u16 requiredFlag;
};

static const struct KantoOutbreak sKantoOutbreaks[] =
{
    {MAP_ROUTE1,                      SPECIES_SKWOVET,     4, FLAG_SYS_POKEDEX_GET},
    {MAP_ROUTE22,                     SPECIES_BIDOOF,      5, FLAG_SYS_POKEDEX_GET},
    {MAP_ROUTE2,                      SPECIES_LECHONK,     6, FLAG_SYS_POKEDEX_GET},
    {MAP_VIRIDIAN_FOREST,             SPECIES_BLIPBUG,     6, FLAG_SYS_POKEDEX_GET},
    {MAP_ROUTE3,                      SPECIES_STARLY,      9, FLAG_BADGE01_GET},
    {MAP_MT_MOON_1F,                  SPECIES_BRONZOR,    12, FLAG_BADGE01_GET},
    {MAP_ROUTE4,                      SPECIES_ROOKIDEE,   10, FLAG_BADGE01_GET},
    {MAP_ROUTE24,                     SPECIES_FLABEBE,    13, FLAG_BADGE01_GET},
    {MAP_ROUTE25,                     SPECIES_FLETCHLING, 13, FLAG_BADGE01_GET},
    {MAP_ROUTE5,                      SPECIES_YAMPER,     16, FLAG_BADGE02_GET},
    {MAP_ROUTE6,                      SPECIES_SHINX,      16, FLAG_BADGE02_GET},
    {MAP_ROUTE9,                      SPECIES_NICKIT,     18, FLAG_BADGE03_GET},
    {MAP_ROUTE10,                     SPECIES_PAWMI,      20, FLAG_BADGE03_GET},
    {MAP_ROCK_TUNNEL_1F,              SPECIES_ROLYCOLY,   20, FLAG_BADGE03_GET},
    {MAP_ROUTE8,                      SPECIES_PURRLOIN,   22, FLAG_BADGE03_GET},
    {MAP_ROUTE7,                      SPECIES_FIDOUGH,    22, FLAG_BADGE03_GET},
    {MAP_ROUTE11,                     SPECIES_TANDEMAUS,  22, FLAG_BADGE03_GET},
    {MAP_ROUTE12,                     SPECIES_SMOLIV,     25, FLAG_BADGE04_GET},
    {MAP_ROUTE13,                     SPECIES_BUNEARY,    25, FLAG_BADGE04_GET},
    {MAP_SAFARI_ZONE_CENTER,          SPECIES_DEERLING,   25, FLAG_BADGE04_GET},
    {MAP_ROUTE14,                     SPECIES_NYMBLE,     27, FLAG_BADGE04_GET},
    {MAP_ROUTE15,                     SPECIES_TINKATINK,  28, FLAG_BADGE04_GET},
    {MAP_ROUTE16,                     SPECIES_NOCTOWL,    28, FLAG_BADGE04_GET},
    {MAP_ROUTE18,                     SPECIES_WINGULL,    28, FLAG_BADGE04_GET},
    {MAP_ROUTE21_NORTH,               SPECIES_SANDYGAST,  30, FLAG_BADGE06_GET},
    {MAP_ONE_ISLAND_KINDLE_ROAD,      SPECIES_LITLEO,     33, FLAG_BADGE07_GET},
    {MAP_THREE_ISLAND_BERRY_FOREST,   SPECIES_PETILIL,    35, FLAG_BADGE07_GET},
    {MAP_ROUTE23,                     SPECIES_RIOLU,      40, FLAG_BADGE08_GET},
    {MAP_FIVE_ISLAND_MEADOW,          SPECIES_ELECTRIKE,  35, FLAG_IS_CHAMPION},
    {MAP_SIX_ISLAND_WATER_PATH,       SPECIES_ZORUA,      38, FLAG_IS_CHAMPION},
    {MAP_SEVEN_ISLAND_SEVAULT_CANYON, SPECIES_AXEW,       40, FLAG_IS_CHAMPION},
};

void TryStartKantoOutbreak(void)
{
    u32 i, count = 0;
    const struct KantoOutbreak *picked = NULL;

    if (IsMassOutbreakActive())
        return;

    // Pick uniformly among the unlocked entries in a single pass
    for (i = 0; i < ARRAY_COUNT(sKantoOutbreaks); i++)
    {
        if (!FlagGet(sKantoOutbreaks[i].requiredFlag))
            continue;
        if (Random() % ++count == 0)
            picked = &sKantoOutbreaks[i];
    }
    if (picked == NULL)
        return;

    struct MassOutbreak outbreak =
    {
        .species = picked->species,
        .moves = {MOVE_NONE}, // keep the level-up moveset
        .level = picked->level,
        .probability = 50,
        .duration = 1,
        .location = picked->map,
    };
    StartMassOutbreak(outbreak);
}
#endif

bool8 SetUpMassOutbreakEncounter(u8 flags)
{
    if (flags & WILD_CHECK_REPEL && !IsWildLevelAllowedByRepel(gSaveBlock1Ptr->outbreakPokemonLevel))
        return FALSE;

    CreateWildMon(gSaveBlock1Ptr->outbreakPokemonSpecies, gSaveBlock1Ptr->outbreakPokemonLevel);
    // Rojo Fuego+: outbreaks without a fixed moveset keep the one CreateWildMon gave
    if (gSaveBlock1Ptr->outbreakPokemonMoves[0] != MOVE_NONE)
    {
        for (u32 i = 0; i < MAX_MON_MOVES; i++)
            SetMonMoveSlot(&gParties[B_TRAINER_OPPONENT_A][0], gSaveBlock1Ptr->outbreakPokemonMoves[i], i);
    }

    return TRUE;
}

bool8 DoMassOutbreakEncounterTest(void)
{
    if (gSaveBlock1Ptr->outbreakDaysLeft == 0)
        return FALSE;

    if (gSaveBlock1Ptr->location.mapNum != gSaveBlock1Ptr->outbreakLocationMapNum || gSaveBlock1Ptr->location.mapGroup != gSaveBlock1Ptr->outbreakLocationMapGroup)
        return FALSE;

    return (RandomPercentage(RNG_NONE,  gSaveBlock1Ptr->outbreakPokemonProbability));
}

void ScrCmd_setstaticoutbreak(struct ScriptContext *ctx)
{
    enum MassOutbreakIndex outbreakIdx = VarGet(ScriptReadHalfword(ctx));
    bool32 noReplacement = ScriptReadByte(ctx);

    if (outbreakIdx == OUTBREAK_COUNT)
        return;

    assertf(!IsMassOutbreakActive() || noReplacement == FALSE, "Trying to start a new outbreak while one is already in progress")
    {
        return;
    }

    assertf(outbreakIdx < OUTBREAK_COUNT, "Trying to read undefined outbreak index %d", outbreakIdx)
    {
        return;
    }
    StartStaticMassOutbreak(outbreakIdx);
}

static u32 ReadMapFromContext(struct ScriptContext *ctx)
{
    u32 map = ScriptReadHalfword(ctx);
    if (map == MAP_UNDEFINED)
        return MAP_UNDEFINED;
    return VarGet(map);
}

void ScrCmd_setdynamicoutbreak(struct ScriptContext *ctx)
{
    enum Species species = VarGet(ScriptReadHalfword(ctx));
    u32 level = VarGet(ScriptReadHalfword(ctx));
    enum Move moves[MAX_MON_MOVES];
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
        moves[i] = VarGet(ScriptReadHalfword(ctx));
    u32 probability = VarGet(ScriptReadHalfword(ctx));
    u32 daysLeft = VarGet(ScriptReadHalfword(ctx));
    u32 map = ReadMapFromContext(ctx);
    bool32 noReplacement = ScriptReadByte(ctx);

    assertf(!IsMassOutbreakActive() || noReplacement == FALSE, "Trying to start a new outbreak while one is already in progress")
    {
        return;
    }

    assertf(IsSpeciesEnabled(species), "Trying to call setdynamicoutbreak with unknown species %d", species)
    {
        return;
    }
    assertf(level >= 1 && level <= MAX_LEVEL, "Trying to call setdynamicoutbreak with invalid level %d", level)
    {
        return;
    }
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        assertf(moves[i] < MOVES_COUNT, "Trying to call setdynamicoutbreak with invalid move %d", moves[i])
        {
            return;
        }
    }
    assertf(probability <= 100, "Trying to call setdynamicoutbreak with invalid percent chance %d", probability)
    {
        return;
    }

    gSaveBlock1Ptr->outbreakPokemonSpecies = species;
    gSaveBlock1Ptr->outbreakPokemonLevel = level;
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        gSaveBlock1Ptr->outbreakPokemonMoves[i] = moves[i];
    }
    gSaveBlock1Ptr->outbreakPokemonProbability = probability;
    gSaveBlock1Ptr->outbreakDaysLeft = daysLeft;
    gSaveBlock1Ptr->outbreakLocationMapNum = MAP_NUM(map);
    gSaveBlock1Ptr->outbreakLocationMapGroup = MAP_GROUP(map);
}

void ScrCmd_editoutbreak(struct ScriptContext *ctx)
{
    enum Species species = VarGet(ScriptReadHalfword(ctx));
    u32 level = VarGet(ScriptReadHalfword(ctx));
    enum Move moves[MAX_MON_MOVES];
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        moves[i] = VarGet(ScriptReadHalfword(ctx));
    }
    u32 probability = VarGet(ScriptReadHalfword(ctx));
    u32 daysLeft = VarGet(ScriptReadHalfword(ctx));
    u32 map = ReadMapFromContext(ctx);

    assertf(IsMassOutbreakActive(), "Trying to edit outbreak but none is active")
    {
        return;
    }

    if (species != SPECIES_NONE)
    {
        if (IsSpeciesEnabled(species))
            gSaveBlock1Ptr->outbreakPokemonSpecies = species;
        else
            errorf("Trying to call editoutbreak with unknown species %d", species);
    }

    if (level != 0)
    {
        if (level <= MAX_LEVEL)
            gSaveBlock1Ptr->outbreakPokemonLevel = level;
        else
            errorf("Trying to call editoutbreak with invalid level %d", level);
    }

    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        if (moves[i] != MOVES_COUNT)
        {
            if (moves[i] < MOVES_COUNT)
                gSaveBlock1Ptr->outbreakPokemonMoves[i] = moves[i];
            else
                errorf("Trying to call editoutbreak with invalid move %d", moves[i]);
        }
    }

    if (probability != 0xFF)
    {
        if (probability <= 100)
            gSaveBlock1Ptr->outbreakPokemonProbability = probability;
        else
            errorf("Trying to call editoutbreak with invalid percent chance %d", probability);
    }

    if (daysLeft != 0x3FFF)
        gSaveBlock1Ptr->outbreakDaysLeft = daysLeft;

    if (map != MAP_UNDEFINED)
    {
        gSaveBlock1Ptr->outbreakLocationMapNum = MAP_NUM(map);
        gSaveBlock1Ptr->outbreakLocationMapGroup = MAP_GROUP(map);
    }
}

void ScrCmd_checkhasactiveoutbreak(struct ScriptContext *ctx)
{
    gSpecialVar_Result = IsMassOutbreakActive();
}

void ScrCmd_getmassoutbreakdata(struct ScriptContext *ctx)
{
    u32 varId = ScriptReadHalfword(ctx);
    enum MassOutbreakData dataField = ScriptReadByte(ctx);
    u32 value = 0;
    switch (dataField)
    {
    case OUTBREAK_DATA_SPECIES:
        value = gSaveBlock1Ptr->outbreakPokemonSpecies;
        break;
    case OUTBREAK_DATA_MOVE1:
        value = gSaveBlock1Ptr->outbreakPokemonMoves[0];
        break;
    case OUTBREAK_DATA_MOVE2:
        value = gSaveBlock1Ptr->outbreakPokemonMoves[1];
        break;
    case OUTBREAK_DATA_MOVE3:
        value = gSaveBlock1Ptr->outbreakPokemonMoves[2];
        break;
    case OUTBREAK_DATA_MOVE4:
        value = gSaveBlock1Ptr->outbreakPokemonMoves[3];
        break;
    case OUTBREAK_DATA_LEVEL:
        value = gSaveBlock1Ptr->outbreakPokemonLevel;
        break;
    case OUTBREAK_DATA_PROBABILITY:
        value = gSaveBlock1Ptr->outbreakPokemonProbability;
        break;
    case OUTBREAK_DATA_DAYS_LEFT:
        value = gSaveBlock1Ptr->outbreakDaysLeft;
        break;
    case OUTBREAK_DATA_MAP:
        value = gSaveBlock1Ptr->outbreakLocationMapNum | (gSaveBlock1Ptr->outbreakLocationMapGroup << 8);
        break;
    }
    VarSet(varId, value);
}

