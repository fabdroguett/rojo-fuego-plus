// Disposable native-menu fixture. Never include in a delivered ROM.
#include "global.h"
#include "main.h"
#include "pokemon.h"
#include "event_data.h"
#include "overworld.h"
#include "script.h"
#include "constants/flags.h"
#include "constants/moves.h"

extern const u8 RF_EventScript_HyperTrainer[];
EWRAM_DATA u32 gRFTrainingFixtureResult = 0;
EWRAM_DATA u32 gRFTrainingFixtureGym = 0;
EWRAM_DATA u32 gRFTrainingFixtureGymLocked = 0;

extern const u8 PewterCity_Gym_EventScript_GymStatuePostVictory[];
extern const u8 CeruleanCity_Gym_EventScript_GymStatuePostVictory[];
extern const u8 VermilionCity_Gym_EventScript_GymStatuePostVictory[];
extern const u8 CeladonCity_Gym_EventScript_GymStatuePostVictory[];
extern const u8 FuchsiaCity_Gym_EventScript_GymStatuePostVictory[];
extern const u8 SaffronCity_Gym_EventScript_GymStatuePostVictory[];
extern const u8 CinnabarIsland_Gym_EventScript_GymStatuePostVictory[];
extern const u8 ViridianCity_Gym_EventScript_GymStatuePostVictory[];

static const u16 sGymMoves[] = {MOVE_ROCK_SLIDE, MOVE_WATER_PULSE, MOVE_THUNDER_PUNCH,
    MOVE_SEED_BOMB, MOVE_GUNK_SHOT, MOVE_ZEN_HEADBUTT, MOVE_HEAT_WAVE, MOVE_EARTH_POWER};

void RFTrainingFixture_GymTutor(void)
{
    static const u8 *const scripts[] = {
        PewterCity_Gym_EventScript_GymStatuePostVictory,
        CeruleanCity_Gym_EventScript_GymStatuePostVictory,
        VermilionCity_Gym_EventScript_GymStatuePostVictory,
        CeladonCity_Gym_EventScript_GymStatuePostVictory,
        FuchsiaCity_Gym_EventScript_GymStatuePostVictory,
        SaffronCity_Gym_EventScript_GymStatuePostVictory,
        CinnabarIsland_Gym_EventScript_GymStatuePostVictory,
        ViridianCity_Gym_EventScript_GymStatuePostVictory,
    };
    u32 i;
    if (gRFTrainingFixtureGym >= ARRAY_COUNT(scripts))
        return;
    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_MEW, 30, 31);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_SPLASH, 0);
    for (i = 1; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_NONE, i);
    CalculatePlayerPartyCount();
    for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; i++)
        FlagClear(i);
    if (!gRFTrainingFixtureGymLocked)
        FlagSet(FLAG_BADGE01_GET + gRFTrainingFixtureGym);
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_INSTANT;
    SetMainCallback1(NULL);
    SetMainCallback2(CB2_Overworld);
    ScriptContext_SetupScript(scripts[gRFTrainingFixtureGym]);
}

void RFTrainingFixture_CheckGym(void)
{
    u32 i;
    bool32 learned = FALSE;
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MOVE1 + i) == sGymMoves[gRFTrainingFixtureGym])
            learned = TRUE;
    }
    gRFTrainingFixtureResult = gRFTrainingFixtureGymLocked ? !learned : learned;
    SetMainCallback2(CB2_Overworld);
}

void RFTrainingFixture_Begin(void)
{
    u32 iv = 15, on = TRUE, ev = 252;
    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_MEWTWO, 30, 31);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP_IV, &iv);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HYPER_TRAINED_HP, &on);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_ATK_EV, &ev);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_DEF_EV, &ev);
    CalculateMonStats(&gParties[B_TRAINER_PLAYER][0]);
    CalculatePlayerPartyCount();
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_INSTANT;
    SetMainCallback1(NULL);
    SetMainCallback2(CB2_Overworld);
    ScriptContext_SetupScript(RF_EventScript_HyperTrainer);
}

void RFTrainingFixture_IV(void)
{
    SetMainCallback2(CB2_Overworld);
    ScriptContext_SetupScript(RF_EventScript_HyperTrainer);
}

void RFTrainingFixture_CheckPreview(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    gRFTrainingFixtureResult = GetMonData(mon, MON_DATA_HP_EV) == 0
        && GetMonData(mon, MON_DATA_HP_IV) == 15
        && GetMonData(mon, MON_DATA_HYPER_TRAINED_HP);
    SetMainCallback2(CB2_Overworld);
}

void RFTrainingFixture_CheckEV(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    gRFTrainingFixtureResult = (GetMonData(mon, MON_DATA_HP_EV) == 6)
        | ((GetMonData(mon, MON_DATA_ATK_EV) == 252 && GetMonData(mon, MON_DATA_DEF_EV) == 252) << 1)
        | ((GetMonData(mon, MON_DATA_HP_IV) == 15) << 2)
        | ((GetMonData(mon, MON_DATA_HYPER_TRAINED_HP) != 0) << 3);
    SetMainCallback2(CB2_Overworld);
}

void RFTrainingFixture_CheckIV(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    gRFTrainingFixtureResult = (GetMonData(mon, MON_DATA_HP_IV) == 0)
        | ((GetMonData(mon, MON_DATA_HYPER_TRAINED_HP) == 0) << 1)
        | ((GetMonData(mon, MON_DATA_HP_EV) == 6) << 2)
        | ((GetMonData(mon, MON_DATA_SPECIES) == SPECIES_MEWTWO) << 3);
    SetMainCallback2(CB2_Overworld);
}
