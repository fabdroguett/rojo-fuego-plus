// Disposable emulator fixture. Never include in a delivered ROM.
#include "global.h"
#include "event_data.h"
#include "key_system.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "script.h"
#include "constants/maps.h"

extern const u8 PalletTown_ProfessorOaksLab_EventScript_BulbasaurBall[];
void CB2_RF_StarterMenu(void);
u16 gRFUiFixture_StarterResult[5];

void RFUiFixture_StarterLab(void)
{
    ZeroPlayerPartyMons();
    CalculatePlayerPartyCount();
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 2);
    FlagClear(FLAG_SYS_POKEMON_GET);
    FlagClear(FLAG_HIDE_OAK_IN_HIS_LAB);
    SetWarpDestination(MAP_GROUP(MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB), MAP_NUM(MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB), -1, 9, 5);
    SetMainCallback1(NULL);
    WarpIntoMap();
    SetMainCallback2(CB2_LoadMap);
}

void RFUiFixture_StarterScript(void)
{
    ScriptContext_SetupScript(PalletTown_ProfessorOaksLab_EventScript_BulbasaurBall);
    SetMainCallback2(CB2_Overworld);
}

void RFUiFixture_StarterRead(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    gRFUiFixture_StarterResult[0] = gPartiesCount[B_TRAINER_PLAYER];
    gRFUiFixture_StarterResult[1] = GetMonData(mon, MON_DATA_SPECIES);
    gRFUiFixture_StarterResult[2] = GetMonData(mon, MON_DATA_ABILITY_NUM);
    gRFUiFixture_StarterResult[3] = VarGet(VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB);
    gRFUiFixture_StarterResult[4] = VarGet(VAR_STARTER_MON);
    SetMainCallback2(CB2_Overworld);
}

void RFUiFixture_StarterDay(void)
{
    KeySystem_Set(KEY_DARK_MODE, FALSE);
    CB2_RF_StarterMenu();
}

extern const u8 PalletTown_ProfessorOaksLab_EventScript_RivalBattleTriggerMid[];

// Starts the real lab rival battle after a starter has been received (scene 3).
void RFUiFixture_StarterRival(void)
{
    ScriptContext_SetupScript(PalletTown_ProfessorOaksLab_EventScript_RivalBattleTriggerMid);
    SetMainCallback2(CB2_Overworld);
}

// Harness writes {mapGroup, mapNum, x, y} here, then calls RFUiFixture_StarterWarp.
u8 gRFUiFixture_Warp[4];

void RFUiFixture_StarterWarp(void)
{
    SetWarpDestination(gRFUiFixture_Warp[0], gRFUiFixture_Warp[1], -1, gRFUiFixture_Warp[2], gRFUiFixture_Warp[3]);
    SetMainCallback1(NULL);
    WarpIntoMap();
    SetMainCallback2(CB2_LoadMap);
}
