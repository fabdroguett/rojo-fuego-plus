// Emulator-only fixture. Copy into /work/src for test builds; remove before shipping.
#include "global.h"
#include "main.h"
#include "pokemon.h"
#include "pokedex.h"
#include "wild_encounter.h"
#include "battle_setup.h"
#include "overworld.h"
#include "dexnav.h"
#include "event_data.h"
#include "key_system.h"
#include "constants/species.h"
#include "constants/maps.h"
#include "pokemon_summary_screen.h"
#include "pokemon_storage_system.h"
#include "item_menu.h"
#include "item.h"
#include "rf_quests.h"
#include "constants/items.h"
#include "constants/moves.h"

void RFUiFixture_Day(void)
{
    KeySystem_Set(KEY_DARK_MODE, FALSE);
    SetMainCallback2(CB2_Overworld);
}

void RFUiFixture_Slow(void)
{
    KeySystem_Set(KEY_BATTLE_SPEED, FALSE);
    SetMainCallback2(CB2_Overworld);
}

void RFUiFixture_Wild(void)
{
    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_BULBASAUR, 10, 31);
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][1], SPECIES_CHARMANDER, 10, 31);
    CalculatePlayerPartyCount();
    CreateWildMon(SPECIES_PIDGEY, 5);
    SetMainCallback2(CB2_Overworld);
    BattleSetup_StartWildBattle();
}

void RFUiFixture_Faint(void)
{
    u16 hp = 1;
    u32 i;
    RFUiFixture_Wild();
    CreateWildMon(SPECIES_PIDGEY, 100);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &hp);
    SetMonMoveSlot(&gParties[B_TRAINER_OPPONENT_A][0], MOVE_TACKLE, 0);
    for (i = 1; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(&gParties[B_TRAINER_OPPONENT_A][0], MOVE_NONE, i);
}

void RFUiFixture_DexNav(void)
{
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_ROUTE1);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_ROUTE1);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    GetSetPokedexFlag(NATIONAL_DEX_PIDGEY, FLAG_SET_SEEN);
    GetSetPokedexFlag(NATIONAL_DEX_PIDGEY, FLAG_SET_CAUGHT);
    GetSetPokedexFlag(NATIONAL_DEX_RATTATA, FLAG_SET_SEEN);
    SetMainCallback1(NULL);
    Task_OpenDexNavFromStartMenu(0);
}

void RFUiFixture_DexNavLight(void)
{
    KeySystem_Set(KEY_DARK_MODE, FALSE);
    RFUiFixture_DexNav();
}

void RFUiFixture_DexNavEmpty(void)
{
    u8 group = gSaveBlock1Ptr->location.mapGroup;
    u8 map = gSaveBlock1Ptr->location.mapNum;
    RFUiFixture_DexNav();
    gSaveBlock1Ptr->location.mapGroup = group;
    gSaveBlock1Ptr->location.mapNum = map;
}

void RFUiFixture_DexNavUnknown(void)
{
    RFUiFixture_DexNav();
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_ROUTE2);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_ROUTE2);
}

void RFUiFixture_Summary(void)
{
    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_BULBASAUR, 10, 31);
    CalculatePlayerPartyCount();
    SetMainCallback1(NULL);
    ShowPokemonSummaryScreen(SUMMARY_MODE_NORMAL, gParties[B_TRAINER_PLAYER], 0, 0, CB2_ReturnToField);
}

void RFUiFixture_Bag(void)
{
    AddBagItem(ITEM_POTION, 5);
    AddBagItem(ITEM_POKE_BALL, 10);
    SetMainCallback1(NULL);
    GoToBagMenu(ITEMMENULOCATION_FIELD, 0, CB2_ReturnToField);
}

void RFUiFixture_Quests(void)
{
    SetMainCallback1(NULL);
    SetMainCallback2(CB2_Overworld);
    RFQuests_OpenMenu();
}

void RFUiFixture_Keys(void)
{
    SetMainCallback1(NULL);
    SetMainCallback2(CB2_Overworld);
    KeySystem_OpenMenu();
}

void RFUiFixture_PC(void)
{
    SetMainCallback1(NULL);
    SetMainCallback2(CB2_Overworld);
    ShowPokemonStorageSystemPC();
}
