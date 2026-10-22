#include "global.h"
#include "achievements.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "bg.h"
#include "data.h"
#include "daycare.h"
#include "decompress.h"
#include "dexnav.h"
#include "rf_collection.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "field_effect.h"
#include "field_effect_helpers.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "item.h"
#include "key_system.h"
#include "international_string_util.h"
#include "m4a.h"
#include "map_name_popup.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "metatile_behavior.h"
#include "move.h"
#include "overworld.h"
#include "ow_abilities.h"
#include "palette.h"
#include "party_menu.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "pokemon_storage_system.h"
#include "pokemon_summary_screen.h"
#include "random.h"
#include "region_map.h"
#include "roamer.h"
#include "rtc.h"
#include "scanline_effect.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "sprite.h"
#include "start_menu.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "wild_encounter.h"
#include "window.h"
#include "constants/species.h"
#include "constants/maps.h"
#include "constants/field_effects.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/abilities.h"
#include "constants/rgb.h"
#include "constants/region_map_sections.h"
#include "gba/m4a_internal.h"

#if DEXNAV_ENABLED
STATIC_ASSERT(DN_FLAG_SEARCHING != 0, DNFlagSearching_Must_Not_Be_Zero);
STATIC_ASSERT(DN_FLAG_DETECTOR_MODE != 0, DNFlagDetectorMode_Must_Not_Be_Zero);
STATIC_ASSERT(DN_VAR_SPECIES != 0, DNVarSpecies_Must_Not_Be_Zero);
STATIC_ASSERT(DN_VAR_STEP_COUNTER != 0, DNVarStepCounter_Must_Not_Be_Zero);
#endif

// Defines
enum WindowIds
{
    WINDOW_INFO,
    WINDOW_REGISTERED,
    WINDOW_COUNT,
};

enum Statuses
{
    STATUS_INVALID_SEARCH,
    STATUS_CHOOSE_MON,
    STATUS_LOCKED,
    STATUS_NO_DATA,
    STATUS_INCORRECT_AREA,
};

struct DexNavSearch
{
    enum Species species;
    enum Move moves[MAX_MON_MOVES];
    enum Item heldItem;
    u8 abilityNum;
    u8 potential;
    u8 searchLevel;
    u8 monLevel;
    u8 proximity;
    u8 environment;
    s16 tileX;
    s16 tileY;
    u8 fldEffSpriteId;
    u8 fldEffId;
    u8 movementCount;
    u8 windowId;
    u8 iconSpriteId;
    u8 eyeSpriteId;
    u8 itemSpriteId;
    u8 starSpriteIds[3];
    u8 ownedIconSpriteId;
    u8 exclamationSpriteId;
    u32 startingTime;
    u8 hiddenSearch:1;
    u8 isHiddenMon:1;
    u8 isShiny:1;
    u8 unk:5;
    u16 palBuffer[16];
    // Rojo Fuego+: the individual is decided when the search starts so the details panel can show it
    u32 personality;
    u8 ivs[NUM_STATS];
    u8 detailsWindowId;
    u8 detailsPage; // 0: closed, 1: this individual, 2: species, capture odds and your best one
};

struct DexNavGUI
{
    MainCallback savedCallback;
    u8 state;
    u8 cursorSpriteId;
    enum Species landSpecies[NUM_LAND_MONS_ENCOUNTER_SLOTS];
    enum Species waterSpecies[NUM_WATER_MONS_ENCOUNTER_SLOTS];
    enum Species hiddenSpecies[NUM_HIDDEN_MONS_ENCOUNTER_SLOTS];
    u8 cursorRow;
    u8 cursorCol;
    u8 environment;
    u8 potential;
    u8 typeIconSpriteIds[2];
    u8 starSpriteIds[3];
};

// RAM

EWRAM_DATA static struct DexNavSearch *sDexNavSearchDataPtr = NULL;
EWRAM_DATA static struct DexNavGUI *sDexNavUiDataPtr = NULL;
EWRAM_DATA static u8 *sBg1TilemapBuffer = NULL;
EWRAM_DATA enum Species gDexNavSpecies = SPECIES_NONE;

//// Function Declarations
//GUI
static void Task_DexNavWaitFadeIn(u8 taskId);
static void Task_DexNavMain(u8 taskId);
static void PrintCollectionSummary(void);
static EWRAM_DATA bool8 sCollectionOpen = FALSE;
static EWRAM_DATA u8 sCollectionTime = 0;
static void PrintCurrentSpeciesInfo(void);
// SEARCH
static bool8 TryStartHiddenMonFieldEffect(enum EncounterType environment, u8 xSize, u8 ySize, bool8 smallScan);
static void DexNavGenerateMoveset(enum Species species, u8 searchLevel, u8 encounterLevel, u16 *moveDst);
static enum Item DexNavGenerateHeldItem(enum Species species, u8 searchLevel);
static u8 DexNavGetAbilityNum(enum Species species, u8 searchLevel);
static u8 DexNavGeneratePotential(u8 searchLevel);
static u8 DexNavTryGenerateMonLevel(enum Species species, enum EncounterType environment);
static u8 GetEncounterLevelFromMapData(enum Species species, enum EncounterType environment);
static void CreateDexNavWildMon(enum Species species, u8 potential, u8 level, u8 abilityNum, enum Item item, enum Move *moves);
static u8 GetPlayerDistance(s16 x, s16 y);
static u8 DexNavPickTile(enum EncounterType environment, u8 xSize, u8 ySize, bool8 smallScan);
static void DexNavProximityUpdate(void);
static void DexNavDrawIcons(void);
static void DexNavUpdateSearchWindow(u8 proximity, u8 searchLevel);
// HIDDEN MONS
static void DexNavDrawHiddenIcons(void);
static void DrawHiddenSearchWindow(u8 width);
static void RevealHiddenMon(void);

//// Const Data
// gui image data
static const u32 sDexNavGuiTiles[] = INCGFX_U32("graphics/dexnav/gui_tiles.png", ".4bpp.smol");
static const u32 sDexNavGuiTilemap[] = INCGFX_U32("graphics/dexnav/gui_tilemap.bin", ".smolTM");
static const u32 sDexNavGuiPal[] = INCGFX_U32("graphics/dexnav/gui.pal", ".gbapal");

static const u32 sSelectionCursorGfx[] = INCGFX_U32("graphics/dexnav/cursor.png", ".4bpp.smol");
static const u16 sSelectionCursorPal[] = INCGFX_U16("graphics/dexnav/cursor.png", ".gbapal");
static const u32 sCapturedAllMonsTiles[] = INCGFX_U32("graphics/dexnav/captured_all.png", ".4bpp.smol");  //uses selection cursor pal

static const u32 sNoDataGfx[] = INCGFX_U32("graphics/dexnav/no_data.png", ".4bpp.smol");

// searching image data
static const u32 sPotentialStarGfx[] = INCGFX_U32("graphics/dexnav/star.png", ".4bpp.smol");
static const u32 sHiddenSearchIconGfx[] = INCGFX_U32("graphics/dexnav/hidden_search.png", ".4bpp.smol");
static const u32 sOwnedIconGfx[] = INCGFX_U32("graphics/dexnav/owned_icon.png", ".4bpp.smol");
static const u32 sHiddenMonIconGfx[] = INCGFX_U32("graphics/dexnav/hidden.png", ".4bpp.smol");

// strings
static const u8 sText_DexNav_NoInfo[] = _("--------");
static const u8 sText_DexNav_CaptureToSee[] = _("¡Captúralo antes!");
static const u8 sText_DexNav_PressRToRegister[] = _("REGISTRAR ESPECIE");
static const u8 sText_DexNav_SearchForRegisteredSpecies[] = _("Buscar: {STR_VAR_1}");
static const u8 sText_DexNav_NotFoundHere[] = _("¡Ese Pokémon no aparece aquí!");
static const u8 sText_ThreeQmarks[] = _("???");
static const u8 sText_SearchLevel[] = _("BÚSQ. {LV}. {STR_VAR_1}");
static const u8 sText_MonLevel[] = _("{LV}. {STR_VAR_1}");
static const u8 sText_EggMove[] = _("MOV.: {STR_VAR_1}");
static const u8 sText_HeldItem[] = _("{STR_VAR_1}");
static const u8 sText_StartExit[] = _("{START_BUTTON} SALIR");
static const u8 sText_DexNavChain[] = _("{NO} {STR_VAR_1}");
static const u8 sText_DexNavChainLong[] = _("{NO}{STR_VAR_1}");

static const u8 sText_ArrowLeft[] = _("{LEFT_ARROW}");
static const u8 sText_ArrowRight[] = _("{RIGHT_ARROW}");
static const u8 sText_ArrowUp[] = _("{UP_ARROW}");
static const u8 sText_ArrowDown[] = _("{DOWN_ARROW}");

static const struct WindowTemplate sDexNavGuiWindowTemplates[] =
{
    [WINDOW_INFO] =
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 5,
        .width = 9,
        .height = 15,
        .paletteNum = 15,
        .baseBlock = 1,
    },
    [WINDOW_REGISTERED] =
    {
        .bg = 0,
        .tilemapLeft = 4,
        .tilemapTop = 0,
        .width = 26,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 200,
    },
    DUMMY_WIN_TEMPLATE
};

//gui font
static const u8 sFontColor_Black[3] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sFontColor_White[3] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};
//search window font
static const u8 sSearchFontColor[3] = {0, 15, 13};

static const struct OamData sNoDataIconOam =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 1,
};

static const struct OamData sHeldItemOam =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(8x8),
    .size = SPRITE_SIZE(8x8),
    .priority = 0,
    .paletteNum = 13,
};

static const struct OamData sCapturedAllOam =
{
    .y = 0,
    .affineMode = 1,
    .objMode = 0,
    .mosaic = 0,
    .bpp = 0,
    .shape = SPRITE_SHAPE(8x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(8x8),
    .tileNum = 0,
    .priority = 0, //Highest
    .paletteNum = 12,
    .affineParam = 0,
};

static const struct OamData sSearchIconOam =
{
    .y = 0,
    .affineMode = 0,
    .objMode = 0,
    .mosaic = 0,
    .bpp = 0,
    .shape = 0,
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x32),
    .tileNum = 0,
    .priority = 0, // above BG layers
    .paletteNum = 13,
    .affineParam = 0
};

static const struct OamData sSelectionCursorOam =
{
    .y = 0,
    .affineMode = 0,
    .objMode = 0,
    .mosaic = 0,
    .bpp = 0,
    .shape = 0,
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x32),
    .tileNum = 0,
    .priority = 0, // above BG layers
    .paletteNum = 12,
    .affineParam = 0
};

static const struct OamData sSightOam =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(16x8),
    .size = SPRITE_SIZE(16x8),
    .priority = 0,
};
static const union AnimCmd sAnimCmdSight0[] =
{
    ANIMCMD_FRAME(0, 1),
    ANIMCMD_END
};
static const union AnimCmd sAnimCmdSight1[] =
{
    ANIMCMD_FRAME(2, 1),
    ANIMCMD_END
};
static const union AnimCmd sAnimCmdSight2[] =
{
    ANIMCMD_FRAME(4, 1),
    ANIMCMD_END
};
static const union AnimCmd *const sAnimCmdTable_Sight[] =
{
    sAnimCmdSight0,
    sAnimCmdSight1,
    sAnimCmdSight2,
};

// gui sprite templates
static const struct SpriteTemplate sNoDataIconTemplate =
{
    .tileTag = ICON_GFX_TAG,
    .paletteTag = ICON_PAL_TAG,
    .oam = &sNoDataIconOam,
};

static const struct SpriteTemplate sCaptureAllMonsSpriteTemplate =
{
    .tileTag = CAPTURED_ALL_TAG,
    .paletteTag = 0xFFFF,
    .oam = &sCapturedAllOam,
};

static const struct SpriteTemplate sSelectionCursorSpriteTemplate =
{
    .tileTag = SELECTION_CURSOR_TAG,
    .paletteTag = 0xFFFF,
    .oam = &sSelectionCursorOam,
    .anims =  gDummySpriteAnimTable,
};

// search window sprite templates
static const struct SpriteTemplate sHeldItemTemplate =
{
    .tileTag = HELD_ITEM_TAG,
    .paletteTag = 0xFFFF,
    .oam = &sHeldItemOam,
};

static const struct SpriteTemplate sPotentialStarTemplate =
{
    .tileTag = LIT_STAR_TILE_TAG,
    .paletteTag = 0xFFFF,   //held item pal
    .oam = &sHeldItemOam,
};

static const struct SpriteTemplate sSearchIconSpriteTemplate =
{
    .tileTag = HIDDEN_SEARCH_TAG,
    .paletteTag = 0xFFFF,   //held item pal
    .oam = &sSearchIconOam,
    .anims =  gDummySpriteAnimTable,
};

static const struct SpriteTemplate sOwnedIconTemplate =
{
    .tileTag = OWNED_ICON_TAG,
    .paletteTag = 0xFFFF,   //held item pal
    .oam = &sHeldItemOam,
    .anims =  gDummySpriteAnimTable,
};

static const struct SpriteTemplate sHiddenMonIconTemplate =
{
    .tileTag = HIDDEN_MON_ICON_TAG,
    .paletteTag = 0xFFFF,   //held item pal
    .oam = &sHeldItemOam,
    .anims =  gDummySpriteAnimTable,
};

// gui sprite sheets
static const struct CompressedSpriteSheet sNoDataIconSpriteSheet = {sNoDataGfx, (32 * 32) / 2, ICON_GFX_TAG};
static const struct CompressedSpriteSheet sCapturedAllPokemonSpriteSheet = {sCapturedAllMonsTiles, (8 * 8) / 2, CAPTURED_ALL_TAG};
// search sprite sheets
static const struct CompressedSpriteSheet sPotentialStarSpriteSheet = {sPotentialStarGfx, (8 * 8) / 2, LIT_STAR_TILE_TAG};
static const struct CompressedSpriteSheet sOwnedIconSpriteSheet = {sOwnedIconGfx, (8 * 8) / 2, OWNED_ICON_TAG};
static const struct CompressedSpriteSheet sHiddenMonIconSpriteSheet = {sHiddenMonIconGfx, (8 * 8) / 2, HIDDEN_MON_ICON_TAG};

//// functions
///////////////////////
//// DEXNAV SEARCH ////
///////////////////////
static s16 GetSearchWindowY(void)
{
    return (GetWindowAttribute(sDexNavSearchDataPtr->windowId, WINDOW_TILEMAP_TOP) * 8);
}

#define SPECIES_ICON_X 28
static void DrawDexNavSearchMonIcon(enum Species species, u8 *dst, bool8 owned)
{
    u8 spriteId;

    LoadMonIconPalette(species);
    spriteId = CreateMonIcon(species, SpriteCB_MonIcon, SPECIES_ICON_X - 6, GetSearchWindowY() + 8, 0, 0xFFFFFFFF);
    gSprites[spriteId].oam.priority = 0;
    *dst = spriteId;

    if (owned)
        sDexNavSearchDataPtr->ownedIconSpriteId = CreateSprite(&sOwnedIconTemplate, SPECIES_ICON_X + 6, GetSearchWindowY() + 4, 0);
}

static void AddSearchWindow(u8 width)
{
    struct WindowTemplate template;
    u16 y = 16;

    if (sDexNavSearchDataPtr->tileY > (gSaveBlock1Ptr->pos.y + 7))
        y = 1;  //draw at top if chosen tile is below

    LoadDexNavWindowGfx(sDexNavSearchDataPtr->windowId, 0x1d5, 14 * 16);

    SetWindowTemplateFields(&template, 0, 1, y, width, 3, 14, 8);

    sDexNavSearchDataPtr->windowId = AddWindow(&template);
    FillWindowPixelBuffer(sDexNavSearchDataPtr->windowId, PIXEL_FILL(1));
    PutWindowTilemap(sDexNavSearchDataPtr->windowId);
    CopyWindowToVram(sDexNavSearchDataPtr->windowId, 3);

    DrawStdFrameWithCustomTileAndPalette(sDexNavSearchDataPtr->windowId, TRUE, 0x214, 14);
}

#define WINDOW_COL_0        (SPECIES_ICON_X + 4)
#define WINDOW_COL_1        (WINDOW_COL_0 + (GetFontAttribute(FONT_SMALL, FONTATTR_MAX_LETTER_WIDTH) * (POKEMON_NAME_LENGTH)))
#define WINDOW_MOVE_NAME_X  (WINDOW_COL_1 + (GetFontAttribute(FONT_SMALL, FONTATTR_MAX_LETTER_WIDTH) * 6))
#define SEARCH_ARROW_X      (WINDOW_MOVE_NAME_X + 90)
#define SEARCH_ARROW_Y      0

static void AddSearchWindowText(enum Species species, u8 proximity, u8 searchLevel, bool8 hidden)
{
    u8 windowId = sDexNavSearchDataPtr->windowId;

    //species name - always present
    if (hidden)
    {
        StringCopy(gStringVar4, sText_ThreeQmarks);
        AddTextPrinterParameterized3(sDexNavSearchDataPtr->windowId, FONT_SMALL, WINDOW_COL_0, 0, sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);
        return;
    }
    else
    {
        StringCopy(gStringVar1, GetSpeciesName(species));
        AddTextPrinterParameterized3(sDexNavSearchDataPtr->windowId, FONT_SMALL, WINDOW_COL_0, 0, sSearchFontColor, TEXT_SKIP_DRAW, gStringVar1);
    }

    //level - always present
    ConvertIntToDecimalStringN(gStringVar1, sDexNavSearchDataPtr->monLevel, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, sText_MonLevel);
    AddTextPrinterParameterized3(sDexNavSearchDataPtr->windowId, FONT_SMALL, WINDOW_COL_1, 0, sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);

    if (proximity <= SNEAKING_PROXIMITY)
    {
        PlaySE(SE_POKENAV_ON);
        // move
        if (searchLevel > 1 && sDexNavSearchDataPtr->moves[0])
        {
            StringCopy(gStringVar1, GetMoveName(sDexNavSearchDataPtr->moves[0]));
            StringExpandPlaceholders(gStringVar4, sText_EggMove);
            AddTextPrinterParameterized3(windowId, FONT_SMALL, WINDOW_MOVE_NAME_X, 0, sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);
        }

        if (searchLevel > 2)
        {
            // ability name
            StringCopy(gStringVar1, gAbilitiesInfo[GetAbilityBySpecies(species, sDexNavSearchDataPtr->abilityNum)].name);
            AddTextPrinterParameterized3(windowId, FONT_SMALL, WINDOW_COL_1 + 16, 12, sSearchFontColor, TEXT_SKIP_DRAW, gStringVar1);

            // item name
            if (sDexNavSearchDataPtr->heldItem)
            {
                CopyItemName(sDexNavSearchDataPtr->heldItem, gStringVar1);
                StringExpandPlaceholders(gStringVar4, sText_HeldItem);
                AddTextPrinterParameterized3(windowId, FONT_SMALL, WINDOW_COL_0, 12, sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);
            }
        }
    }

    //chain level - always present
    ConvertIntToDecimalStringN(gStringVar1, gSaveBlock3Ptr->dexNavChain, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (gSaveBlock3Ptr->dexNavChain > 99)
        StringExpandPlaceholders(gStringVar4, sText_DexNavChainLong);
    else
        StringExpandPlaceholders(gStringVar4, sText_DexNavChain);
    AddTextPrinterParameterized3(windowId, FONT_SMALL, SEARCH_ARROW_X - 16, 12, sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);

    CopyWindowToVram(sDexNavSearchDataPtr->windowId, 2);
}

#define SEARCH_WINDOW_WIDTH     28

static void DrawSearchWindow(enum Species species, u8 potential, bool8 hidden)
{
    u8 searchLevel = sDexNavSearchDataPtr->searchLevel;

    AddSearchWindow(SEARCH_WINDOW_WIDTH);
    AddSearchWindowText(species, sDexNavSearchDataPtr->proximity, searchLevel, hidden);
}

#undef SEARCH_WINDOW_WIDTH

//// Rojo Fuego+ details panel (SELECT while searching) ////
#define SEARCH_WINDOW_WIDTH_DETAILS 28
#define DETAILS_WINDOW_HEIGHT 10
#define DETAILS_LINE(n)       ((n) * 13)

static const u8 sText_Details_Shiny[] = _("¡VARIOCOLOR!");
static const u8 sText_Details_NotShiny[] = _("No es variocolor");
static const u8 sText_Details_ShinyOdds[] = _("Prob. shiny: 1/{STR_VAR_1}");
static const u8 sText_Details_Nature[] = _("Naturaleza: {STR_VAR_1}");
static const u8 sText_Details_HiddenPower[] = _("P. Oculto: {STR_VAR_1}");
static const u8 sText_Details_Ability[] = _("Habilidad: {STR_VAR_1}");
static const u8 sText_Details_HiddenAbility[] = _("Habilidad: {STR_VAR_1} (oculta)");
static const u8 sText_Details_Male[] = _("♂");
static const u8 sText_Details_Female[] = _("♀");
static const u8 sText_Details_IVs[] = _("IV");
static const u8 *const sText_Details_StatNames[NUM_STATS] =
{
    [STAT_HP]    = COMPOUND_STRING("PS"),
    [STAT_ATK]   = COMPOUND_STRING("At"),
    [STAT_DEF]   = COMPOUND_STRING("Df"),
    [STAT_SPEED] = COMPOUND_STRING("Ve"),
    [STAT_SPATK] = COMPOUND_STRING("AE"),
    [STAT_SPDEF] = COMPOUND_STRING("DE"),
};
static const u8 sDetailsShinyFontColor[3] = {0, 2, 13};

// Approximate odds shown to the player: base odds split across every reroll the game will make.
// The 4% random DexNav bonus is left out since it is rolled per encounter.
static u32 DexNavGetShinyOddsDenominator(void)
{
    u32 rolls = 1;
    u8 chain = gSaveBlock3Ptr->dexNavChain;

    if (CheckBagHasItem(ITEM_SHINY_CHARM, 1))
        rolls += I_SHINY_CHARM_ADDITIONAL_ROLLS;
    if (LURE_STEP_COUNT != 0)
        rolls += 1;
    rolls += (chain >= 100) ? 10 : (chain >= 50) ? 5 : 0;

    return (65536 / KeySystem_GetShinyThreshold()) / rolls;
}

static enum Type DexNavGetHiddenPowerType(void)
{
    const u8 *ivs = sDexNavSearchDataPtr->ivs;
    u32 typeBits = ((ivs[STAT_HP] & 1) << 0)
                 | ((ivs[STAT_ATK] & 1) << 1)
                 | ((ivs[STAT_DEF] & 1) << 2)
                 | ((ivs[STAT_SPEED] & 1) << 3)
                 | ((ivs[STAT_SPATK] & 1) << 4)
                 | ((ivs[STAT_SPDEF] & 1) << 5);
    u32 hpTypes[NUMBER_OF_MON_TYPES] = {0};
    u32 i, hpTypeCount = 0;

    for (i = 0; i < NUMBER_OF_MON_TYPES; i++)
    {
        if (gTypesInfo[i].isHiddenPowerType)
            hpTypes[hpTypeCount++] = i;
    }
    return hpTypes[((hpTypeCount - 1) * typeBits) / 63];
}

static void DexNavPrintDetailsText(u8 windowId)
{
    enum Species species = sDexNavSearchDataPtr->species;
    u32 personality = sDexNavSearchDataPtr->personality;
    u32 i, x;
    u8 gender;

    // Line 0: shiny or not, and the current odds
    if (sDexNavSearchDataPtr->isShiny)
        AddTextPrinterParameterized3(windowId, FONT_SMALL, 0, DETAILS_LINE(0), sDetailsShinyFontColor, TEXT_SKIP_DRAW, sText_Details_Shiny);
    else
        AddTextPrinterParameterized3(windowId, FONT_SMALL, 0, DETAILS_LINE(0), sSearchFontColor, TEXT_SKIP_DRAW, sText_Details_NotShiny);
    ConvertIntToDecimalStringN(gStringVar1, DexNavGetShinyOddsDenominator(), STR_CONV_MODE_LEFT_ALIGN, 5);
    StringExpandPlaceholders(gStringVar4, sText_Details_ShinyOdds);
    AddTextPrinterParameterized3(windowId, FONT_SMALL, 112, DETAILS_LINE(0), sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);

    // Line 1: nature, gender and Hidden Power type
    StringCopy(gStringVar1, gNaturesInfo[GetNatureFromPersonality(personality)].name);
    StringExpandPlaceholders(gStringVar4, sText_Details_Nature);
    AddTextPrinterParameterized3(windowId, FONT_SMALL, 0, DETAILS_LINE(1), sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);
    gender = GetGenderFromSpeciesAndPersonality(species, personality);
    if (gender == MON_MALE)
        AddTextPrinterParameterized3(windowId, FONT_SMALL, 92, DETAILS_LINE(1), sSearchFontColor, TEXT_SKIP_DRAW, sText_Details_Male);
    else if (gender == MON_FEMALE)
        AddTextPrinterParameterized3(windowId, FONT_SMALL, 92, DETAILS_LINE(1), sSearchFontColor, TEXT_SKIP_DRAW, sText_Details_Female);
    StringCopy(gStringVar1, gTypesInfo[DexNavGetHiddenPowerType()].name);
    StringExpandPlaceholders(gStringVar4, sText_Details_HiddenPower);
    AddTextPrinterParameterized3(windowId, FONT_SMALL, 112, DETAILS_LINE(1), sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);

    // Line 2: ability, marking the hidden one
    StringCopy(gStringVar1, gAbilitiesInfo[GetAbilityBySpecies(species, sDexNavSearchDataPtr->abilityNum)].name);
    StringExpandPlaceholders(gStringVar4, (sDexNavSearchDataPtr->abilityNum == 2) ? sText_Details_HiddenAbility : sText_Details_Ability);
    AddTextPrinterParameterized3(windowId, FONT_SMALL, 0, DETAILS_LINE(2), sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);

    // Line 3: IVs, perfect ones in the shiny color
    AddTextPrinterParameterized3(windowId, FONT_SMALL, 0, DETAILS_LINE(3), sSearchFontColor, TEXT_SKIP_DRAW, sText_Details_IVs);
    for (i = 0, x = 16; i < NUM_STATS; i++, x += 34)
    {
        const u8 *color = (sDexNavSearchDataPtr->ivs[i] == MAX_PER_STAT_IVS) ? sDetailsShinyFontColor : sSearchFontColor;

        StringCopy(gStringVar4, sText_Details_StatNames[i]);
        ConvertIntToDecimalStringN(gStringVar1, sDexNavSearchDataPtr->ivs[i], STR_CONV_MODE_LEADING_ZEROS, 2);
        StringAppend(gStringVar4, gStringVar1);
        AddTextPrinterParameterized3(windowId, FONT_SMALL, x, DETAILS_LINE(3), color, TEXT_SKIP_DRAW, gStringVar4);
    }

    // Lines 4-5: the four moves
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (sDexNavSearchDataPtr->moves[i] == MOVE_NONE)
            continue;
        AddTextPrinterParameterized3(windowId, FONT_SMALL, (i % 2) * 104, DETAILS_LINE(4 + i / 2), sSearchFontColor, TEXT_SKIP_DRAW,
                                     GetMoveName(sDexNavSearchDataPtr->moves[i]));
    }
}

static const u8 sText_Details_Types[] = _("Tipo: {STR_VAR_1}");
static const u8 sText_Details_Appears[] = _("Aparece: {STR_VAR_1}%");
static const u8 sText_Details_AppearsHidden[] = _("Aparece: oculto");
static const u8 sText_Details_Base[] = _("Base");
static const u8 sText_Details_EvYield[] = _("Da EV:");
static const u8 sText_Details_CatchRate[] = _("Ratio captura: {STR_VAR_1}");
static const u8 sText_Details_CatchHeader[] = _("Captura aprox. (PS llenos / PS bajos):");
static const u8 sText_Details_NoneOwned[] = _("No tienes ninguno de esta especie.");
static const u8 sText_Details_BestOwned[] = _("Tu mejor: suma IV {STR_VAR_1} · Este: {STR_VAR_2}");
static const u8 sText_Details_SearchLevel[] = _("Nv. búsqueda {STR_VAR_1}");
static const u8 *const sText_Details_Balls[] =
{
    COMPOUND_STRING("Poké"),
    COMPOUND_STRING("Super"),
    COMPOUND_STRING("Ultra"),
};
// Ball multipliers in tenths for Poké Ball, Great Ball and Ultra Ball
static const u8 sDetailsBallMultipliers[] = {10, 15, 20};

// Gen 3-style odds without status or special-case modifiers: a = rate * ball * (3M - 2H) / 3M,
// and the chance of a capture is about a / 255.
static u32 DexNavCatchPercent(u32 catchRate, u32 ballTenths, bool32 lowHp)
{
    u32 a = catchRate * ballTenths / (lowHp ? 10 : 30);
    return min(100, a * 100 / 255);
}

static u32 DexNavIvTotal(const u8 *ivs)
{
    u32 i, total = 0;
    for (i = 0; i < NUM_STATS; i++)
        total += ivs[i];
    return total;
}

// Highest IV total among the player's party and boxes for this species, or -1 if none is owned
static s32 DexNavGetBestOwnedIvTotal(enum Species species)
{
    s32 best = -1;
    u32 i, j, stat, total;

    for (i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
    {
        if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES) != species || GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_IS_EGG))
            continue;
        for (stat = 0, total = 0; stat < NUM_STATS; stat++)
            total += GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_HP_IV + stat);
        best = max(best, (s32)total);
    }
    for (i = 0; i < TOTAL_BOXES_COUNT; i++)
    {
        for (j = 0; j < IN_BOX_COUNT; j++)
        {
            if (GetBoxMonDataAt(i, j, MON_DATA_SPECIES) != species || GetBoxMonDataAt(i, j, MON_DATA_IS_EGG))
                continue;
            for (stat = 0, total = 0; stat < NUM_STATS; stat++)
                total += GetBoxMonDataAt(i, j, MON_DATA_HP_IV + stat);
            best = max(best, (s32)total);
        }
    }
    return best;
}

static void DexNavPrintSpeciesPageText(u8 windowId)
{
    enum Species species = sDexNavSearchDataPtr->species;
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    const u8 baseStats[NUM_STATS] =
    {
        [STAT_HP] = info->baseHP, [STAT_ATK] = info->baseAttack, [STAT_DEF] = info->baseDefense,
        [STAT_SPEED] = info->baseSpeed, [STAT_SPATK] = info->baseSpAttack, [STAT_SPDEF] = info->baseSpDefense,
    };
    const u8 evYield[NUM_STATS] =
    {
        [STAT_HP] = info->evYield_HP, [STAT_ATK] = info->evYield_Attack, [STAT_DEF] = info->evYield_Defense,
        [STAT_SPEED] = info->evYield_Speed, [STAT_SPATK] = info->evYield_SpAttack, [STAT_SPDEF] = info->evYield_SpDefense,
    };
    u32 i, x;
    s32 bestOwned;

    // Line 0: types and how often it appears here
    StringCopy(gStringVar1, gTypesInfo[info->types[0]].name);
    if (info->types[1] != info->types[0])
    {
        StringAppend(gStringVar1, COMPOUND_STRING("/"));
        StringAppend(gStringVar1, gTypesInfo[info->types[1]].name);
    }
    StringExpandPlaceholders(gStringVar4, sText_Details_Types);
    AddTextPrinterParameterized3(windowId, FONT_SMALL, 0, DETAILS_LINE(0), sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);
    if (sDexNavSearchDataPtr->isHiddenMon)
    {
        StringCopy(gStringVar4, sText_Details_AppearsHidden);
    }
    else
    {
        enum WildPokemonArea area = (sDexNavSearchDataPtr->environment == ENCOUNTER_TYPE_WATER) ? WILD_AREA_WATER : WILD_AREA_LAND;
        ConvertIntToDecimalStringN(gStringVar1, GetCurrentMapWildMonEncounterPercent(species, area), STR_CONV_MODE_LEFT_ALIGN, 3);
        StringExpandPlaceholders(gStringVar4, sText_Details_Appears);
    }
    AddTextPrinterParameterized3(windowId, FONT_SMALL, 140, DETAILS_LINE(0), sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);

    // Line 1: base stats
    AddTextPrinterParameterized3(windowId, FONT_SMALL, 0, DETAILS_LINE(1), sSearchFontColor, TEXT_SKIP_DRAW, sText_Details_Base);
    for (i = 0, x = 24; i < NUM_STATS; i++, x += 33)
    {
        StringCopy(gStringVar4, sText_Details_StatNames[i]);
        ConvertIntToDecimalStringN(gStringVar1, baseStats[i], STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(gStringVar4, gStringVar1);
        AddTextPrinterParameterized3(windowId, FONT_SMALL, x, DETAILS_LINE(1), sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);
    }

    // Line 2: EV yield and catch rate
    AddTextPrinterParameterized3(windowId, FONT_SMALL, 0, DETAILS_LINE(2), sSearchFontColor, TEXT_SKIP_DRAW, sText_Details_EvYield);
    for (i = 0, x = 30; i < NUM_STATS; i++)
    {
        if (evYield[i] == 0)
            continue;
        StringCopy(gStringVar4, COMPOUND_STRING("+"));
        ConvertIntToDecimalStringN(gStringVar1, evYield[i], STR_CONV_MODE_LEFT_ALIGN, 1);
        StringAppend(gStringVar4, gStringVar1);
        StringAppend(gStringVar4, sText_Details_StatNames[i]);
        AddTextPrinterParameterized3(windowId, FONT_SMALL, x, DETAILS_LINE(2), sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);
        x += 30;
    }
    ConvertIntToDecimalStringN(gStringVar1, info->catchRate, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, sText_Details_CatchRate);
    AddTextPrinterParameterized3(windowId, FONT_SMALL, 140, DETAILS_LINE(2), sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);

    // Lines 3-4: approximate capture odds with the basic balls
    AddTextPrinterParameterized3(windowId, FONT_SMALL, 0, DETAILS_LINE(3), sSearchFontColor, TEXT_SKIP_DRAW, sText_Details_CatchHeader);
    for (i = 0; i < ARRAY_COUNT(sDetailsBallMultipliers); i++)
    {
        StringCopy(gStringVar4, sText_Details_Balls[i]);
        StringAppend(gStringVar4, COMPOUND_STRING(" "));
        ConvertIntToDecimalStringN(gStringVar1, DexNavCatchPercent(info->catchRate, sDetailsBallMultipliers[i], FALSE), STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(gStringVar4, gStringVar1);
        StringAppend(gStringVar4, COMPOUND_STRING("/"));
        ConvertIntToDecimalStringN(gStringVar1, DexNavCatchPercent(info->catchRate, sDetailsBallMultipliers[i], TRUE), STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(gStringVar4, gStringVar1);
        StringAppend(gStringVar4, COMPOUND_STRING("%"));
        AddTextPrinterParameterized3(windowId, FONT_SMALL, i * 72, DETAILS_LINE(4), sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);
    }

    // Line 5: your best one of this species, and the search level
    bestOwned = DexNavGetBestOwnedIvTotal(species);
    if (bestOwned < 0)
    {
        StringCopy(gStringVar4, sText_Details_NoneOwned);
    }
    else
    {
        ConvertIntToDecimalStringN(gStringVar1, bestOwned, STR_CONV_MODE_LEFT_ALIGN, 3);
        ConvertIntToDecimalStringN(gStringVar2, DexNavIvTotal(sDexNavSearchDataPtr->ivs), STR_CONV_MODE_LEFT_ALIGN, 3);
        StringExpandPlaceholders(gStringVar4, sText_Details_BestOwned);
    }
    AddTextPrinterParameterized3(windowId, FONT_SMALL, 0, DETAILS_LINE(5), sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);
    ConvertIntToDecimalStringN(gStringVar1, sDexNavSearchDataPtr->searchLevel, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, sText_Details_SearchLevel);
    AddTextPrinterParameterized3(windowId, FONT_SMALL, 160, DETAILS_LINE(5), sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);
}

static void DexNavCloseDetails(void)
{
    sDexNavSearchDataPtr->detailsPage = 0;
    if (sDexNavSearchDataPtr->detailsWindowId == WINDOW_NONE)
        return;
    ClearStdWindowAndFrameToTransparent(sDexNavSearchDataPtr->detailsWindowId, FALSE);
    CopyWindowToVram(sDexNavSearchDataPtr->detailsWindowId, COPYWIN_FULL);
    RemoveWindow(sDexNavSearchDataPtr->detailsWindowId);
    sDexNavSearchDataPtr->detailsWindowId = WINDOW_NONE;
}

static void DexNavShowDetailsPage(u8 page)
{
    FillWindowPixelBuffer(sDexNavSearchDataPtr->detailsWindowId, PIXEL_FILL(1));
    if (page == 1)
        DexNavPrintDetailsText(sDexNavSearchDataPtr->detailsWindowId);
    else
        DexNavPrintSpeciesPageText(sDexNavSearchDataPtr->detailsWindowId);
    CopyWindowToVram(sDexNavSearchDataPtr->detailsWindowId, COPYWIN_GFX);
    sDexNavSearchDataPtr->detailsPage = page;
}

static void DexNavOpenDetails(void)
{
    struct WindowTemplate template;
    // Sits on the opposite half of the screen from the search window
    u8 y = (GetWindowAttribute(sDexNavSearchDataPtr->windowId, WINDOW_TILEMAP_TOP) == 1) ? 7 : 1;

    SetWindowTemplateFields(&template, 0, 1, y, SEARCH_WINDOW_WIDTH_DETAILS, DETAILS_WINDOW_HEIGHT, 14, 100);
    sDexNavSearchDataPtr->detailsWindowId = AddWindow(&template);
    // The frame call clears the window, so it goes before the text
    DrawStdFrameWithCustomTileAndPalette(sDexNavSearchDataPtr->detailsWindowId, FALSE, 0x214, 14);
    DexNavShowDetailsPage(1);
    CopyWindowToVram(sDexNavSearchDataPtr->detailsWindowId, COPYWIN_FULL);
}

void DexNav_CloseDetailsPanel(void)
{
    if (FlagGet(DN_FLAG_SEARCHING) && sDexNavSearchDataPtr != NULL)
        DexNavCloseDetails();
}

// Called from the field input handler. Returns TRUE when it used the SELECT press.
bool32 DexNav_TryToggleDetails(void)
{
    if (!FlagGet(DN_FLAG_SEARCHING) || sDexNavSearchDataPtr == NULL || sDexNavSearchDataPtr->hiddenSearch)
        return FALSE;

    PlaySE(SE_SELECT);
    if (sDexNavSearchDataPtr->detailsWindowId == WINDOW_NONE)
        DexNavOpenDetails();
    else if (sDexNavSearchDataPtr->detailsPage == 1)
        DexNavShowDetailsPage(2);
    else
        DexNavCloseDetails();
    return TRUE;
}

static void RemoveDexNavWindowAndGfx(void)
{
    DexNavCloseDetails();

    // try remove sprites
    if (sDexNavSearchDataPtr->iconSpriteId != MAX_SPRITES)
        DestroySprite(&gSprites[sDexNavSearchDataPtr->iconSpriteId]);
    if (sDexNavSearchDataPtr->itemSpriteId != MAX_SPRITES)
        DestroySprite(&gSprites[sDexNavSearchDataPtr->itemSpriteId]);
    if (sDexNavSearchDataPtr->eyeSpriteId != MAX_SPRITES)
        DestroySprite(&gSprites[sDexNavSearchDataPtr->eyeSpriteId]);
    if (sDexNavSearchDataPtr->ownedIconSpriteId != MAX_SPRITES)
        DestroySprite(&gSprites[sDexNavSearchDataPtr->ownedIconSpriteId]);
    if (sDexNavSearchDataPtr->exclamationSpriteId != MAX_SPRITES)
        DestroySprite(&gSprites[sDexNavSearchDataPtr->exclamationSpriteId]);

    for (u32 i = 0; i < NELEMS(sDexNavSearchDataPtr->starSpriteIds); i++)
    {
        if (sDexNavSearchDataPtr->starSpriteIds[i] != MAX_SPRITES)
            DestroySprite(&gSprites[sDexNavSearchDataPtr->starSpriteIds[i]]);
    }

    FreeSpriteTilesByTag(HELD_ITEM_TAG);
    FreeSpriteTilesByTag(OWNED_ICON_TAG);
    FreeSpriteTilesByTag(HIDDEN_SEARCH_TAG);
    FreeSpriteTilesByTag(HIDDEN_MON_ICON_TAG);
    FreeSpriteTilesByTag(LIT_STAR_TILE_TAG);
    FreeSpritePaletteByTag(HELD_ITEM_TAG);
    SafeFreeMonIconPalette(sDexNavSearchDataPtr->species);

    // remove window
    ClearStdWindowAndFrameToTransparent(sDexNavSearchDataPtr->windowId, FALSE);
    CopyWindowToVram(sDexNavSearchDataPtr->windowId, 3);
    RemoveWindow(sDexNavSearchDataPtr->windowId);
}


//////////////////////
////DEXNAV SEARCH/////
//////////////////////
static u8 GetPlayerDistance(s16 x, s16 y)
{
    u16 deltaX = abs(x - (gSaveBlock1Ptr->pos.x + 7));
    u16 deltaY = abs(y - (gSaveBlock1Ptr->pos.y + 7));
    return deltaX + deltaY;
}

static void DexNavProximityUpdate(void)
{
    sDexNavSearchDataPtr->proximity = GetPlayerDistance(sDexNavSearchDataPtr->tileX, sDexNavSearchDataPtr->tileY);
}

//Pick a specific tile based on environment
static bool8 DexNavPickTile(enum EncounterType environment, u8 areaX, u8 areaY, bool8 smallScan)
{
    // area of map to cover starting from camera position {-7, -7}
    s16 topX = gSaveBlock1Ptr->pos.x - SCANSTART_X + (smallScan * 5);
    s16 topY = gSaveBlock1Ptr->pos.y - SCANSTART_Y + (smallScan * 5);
    s16 botX = topX + areaX;
    s16 botY = topY + areaY;
    u8 i;
    bool8 nextIter;
    u8 scale = 0;
    u8 weight = 0;
    enum MapType currMapType = GetCurrentMapType();
    u8 tileBehaviour;
    u8 tileBuffer = 2;
    u8 *xPos = AllocZeroed((botX - topX) * (botY - topY) * sizeof(u8));
    u8 *yPos = AllocZeroed((botX - topX) * (botY - topY) * sizeof(u8));
    u32 iter = 0;
    bool32 ret = FALSE;

    // loop through every tile in area and evaluate
    while (topY < botY)
    {
        while (topX < botX)
        {
            tileBehaviour = MapGridGetMetatileBehaviorAt(topX, topY);
            //Check for objects
            nextIter = FALSE;
            if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_BIKE))
                tileBuffer = SNEAKING_PROXIMITY + 3;
            else if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_DASH))
                tileBuffer = SNEAKING_PROXIMITY + 1;

            if (GetPlayerDistance(topX, topY) <= tileBuffer)
            {
                // tile too close to player
                topX++;
                continue;
            }

            for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
            {
                if (gObjectEvents[i].currentCoords.x == topX && gObjectEvents[i].currentCoords.y == topY)
                {
                    // cannot be on a tile where an object exists
                    nextIter = TRUE;
                    break;
                }
            }

            if (nextIter)
            {
                topX++;
                continue;
            }

            weight = 0; // initiliaze weight
            switch (environment)
            {
            case ENCOUNTER_TYPE_LAND:
                if (MetatileBehavior_IsLandWildEncounter(tileBehaviour))
                {
                    if (currMapType == MAP_TYPE_UNDERGROUND)
                    {
                        // inside (cave)
                        if (IsElevationMismatchAt(gObjectEvents[gPlayerAvatar.spriteId].currentElevation, topX, topY))
                            break; //occurs at same z coord

                        scale = 440 - (smallScan * 200) - (GetPlayerDistance(topX, topY) / 2)  - (2 * (topX + topY));
                        weight = ((Random() % scale) < 1) && !MapGridGetCollisionAt(topX, topY);
                    }
                    else
                    {
                        // outdoors: grass
                        scale = 100 - (GetPlayerDistance(topX, topY) * 2);
                        weight = (Random() % scale <= 5) && !MapGridGetCollisionAt(topX, topY);
                    }
                }
                break;
            case ENCOUNTER_TYPE_WATER:
                if (MetatileBehavior_IsSurfableWaterOrUnderwater(tileBehaviour))
                {
                    u8 scale = 320 - (smallScan * 200) - (GetPlayerDistance(topX, topY) / 2);
                    if (IsElevationMismatchAt(gObjectEvents[gPlayerAvatar.spriteId].currentElevation, topX, topY))
                        break;

                    weight = (Random() % scale <= 1) && !MapGridGetCollisionAt(topX, topY);
                }
                break;
            default:
                break;
            }

            if (weight > 0)
            {
                xPos[iter] = topX;
                yPos[iter] = topY;
                iter++;
            }

            topX++;
        }

        topY++;
        topX = gSaveBlock1Ptr->pos.x - SCANSTART_X + (smallScan * 5);
    }

    if (iter > 0)
    {
        i = Random() % iter;
        sDexNavSearchDataPtr->tileX = xPos[i];
        sDexNavSearchDataPtr->tileY = yPos[i];
        ret = TRUE;
    }

    Free(xPos);
    Free(yPos);

    return ret;
}


static bool8 TryStartHiddenMonFieldEffect(enum EncounterType environment, u8 xSize, u8 ySize, bool8 smallScan)
{
    enum MapType currMapType = GetCurrentMapType();
    u8 fldEffId = 0;

    if (DexNavPickTile(environment, xSize, ySize, smallScan))
    {
        u8 metatileBehaviour = MapGridGetMetatileBehaviorAt(sDexNavSearchDataPtr->tileX, sDexNavSearchDataPtr->tileY);

        switch (environment)
        {
        case ENCOUNTER_TYPE_LAND:
            if (currMapType == MAP_TYPE_UNDERGROUND)
            {
                fldEffId = FLDEFF_CAVE_DUST;
            }
            else if (IsMapTypeIndoors(currMapType))
            {
                if (MetatileBehavior_IsTallGrass(metatileBehaviour)) //Grass in cave
                    fldEffId = FLDEFF_SHAKING_GRASS;
                else if (MetatileBehavior_IsLongGrass(metatileBehaviour)) //Really tall grass
                    fldEffId = FLDEFF_SHAKING_LONG_GRASS;
                else if (MetatileBehavior_IsSandOrDeepSand(metatileBehaviour))
                    fldEffId = FLDEFF_SAND_HOLE;
                else
                    fldEffId = FLDEFF_CAVE_DUST;
            }
            else //outdoor, underwater
            {
                if (MetatileBehavior_IsTallGrass(metatileBehaviour)) //Regular grass
                    fldEffId = FLDEFF_SHAKING_GRASS;
                else if (MetatileBehavior_IsLongGrass(metatileBehaviour)) //Really tall grass
                    fldEffId = FLDEFF_SHAKING_LONG_GRASS;
                else if (MetatileBehavior_IsSandOrDeepSand(metatileBehaviour)) //Desert Sand
                    fldEffId = FLDEFF_SAND_HOLE;
                else if (MetatileBehavior_IsMountain(metatileBehaviour)) //Rough Terrain
                    fldEffId = FLDEFF_CAVE_DUST;
                else
                    fldEffId = FLDEFF_BERRY_TREE_GROWTH_SPARKLE; //default
            }
            break;
        case ENCOUNTER_TYPE_WATER:
            fldEffId = FLDEFF_WATER_SURFACING;
            break;
        default:
            return FALSE;
        }

        if (fldEffId != 0)
        {
            gFieldEffectArguments[0] = sDexNavSearchDataPtr->tileX;
            gFieldEffectArguments[1] = sDexNavSearchDataPtr->tileY;
            gFieldEffectArguments[2] = 0xFF; // subpriority
            gFieldEffectArguments[3] = 2;   //priority
            sDexNavSearchDataPtr->fldEffSpriteId = FieldEffectStart(fldEffId);
            if (sDexNavSearchDataPtr->fldEffSpriteId == MAX_SPRITES)
                return FALSE;

            sDexNavSearchDataPtr->fldEffId = fldEffId;
            return TRUE;
        }
    }

    return FALSE;
}

static void DrawDexNavSearchHeldItem(u8 *dst)
{
    *dst = CreateSpriteUnchecked(&sHeldItemTemplate, SPECIES_ICON_X + 6, GetSearchWindowY() + 18, 0);
    if (*dst != MAX_SPRITES)
        gSprites[*dst].invisible = TRUE;
}

static void LoadSearchIconData(void)
{
    // palettes clash with mon icon, so must load manually
    LoadSpriteSheet(&gSpriteSheet_HeldItem);
    LoadPalette(gHeldItemPalette, OBJ_PLTT_ID(sHeldItemOam.paletteNum), PLTT_SIZE_4BPP);
    LoadCompressedSpriteSheetUsingHeap(&sPotentialStarSpriteSheet);
    //LoadCompressedSpriteSheetUsingHeap(&sSightSpriteSheet);   //eye replaced with arrow
    LoadCompressedSpriteSheetUsingHeap(&sOwnedIconSpriteSheet);
    LoadCompressedSpriteSheetUsingHeap(&sHiddenMonIconSpriteSheet);
}

static u8 GetSearchLevel(enum Species species)
{
    u8 searchLevel;
#if USE_DEXNAV_SEARCH_LEVELS == TRUE
    searchLevel = gSaveBlock3Ptr->dexNavSearchLevels[SpeciesToNationalPokedexNum(species)];
#else
    searchLevel = 0;
#endif
    return searchLevel;
}

// Rojo Fuego+: rolls nature, gender, IVs and shininess up front with the same rules as a normal
// wild encounter, so the details panel can show them before the battle.
static void DexNavRollIndividual(void)
{
    enum Species species = sDexNavSearchDataPtr->species;
    u32 i, numPerfect = min(3, sDexNavSearchDataPtr->potential); // Will not exceed 3 Perfect IVs
    u32 ivRandom = Random32();
    u8 stats[NUM_STATS];

    sDexNavSearchDataPtr->personality = GetMonPersonality(species,
                                                          GetSynchronizedGender(WILDMON_ORIGIN, species),
                                                          GetSynchronizedNature(WILDMON_ORIGIN, species),
                                                          RANDOM_UNOWN_LETTER);

    // The DexNav chain bonus only counts while gDexNavSpecies is set
    gDexNavSpecies = species;
    sDexNavSearchDataPtr->isShiny = ComputePlayerShinyOdds(sDexNavSearchDataPtr->personality, READ_OTID_FROM_SAVE);
    gDexNavSpecies = SPECIES_NONE;

    for (i = 0; i < NUM_STATS; i++)
    {
        sDexNavSearchDataPtr->ivs[i] = (ivRandom >> (i * 5)) & MAX_IV_MASK;
        stats[i] = i;
    }
    Shuffle(stats, NUM_STATS, sizeof(stats[0]));
    for (i = 0; i < numPerfect; i++)
        sDexNavSearchDataPtr->ivs[stats[i]] = MAX_PER_STAT_IVS;
}

static const u8 *const sText_DexNavFilterNames[DEXNAV_FILTER_COUNT] =
{
    [DEXNAV_FILTER_NONE]           = COMPOUND_STRING("Todo"),
    [DEXNAV_FILTER_SHINY]          = COMPOUND_STRING("Shiny"),
    [DEXNAV_FILTER_HIDDEN_ABILITY] = COMPOUND_STRING("H.Oculta"),
    [DEXNAV_FILTER_THREE_STARS]    = COMPOUND_STRING("3 estr."),
};

static bool32 DexNavMatchesFilter(void)
{
    switch (VarGet(VAR_DEXNAV_FILTER))
    {
    case DEXNAV_FILTER_SHINY:
        return sDexNavSearchDataPtr->isShiny;
    case DEXNAV_FILTER_HIDDEN_ABILITY:
        return sDexNavSearchDataPtr->abilityNum == 2;
    case DEXNAV_FILTER_THREE_STARS:
        return sDexNavSearchDataPtr->potential >= 3;
    default:
        return TRUE;
    }
}

static void SetUpDexNavSearch(void)
{
    enum Species species = sDexNavSearchDataPtr->species;
    u8 searchLevel = GetSearchLevel(species);

    // init sprites
    sDexNavSearchDataPtr->iconSpriteId = MAX_SPRITES;
    sDexNavSearchDataPtr->itemSpriteId = MAX_SPRITES;
    sDexNavSearchDataPtr->eyeSpriteId = MAX_SPRITES;
    sDexNavSearchDataPtr->starSpriteIds[0] = MAX_SPRITES;
    sDexNavSearchDataPtr->starSpriteIds[1] = MAX_SPRITES;
    sDexNavSearchDataPtr->starSpriteIds[2] = MAX_SPRITES;
    sDexNavSearchDataPtr->ownedIconSpriteId = MAX_SPRITES;
    sDexNavSearchDataPtr->exclamationSpriteId = MAX_SPRITES;
    sDexNavSearchDataPtr->searchLevel = searchLevel;

    DexNavGenerateMoveset(species, searchLevel, sDexNavSearchDataPtr->monLevel, &sDexNavSearchDataPtr->moves[0]);
    sDexNavSearchDataPtr->heldItem = DexNavGenerateHeldItem(species, searchLevel);
    sDexNavSearchDataPtr->abilityNum = DexNavGetAbilityNum(species, searchLevel);
    sDexNavSearchDataPtr->potential = DexNavGeneratePotential(searchLevel);
    DexNavRollIndividual();
    sDexNavSearchDataPtr->detailsWindowId = WINDOW_NONE;
    DexNavProximityUpdate();

    LoadSearchIconData();
    if (sDexNavSearchDataPtr->hiddenSearch)
    {
        DexNavDrawHiddenIcons();
    }
    else
    {
        DexNavDrawIcons();
        DexNavUpdateSearchWindow(sDexNavSearchDataPtr->proximity, searchLevel);
    }

    gPlayerAvatar.creeping = TRUE;  //initialize as true in case mon appears beside you
    sDexNavSearchDataPtr->proximity = gSprites[gPlayerAvatar.spriteId].x;
    sDexNavSearchDataPtr->startingTime = gMain.vblankCounter1;
    IncrementGameStat(GAME_STAT_DEXNAV_SCANNED);
}

static void DexNavSearchBail(const u8 *script)
{
    TRY_FREE_AND_SET_NULL(sDexNavSearchDataPtr);
    FlagClear(DN_FLAG_SEARCHING);
    FreeMonIconPalettes();
    ScriptContext_SetupScript(script);
}

static bool8 InitDexNavSearch(enum Species species, u32 environment)
{
    sDexNavSearchDataPtr = AllocZeroed(sizeof(struct DexNavSearch));
    if (sDexNavSearchDataPtr == NULL)
    {
        DexNavSearchBail(EventScript_NotFoundNearby);
        return TRUE;
    }
    FlagSet(DN_FLAG_SEARCHING);

    // assign non-objects to struct
    sDexNavSearchDataPtr->species = species;
    sDexNavSearchDataPtr->environment = environment;  //updated in DexNavTryGenerateMonLevel if hidden mon
    sDexNavSearchDataPtr->isHiddenMon = (environment == ENCOUNTER_TYPE_HIDDEN) ? TRUE : FALSE;
    sDexNavSearchDataPtr->monLevel = DexNavTryGenerateMonLevel(species, environment);

    if (GetFlashLevel() > 0)
    {
        DexNavSearchBail(EventScript_TooDark);
        return TRUE;
    }

    if (sDexNavSearchDataPtr->monLevel == MON_LEVEL_NONEXISTENT || !TryStartHiddenMonFieldEffect(sDexNavSearchDataPtr->environment, 12, 12, FALSE))
    {
        DexNavSearchBail(EventScript_NotFoundNearby);
        return TRUE;
    }

    sDexNavSearchDataPtr->hiddenSearch = FALSE;
    SetUpDexNavSearch();

    // Rojo Fuego+ hunter mode: a Pokémon that does not match the filter is not worth chasing.
    // The chain is kept, the player just has to search again.
    if (!DexNavMatchesFilter())
    {
        EndDexNavSearch();
        ScriptContext_SetupScript(EventScript_DexNavFilterNoMatch);
        return TRUE;
    }
    return FALSE;
}

static void DexNavDrawPotentialStars(u8 potential, u8 *dst)
{
    u8 spriteId;
    u32 i;

    for (i = 0; i < NELEMS(sDexNavSearchDataPtr->starSpriteIds); i++)
    {
        spriteId = MAX_SPRITES;
        if (potential > i)
            spriteId = CreateSpriteUnchecked(&sPotentialStarTemplate, SPECIES_ICON_X - 20, GetSearchWindowY() + 4 + (i * 8), 0);

        dst[i] = spriteId;
        if (spriteId != MAX_SPRITES)
            gSprites[spriteId].invisible = TRUE;
    }
}

static void DexNavUpdateDirectionArrow(void)
{
    u16 tileX = sDexNavSearchDataPtr->tileX;
    u16 tileY = sDexNavSearchDataPtr->tileY;
    u16 playerX = gSaveBlock1Ptr->pos.x + MAP_OFFSET;
    u16 playerY = gSaveBlock1Ptr->pos.y + MAP_OFFSET;
    u16 deltaX = abs(tileX - playerX);
    u16 deltaY = abs(tileY - playerY);
    const u8 *str;
    u8 windowId = sDexNavSearchDataPtr->windowId;

    FillWindowPixelRect(windowId, PIXEL_FILL(1), SEARCH_ARROW_X, SEARCH_ARROW_Y, 12, 12);
    if (deltaX <= 1 && deltaY <= 1)
    {
        str = gText_EmptyString2;
    }
    else if (deltaX > deltaY)
    {
        if (playerX > tileX)
            str = sText_ArrowLeft;  //player to right
        else
            str = sText_ArrowRight; //player to left
    }
    else //greater Y diff
    {
        if (playerY > tileY)
            str = sText_ArrowUp;    //player below
        else
            str = sText_ArrowDown;  //player above
    }

    AddTextPrinterParameterized3(windowId, FONT_NORMAL, SEARCH_ARROW_X, SEARCH_ARROW_Y, sSearchFontColor, TEXT_SKIP_DRAW, str);
    CopyWindowToVram(windowId, 2);
}

static void DexNavDrawIcons(void)
{
    enum Species species = sDexNavSearchDataPtr->species;

    DrawSearchWindow(species, sDexNavSearchDataPtr->potential, FALSE);
    DrawDexNavSearchMonIcon(species, &sDexNavSearchDataPtr->iconSpriteId, GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT));
    DrawDexNavSearchHeldItem(&sDexNavSearchDataPtr->itemSpriteId);
    DexNavDrawPotentialStars(sDexNavSearchDataPtr->potential, &sDexNavSearchDataPtr->starSpriteIds[0]);
    DexNavUpdateDirectionArrow();
}

/////////////////////
//// SEARCH TASK ////
/////////////////////
static void RevealHiddenSearch(void)
{
    PlaySE(SE_DEX_SEARCH);
    ClearStdWindowAndFrameToTransparent(sDexNavSearchDataPtr->windowId, FALSE);
    CopyWindowToVram(sDexNavSearchDataPtr->windowId, 3);
    RemoveWindow(sDexNavSearchDataPtr->windowId);
    DestroySprite(&gSprites[sDexNavSearchDataPtr->iconSpriteId]);
    sDexNavSearchDataPtr->hiddenSearch = FALSE; //now its a regular dexnav search
    RevealHiddenMon();
}

bool32 TryStartDexNavSearch(void)
{
    u16 val = VarGet(DN_VAR_SPECIES);

    if (FlagGet(DN_FLAG_SEARCHING) && sDexNavSearchDataPtr->hiddenSearch)
    {
        RevealHiddenSearch();
        return FALSE;
    }

    if (FlagGet(DN_FLAG_SEARCHING) || (val & DEXNAV_MASK_SPECIES) == SPECIES_NONE)
        return FALSE;

    HideMapNamePopUpWindow();
    ChangeBgY_ScreenOff(0, 0, 0);
    PlaySE(SE_DEX_SEARCH);
    return InitDexNavSearch(val & DEXNAV_MASK_SPECIES, val >> 14);
}

void EndDexNavSearch(void)
{
    if (!FlagGet(DN_FLAG_SEARCHING) || sDexNavSearchDataPtr == NULL)
        return;
    RemoveDexNavWindowAndGfx();
    FieldEffectStop(&gSprites[sDexNavSearchDataPtr->fldEffSpriteId], sDexNavSearchDataPtr->fldEffId);
    FREE_AND_SET_NULL(sDexNavSearchDataPtr);
    FlagClear(DN_FLAG_SEARCHING);
}

static void EndDexNavSearchSetupScript(const u8 *script)
{
    gSaveBlock3Ptr->dexNavChain = 0;   //reset chain
    EndDexNavSearch();
    ScriptContext_SetupScript(script);
}

static u8 GetMovementProximityBySearchLevel(void)
{
    if (sDexNavSearchDataPtr->searchLevel < 20)
        return 2;
    else if (sDexNavSearchDataPtr->searchLevel < 50)
        return 3;
    else
        return 4;
}

static void RevealHiddenMon(void)
{
    enum Species species = sDexNavSearchDataPtr->species;

    // remove owned icon if it exists
    if (sDexNavSearchDataPtr->ownedIconSpriteId != MAX_SPRITES)
    {
        DestroySprite(&gSprites[sDexNavSearchDataPtr->ownedIconSpriteId]);
        sDexNavSearchDataPtr->ownedIconSpriteId = MAX_SPRITES;
    }

    // remove exclamation if it exists
    if (sDexNavSearchDataPtr->exclamationSpriteId != MAX_SPRITES)
    {
        DestroySprite(&gSprites[sDexNavSearchDataPtr->exclamationSpriteId]);
        sDexNavSearchDataPtr->exclamationSpriteId = MAX_SPRITES;
    }


    if (!GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN))
    {
        u8 index;

        //if not seen, hide name and whiteout mon
        DrawSearchWindow(species, sDexNavSearchDataPtr->potential, TRUE);
        DrawDexNavSearchMonIcon(species, &sDexNavSearchDataPtr->iconSpriteId, FALSE);
        // whiteout icon
        index = IndexOfSpritePaletteTag(gSprites[sDexNavSearchDataPtr->iconSpriteId].template->paletteTag);
        CpuCopy16(&gPlttBufferUnfaded[OBJ_PLTT_ID(index)], sDexNavSearchDataPtr->palBuffer, 32);
        TintPalette_CustomTone(sDexNavSearchDataPtr->palBuffer, 16, 510, 510, 510);
        LoadPalette(sDexNavSearchDataPtr->palBuffer, OBJ_PLTT_ID(index), PLTT_SIZE_4BPP);
    }
    else
    {
        DrawSearchWindow(species, sDexNavSearchDataPtr->potential, FALSE);
        DrawDexNavSearchMonIcon(species, &sDexNavSearchDataPtr->iconSpriteId, GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT));
    }

    sDexNavSearchDataPtr->startingTime = gMain.vblankCounter1;
    DexNavUpdateDirectionArrow();
}

bool32 OnStep_DexNavSearch(void)
{
    if (!FlagGet(DN_FLAG_SEARCHING) || sDexNavSearchDataPtr == NULL)
        return FALSE;

    u32 frameCount = gMain.vblankCounter1 - sDexNavSearchDataPtr->startingTime;
    DexNavProximityUpdate();
    if (!sDexNavSearchDataPtr->hiddenSearch)    //update search window info only if revealed mon
        DexNavUpdateSearchWindow(sDexNavSearchDataPtr->proximity, sDexNavSearchDataPtr->searchLevel);

    if (sDexNavSearchDataPtr->proximity > MAX_PROXIMITY)
    { // out of range
        if (sDexNavSearchDataPtr->hiddenSearch)
        {
            EndDexNavSearch();
            return FALSE;
        }
        else
        {
            EndDexNavSearchSetupScript(EventScript_LostSignal);
            return TRUE;
        }
    }

    if (sDexNavSearchDataPtr->proximity <= CREEPING_PROXIMITY && !gPlayerAvatar.creeping && frameCount > 60)
    { //should be creeping but player walks normally
        if (sDexNavSearchDataPtr->hiddenSearch)
        {
            EndDexNavSearch();
            return FALSE;
        }
        else
        {
            EndDexNavSearchSetupScript(EventScript_MovedTooFast);
            return TRUE;
        }
    }

    if (sDexNavSearchDataPtr->proximity <= SNEAKING_PROXIMITY && TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_DASH | PLAYER_AVATAR_FLAG_BIKE))
    { // running/biking too close
        //always do event script, even if player hasn't revealed a hidden mon. It's assumed they would be creeping towards it
        EndDexNavSearchSetupScript(EventScript_MovedTooFast);
        return TRUE;
    }

    if (frameCount > DEXNAV_TIMEOUT * 60)
    { // player took too long
        if (sDexNavSearchDataPtr->hiddenSearch)
        {
            EndDexNavSearch();
            return FALSE;
        }
        else
        {
            EndDexNavSearchSetupScript(EventScript_PokemonGotAway);
            return TRUE;
        }
        return FALSE;
    }

    if (sDexNavSearchDataPtr->proximity < 1)
    {
        gDexNavSpecies = sDexNavSearchDataPtr->species;
        CreateDexNavWildMon(sDexNavSearchDataPtr->species, sDexNavSearchDataPtr->potential, sDexNavSearchDataPtr->monLevel,
                            sDexNavSearchDataPtr->abilityNum, sDexNavSearchDataPtr->heldItem, sDexNavSearchDataPtr->moves);

        ScriptContext_SetupScript(EventScript_StartDexNavBattle);
        FREE_AND_SET_NULL(sDexNavSearchDataPtr);
        FlagClear(DN_FLAG_SEARCHING);
        return TRUE;
    }

    if (sDexNavSearchDataPtr->hiddenSearch && sDexNavSearchDataPtr->proximity < CREEPING_PROXIMITY)
    {
        RevealHiddenSearch();
        return FALSE;
    }

    //Caves and water the Pokémon moves around
    if ((sDexNavSearchDataPtr->environment == ENCOUNTER_TYPE_WATER || GetCurrentMapType() == MAP_TYPE_UNDERGROUND)
        && sDexNavSearchDataPtr->proximity < GetMovementProximityBySearchLevel() && sDexNavSearchDataPtr->movementCount < 2
        && !sDexNavSearchDataPtr->hiddenSearch)
    {
        FieldEffectStop(&gSprites[sDexNavSearchDataPtr->fldEffSpriteId], sDexNavSearchDataPtr->fldEffId);

        if (!TryStartHiddenMonFieldEffect(sDexNavSearchDataPtr->environment, 10, 10, TRUE))
        {
            EndDexNavSearchSetupScript(EventScript_PokemonGotAway);
            return TRUE;
        }

        sDexNavSearchDataPtr->movementCount++;
    }
    return FALSE;
}

static void DexNavUpdateSearchWindow(u8 proximity, u8 searchLevel)
{
    bool8 hideName = FALSE;

    if (sDexNavSearchDataPtr->hiddenSearch && !GetSetPokedexFlag(SpeciesToNationalPokedexNum(sDexNavSearchDataPtr->species), FLAG_GET_SEEN))
        hideName = TRUE;    //if a detector mode hidden search and player hasn't seen the mon, hide info

    FillWindowPixelBuffer(sDexNavSearchDataPtr->windowId, PIXEL_FILL(1));   //clear window
    AddSearchWindowText(sDexNavSearchDataPtr->species, proximity, searchLevel, hideName);

    DexNavUpdateDirectionArrow();

    //init hidden sprites
    if (sDexNavSearchDataPtr->itemSpriteId != MAX_SPRITES)
        gSprites[sDexNavSearchDataPtr->itemSpriteId].invisible = TRUE;
    if (sDexNavSearchDataPtr->starSpriteIds[0] != MAX_SPRITES)
        gSprites[sDexNavSearchDataPtr->starSpriteIds[0]].invisible = TRUE;
    if (sDexNavSearchDataPtr->starSpriteIds[1] != MAX_SPRITES)
        gSprites[sDexNavSearchDataPtr->starSpriteIds[1]].invisible = TRUE;
    if (sDexNavSearchDataPtr->starSpriteIds[2] != MAX_SPRITES)
        gSprites[sDexNavSearchDataPtr->starSpriteIds[2]].invisible = TRUE;

    if (proximity <= SNEAKING_PROXIMITY)
    {
        if (searchLevel > 2 && sDexNavSearchDataPtr->heldItem)
        {
            // toggle item view
            if (sDexNavSearchDataPtr->itemSpriteId != MAX_SPRITES)
                gSprites[sDexNavSearchDataPtr->itemSpriteId].invisible = FALSE;
        }

        if (searchLevel > 4)
        {
            if (sDexNavSearchDataPtr->starSpriteIds[0] != MAX_SPRITES)
                gSprites[sDexNavSearchDataPtr->starSpriteIds[0]].invisible = FALSE;

            if (sDexNavSearchDataPtr->starSpriteIds[1] != MAX_SPRITES)
                gSprites[sDexNavSearchDataPtr->starSpriteIds[1]].invisible = FALSE;

            if (sDexNavSearchDataPtr->starSpriteIds[2] != MAX_SPRITES)
                gSprites[sDexNavSearchDataPtr->starSpriteIds[2]].invisible = FALSE;
        }
    }
}

//////////////////////////////
//// DEXNAV MON GENERATOR ////
//////////////////////////////
static void CreateDexNavWildMon(enum Species species, u8 potential, u8 level, u8 abilityNum, enum Item item, enum Move *moves)
{
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][0];
    u32 isShiny = sDexNavSearchDataPtr->isShiny;

    gRfDexNavBattle = TRUE; // Rojo Fuego+ achievements

    // Rojo Fuego+: build the exact individual that the details panel showed
    ZeroEnemyPartyMons();
    CreateMonWithIVs(mon, species, level, sDexNavSearchDataPtr->personality, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    GiveMonInitialMoveset(mon);
    for (u32 i = 0; i < NUM_STATS; i++)
        SetMonData(mon, MON_DATA_HP_IV + i, &sDexNavSearchDataPtr->ivs[i]);
    SetMonData(mon, MON_DATA_IS_SHINY, &isShiny);

    //Set ability
    SetMonData(mon, MON_DATA_ABILITY_NUM, &abilityNum);

    // Set Held Item
    if (item)
        SetMonData(mon, MON_DATA_HELD_ITEM, &item);

    //Set moves
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(mon, moves[i], i);

    CalculateMonStats(mon);
}

// gets a random level of the species based on map data.
//if it was a hidden encounter, updates the environment it is to be found from the wildheader encounterRate
static u8 DexNavTryGenerateMonLevel(enum Species species, enum EncounterType environment)
{
    u8 levelBase = GetEncounterLevelFromMapData(species, environment);
    u8 levelBonus = gSaveBlock3Ptr->dexNavChain / 5;

    if (levelBase == MON_LEVEL_NONEXISTENT)
        return MON_LEVEL_NONEXISTENT;   //species not found in the area

    if (Random() % 100 < 4)
        levelBonus += 10; //4% chance of having a +10 level

    if (levelBase + levelBonus > MAX_LEVEL)
        return MAX_LEVEL;
    else
        return levelBase + levelBonus;
}

static enum Move GetRandomEggMove(enum Species species)
{
    u32 numEggMoves = 0;
    const u16 *eggMoveLearnset = GetSpeciesEggMoves(species);
    for (u32 i = 0; eggMoveLearnset[i] != MOVE_UNAVAILABLE; i++)
        numEggMoves++;

    enum Move result = *(const u16 *)(RandomElementArray(RNG_DEXNAV_RANDOM_EGG_MOVE, eggMoveLearnset, sizeof(u16), numEggMoves));
    return result;
}

static void DexNavGenerateMoveset(enum Species species, u8 searchLevel, u8 encounterLevel, u16 *moveDst)
{
    bool8 genMove = FALSE;
    u16 randVal = Random() % 100;
    u16 i;

    // see if first move slot should be an egg move
    if (searchLevel < 5)
    {
        if (SEARCHLEVEL0_MOVECHANCE != 0 && randVal < SEARCHLEVEL0_MOVECHANCE)
            genMove = TRUE;
    }
    else if (searchLevel < 10)
    {
        if (SEARCHLEVEL5_MOVECHANCE != 0 && randVal < SEARCHLEVEL5_MOVECHANCE)
            genMove = TRUE;
    }
    else if (searchLevel < 25)
    {
        if (SEARCHLEVEL10_MOVECHANCE != 0 && randVal < SEARCHLEVEL10_MOVECHANCE)
            genMove = TRUE;
    }
    else if (searchLevel < 50)
    {
        if (SEARCHLEVEL25_MOVECHANCE != 0 && randVal < SEARCHLEVEL25_MOVECHANCE)
            genMove = TRUE;
    }
    else if (searchLevel < 100)
    {
        if (SEARCHLEVEL50_MOVECHANCE != 0 && randVal < SEARCHLEVEL50_MOVECHANCE)
            genMove = TRUE;
    }
    else
    {
        if (SEARCHLEVEL100_MOVECHANCE != 0 && randVal < SEARCHLEVEL100_MOVECHANCE)
            genMove = TRUE;
    }

    // Generate a wild mon just to get the initial moveset (later overwritten by CreateDexNavWildMon)
    CreateWildMon(species, encounterLevel);

    // Store generated mon moves into Dex Nav Struct
    for (i = 0; i < MAX_MON_MOVES; i++)
        moveDst[i] = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_MOVE1 + i);

    // set first move slot to a random egg move if search level is good enough
    if (genMove)
        moveDst[0] = GetRandomEggMove(GetEggSpecies(species));
}

static enum Item DexNavGenerateHeldItem(enum Species species, u8 searchLevel)
{
    u16 randVal = Random() % 100;
    u8 searchLevelInfluence = searchLevel >> 1;
    enum Item item1 = gSpeciesInfo[species].itemCommon;
    enum Item item2 = gSpeciesInfo[species].itemRare;

    // if both are the same, 100% to hold
    if (item1 == item2)
        return item1;

    // if no items can be held, then yeah...no items
    if (item2 == ITEM_NONE && item1 == ITEM_NONE)
        return ITEM_NONE;

    // if only one entry, 50% chance
    if (item2 == ITEM_NONE && item1 != ITEM_NONE)
        return (randVal < 50) ? item1 : ITEM_NONE;

    // if both are distinct item1 = 50% + srclvl/2; item2 = 5% + srchlvl/2
    if (randVal < (50 + searchLevelInfluence + 5 + searchLevel))
        return (randVal > 5 + searchLevelInfluence) ? item1 : item2;
    else
        return ITEM_NONE;

    return ITEM_NONE;
}

static u8 DexNavGetAbilityNum(enum Species species, u8 searchLevel)
{
    bool8 genAbility = FALSE;
    u16 randVal = Random() % 100;
    u8 abilityNum = 0;

    if (searchLevel < 5)
    {
        #if (SEARCHLEVEL0_ABILITYCHANCE != 0)
        if (randVal < SEARCHLEVEL0_ABILITYCHANCE)
            genAbility = TRUE;
        #endif
    }
    else if (searchLevel < 10)
    {
        #if (SEARCHLEVEL5_ABILITYCHANCE != 0)
        if (randVal < SEARCHLEVEL5_ABILITYCHANCE)
            genAbility = TRUE;
        #endif
    }
    else if (searchLevel < 25)
    {
        #if (SEARCHLEVEL10_ABILITYCHANCE != 0)
        if (randVal < SEARCHLEVEL10_ABILITYCHANCE)
            genAbility = TRUE;
        #endif
    }
    else if (searchLevel < 50)
    {
        #if (SEARCHLEVEL25_ABILITYCHANCE != 0)
        if (randVal < SEARCHLEVEL25_ABILITYCHANCE)
            genAbility = TRUE;
        #endif
    }
    else if (searchLevel < 100)
    {
        #if (SEARCHLEVEL50_ABILITYCHANCE != 0)
        if (randVal < SEARCHLEVEL50_ABILITYCHANCE)
            genAbility = TRUE;
        #endif
    }
    else
    {
        #if (SEARCHLEVEL100_ABILITYCHANCE != 0)
        if (randVal < SEARCHLEVEL100_ABILITYCHANCE)
            genAbility = TRUE;
        #endif
    }

    if (genAbility
            && GetSpeciesAbility(species, 2) != ABILITY_NONE
            && GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT))
    {
        //Only give hidden ability if Pokemon has been caught before
        abilityNum = 2;
    }
    else
    {
        //Pick a normal ability of that Pokemon
        if (GetSpeciesAbility(species, 1) != ABILITY_NONE)
            abilityNum = Random() & 1;
        else
            abilityNum = 0;
    }

    return abilityNum;
}

static u8 DexNavGeneratePotential(u8 searchLevel)
{
    u8 genChance = 0;
    int randVal = Random() % 100;

    if (searchLevel < 5)
    {
        genChance = SEARCHLEVEL0_ONESTAR + SEARCHLEVEL0_TWOSTAR + SEARCHLEVEL0_THREESTAR;
        if (randVal < genChance)
        {
            // figure out which star it is
            if (randVal < SEARCHLEVEL0_ONESTAR)
                return 1;
            else if (randVal < (SEARCHLEVEL0_ONESTAR + SEARCHLEVEL0_TWOSTAR))
                return 2;
            else
                return 3;
        }
    }
    else if (searchLevel < 10)
    {
        genChance = SEARCHLEVEL5_ONESTAR + SEARCHLEVEL5_TWOSTAR + SEARCHLEVEL5_THREESTAR;
        if (randVal < genChance)
        {
            // figure out which star it is
            if (randVal < SEARCHLEVEL5_ONESTAR)
                return 1;
            else if (randVal < (SEARCHLEVEL5_ONESTAR + SEARCHLEVEL5_TWOSTAR))
                return 2;
            else
                return 3;
        }
    }
    else if (searchLevel < 25)
    {
        genChance = SEARCHLEVEL10_ONESTAR + SEARCHLEVEL10_TWOSTAR + SEARCHLEVEL10_THREESTAR;
        if (randVal < genChance)
        {
            // figure out which star it is
            if (randVal < SEARCHLEVEL10_ONESTAR)
                return 1;
            else if (randVal < (SEARCHLEVEL10_ONESTAR + SEARCHLEVEL10_TWOSTAR))
                return 2;
            else
                return 3;
        }
    }
    else if (searchLevel < 50)
    {
        genChance = SEARCHLEVEL25_ONESTAR + SEARCHLEVEL25_TWOSTAR + SEARCHLEVEL25_THREESTAR;
        if (randVal < genChance)
        {
            // figure out which star it is
            if (randVal < SEARCHLEVEL25_ONESTAR)
                return 1;
            else if (randVal < (SEARCHLEVEL25_ONESTAR + SEARCHLEVEL25_TWOSTAR))
                return 2;
            else
                return 3;
        }
    }
    else if (searchLevel < 100)
    {
        genChance = SEARCHLEVEL50_ONESTAR + SEARCHLEVEL50_TWOSTAR + SEARCHLEVEL50_THREESTAR;
        if (randVal < genChance)
        {
            // figure out which star it is
            if (randVal < SEARCHLEVEL50_ONESTAR)
                return 1;
            else if (randVal < (SEARCHLEVEL50_ONESTAR + SEARCHLEVEL50_TWOSTAR))
                return 2;
            else
                return 3;
        }
    }
    else
    {
        genChance = SEARCHLEVEL100_ONESTAR + SEARCHLEVEL100_TWOSTAR + SEARCHLEVEL100_THREESTAR;
        if (randVal < genChance)
        {
            // figure out which star it is
            if (randVal < SEARCHLEVEL100_ONESTAR)
                return 1;
            else if (randVal < (SEARCHLEVEL100_ONESTAR + SEARCHLEVEL100_TWOSTAR))
                return 2;
            else
                return 3;
        }
    }

    return 0;   // No potential
}

static u8 GetEncounterLevelFromMapData(enum Species species, enum EncounterType environment)
{
    u32 headerId = GetCurrentMapWildMonHeaderId();
    enum TimeOfDay timeOfDay;
    u8 min = MAX_LEVEL;
    u8 max = 0;
    u8 i;

    if (headerId == HEADER_NONE)
        return MON_LEVEL_NONEXISTENT;

    switch (environment)
    {
    case ENCOUNTER_TYPE_LAND:    // grass
        timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_LAND);
        const struct WildPokemonInfo *landMonsInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo;

        if (landMonsInfo == NULL)
            return MON_LEVEL_NONEXISTENT; //Hidden Pokémon should only appear on walkable tiles or surf tiles

        for (i = 0; i < NUM_LAND_MONS_ENCOUNTER_SLOTS; i++)
        {
            if (landMonsInfo->wildPokemon[i].species == species)
            {
                min = (min < landMonsInfo->wildPokemon[i].minLevel) ? min : landMonsInfo->wildPokemon[i].minLevel;
                max = (max > landMonsInfo->wildPokemon[i].maxLevel) ? max : landMonsInfo->wildPokemon[i].maxLevel;
            }
        }
        break;
    case ENCOUNTER_TYPE_WATER:    //water
        timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_WATER);
        const struct WildPokemonInfo *waterMonsInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].waterMonsInfo;

        if (waterMonsInfo == NULL)
            return MON_LEVEL_NONEXISTENT; //Hidden Pokémon should only appear on walkable tiles or surf tiles

        for (i = 0; i < NUM_WATER_MONS_ENCOUNTER_SLOTS; i++)
        {
            if (waterMonsInfo->wildPokemon[i].species == species)
            {
                min = (min < waterMonsInfo->wildPokemon[i].minLevel) ? min : waterMonsInfo->wildPokemon[i].minLevel;
                max = (max > waterMonsInfo->wildPokemon[i].maxLevel) ? max : waterMonsInfo->wildPokemon[i].maxLevel;
            }
        }
        break;
    case ENCOUNTER_TYPE_HIDDEN:
        timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_HIDDEN);
        const struct WildPokemonInfo *hiddenMonsInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].hiddenMonsInfo;

        if (hiddenMonsInfo == NULL)
            return MON_LEVEL_NONEXISTENT;

        for (i = 0; i < NUM_HIDDEN_MONS_ENCOUNTER_SLOTS; i++)
        {
            if (hiddenMonsInfo->wildPokemon[i].species == species)
            {
                min = (min < hiddenMonsInfo->wildPokemon[i].minLevel) ? min : hiddenMonsInfo->wildPokemon[i].minLevel;
                max = (max > hiddenMonsInfo->wildPokemon[i].maxLevel) ? max : hiddenMonsInfo->wildPokemon[i].maxLevel;
            }
        }

        // use encounter rate to signify is hidden Pokémon are on land or in water
        if (hiddenMonsInfo->encounterRate == 1)
            sDexNavSearchDataPtr->environment = ENCOUNTER_TYPE_WATER;
        else
            sDexNavSearchDataPtr->environment = ENCOUNTER_TYPE_LAND;
        break;
    default:
        return MON_LEVEL_NONEXISTENT;
    }

    if (max == 0)
        return MON_LEVEL_NONEXISTENT;

    return RandomUniform(RNG_DEXNAV_ENCOUNTER_LEVEL, min, max);
}


///////////
/// GUI ///
///////////
static const struct BgTemplate sDexNavMenuBgTemplates[2] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .priority = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 3,
        .mapBaseIndex = 30,
        .priority = 1
    }
};

static void DexNav_VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void DexNav_MainCB(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static bool8 DexNav_InitBgs(void)
{
    ResetVramOamAndBgCntRegs();
    ResetAllBgsCoordinates();
    sBg1TilemapBuffer = Alloc(0x800);
    if (sBg1TilemapBuffer == NULL)
        return FALSE;

    memset(sBg1TilemapBuffer, 0, 0x800);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sDexNavMenuBgTemplates, NELEMS(sDexNavMenuBgTemplates));
    SetBgTilemapBuffer(1, sBg1TilemapBuffer);
    ScheduleBgCopyTilemapToVram(1);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
    SetGpuReg(REG_OFFSET_BLDCNT , 0);
    ShowBg(0);
    ShowBg(1);
    return TRUE;
}

static bool8 DexNav_LoadGraphics(void)
{
    switch (sDexNavUiDataPtr->state)
    {
    case 0:
        ResetTempTileDataBuffers();
        DecompressAndCopyTileDataToVram(1, sDexNavGuiTiles, 0, 0, 0);
        sDexNavUiDataPtr->state++;
        break;
    case 1:
        if (FreeTempTileDataBuffersIfPossible() != TRUE)
        {
            DecompressDataWithHeaderWram(sDexNavGuiTilemap, sBg1TilemapBuffer);
            sDexNavUiDataPtr->state++;
        }
        break;
    case 2:
        LoadPalette(sDexNavGuiPal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
        sDexNavUiDataPtr->state++;
        break;
    default:
        sDexNavUiDataPtr->state = 0;
        return TRUE;
    }

    return FALSE;
}

static void UpdateCursorPosition(void)
{
    u16 x, y;

    switch (sDexNavUiDataPtr->cursorRow)
    {
    case ROW_WATER:
        x = ROW_WATER_ICON_X + (24 * sDexNavUiDataPtr->cursorCol);
        y = ROW_WATER_ICON_Y;
        sDexNavUiDataPtr->environment = ENCOUNTER_TYPE_WATER;
        break;
    case ROW_LAND_TOP: //land 1
        x = ROW_LAND_ICON_X + (24 * sDexNavUiDataPtr->cursorCol);
        y = ROW_LAND_TOP_ICON_Y;
        sDexNavUiDataPtr->environment = ENCOUNTER_TYPE_LAND;
        break;
    case ROW_LAND_BOT: //land 2
        x = ROW_LAND_ICON_X + (24 * sDexNavUiDataPtr->cursorCol);
        y = ROW_LAND_BOT_ICON_Y;
        sDexNavUiDataPtr->environment = ENCOUNTER_TYPE_LAND;
        break;
    case ROW_HIDDEN:
        x = ROW_HIDDEN_ICON_X + (24 * sDexNavUiDataPtr->cursorCol);
        y = ROW_HIDDEN_ICON_Y;
        sDexNavUiDataPtr->environment = ENCOUNTER_TYPE_HIDDEN;
        break;
    default:
        return;
    }

    gSprites[sDexNavUiDataPtr->cursorSpriteId].x = x;
    gSprites[sDexNavUiDataPtr->cursorSpriteId].y = y;

    PrintCurrentSpeciesInfo();
}

static void CreateSelectionCursor(void)
{
    u8 spriteId;
    struct CompressedSpriteSheet spriteSheet;

    spriteSheet.data = sSelectionCursorGfx;
    spriteSheet.size = 0x200;
    spriteSheet.tag = SELECTION_CURSOR_TAG;
    LoadCompressedSpriteSheet(&spriteSheet);

    LoadPalette(sSelectionCursorPal, OBJ_PLTT_ID(sSelectionCursorOam.paletteNum), PLTT_SIZE_4BPP);

    spriteId = CreateSprite(&sSelectionCursorSpriteTemplate, 12, 32, 0);
    //gSprites[spriteId].data[1] = -1;

    sDexNavUiDataPtr->cursorSpriteId = spriteId;
    UpdateCursorPosition();
}

static void CreateNoDataIcon(s16 x, s16 y)
{
    CreateSprite(&sNoDataIconTemplate, x, y, 0);
}

static bool8 CapturedAllLandMons(u32 headerId)
{
    u16 i;
    enum Species species;
    int count = 0;
    enum TimeOfDay timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_LAND);

    const struct WildPokemonInfo *landMonsInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo;

    if (landMonsInfo != NULL)
    {
        for (i = 0; i < NUM_LAND_MONS_ENCOUNTER_SLOTS; ++i)
        {
            species = landMonsInfo->wildPokemon[i].species;
            if (species != SPECIES_NONE)
            {
                if (!GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT))
                    break;

                count++;
            }
        }

        if (i >= NUM_LAND_MONS_ENCOUNTER_SLOTS && count > 0) //All land mons caught
            return TRUE;
        else
            return FALSE;
    }

    return TRUE;    //technically, no mon data means you caught them all
}

//Checks if all Pokemon that can be encountered while surfing have been capture
static bool8 CapturedAllWaterMons(u32 headerId)
{
    u32 i;
    enum Species species;
    u8 count = 0;
    enum TimeOfDay timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_WATER);

    const struct WildPokemonInfo *waterMonsInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].waterMonsInfo;

    if (waterMonsInfo != NULL)
    {
        for (i = 0; i < NUM_WATER_MONS_ENCOUNTER_SLOTS; ++i)
        {
            species = waterMonsInfo->wildPokemon[i].species;
            if (species != SPECIES_NONE)
            {
                count++;
                if (!GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT))
                    break;
            }
        }

        if (i >= NUM_WATER_MONS_ENCOUNTER_SLOTS && count > 0)
            return TRUE;
        else
            return FALSE;
    }

    return TRUE;    //technically, no mon data means you caught them all
}

static bool8 CapturedAllHiddenMons(u32 headerId)
{
    u32 i;
    enum Species species;
    u8 count = 0;
    enum TimeOfDay timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_HIDDEN);

        const struct WildPokemonInfo *hiddenMonsInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].hiddenMonsInfo;

    if (hiddenMonsInfo != NULL)
    {
        for (i = 0; i < NUM_HIDDEN_MONS_ENCOUNTER_SLOTS; ++i)
        {
            species = hiddenMonsInfo->wildPokemon[i].species;
            if (species != SPECIES_NONE)
            {
                count++;
                if (!GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT))
                    break;
            }
        }

        if (i >= NUM_HIDDEN_MONS_ENCOUNTER_SLOTS && count > 0)
            return TRUE;
        else
            return FALSE;
    }

    return TRUE;    //technically, no mon data means you caught them all
}

static void DexNavLoadCapturedAllSymbols(void)
{
    u32 headerId = GetCurrentMapWildMonHeaderId();
    if (headerId == HEADER_NONE)
        return;

    LoadCompressedSpriteSheetUsingHeap(&sCapturedAllPokemonSpriteSheet);

    if (CapturedAllLandMons(headerId))
        CreateSprite(&sCaptureAllMonsSpriteTemplate, 152, 58, 0);

    if (CapturedAllWaterMons(headerId))
        CreateSprite(&sCaptureAllMonsSpriteTemplate, 139, 17, 0);

    if (CapturedAllHiddenMons(headerId))
        CreateSprite(&sCaptureAllMonsSpriteTemplate, 114, 123, 0);
}

//#define WIN_DETAILS_TILE        0x3a3
static void DexNav_InitWindows(void)
{
    sCollectionOpen = FALSE;
    InitWindows(sDexNavGuiWindowTemplates);
    DeactivateAllTextPrinters();
    ScheduleBgCopyTilemapToVram(0);
}

static void DexNavGuiFreeResources(void)
{
    Free(sDexNavUiDataPtr);
    Free(sBg1TilemapBuffer);
    FreeAllWindowBuffers();
}

static void CB1_InitDexNavSearch(void)
{
    if (!gPaletteFade.active && !ArePlayerFieldControlsLocked() && gMain.callback2 == CB2_Overworld)
    {
        SetMainCallback1(CB1_Overworld);
        InitDexNavSearch(gSpecialVar_0x8000, gSpecialVar_0x8001);
    }
}

static void CB1_DexNavSearchCallback(void)
{
    CB1_InitDexNavSearch();
}

static void Task_DexNavExitAndSearch(u8 taskId)
{
    DespawnAllOverworldWildEncounters(OWE_GENERATED, 0);
    DexNavGuiFreeResources();
    DestroyTask(taskId);
    SetMainCallback1(CB1_DexNavSearchCallback);
    SetMainCallback2(CB2_ReturnToField);
}

static void Task_DexNavFadeAndExit(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        SetMainCallback2(sDexNavUiDataPtr->savedCallback);
        DexNavGuiFreeResources();
        DestroyTask(taskId);
    }
}

static void DexNavFadeAndExit(void)
{
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
    CreateTask(Task_DexNavFadeAndExit, 0);
    SetVBlankCallback(DexNav_VBlankCB);
    SetMainCallback2(DexNav_MainCB);
}

static bool8 SpeciesInArray(enum Species species, u8 section)
{
    u32 i;
    enum NationalDexOrder dexNum = SpeciesToNationalPokedexNum(species);

    switch (section)
    {
    case 0: //land
        for (i = 0; i < NUM_LAND_MONS_ENCOUNTER_SLOTS; i++)
        {
            if (SpeciesToNationalPokedexNum(sDexNavUiDataPtr->landSpecies[i]) == dexNum)
                return TRUE;
        }
        break;
    case 1: //water
        for (i = 0; i < NUM_WATER_MONS_ENCOUNTER_SLOTS; i++)
        {
            if (SpeciesToNationalPokedexNum(sDexNavUiDataPtr->waterSpecies[i]) == dexNum)
                return TRUE;
        }
        break;
    case 2: //hidden
        for (i = 0; i < NUM_HIDDEN_MONS_ENCOUNTER_SLOTS; i++)
        {
            if (SpeciesToNationalPokedexNum(sDexNavUiDataPtr->hiddenSpecies[i]) == dexNum)
                return TRUE;
        }
        break;
    default:
        break;
    }

    return FALSE;
}

// get unique wild encounters on current map
static void DexNavLoadEncounterData(void)
{
    u8 grassIndex = 0;
    u8 waterIndex = 0;
    u8 hiddenIndex = 0;
    enum Species species;
    u32 i;
    u32 headerId = GetCurrentMapWildMonHeaderId();
    enum TimeOfDay timeOfDay;

    if (headerId == HEADER_NONE)
        return;

    timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_LAND);
    const struct WildPokemonInfo *landMonsInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo;
    timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_WATER);
    const struct WildPokemonInfo *waterMonsInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].waterMonsInfo;
    timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_HIDDEN);
    const struct WildPokemonInfo *hiddenMonsInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].hiddenMonsInfo;

    // nop struct data
    memset(sDexNavUiDataPtr->landSpecies, 0, sizeof(sDexNavUiDataPtr->landSpecies));
    memset(sDexNavUiDataPtr->waterSpecies, 0, sizeof(sDexNavUiDataPtr->waterSpecies));
    memset(sDexNavUiDataPtr->hiddenSpecies, 0, sizeof(sDexNavUiDataPtr->hiddenSpecies));

    // land mons
    if (landMonsInfo != NULL && landMonsInfo->encounterRate != 0)
    {
        for (i = 0; i < NUM_LAND_MONS_ENCOUNTER_SLOTS; i++)
        {
            species = landMonsInfo->wildPokemon[i].species;
            if (species != SPECIES_NONE && !SpeciesInArray(species, 0))
                sDexNavUiDataPtr->landSpecies[grassIndex++] = landMonsInfo->wildPokemon[i].species;
        }
    }

    // water mons
    if (waterMonsInfo != NULL && waterMonsInfo->encounterRate != 0)
    {
        for (i = 0; i < NUM_WATER_MONS_ENCOUNTER_SLOTS; i++)
        {
            species = waterMonsInfo->wildPokemon[i].species;
            if (species != SPECIES_NONE && !SpeciesInArray(species, 1))
                sDexNavUiDataPtr->waterSpecies[waterIndex++] = waterMonsInfo->wildPokemon[i].species;
        }
    }

    // hidden mons
    if (hiddenMonsInfo != NULL) // no encounter rate check since 0 means land, 1 means water encounters
    {
        for (i = 0; i < NUM_HIDDEN_MONS_ENCOUNTER_SLOTS; i++)
        {
            species = hiddenMonsInfo->wildPokemon[i].species;
            if (species != SPECIES_NONE && !SpeciesInArray(species, 2))
                sDexNavUiDataPtr->hiddenSpecies[hiddenIndex++] = hiddenMonsInfo->wildPokemon[i].species;
        }
    }
}

static void TryDrawIconInSlot(enum Species species, s16 x, s16 y)
{
    if (species == SPECIES_NONE || species > NUM_SPECIES)
        CreateNoDataIcon(x, y);   //'X' in slot
    else if (!GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN))
        CreateMonIcon(SPECIES_NONE, SpriteCB_MonIcon, x, y, 0, 0xFFFFFFFF); //question mark
    else
        CreateMonIcon(species, SpriteCB_MonIcon, x, y, 0, 0xFFFFFFFF);
}

static void DrawSpeciesIcons(void)
{
    s16 x, y;
    u32 i;
    enum Species species;

    LoadCompressedSpriteSheetUsingHeap(&sNoDataIconSpriteSheet);
    for (i = 0; i < NUM_LAND_MONS_ENCOUNTER_SLOTS; i++)
    {
        species = sDexNavUiDataPtr->landSpecies[i];
        x = ROW_LAND_ICON_X + (24 * (i % COL_LAND_COUNT));
        y = ROW_LAND_TOP_ICON_Y + (i > COL_LAND_MAX ? 28 : 0);
        TryDrawIconInSlot(species, x, y);
    }

    for (i = 0; i < NUM_WATER_MONS_ENCOUNTER_SLOTS; i++)
    {
        species = sDexNavUiDataPtr->waterSpecies[i];
        x = ROW_WATER_ICON_X + 24 * i;
        y = ROW_WATER_ICON_Y;
        TryDrawIconInSlot(species, x, y);
    }

    for (i = 0; i < NUM_HIDDEN_MONS_ENCOUNTER_SLOTS; i++)
    {
        species = sDexNavUiDataPtr->hiddenSpecies[i];
        x = ROW_HIDDEN_ICON_X + 24 * i;
        y = ROW_HIDDEN_ICON_Y;
        if (FlagGet(DN_FLAG_DETECTOR_MODE))
            TryDrawIconInSlot(species, x, y);
       else if (species == SPECIES_NONE || species > NUM_SPECIES)
            CreateNoDataIcon(x, y);
        else
            CreateMonIcon(SPECIES_NONE, SpriteCB_MonIcon, x, y, 0, 0xFFFFFFFF); //question mark if detector mode inactive
    }
}

static enum Species DexNavGetSpecies(void)
{
    enum Species species;

    switch (sDexNavUiDataPtr->cursorRow)
    {
    case ROW_WATER:
        species = sDexNavUiDataPtr->waterSpecies[sDexNavUiDataPtr->cursorCol];
        break;
    case ROW_LAND_TOP:
        species = sDexNavUiDataPtr->landSpecies[sDexNavUiDataPtr->cursorCol];
        break;
    case ROW_LAND_BOT:
        species = sDexNavUiDataPtr->landSpecies[sDexNavUiDataPtr->cursorCol + COL_LAND_COUNT];
        break;
    case ROW_HIDDEN:
        if (!FlagGet(DN_FLAG_DETECTOR_MODE))
            species = SPECIES_NONE;
        else
            species = sDexNavUiDataPtr->hiddenSpecies[sDexNavUiDataPtr->cursorCol];
        break;
    default:
        return SPECIES_NONE;
    }

    if (!GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN))
        return SPECIES_NONE;

    return species;
}

static void SetSpriteInvisibility(u8 spriteArrayId, bool8 invisible)
{
    gSprites[sDexNavUiDataPtr->typeIconSpriteIds[spriteArrayId]].invisible = invisible;
}

// different from pokemon_summary_screen
#define TYPE_ICON_PAL_NUM_0     13
#define TYPE_ICON_PAL_NUM_1     14
#define TYPE_ICON_PAL_NUM_2     15
static const u8 sMoveTypeToOamPaletteNum[NUMBER_OF_MON_TYPES] =
{
    [TYPE_NORMAL] = TYPE_ICON_PAL_NUM_0,
    [TYPE_FIGHTING] = TYPE_ICON_PAL_NUM_0,
    [TYPE_FLYING] = TYPE_ICON_PAL_NUM_1,
    [TYPE_POISON] = TYPE_ICON_PAL_NUM_1,
    [TYPE_GROUND] = TYPE_ICON_PAL_NUM_0,
    [TYPE_ROCK] = TYPE_ICON_PAL_NUM_0,
    [TYPE_BUG] = TYPE_ICON_PAL_NUM_2,
    [TYPE_GHOST] = TYPE_ICON_PAL_NUM_1,
    [TYPE_STEEL] = TYPE_ICON_PAL_NUM_0,
    [TYPE_MYSTERY] = TYPE_ICON_PAL_NUM_2,
    [TYPE_FIRE] = TYPE_ICON_PAL_NUM_0,
    [TYPE_WATER] = TYPE_ICON_PAL_NUM_1,
    [TYPE_GRASS] = TYPE_ICON_PAL_NUM_2,
    [TYPE_ELECTRIC] = TYPE_ICON_PAL_NUM_0,
    [TYPE_PSYCHIC] = TYPE_ICON_PAL_NUM_1,
    [TYPE_ICE] = TYPE_ICON_PAL_NUM_1,
    [TYPE_DRAGON] = TYPE_ICON_PAL_NUM_2,
    [TYPE_DARK] = TYPE_ICON_PAL_NUM_0,
    [TYPE_FAIRY] = TYPE_ICON_PAL_NUM_1,
};
static void SetTypeIconPosAndPal(u8 typeId, u8 x, u8 y, u8 spriteArrayId)
{
    struct Sprite *sprite;

    sprite = &gSprites[sDexNavUiDataPtr->typeIconSpriteIds[spriteArrayId]];
    StartSpriteAnim(sprite, typeId);
    sprite->oam.paletteNum = sMoveTypeToOamPaletteNum[typeId];
    sprite->x = x + 16;
    sprite->y = y + 8;
    SetSpriteInvisibility(spriteArrayId, FALSE);
}

static void PrintCollectionSummary(void)
{
    static const u8 *const methods[] =
    {
        COMPOUND_STRING("Hierba "), COMPOUND_STRING("Surf "), COMPOUND_STRING("Rocas "),
        COMPOUND_STRING("Pesca "), COMPOUND_STRING("Ocultos "), COMPOUND_STRING("Total "),
    };
    static const u8 *const times[] =
    {
        COMPOUND_STRING("Manana"), COMPOUND_STRING("Dia"), COMPOUND_STRING("Tarde"),
        COMPOUND_STRING("Noche"), COMPOUND_STRING("Todo el dia"),
    };
    struct RFCollectionSummary summary;
    u32 i;
    u16 header = GetCurrentMapWildMonHeaderId();
    RFCollection_Count(header == HEADER_NONE ? NULL : &gWildMonHeaders[header], sCollectionTime, &summary);
    for (i = 0; i < 2; i++)
        if (sDexNavUiDataPtr->typeIconSpriteIds[i] < MAX_SPRITES)
            SetSpriteInvisibility(i, TRUE);
    // Opaque pixels also cover the fixed labels in the underlying DexNav background.
    FillWindowPixelBuffer(WINDOW_INFO, PIXEL_FILL(TEXT_COLOR_WHITE));
    AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 0, 0, sFontColor_Black, TEXT_SKIP_DRAW, COMPOUND_STRING("Coleccion"));
    AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 0, 12, sFontColor_Black, TEXT_SKIP_DRAW, times[sCollectionTime]);
    for (i = 0; i <= RF_COLLECTION_METHODS; i++)
    {
        const struct RFCollectionCount *count = i == RF_COLLECTION_METHODS ? &summary.total : &summary.methods[i];
        StringCopy(gStringVar4, methods[i]);
        ConvertIntToDecimalStringN(gStringVar4 + StringLength(gStringVar4), count->caught, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(gStringVar4, COMPOUND_STRING("/"));
        ConvertIntToDecimalStringN(gStringVar4 + StringLength(gStringVar4), count->total, STR_CONV_MODE_LEFT_ALIGN, 3);
        AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 0, 24 + i * 12, sFontColor_Black, TEXT_SKIP_DRAW, gStringVar4);
    }
    AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 0, 96, sFontColor_Black, TEXT_SKIP_DRAW, COMPOUND_STRING("L/R: horario"));
    AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 0, 108, sFontColor_Black, TEXT_SKIP_DRAW, COMPOUND_STRING("START: volver"));
    CopyWindowToVram(WINDOW_INFO, COPYWIN_FULL);
    PutWindowTilemap(WINDOW_INFO);
}

static void PrintCurrentSpeciesInfo(void)
{
    enum Species species = DexNavGetSpecies();
    enum NationalDexOrder dexNum = SpeciesToNationalPokedexNum(species);
    enum Type type1, type2;

    if (!GetSetPokedexFlag(dexNum, FLAG_GET_SEEN))
        species = SPECIES_NONE;

    // clear windows
    FillWindowPixelBuffer(WINDOW_INFO, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));

    //species name
    if (species == SPECIES_NONE)
        AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 0, SPECIES_INFO_Y, sFontColor_Black, 0, sText_DexNav_NoInfo);
    else
        AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 0, SPECIES_INFO_Y, sFontColor_Black, 0, GetSpeciesName(species));

    //type icon(s)
    type1 = GetSpeciesType(species, 0);
    type2 = GetSpeciesType(species, 1);
    if (species == SPECIES_NONE)
        type1 = type2 = TYPE_MYSTERY;

    if (type1 == type2)
    {
        SetTypeIconPosAndPal(type1, 186, 69, 0);
        SetSpriteInvisibility(1, TRUE);
    }
    else
    {
        SetTypeIconPosAndPal(type1, 168, 69, 0);
        SetTypeIconPosAndPal(type2, 168 + 33, 69, 1);
    }

    //search level
    if (species == SPECIES_NONE)
    {
        AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 0, SEARCH_LEVEL_Y, sFontColor_Black, 0, sText_DexNav_NoInfo);
    }
    else
    {
        ConvertIntToDecimalStringN(gStringVar4, GetSearchLevel(species), 0, 4);
        AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 0, SEARCH_LEVEL_Y, sFontColor_Black, 0, gStringVar4);

        // Rojo Fuego+: how often it appears on this map
        if (sDexNavUiDataPtr->cursorRow != ROW_HIDDEN)
        {
            enum WildPokemonArea area = (sDexNavUiDataPtr->cursorRow == ROW_WATER) ? WILD_AREA_WATER : WILD_AREA_LAND;
            ConvertIntToDecimalStringN(gStringVar4, GetCurrentMapWildMonEncounterPercent(species, area), STR_CONV_MODE_LEFT_ALIGN, 3);
            StringAppend(gStringVar4, COMPOUND_STRING("%"));
            AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 40, SEARCH_LEVEL_Y, sFontColor_Black, 0, gStringVar4);
        }
    }

    //hidden ability
    if (species == SPECIES_NONE)
    {
        AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 0, HA_INFO_Y, sFontColor_Black, 0, sText_DexNav_NoInfo);
    }
    else if (GetSetPokedexFlag(dexNum, FLAG_GET_CAUGHT))
    {
        if (GetSpeciesAbility(species, 2) != ABILITY_NONE)
            AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 0, HA_INFO_Y, sFontColor_Black, 0, gAbilitiesInfo[GetSpeciesAbility(species, 2)].name);
        else
            AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 0, HA_INFO_Y, sFontColor_Black, 0, gText_None);
    }
    else
    {
        AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 0, HA_INFO_Y, sFontColor_Black, 0, sText_DexNav_CaptureToSee);
    }

    //current chain
    ConvertIntToDecimalStringN(gStringVar1, gSaveBlock3Ptr->dexNavChain, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 0, CHAIN_BONUS_Y, sFontColor_Black, 0, gStringVar1);

    // Rojo Fuego+: active hunter mode filter (SELECT changes it)
    AddTextPrinterParameterized3(WINDOW_INFO, FONT_SMALL, 24, CHAIN_BONUS_Y, sFontColor_Black, 0,
                                 sText_DexNavFilterNames[VarGet(VAR_DEXNAV_FILTER) % DEXNAV_FILTER_COUNT]);

    CopyWindowToVram(WINDOW_INFO, 3);
    PutWindowTilemap(WINDOW_INFO);
}

static void PrintMapName(void)
{
    GetMapName(gStringVar3, GetCurrentRegionMapSectionId(), 0);
    AddTextPrinterParameterized3(WINDOW_REGISTERED, FONT_NORMAL, 108 +
                                 GetStringRightAlignXOffset(1, gStringVar3, MAP_NAME_LENGTH * GetFontAttribute(1, FONTATTR_MAX_LETTER_WIDTH)),
                                 0, sFontColor_White, 0, gStringVar3);
    CopyWindowToVram(WINDOW_REGISTERED, 3);
}

static bool32 DexNavIsRoamerOnMap(void)
{
    for (u32 i = 0; i < ROAMER_COUNT; i++)
    {
        if (IsRoamerAt(i, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum))
            return TRUE;
    }
    return FALSE;
}

static void PrintSearchableSpecies(enum Species species)
{
    FillWindowPixelBuffer(WINDOW_REGISTERED, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    PutWindowTilemap(WINDOW_REGISTERED);
    if (DexNavIsRoamerOnMap())
    {
        // Rojo Fuego+: a roaming legendary is on this map right now
        AddTextPrinterParameterized3(WINDOW_REGISTERED, FONT_NORMAL, 0, 0, sFontColor_White, TEXT_SKIP_DRAW, COMPOUND_STRING("¡Legendario errante!"));
    }
    else if (species == SPECIES_NONE)
    {
        AddTextPrinterParameterized3(WINDOW_REGISTERED, FONT_NORMAL, 0, 0, sFontColor_White, TEXT_SKIP_DRAW, KeySystem_IsOn(KEY_DEXNAV_COLLECTION) ? COMPOUND_STRING("R: registrar  START: coleccion") : sText_DexNav_PressRToRegister);
    }
    else
    {
        StringCopy(gStringVar1, GetSpeciesName(species));
        StringExpandPlaceholders(gStringVar4, sText_DexNav_SearchForRegisteredSpecies);
        AddTextPrinterParameterized3(WINDOW_REGISTERED, FONT_NORMAL, 0, 0, sFontColor_White, TEXT_SKIP_DRAW, gStringVar4);
    }

    PrintMapName();
}

static void CreateTypeIconSprites(void)
{
    u8 i;

    LoadCompressedSpriteSheet(&gSpriteSheet_MoveTypes);
    LoadPalette(gMoveTypes_Pal, OBJ_PLTT_ID(13), 3 * PLTT_SIZE_4BPP);
    for (i = 0; i < 2; i++)
    {
        if (sDexNavUiDataPtr->typeIconSpriteIds[i] == 0xFF)
            sDexNavUiDataPtr->typeIconSpriteIds[i] = CreateSprite(&gSpriteTemplate_MoveTypes, 10, 10, 2);

        SetSpriteInvisibility(i, TRUE);
    }
}

// Rojo Fuego+: "caught/total" on each tab, drawn straight into BG1 tiles reserved for it
// (tools/rojo_fuego/dexnav_gui.py), in the same pixel font as the tab labels
enum
{
    COUNTER_WATER,
    COUNTER_LAND,
    COUNTER_HIDDEN,
};

struct DexNavCounterArea
{
    u8 textY;
    u16 tiles[8];
};

#include "data/dexnav_counter_tiles.h"

#define COUNTER_AREA_W      (DEXNAV_COUNTER_WIDTH * 8)
#define COUNTER_AREA_H      (DEXNAV_COUNTER_HEIGHT * 8)
#define COUNTER_COLOR       1 // white
#define COUNTER_SHADOW      7
#define COUNTER_SLASH       10

// 4x7 glyphs, one row per byte, bit 3 is the leftmost pixel
static const u8 sCounterGlyphs[11][7] =
{
    {0x6, 0x9, 0x9, 0x9, 0x9, 0x9, 0x6}, // 0
    {0x2, 0x6, 0x2, 0x2, 0x2, 0x2, 0x7}, // 1
    {0x6, 0x9, 0x1, 0x2, 0x4, 0x8, 0xF}, // 2
    {0xE, 0x1, 0x1, 0x6, 0x1, 0x1, 0xE}, // 3
    {0x2, 0x6, 0xA, 0xA, 0xF, 0x2, 0x2}, // 4
    {0xF, 0x8, 0xE, 0x1, 0x1, 0x9, 0x6}, // 5
    {0x6, 0x8, 0x8, 0xE, 0x9, 0x9, 0x6}, // 6
    {0xF, 0x1, 0x1, 0x2, 0x2, 0x4, 0x4}, // 7
    {0x6, 0x9, 0x9, 0x6, 0x9, 0x9, 0x6}, // 8
    {0x6, 0x9, 0x9, 0x7, 0x1, 0x1, 0x6}, // 9
    {0x1, 0x1, 0x2, 0x2, 0x4, 0x4, 0x8}, // /
};

static u8 *GetCounterPixel(u8 *tiles, u32 x, u32 y)
{
    // tiles holds the area's tiles one after another, 32 bytes each, 4 bits per pixel
    u32 tile = (y / 8) * DEXNAV_COUNTER_WIDTH + (x / 8);
    return &tiles[tile * 32 + (y % 8) * 4 + (x % 8) / 2];
}

static u32 GetCounterPixelColor(u8 *tiles, u32 x, u32 y)
{
    u8 *byte = GetCounterPixel(tiles, x, y);
    return (x & 1) ? (*byte >> 4) : (*byte & 0xF);
}

static void SetCounterPixel(u8 *tiles, u32 x, u32 y, u32 color)
{
    u8 *byte = GetCounterPixel(tiles, x, y);

    if (x >= COUNTER_AREA_W || y >= COUNTER_AREA_H)
        return;
    if (x & 1)
        *byte = (*byte & 0x0F) | (color << 4);
    else
        *byte = (*byte & 0xF0) | color;
}

static void DrawCounterGlyph(u8 *tiles, u32 glyph, u32 left, u32 top)
{
    u32 x, y;

    for (y = 0; y < 7; y++)
    {
        for (x = 0; x < 4; x++)
        {
            if (!(sCounterGlyphs[glyph][y] & (8 >> x)))
                continue;
            SetCounterPixel(tiles, left + x, top + y, COUNTER_COLOR);
            // Shadow to the right and below, like the tab labels
            if (left + x + 1 < COUNTER_AREA_W && GetCounterPixelColor(tiles, left + x + 1, top + y) != COUNTER_COLOR)
                SetCounterPixel(tiles, left + x + 1, top + y, COUNTER_SHADOW);
            if (top + y + 1 < COUNTER_AREA_H && GetCounterPixelColor(tiles, left + x, top + y + 1) != COUNTER_COLOR)
                SetCounterPixel(tiles, left + x, top + y + 1, COUNTER_SHADOW);
        }
    }
}

static void DrawAreaCounter(u32 area, const enum Species *species, u32 slots)
{
    const struct DexNavCounterArea *counter = &sDexNavCounterAreas[area];
    u8 tiles[DEXNAV_COUNTER_WIDTH * DEXNAV_COUNTER_HEIGHT * 32];
    u8 *vram = (u8 *)BG_CHAR_ADDR(3);
    u8 glyphs[8];
    u32 i, caught = 0, total = 0, count = 0, x;

    for (i = 0; i < slots; i++)
    {
        if (species[i] == SPECIES_NONE)
            continue;
        total++;
        if (GetSetPokedexFlag(SpeciesToNationalPokedexNum(species[i]), FLAG_GET_CAUGHT))
            caught++;
    }
    if (total == 0)
        return;

    // Digits of "caught/total"
    if (caught >= 10)
        glyphs[count++] = caught / 10;
    glyphs[count++] = caught % 10;
    glyphs[count++] = COUNTER_SLASH;
    if (total >= 10)
        glyphs[count++] = total / 10;
    glyphs[count++] = total % 10;

    for (i = 0; i < DEXNAV_COUNTER_WIDTH * DEXNAV_COUNTER_HEIGHT; i++)
        CpuCopy16(vram + counter->tiles[i] * 32, tiles + i * 32, 32);

    // Right-aligned, glyphs 4 wide with 1 pixel between them and room for the shadow
    x = COUNTER_AREA_W - 2 - (count * 5 - 1);
    for (i = 0; i < count; i++, x += 5)
        DrawCounterGlyph(tiles, glyphs[i], x, counter->textY);

    for (i = 0; i < DEXNAV_COUNTER_WIDTH * DEXNAV_COUNTER_HEIGHT; i++)
        CpuCopy16(tiles + i * 32, vram + counter->tiles[i] * 32, 32);
}

static void DrawAreaCounters(void)
{
    DrawAreaCounter(COUNTER_WATER, sDexNavUiDataPtr->waterSpecies, NUM_WATER_MONS_ENCOUNTER_SLOTS);
    DrawAreaCounter(COUNTER_LAND, sDexNavUiDataPtr->landSpecies, NUM_LAND_MONS_ENCOUNTER_SLOTS);
    DrawAreaCounter(COUNTER_HIDDEN, sDexNavUiDataPtr->hiddenSpecies, NUM_HIDDEN_MONS_ENCOUNTER_SLOTS);
}

#define tSpecies            data[2]
#define tEnvironment        data[3]

static bool8 DexNav_DoGfxSetup(void)
{
    u8 taskId;

    switch (gMain.state)
    {
    case 0:
        SetVBlankHBlankCallbacksToNull();
        ClearScheduledBgCopiesToVram();
        gMain.state++;
        break;
    case 1:
        ScanlineEffect_Stop();
        gMain.state++;
        break;
    case 2:
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    case 3:
        ResetPaletteFade();
        ResetSpriteData();
        ResetTasks();
        gMain.state++;
        break;
    case 4:
        if (DexNav_InitBgs())
        {
            sDexNavUiDataPtr->state = 0;
            gMain.state++;
        }
        else
        {
            DexNavFadeAndExit();
            return TRUE;
        }
        break;
    case 5:
        if (DexNav_LoadGraphics() == TRUE)
            gMain.state++;
        break;
    case 6:
        DexNav_InitWindows();
        sDexNavUiDataPtr->cursorRow = ROW_LAND_TOP;
        sDexNavUiDataPtr->cursorCol = 0;
        sDexNavUiDataPtr->environment = ENCOUNTER_TYPE_LAND;
        gMain.state++;
        break;
    case 7:
        PrintSearchableSpecies(VarGet(DN_VAR_SPECIES) & DEXNAV_MASK_SPECIES);
        DexNavLoadEncounterData();
        DrawAreaCounters();
        gMain.state++;
        break;
    case 8:
        taskId = CreateTask(Task_DexNavWaitFadeIn, 0);
        gTasks[taskId].tSpecies = 0;
        gTasks[taskId].tEnvironment = sDexNavUiDataPtr->environment;
        gMain.state++;
        break;
    case 9:
        sDexNavUiDataPtr->typeIconSpriteIds[0] = 0xFF;
        sDexNavUiDataPtr->typeIconSpriteIds[1] = 0xFF;
        CreateTypeIconSprites();
        gMain.state++;
        break;
    case 10:
        LoadMonIconPalettes();
        DrawSpeciesIcons();
        CreateSelectionCursor();
        DexNavLoadCapturedAllSymbols();
        gMain.state++;
        break;
    case 11:
        BlendPalettes(0xFFFFFFFF, 16, RGB_BLACK);
        gMain.state++;
        break;
    case 12:
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
        gMain.state++;
        break;
    default:
        SetVBlankCallback(DexNav_VBlankCB);
        SetMainCallback2(DexNav_MainCB);
        return TRUE;
    }

    return FALSE;
}

static void DexNav_RunSetup(void)
{
    while (!DexNav_DoGfxSetup()) {}
}

// Entry point for the dexnav GUI
static void DexNavGuiInit(MainCallback callback)
{
    assertf(DEXNAV_ENABLED, "DexNav was opened when DEXNAV_ENABLED config was disabled.\nCheck include/config/dexnav.h")
    {
        SetMainCallback2(callback);
        return;
    }

    if ((sDexNavUiDataPtr = AllocZeroed(sizeof(struct DexNavGUI))) == NULL)
    {
        SetMainCallback2(callback);
        return;
    }

    sDexNavUiDataPtr->state = 0;
    sDexNavUiDataPtr->savedCallback = callback;
    SetMainCallback2(DexNav_RunSetup);
}

void Task_OpenDexNavFromStartMenu(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        CleanupOverworldWindowsAndTilemaps();
        DexNavGuiInit(CB2_ReturnToFieldWithOpenMenu);
        DestroyTask(taskId);
    }
}

static void Task_DexNavWaitFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_DexNavMain;
}

static void Task_DexNavMain(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    enum Species species;

    if (IsSEPlaying())
        return;

    if (sCollectionOpen)
    {
        if (JOY_NEW(START_BUTTON | B_BUTTON))
        {
            sCollectionOpen = FALSE;
            PrintCurrentSpeciesInfo();
            PlaySE(SE_SELECT);
        }
        else if (JOY_NEW(L_BUTTON | R_BUTTON))
        {
            sCollectionTime = (sCollectionTime + (JOY_NEW(R_BUTTON) ? 1 : TIMES_OF_DAY_COUNT)) % (TIMES_OF_DAY_COUNT + 1);
            PrintCollectionSummary();
            PlaySE(SE_SELECT);
        }
        return;
    }
    if (JOY_NEW(START_BUTTON) && KeySystem_IsOn(KEY_DEXNAV_COLLECTION))
    {
        sCollectionOpen = TRUE;
        sCollectionTime = GetTimeOfDay();
        PrintCollectionSummary();
        PlaySE(SE_SELECT);
        return;
    }

    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_POKENAV_OFF);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        task->func = Task_DexNavFadeAndExit;
    }
    else if (JOY_NEW(DPAD_UP))
    {
        if (sDexNavUiDataPtr->cursorRow == ROW_WATER)
        {
            sDexNavUiDataPtr->cursorRow = ROW_HIDDEN;
            if (sDexNavUiDataPtr->cursorCol >= COL_HIDDEN_COUNT)
                sDexNavUiDataPtr->cursorCol = COL_HIDDEN_MAX;
        }
        else
        {
            if (sDexNavUiDataPtr->cursorRow == ROW_LAND_TOP && sDexNavUiDataPtr->cursorCol == COL_LAND_MAX)
                sDexNavUiDataPtr->cursorCol = COL_WATER_MAX;

            sDexNavUiDataPtr->cursorRow--;
        }

        PlaySE(SE_RG_BAG_CURSOR);
        UpdateCursorPosition();
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (sDexNavUiDataPtr->cursorRow == ROW_HIDDEN)
        {
            sDexNavUiDataPtr->cursorRow = ROW_WATER;
        }
        else if (sDexNavUiDataPtr->cursorRow == ROW_LAND_BOT)
        {
            if (sDexNavUiDataPtr->cursorCol >= COL_HIDDEN_COUNT)
                sDexNavUiDataPtr->cursorCol = COL_HIDDEN_MAX;

            sDexNavUiDataPtr->cursorRow++;
        }
        else
        {
            sDexNavUiDataPtr->cursorRow++;
        }

        PlaySE(SE_RG_BAG_CURSOR);
        UpdateCursorPosition();
    }
    else if (JOY_NEW(DPAD_LEFT))
    {
        if (sDexNavUiDataPtr->cursorCol == 0)
        {
            switch (sDexNavUiDataPtr->cursorRow)
            {
            case ROW_WATER:
                sDexNavUiDataPtr->cursorCol = COL_WATER_MAX;
                break;
            case ROW_HIDDEN:
                sDexNavUiDataPtr->cursorCol = COL_HIDDEN_MAX;
                break;
            default:
                sDexNavUiDataPtr->cursorCol = COL_LAND_MAX;
                break;
            }
        }
        else
        {
            sDexNavUiDataPtr->cursorCol--;
        }

        PlaySE(SE_RG_BAG_CURSOR);
        UpdateCursorPosition();
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        switch (sDexNavUiDataPtr->cursorRow)
        {
        case ROW_WATER:
            if (sDexNavUiDataPtr->cursorCol == COL_WATER_MAX)
                sDexNavUiDataPtr->cursorCol = 0;
            else
                sDexNavUiDataPtr->cursorCol++;
            break;
        case ROW_HIDDEN:
            if (sDexNavUiDataPtr->cursorCol == COL_HIDDEN_MAX)
                sDexNavUiDataPtr->cursorCol = 0;
            else
                sDexNavUiDataPtr->cursorCol++;
            break;
        default:
            if (sDexNavUiDataPtr->cursorCol == COL_LAND_MAX)
                sDexNavUiDataPtr->cursorCol = 0;
            else
                sDexNavUiDataPtr->cursorCol++;
            break;
        }

        PlaySE(SE_RG_BAG_CURSOR);
        UpdateCursorPosition();
    }
    else if (JOY_NEW(SELECT_BUTTON))
    {
        // Rojo Fuego+ hunter mode: cycle through the search filters
        VarSet(VAR_DEXNAV_FILTER, (VarGet(VAR_DEXNAV_FILTER) + 1) % DEXNAV_FILTER_COUNT);
        PlaySE(SE_SELECT);
        PrintCurrentSpeciesInfo();
    }
    else if (JOY_NEW(R_BUTTON))
    {
        // check selection is valid. Play sound if invalid
        species = DexNavGetSpecies();

        if (species != SPECIES_NONE)
        {
            PrintSearchableSpecies(species);
            //PlaySE(SE_DEX_SEARCH);
            PlayCry_Script(species, 0);

            // create value to store in a var
            VarSet(DN_VAR_SPECIES, ((sDexNavUiDataPtr->environment << 14) | species));
        }
        else
        {
            PlaySE(SE_FAILURE);
        }
    }
    else if (JOY_NEW(A_BUTTON))
    {
        species = DexNavGetSpecies();
        if (species == SPECIES_NONE)
        {
            PlaySE(SE_FAILURE);
        }
        else
        {
            gSpecialVar_0x8000 = species;
            gSpecialVar_0x8001 = sDexNavUiDataPtr->environment;
            gSpecialVar_0x8002 = (sDexNavUiDataPtr->cursorRow == ROW_HIDDEN) ? TRUE : FALSE;
            PlaySE(SE_DEX_SEARCH);
            BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
            task->func = Task_DexNavExitAndSearch;
        }
    }
}

/////////////////////////
//// HIDDEN POKEMON /////
/////////////////////////
bool32 TryFindHiddenPokemon(void)
{
    u16 *stepPtr = GetVarPointer(DN_VAR_STEP_COUNTER);

    // Rojo Fuego+: was "== NULL", which made detector mode never trigger outside of a search
    if (DEXNAV_ENABLED == 0
            || sDexNavSearchDataPtr != NULL
            || !FlagGet(DN_FLAG_DETECTOR_MODE)
            || FlagGet(DN_FLAG_SEARCHING)
            || GetFlashLevel() > 0)
    {
        if (stepPtr != NULL)
            (*stepPtr) = 0;
        return FALSE;
    }

    (*stepPtr)++;
    (*stepPtr) %= HIDDEN_MON_STEP_COUNT;
    if ((*stepPtr) == 0 && (Random() % 100 < HIDDEN_MON_SEARCH_RATE))
    {
        // hidden Pokémon
        u32 headerId = GetCurrentMapWildMonHeaderId();
        u8 index;
        enum Species species;
        enum EncounterType environment;

        if (headerId == HEADER_NONE)
            return FALSE;

        enum TimeOfDay timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_HIDDEN);
        const struct WildPokemonInfo *hiddenMonsInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].hiddenMonsInfo;
        bool8 isHiddenMon = FALSE;

        // while you can still technically find hidden Pokémon if there are not hidden-only Pokémon on a map,
        // this prevents any potential lagging on maps you dont want hidden Pokémon to appear on
        if (hiddenMonsInfo == NULL)
            return FALSE;

        // encounter rate signifies surfing (1) or land mons (0)!
        // again, for simplicity
        switch (hiddenMonsInfo->encounterRate)
        {
        case 0: // land
            // there are surely better ways to do this, but this allows greatest flexibility
            if (Random() % 100 < HIDDEN_MON_PROBABILTY)
            {
                index = ChooseHiddenMonIndex();
                if (index == 0xFF)
                    return FALSE;//no hidden info
                species = hiddenMonsInfo->wildPokemon[index].species;
                isHiddenMon = TRUE;
                environment = ENCOUNTER_TYPE_HIDDEN;
            }
            else
            {
                species = gWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo->wildPokemon[ChooseWildMonIndex_Land()].species;
                environment = ENCOUNTER_TYPE_LAND;
            }
            break;
        case 1: // water
            if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING))
            {
                if (Random() % 100 < HIDDEN_MON_PROBABILTY)
                {
                    index = ChooseHiddenMonIndex();
                    if (index == 0xFF)
                        return FALSE;//no hidden info
                    species = hiddenMonsInfo->wildPokemon[index].species;
                    isHiddenMon = TRUE;
                    environment = ENCOUNTER_TYPE_HIDDEN;
                }
                else
                {
                    species = gWildMonHeaders[headerId].encounterTypes[timeOfDay].waterMonsInfo->wildPokemon[ChooseWildMonIndex_Water()].species;
                    environment = ENCOUNTER_TYPE_WATER;

                }
            }
            else
            {
                // not surfing -> can't find hidden water mons
                return FALSE;
            }
            break;
        default:
            return FALSE;
        }

        if (species == SPECIES_NONE)
            return FALSE;

        sDexNavSearchDataPtr = AllocZeroed(sizeof(struct DexNavSearch));
        FlagSet(DN_FLAG_SEARCHING);
        // init search data
        sDexNavSearchDataPtr->isHiddenMon = isHiddenMon;
        sDexNavSearchDataPtr->species = species;
        sDexNavSearchDataPtr->hiddenSearch = TRUE;
        sDexNavSearchDataPtr->environment = environment;    // updated in DexNavTryGenerateMonLevel if hidden mon
        sDexNavSearchDataPtr->monLevel = DexNavTryGenerateMonLevel(species, environment);
        if (sDexNavSearchDataPtr->monLevel == MON_LEVEL_NONEXISTENT)
        {
            FREE_AND_SET_NULL(sDexNavSearchDataPtr);
            FlagClear(DN_FLAG_SEARCHING);
            return FALSE;
        }

        // find tile for hidden mon and start effect if possible
        if (!TryStartHiddenMonFieldEffect(sDexNavSearchDataPtr->environment, 8, 8, TRUE))
        {
            FREE_AND_SET_NULL(sDexNavSearchDataPtr);
            FlagClear(DN_FLAG_SEARCHING);
            return FALSE;
        }

        // exclamation mark over player
        gFieldEffectArguments[0] = gSaveBlock1Ptr->pos.x;
        gFieldEffectArguments[1] = gSaveBlock1Ptr->pos.y;
        gFieldEffectArguments[2] = gSprites[gPlayerAvatar.spriteId].subpriority - 1;
        gFieldEffectArguments[3] = 2;
        ObjectEventGetLocalIdAndMap(&gObjectEvents[gPlayerAvatar.objectEventId], &gFieldEffectArguments[0], &gFieldEffectArguments[1], &gFieldEffectArguments[2]);
        FieldEffectStart(FLDEFF_EXCLAMATION_MARK_ICON);

        PlayCry_Script(species, 0);
        SetUpDexNavSearch();
        // Rojo Fuego+: shiny alert for Pokémon found while walking
        if (sDexNavSearchDataPtr->isShiny)
            PlaySE(SE_SHINY);
        HideMapNamePopUpWindow();
        ChangeBgY_ScreenOff(0, 0, 0);
        return FALSE;   // we dont actually want to enable the script context or the game will freeze
    }

    return FALSE;
}

static void DrawSearchIcon(void)
{
    struct CompressedSpriteSheet spriteSheet;

    spriteSheet.data = sHiddenSearchIconGfx;
    spriteSheet.size = 0x200;
    spriteSheet.tag = SELECTION_CURSOR_TAG;
    LoadCompressedSpriteSheet(&spriteSheet);
    sDexNavSearchDataPtr->iconSpriteId = CreateSprite(&sSearchIconSpriteTemplate, 18, GetSearchWindowY() + 12, 0);
}

// the initial hidden icon window ONLY shows search icon, ??? instead of name, and the search level (and pokeball icon if owned)
// if the player presses R or moves close enough, the full search window will be created
// this way, if the player is not interested in hidden Pokémon it will not be too intrusive
static void DrawHiddenSearchWindow(u8 width)
{
    AddSearchWindow(width);
    AddTextPrinterParameterized3(sDexNavSearchDataPtr->windowId, FONT_SMALL, SPECIES_ICON_X + 4, 0, sSearchFontColor, TEXT_SKIP_DRAW, sText_ThreeQmarks);

    ConvertIntToDecimalStringN(gStringVar1, sDexNavSearchDataPtr->searchLevel, STR_CONV_MODE_LEFT_ALIGN, 2);
    StringExpandPlaceholders(gStringVar4, sText_SearchLevel);
    AddTextPrinterParameterized3(sDexNavSearchDataPtr->windowId, FONT_SMALL, SPECIES_ICON_X + 4, 12, sSearchFontColor, TEXT_SKIP_DRAW, gStringVar4);
    CopyWindowToVram(sDexNavSearchDataPtr->windowId, 2);
}

static void DexNavDrawHiddenIcons(void)
{
    enum Species species = sDexNavSearchDataPtr->species;

    DrawHiddenSearchWindow(12);
    DrawSearchIcon();

    if (GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT))
        sDexNavSearchDataPtr->ownedIconSpriteId = CreateSprite(&sOwnedIconTemplate, SPECIES_ICON_X + 6, GetSearchWindowY() + 2, 0);

    if (sDexNavSearchDataPtr->isHiddenMon)
        sDexNavSearchDataPtr->exclamationSpriteId = CreateSprite(&sHiddenMonIconTemplate, SPECIES_ICON_X + 34, GetSearchWindowY() + 8, 0);
}

/////////////////////////
//// GENERAL UTILITY ////
/////////////////////////
u32 CalculateDexNavShinyRolls(void)
{
    u32 chainBonus, rndBonus;
    u8 chain = gSaveBlock3Ptr->dexNavChain;

    chainBonus = (chain >= 100) ? 10 : (chain >= 50) ? 5 : 0;
    rndBonus = (Random() % 100 < 4) ? 4 : 0;
    return chainBonus + rndBonus;
}

void TryIncrementSpeciesSearchLevel()
{
#if USE_DEXNAV_SEARCH_LEVELS == TRUE
    u32 dexNum = SpeciesToNationalPokedexNum(gDexNavSpecies);
    if (gMapHeader.regionMapSectionId != MAPSEC_BATTLE_FRONTIER && gSaveBlock3Ptr->dexNavSearchLevels[dexNum] < 255)
        gSaveBlock3Ptr->dexNavSearchLevels[dexNum]++;
#endif
}

void ResetDexNavSearch(void)
{
    gSaveBlock3Ptr->dexNavChain = 0;    //reset dex nav chaining on new map
    VarSet(DN_VAR_STEP_COUNTER, 0); //reset hidden Pokémon step counter
    if (FlagGet(DN_FLAG_SEARCHING))
        EndDexNavSearch();   //moving to new map ends dexnav search
}

void IncrementDexNavChain(void)
{
    if (gSaveBlock3Ptr->dexNavChain < DEXNAV_CHAIN_MAX)
        gSaveBlock3Ptr->dexNavChain++;
}
