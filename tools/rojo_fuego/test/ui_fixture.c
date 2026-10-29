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
#include "battle.h"
#include "battle_controllers.h"
#include "battle_message.h"
#include "battle_main.h"
#include "text_window.h"
#include "palette.h"
#include "bg.h"
#include "start_menu.h"
#include "menu.h"
#include "party_menu.h"
#include "option_menu.h"
#include "trainer_card.h"
#include "naming_screen.h"
#include "fieldmap.h"
#include "region_map.h"
#include "achievements.h"
#include "registered_items.h"
#include "shop.h"
#include "move_relearner.h"
#include "evolution_scene.h"

void RFUiFixture_Pokedex(void)
{
    u32 dex;
    FlagSet(FLAG_SYS_POKEDEX_GET);
    for (dex = 1; dex <= 30; dex++)
    {
        GetSetPokedexFlag(dex, FLAG_SET_SEEN);
        if (dex <= 9)
            GetSetPokedexFlag(dex, FLAG_SET_CAUGHT);
    }
    SetMainCallback1(NULL);
    CB2_OpenPokedex();
}

u8 gRFUiFixture_OrderOK;
void RFUiFixture_CheckOrder(void)
{
    gRFUiFixture_OrderOK = GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES) == SPECIES_PIKACHU
        && GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_SPECIES) == SPECIES_BLASTOISE
        && GetMonData(&gParties[B_TRAINER_PLAYER][2], MON_DATA_SPECIES) == SPECIES_CHARIZARD;
    SetMainCallback2(CB2_Overworld);
}

void RFUiFixture_FieldParty(void)
{
    static const enum Species species[] = {SPECIES_PIKACHU, SPECIES_CHARIZARD, SPECIES_BLASTOISE, SPECIES_VENUSAUR, SPECIES_PIDGEOT, SPECIES_LAPRAS};
    u32 i;
    ZeroPlayerPartyMons();
    for (i = 0; i < ARRAY_COUNT(species); i++)
        CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][i], species[i], 20 + i * 5, 31);
    CalculatePlayerPartyCount();
    SetMainCallback1(NULL);
    CB2_PartyMenuFromStartMenu();
}

void RFUiFixture_Start(void)
{
    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_BULBASAUR, 10, 31);
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][1], SPECIES_CHARMANDER, 10, 31);
    CalculatePlayerPartyCount();
    FlagSet(FLAG_SYS_POKEDEX_GET);
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(DN_FLAG_DEXNAV_GET);
    GetSetPokedexFlag(NATIONAL_DEX_PIDGEY, FLAG_SET_SEEN);
    SetMainCallback1(NULL);
    SetMainCallback2(CB2_Overworld);
    ShowStartMenu();
}

void RFUiFixture_Voiceover(void)
{
    SetMainCallback1(NULL);
    LoadMessageBoxGfx(0, 0x30, BG_PLTT_ID(7));
    BtlCtrl_DrawVoiceoverMessageFrame();
    BattlePutTextOnWindow(COMPOUND_STRING("You've never had a POKEMON battle\nbefore, have you?"), B_WIN_OAK_OLD_MAN);
    CopyBgTilemapBufferToVram(0);
    SetMainCallback2(BattleMainCB2);
}

void RFUiFixture_Day(void)
{
    KeySystem_Set(KEY_DARK_MODE, FALSE);
    LoadMessageBoxAndBorderGfx(); // as the Key System menu does after the toggle
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
    if (!CheckBagHasItem(ITEM_POKE_BALL, 1))
        AddBagItem(ITEM_POKE_BALL, 5); // shows the last used Ball window
    SetMainCallback2(CB2_Overworld);
    BattleSetup_StartWildBattle();
}

// Stands the player in Route 1's tall grass so the next RFUiFixture_Wild uses the grass background.
void RFUiFixture_GrassWarp(void)
{
    SetWarpDestination(MAP_GROUP(MAP_ROUTE1), MAP_NUM(MAP_ROUTE1), -1, 12, 7);
    SetMainCallback1(NULL);
    WarpIntoMap();
    SetMainCallback2(CB2_LoadMap);
}

// Stand the player on a tile of each battle environment; RFUiFixture_Wild then fights there.
static void RFUiFixture_WarpTo(u8 group, u8 num, s8 x, s8 y)
{
    SetWarpDestination(group, num, -1, x, y);
    SetMainCallback1(NULL);
    WarpIntoMap();
    SetMainCallback2(CB2_LoadMap);
}

void RFUiFixture_EnvCave(void)  { RFUiFixture_WarpTo(MAP_GROUP(MAP_MT_MOON_1F), MAP_NUM(MAP_MT_MOON_1F), 6, 7); }
void RFUiFixture_EnvSea(void)   { RFUiFixture_WarpTo(MAP_GROUP(MAP_CERULEAN_CITY), MAP_NUM(MAP_CERULEAN_CITY), 17, 0); }
void RFUiFixture_EnvPond(void)  { RFUiFixture_WarpTo(MAP_GROUP(MAP_ROUTE22), MAP_NUM(MAP_ROUTE22), 23, 8); }
void RFUiFixture_EnvSand(void)  { RFUiFixture_WarpTo(MAP_GROUP(MAP_FIVE_ISLAND_MEADOW), MAP_NUM(MAP_FIVE_ISLAND_MEADOW), 2, 11); }
void RFUiFixture_EnvMountain(void) { RFUiFixture_WarpTo(MAP_GROUP(MAP_BIRTH_ISLAND_EXTERIOR_FRLG), MAP_NUM(MAP_BIRTH_ISLAND_EXTERIOR_FRLG), 14, 8); }

// One Tackle away from level 11: the faint grants the experience and opens the level-up box.
void RFUiFixture_LevelUp(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    u32 exp, hp = 1;
    u16 move = MOVE_TACKLE;

    RFUiFixture_Wild();
    exp = gExperienceTables[gSpeciesInfo[SPECIES_BULBASAUR].growthRate][11] - 1;
    SetMonData(mon, MON_DATA_EXP, &exp);
    SetMonData(mon, MON_DATA_MOVE1, &move);
    SetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_HP, &hp);
}

void RFUiFixture_Tutorial(void)
{
    RFUiFixture_Wild();
    gBattleTypeFlags |= BATTLE_TYPE_FIRST_BATTLE;
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
    AddBagItem(ITEM_RARE_CANDY, 2);
    AddBagItem(ITEM_X_ATTACK, 1);
    AddBagItem(ITEM_FIRE_STONE, 1);
    AddBagItem(ITEM_LEFTOVERS, 1);
    AddBagItem(ITEM_NUGGET, 1);
    AddBagItem(ITEM_ESCAPE_ROPE, 1);
    AddBagItem(ITEM_POKE_BALL, 10);
    AddBagItem(ITEM_TM01, 1);
    AddBagItem(ITEM_ORAN_BERRY, 3);
    SetMainCallback1(NULL);
    GoToBagMenu(ITEMMENULOCATION_FIELD, 0, CB2_ReturnToField);
}

void RFUiFixture_BagFemale(void)
{
    gSaveBlock2Ptr->playerGender = 1;
    RFUiFixture_Bag();
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

void RFUiFixture_Options(void)
{
    SetMainCallback1(NULL);
    gMain.savedCallback = CB2_ReturnToField;
    SetMainCallback2(CB2_InitOptionMenu);
}

void RFUiFixture_Trainer(void)
{
    SetMainCallback1(NULL);
    ShowPlayerTrainerCard(CB2_ReturnToField);
}

void RFUiFixture_Naming(void)
{
    SetMainCallback1(NULL);
    DoNamingScreen(NAMING_SCREEN_PLAYER, gSaveBlock2Ptr->playerName, gSaveBlock2Ptr->playerGender, 0, 0, CB2_ReturnToField);
}

void RFUiFixture_TownMap(void)
{
    SetMainCallback1(NULL);
    FieldInitRegionMap(CB2_ReturnToField);
}

void RFUiFixture_FlyMap(void)
{
    SetMainCallback1(NULL);
    gMain.state = 0;
    SetMainCallback2(CB2_OpenFlyMap);
}

void RFUiFixture_Achievements(void)
{
    SetMainCallback1(NULL);
    Achievements_OpenScreen(CB2_ReturnToField);
}

void RFUiFixture_Registered(void)
{
    AddBagItem(ITEM_BICYCLE, 1);
    AddBagItem(ITEM_TOWN_MAP, 1);
    if (!IsItemRegistered(ITEM_BICYCLE))
        ToggleRegisteredItem(ITEM_BICYCLE);
    if (!IsItemRegistered(ITEM_TOWN_MAP))
        ToggleRegisteredItem(ITEM_TOWN_MAP);
    SetMainCallback1(NULL);
    SetMainCallback2(CB2_Overworld);
    RegisteredItems_OpenMenu();
}

void RFUiFixture_Mart(void)
{
    static const u16 items[] = {ITEM_POKE_BALL, ITEM_POTION, ITEM_ANTIDOTE, ITEM_REPEL, ITEM_ESCAPE_ROPE, ITEM_NONE};
    SetMainCallback1(NULL);
    SetMainCallback2(CB2_Overworld);
    CreatePokemartMenu(items);
}

void RFUiFixture_Relearner(void)
{
    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_IVYSAUR, 30, 31);
    CalculatePlayerPartyCount();
    gSpecialVar_0x8004 = 0;
    SetMainCallback1(NULL);
    gMain.savedCallback = CB2_ReturnToField;
    SetMainCallback2(CB2_InitLearnMove);
}

void RFUiFixture_Evolution(void)
{
    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_BULBASAUR, 16, 31);
    CalculatePlayerPartyCount();
    SetMainCallback1(NULL);
    gCB2_AfterEvolution = CB2_ReturnToField; // the game always sets it before evolving
    BeginEvolutionScene(&gParties[B_TRAINER_PLAYER][0], SPECIES_IVYSAUR, TRUE, 0);
}

void RFUiFixture_WildIntimidate(void)
{
    u8 abilityNum = 0; // Growlithe's first ability: Intimidate

    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_BULBASAUR, 10, 31);
    CalculatePlayerPartyCount();
    CreateWildMon(SPECIES_GROWLITHE, 8);
    SetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_ABILITY_NUM, &abilityNum);
    SetMainCallback2(CB2_Overworld);
    BattleSetup_StartWildBattle();
}
