// Disposable mGBA fixture. Never include this file in a production ROM.
#include "global.h"
#include "main.h"
#include "pokemon.h"
#include "item.h"
#include "item_menu.h"
#include "overworld.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "wild_encounter.h"
#include "registered_items.h"
#include "rf_ui_theme.h"
#include "window.h"
#include "sprite.h"
#include "constants/items.h"
#include "constants/species.h"
#include "constants/moves.h"

EWRAM_DATA u32 gRFBagFixtureHP = 0;
EWRAM_DATA u32 gRFBagFixturePotions = 0;
EWRAM_DATA u32 gRFBagFixtureResult = 0;
EWRAM_DATA u32 gRFBagFixtureMatrices = 0;
EWRAM_DATA u32 gRFBagFixturePalettes = 0;
EWRAM_DATA u32 gRFBagFixtureSprites = 0;
EWRAM_DATA u32 gRFBagFixtureOBJBytes = 0;
extern u32 gOamMatrixAllocBitmap;
void RFUiFixture_Bag(void);

static void OpenFixtureBag(enum Pocket pocket)
{
    ResetBagScrollPositions();
    SetMainCallback1(NULL);
    GoToBagMenu(ITEMMENULOCATION_FIELD, pocket, CB2_ReturnToField);
}

void RFBagFixture_Empty(void)
{
    ClearBag();
    OpenFixtureBag(POCKET_ITEMS);
}

void RFBagFixture_Edges(void)
{
    ClearBag();
    AddBagItem(ITEM_POTION, 999);
    AddBagItem(ITEM_SUPER_POTION, 99);
    AddBagItem(ITEM_HYPER_POTION, 9);
    AddBagItem(ITEM_ABILITY_CAPSULE, 7);
    AddBagItem(ITEM_STRAWBERRY_SWEET, 3);
    AddBagItem(ITEM_GALARICA_WREATH, 1);
    AddBagItem(ITEM_PEWTER_CRUNCHIES, 2);
    AddBagItem(ITEM_ADAMANT_CRYSTAL, 1);
    AddBagItem(ITEM_TOWN_MAP, 1);
    AddBagItem(ITEM_BICYCLE, 1);
    OpenFixtureBag(POCKET_ITEMS);
}

void RFBagFixture_Keys(void)
{
    ClearBag();
    AddBagItem(ITEM_TOWN_MAP, 1);
    AddBagItem(ITEM_BICYCLE, 1);
    OpenFixtureBag(POCKET_KEY_ITEMS);
}

void RFBagFixture_Battle(void)
{
    u16 hp = 1;
    u32 i;
    ClearBag();
    AddBagItem(ITEM_POTION, 2);
    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_BULBASAUR, 40, 31);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &hp);
    CalculatePlayerPartyCount();
    CreateWildMon(SPECIES_PIDGEY, 5);
    SetMonMoveSlot(&gParties[B_TRAINER_OPPONENT_A][0], MOVE_SPLASH, 0);
    for (i = 1; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(&gParties[B_TRAINER_OPPONENT_A][0], MOVE_NONE, i);
    ResetBagScrollPositions();
    SetMainCallback1(NULL);
    SetMainCallback2(CB2_Overworld);
    BattleSetup_StartWildBattle();
}

void RFBagFixture_FieldMedicine(void)
{
    u16 hp = 1;
    ClearBag();
    AddBagItem(ITEM_POTION, 2);
    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_BULBASAUR, 40, 31);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &hp);
    CalculatePlayerPartyCount();
    OpenFixtureBag(POCKET_ITEMS);
}

void RFBagFixture_ReadField(void)
{
    gRFBagFixtureHP = GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP);
    gRFBagFixturePotions = CountTotalItemQuantityInBag(ITEM_POTION);
    gRFBagFixtureResult = gRFBagFixtureHP == 21 && gRFBagFixturePotions == 1;
}

void RFBagFixture_Sort(void)
{
    ClearBag();
    ResetBagScrollPositions();
    RFUiFixture_Bag();
}

void RFBagFixture_ReadSort(void)
{
    static const enum Item expected[] = {ITEM_FIRE_STONE, ITEM_LEFTOVERS, ITEM_NUGGET,
                                         ITEM_POTION, ITEM_RARE_CANDY, ITEM_X_ATTACK};
    u32 i;
    gRFBagFixtureResult = TRUE;
    for (i = 0; i < ARRAY_COUNT(expected); i++)
        if (GetBagItemId(POCKET_ITEMS, i) != expected[i])
            gRFBagFixtureResult = FALSE;
}

void RFBagFixture_ReadBattle(void)
{
    gRFBagFixtureHP = gBattleMons[0].hp;
    gRFBagFixturePotions = CountTotalItemQuantityInBag(ITEM_POTION);
    gRFBagFixtureResult = gRFBagFixtureHP > 1 && gRFBagFixturePotions == 1;
    SetMainCallback2(BattleMainCB2);
}

void RFBagFixture_ReadKey(void)
{
    gRFBagFixtureResult = IsItemRegistered(ITEM_TOWN_MAP);
    SetMainCallback2(CB2_Overworld);
}

void RFBagFixture_NoDescription(void)
{
    FillWindowPixelBuffer(1, PIXEL_FILL(0));
    gRFBagFixtureResult = RFUI_PrintParagraph(1, NULL, 3, 2, 98, 4, (const u8[]){0, 1, 3});
    CopyWindowToVram(1, COPYWIN_GFX);
}

void RFBagFixture_LongDescription(void)
{
    FillWindowPixelBuffer(1, PIXEL_FILL(0));
    gRFBagFixtureResult = !RFUI_PrintParagraph(1,
        COMPOUND_STRING("Esta descripción de prueba es deliberadamente extensa para comprobar que el texto se ajuste al panel sin invadir la lista ni el pie de controles de la mochila."),
        3, 2, 98, 4, (const u8[]){0, 1, 3});
    CopyWindowToVram(1, COPYWIN_GFX);
}

void RFBagFixture_ReadResources(void)
{
    u32 i, bitmap = gOamMatrixAllocBitmap;
    gRFBagFixtureMatrices = 0;
    gRFBagFixturePalettes = 0;
    gRFBagFixtureSprites = 0;
    gRFBagFixtureOBJBytes = 0;
    while (bitmap)
    {
        gRFBagFixtureMatrices += bitmap & 1;
        bitmap >>= 1;
    }
    for (i = 0; i < 16; i++)
        gRFBagFixturePalettes += GetSpritePaletteTagByPaletteNum(i) != TAG_NONE;
    for (i = 0; i < MAX_SPRITES; i++)
        gRFBagFixtureSprites += gSprites[i].inUse;
    for (i = 0; i < 1024; i++)
        gRFBagFixtureOBJBytes += SpriteTileAllocBitmapOp(i, 2) ? 32 : 0;
    gRFBagFixtureResult = gRFBagFixtureMatrices <= 7
        && gRFBagFixturePalettes <= 12 && gRFBagFixtureSprites <= 40
        && gRFBagFixtureOBJBytes <= 32768;
}
