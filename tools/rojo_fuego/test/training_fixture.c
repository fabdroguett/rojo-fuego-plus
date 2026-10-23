// Disposable native-menu fixture. Never include in a delivered ROM.
#include "global.h"
#include "main.h"
#include "pokemon.h"
#include "event_data.h"
#include "overworld.h"
#include "script.h"
#include "constants/flags.h"

extern const u8 RF_EventScript_HyperTrainer[];
EWRAM_DATA u32 gRFTrainingFixtureResult = 0;

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
