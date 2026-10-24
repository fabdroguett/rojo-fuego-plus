// Rojo Fuego+ Pokémon summary screen, drawn from scratch with the RF UI theme.
// It keeps the public API and the behaviour of pokemon_summary_screen.c (move reordering and
// PP bonuses, forgetting a move with the HM rule, nickname, move relearner, PC boxes, multi
// battles, Eggs) while replacing its presentation: four pages on one full-screen canvas.
// The previous screen is still compiled under Legacy_* names (RF_SUMMARY_SCREEN in
// config/summary_screen.h) as a fallback.
#include "global.h"
#include "battle.h"
#include "battle_factory.h"
#include "battle_tent.h"
#include "bg.h"
#include "data.h"
#include "daycare.h"
#include "decompress.h"
#include "event_data.h"
#include "frontier_util.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "item.h"
#include "key_system.h"
#include "link.h"
#include "m4a.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "mon_markings.h"
#include "move.h"
#include "move_relearner.h"
#include "palette.h"
#include "party_menu.h"
#include "pokeball.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "pokemon_summary_screen.h"
#include "pokerus.h"
#include "region_map.h"
#include "rf_ui_theme.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "tv.h"
#include "window.h"
#include "constants/party_menu.h"
#include "constants/region_map_sections.h"
#include "constants/songs.h"

#if RF_SUMMARY_SCREEN

enum
{
    PAGE_DATA,
    PAGE_STATS,
    PAGE_MOVES,
    PAGE_MEMO,
    PAGE_COUNT,
};

enum
{
    MOVES_IDLE,   // moves page without a cursor
    MOVES_BROWSE, // cursor on a move, details shown
    MOVES_SWAP,   // second cursor choosing where the first move goes
};

// Canvas palette (BG bank 0). 0 is the backdrop and the transparent canvas pixel.
enum
{
    C_GROUND,
    C_PANEL,
    C_TEXT,
    C_SHADOW,
    C_MUTED,
    C_EDGE,
    C_SELECTED,
    C_GOLD,
    C_ACCENT,
    C_SWAP,
    C_RED,
    C_BLUE,
    C_GREEN,
    C_YELLOW,
    C_TRACK,
    C_WHITE,
};

#define TAG_RFSUM_MARKINGS 30100
#define TAG_RFSUM_CATEGORY 30101
#define MOVE_ROWS (MAX_MON_MOVES + 1)
#define NO_SPRITE 0xFF

// Layout, in canvas pixels. Every text keeps at least PAD pixels from its panel's edge.
#define PAD 6
#define LEFT_X 4
#define LEFT_W 84
#define RIGHT_X 92
#define RIGHT_W 144
#define BODY_Y 20
#define BODY_H 126
#define LINE 12
#define LABEL_X (RIGHT_X + PAD)
#define VALUE_X (RIGHT_X + 64)
#define VALUE_W (RIGHT_X + RIGHT_W - PAD - VALUE_X)
#define TEXT_W (RIGHT_W - 2 * PAD)
#define ROW_Y(n) (BODY_Y + 5 + LINE * (n))
#define MOVE_ROW_Y(i) (BODY_Y + 4 + 16 * (i))
#define DETAIL_Y (BODY_Y + 88)
#define PORTRAIT_MON_Y (BODY_Y + 42)  // centre of the 64x64 picture
#define PORTRAIT_NAME_Y (BODY_Y + 80)

struct RFSummary
{
    union {
        struct Pokemon *mons;
        struct BoxPokemon *boxMons;
    } list;
    MainCallback callback;
    struct Pokemon mon;
    u8 mode;
    bool8 isBoxMon;
    u8 curIndex;
    u8 maxIndex;
    u8 page;
    u8 minPage;
    bool8 lockMoves;
    bool8 lockMon;
    u16 newMove;
    u8 movesState;
    u8 cursor;
    u8 swapCursor;
    bool8 hmWarning;
    bool8 hasRelearnableMoves;
    u8 window;
    u8 monSpriteId;
    u8 ballSpriteId;
    u8 categorySpriteId;
    u8 typeSpriteIds[2 + MOVE_ROWS]; // species types, then one per move row
    struct Sprite *markings;
    s16 loadState;
};

static EWRAM_DATA struct RFSummary *sRFSum = NULL;
static EWRAM_DATA u8 sMoveSlotToReplace = 0;
static EWRAM_DATA u8 sAnimDelayTaskId = 0;

static const s8 sMultiBattleOrder[] = {0, 2, 3, 1, 4, 5};
static const u16 sMarkingsPal[] = INCGFX_U16("graphics/summary_screen/markings.pal", ".gbapal");

static const struct BgTemplate sBgTemplates[] =
{
    {.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 1, .baseTile = 0},
};

static const struct WindowTemplate sCanvasTemplate =
{
    .bg = 0, .tilemapLeft = 0, .tilemapTop = 0, .width = DISPLAY_TILE_WIDTH, .height = DISPLAY_TILE_HEIGHT,
    .paletteNum = 0, .baseBlock = 1,
};

static const u8 *const sPageNames[PAGE_COUNT] =
{
    [PAGE_DATA]  = COMPOUND_STRING("DATOS"),
    [PAGE_STATS] = COMPOUND_STRING("VALORES"),
    [PAGE_MOVES] = COMPOUND_STRING("MOVIMIENTOS"),
    [PAGE_MEMO]  = COMPOUND_STRING("MEMORIA"),
};

// Display order of the stats table and its Spanish labels; indices are enum Stat.
static const u8 sStatOrder[NUM_STATS] = {STAT_HP, STAT_ATK, STAT_DEF, STAT_SPATK, STAT_SPDEF, STAT_SPEED};
static const u8 *const sStatNames[NUM_STATS] =
{
    [STAT_HP]    = COMPOUND_STRING("PS"),
    [STAT_ATK]   = COMPOUND_STRING("Ataque"),
    [STAT_DEF]   = COMPOUND_STRING("Defensa"),
    [STAT_SPEED] = COMPOUND_STRING("Velocidad"),
    [STAT_SPATK] = COMPOUND_STRING("At. Esp."),
    [STAT_SPDEF] = COMPOUND_STRING("Def. Esp."),
};

static const u8 *const sStatShortNames[NUM_STATS] =
{
    [STAT_HP]    = COMPOUND_STRING("PS"),
    [STAT_ATK]   = COMPOUND_STRING("Atq"),
    [STAT_DEF]   = COMPOUND_STRING("Def"),
    [STAT_SPEED] = COMPOUND_STRING("Vel"),
    [STAT_SPATK] = COMPOUND_STRING("AtE"),
    [STAT_SPDEF] = COMPOUND_STRING("DfE"),
};

static const u8 *const sNatureNames[NUM_NATURES] =
{
    COMPOUND_STRING("Fuerte"), COMPOUND_STRING("Huraña"), COMPOUND_STRING("Audaz"), COMPOUND_STRING("Firme"),
    COMPOUND_STRING("Pícara"), COMPOUND_STRING("Osada"), COMPOUND_STRING("Dócil"), COMPOUND_STRING("Plácida"),
    COMPOUND_STRING("Agitada"), COMPOUND_STRING("Floja"), COMPOUND_STRING("Miedosa"), COMPOUND_STRING("Activa"),
    COMPOUND_STRING("Seria"), COMPOUND_STRING("Alegre"), COMPOUND_STRING("Ingenua"), COMPOUND_STRING("Modesta"),
    COMPOUND_STRING("Afable"), COMPOUND_STRING("Mansa"), COMPOUND_STRING("Tímida"), COMPOUND_STRING("Alocada"),
    COMPOUND_STRING("Serena"), COMPOUND_STRING("Amable"), COMPOUND_STRING("Grosera"), COMPOUND_STRING("Cauta"),
    COMPOUND_STRING("Rara"),
};

// Characteristic from the highest IV (ties start at personality % 6) and that IV % 5.
static const u8 sCharacteristicStats[NUM_STATS] = {STAT_HP, STAT_ATK, STAT_DEF, STAT_SPEED, STAT_SPATK, STAT_SPDEF};
static const u8 *const sCharacteristics[NUM_STATS][5] =
{
    [STAT_HP] = {COMPOUND_STRING("Le encanta comer"), COMPOUND_STRING("A menudo se duerme"), COMPOUND_STRING("Duerme mucho"),
                 COMPOUND_STRING("Suele desordenar"), COMPOUND_STRING("Le gusta relajarse")},
    [STAT_ATK] = {COMPOUND_STRING("Orgulloso de su fuerza"), COMPOUND_STRING("Le gusta revolverse"), COMPOUND_STRING("A veces se enfada"),
                  COMPOUND_STRING("Le gusta luchar"), COMPOUND_STRING("Tiene mal genio")},
    [STAT_DEF] = {COMPOUND_STRING("Cuerpo resistente"), COMPOUND_STRING("Aguanta los golpes"), COMPOUND_STRING("Muy persistente"),
                  COMPOUND_STRING("Muy perseverante"), COMPOUND_STRING("Muy tenaz")},
    [STAT_SPEED] = {COMPOUND_STRING("Le gusta correr"), COMPOUND_STRING("Atento a los sonidos"), COMPOUND_STRING("Impetuoso y bobo"),
                    COMPOUND_STRING("Es un poco payaso"), COMPOUND_STRING("Huye rápido")},
    [STAT_SPATK] = {COMPOUND_STRING("Muy curioso"), COMPOUND_STRING("Muy travieso"), COMPOUND_STRING("Muy astuto"),
                    COMPOUND_STRING("Muy distraído"), COMPOUND_STRING("Muy quisquilloso")},
    [STAT_SPDEF] = {COMPOUND_STRING("Voluntarioso"), COMPOUND_STRING("Algo vanidoso"), COMPOUND_STRING("Muy insolente"),
                    COMPOUND_STRING("Odia perder"), COMPOUND_STRING("Un poco cabezota")},
};

static const u8 *const sAilmentNames[] =
{
    [AILMENT_PSN]  = COMPOUND_STRING("ENV"),
    [AILMENT_PRZ]  = COMPOUND_STRING("PAR"),
    [AILMENT_SLP]  = COMPOUND_STRING("DOR"),
    [AILMENT_FRZ]  = COMPOUND_STRING("CON"),
    [AILMENT_BRN]  = COMPOUND_STRING("QUE"),
    [AILMENT_PKRS] = COMPOUND_STRING("PKRS"),
    [AILMENT_FNT]  = COMPOUND_STRING("DEB"),
};

static void CB2_InitRFSummary(void);
static void Task_RFSumInput(u8 taskId);
static void Task_RFSumMoves(u8 taskId);
static void Task_RFSumChangeMon(u8 taskId);
static void Task_RFSumClose(u8 taskId);
static void RFSum_Draw(void);
static void RFSum_UpdateSprites(void);

// ---------------------------------------------------------------------------------------------
// Data helpers

static bool32 RFSum_IsEgg(void)
{
    return GetMonData(&sRFSum->mon, MON_DATA_SANITY_IS_BAD_EGG) || GetMonData(&sRFSum->mon, MON_DATA_IS_EGG);
}

static enum Species RFSum_Species(void)
{
    return GetMonData(&sRFSum->mon, MON_DATA_SPECIES);
}

static void RFSum_CopyMon(void)
{
    if (!sRFSum->isBoxMon)
        sRFSum->mon = sRFSum->list.mons[sRFSum->curIndex];
    else
        BoxMonToMon(&sRFSum->list.boxMons[sRFSum->curIndex], &sRFSum->mon);
}

static u32 RFSum_IV(u32 stat)
{
    if (P_SUMMARY_SCREEN_IV_HYPERTRAIN && GetMonData(&sRFSum->mon, MON_DATA_HYPER_TRAINED_HP + stat))
        return MAX_PER_STAT_IVS;
    return GetMonData(&sRFSum->mon, MON_DATA_HP_IV + stat);
}

static enum Type RFSum_HiddenPowerType(void)
{
    u32 bits = 0, i, count = 0;
    enum Type types[NUMBER_OF_MON_TYPES];

    for (i = 0; i < NUM_STATS; i++)
        bits |= (GetMonData(&sRFSum->mon, MON_DATA_HP_IV + i) & 1) << i;
    for (i = 0; i < NUMBER_OF_MON_TYPES; i++)
    {
        if (gTypesInfo[i].isHiddenPowerType)
            types[count++] = i;
    }
    return count ? types[((count - 1) * bits) / 63] : TYPE_NORMAL;
}

static const u8 *RFSum_Characteristic(void)
{
    u32 start = GetMonData(&sRFSum->mon, MON_DATA_PERSONALITY) % NUM_STATS;
    u32 best = sCharacteristicStats[start], bestIv = RFSum_IV(best), i;

    for (i = 1; i < NUM_STATS; i++)
    {
        u32 stat = sCharacteristicStats[(start + i) % NUM_STATS];
        if (RFSum_IV(stat) > bestIv)
        {
            best = stat;
            bestIv = RFSum_IV(stat);
        }
    }
    return sCharacteristics[best][bestIv % 5];
}

static bool32 RFSum_OTMatchesOwner(void)
{
    u8 name[PLAYER_NAME_LENGTH + 1];
    u32 trainerId;
    u8 gender;

    if (sRFSum->list.mons == gParties[B_TRAINER_OPPONENT_A])
    {
        u8 multiId = GetMultiplayerId() ^ 1;
        trainerId = gLinkPlayers[multiId].trainerId & 0xFFFF;
        gender = gLinkPlayers[multiId].gender;
        StringCopy(gStringVar1, gLinkPlayers[multiId].name);
    }
    else
    {
        trainerId = GetPlayerIDAsU32() & 0xFFFF;
        gender = gSaveBlock2Ptr->playerGender;
        StringCopy(gStringVar1, gSaveBlock2Ptr->playerName);
    }
    GetMonData(&sRFSum->mon, MON_DATA_OT_NAME, name);
    ConvertInternationalString(name, GetMonData(&sRFSum->mon, MON_DATA_LANGUAGE));
    return gender == GetMonData(&sRFSum->mon, MON_DATA_OT_GENDER)
        && trainerId == (GetMonData(&sRFSum->mon, MON_DATA_OT_ID) & 0xFFFF)
        && !StringCompareWithoutExtCtrlCodes(gStringVar1, name);
}

static bool32 RFSum_IsInGamePartnerMon(void)
{
    if (gPartyMenu.layout == PARTY_LAYOUT_MULTI_FULL)
        return FALSE;
    if (gPartyMenu.layout == PARTY_LAYOUT_MULTI_FULL_PARTNER)
        return TRUE;
    if ((gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER) && gMain.inBattle)
        return sRFSum->curIndex == 1 || sRFSum->curIndex == 4 || sRFSum->curIndex == 5;
    return FALSE;
}

static bool32 RFSum_CanRename(void)
{
    return P_SUMMARY_SCREEN_RENAME && !sRFSum->lockMoves && !RFSum_IsEgg()
        && sRFSum->mode != SUMMARY_MODE_BOX_CURSOR && !InBattleFactory() && !InSlateportBattleTent()
        && GetPlayerIDAsU32() == GetMonData(&sRFSum->mon, MON_DATA_OT_ID);
}

static bool32 RFSum_CanRelearn(void)
{
    return P_SUMMARY_SCREEN_MOVE_RELEARNER && !sRFSum->lockMoves && sRFSum->mode != SUMMARY_MODE_BOX_CURSOR
        && sRFSum->mode != SUMMARY_MODE_SELECT_MOVE && sRFSum->hasRelearnableMoves
        && !InBattleFactory() && !InSlateportBattleTent();
}

static void RFSum_UpdateRelearnable(void)
{
    u32 i;

    sRFSum->hasRelearnableMoves = FALSE;
    for (i = 0; i < MOVE_RELEARNER_COUNT; i++)
    {
        enum MoveRelearnerStates state = (gMoveRelearnerState + i) % MOVE_RELEARNER_COUNT;
        if (CanBoxMonRelearnMoves(&sRFSum->mon.box, state))
        {
            sRFSum->hasRelearnableMoves = TRUE;
            gMoveRelearnerState = state;
            break;
        }
    }
}

static enum Move RFSum_RowMove(u32 row)
{
    if (row == MAX_MON_MOVES)
        return sRFSum->newMove;
    return GetMonData(&sRFSum->mon, MON_DATA_MOVE1 + row);
}

static u32 RFSum_RowCount(void)
{
    return sRFSum->mode == SUMMARY_MODE_SELECT_MOVE ? MOVE_ROWS : MAX_MON_MOVES;
}

// ---------------------------------------------------------------------------------------------
// Drawing

static void RFSum_LoadPalettes(void)
{
    bool32 dark = KeySystem_IsOn(KEY_DARK_MODE);
    u16 colors[16];

    colors[C_GROUND] = RFUI_Color(dark, RF_ROLE_GROUND);
    colors[C_PANEL] = RFUI_Color(dark, RF_ROLE_PANEL);
    colors[C_TEXT] = RFUI_Color(dark, RF_ROLE_TEXT);
    colors[C_SHADOW] = RFUI_Color(dark, RF_ROLE_SHADOW);
    colors[C_MUTED] = RFUI_Color(dark, RF_ROLE_MUTED);
    colors[C_EDGE] = RFUI_Color(dark, RF_ROLE_EDGE);
    colors[C_SELECTED] = RFUI_Color(dark, RF_ROLE_SELECTED);
    colors[C_GOLD] = RFUI_Color(dark, RF_ROLE_GOLD);
    colors[C_ACCENT] = RFUI_Color(dark, RF_ROLE_ACCENT);
    colors[C_SWAP] = RFUI_Color(dark, RF_ROLE_BAR);
    colors[C_RED] = dark ? RGB(31, 12, 10) : RGB(25, 4, 4);
    colors[C_BLUE] = dark ? RGB(12, 20, 31) : RGB(4, 10, 26);
    colors[C_GREEN] = dark ? RGB(10, 26, 12) : RGB(6, 21, 8);
    colors[C_YELLOW] = RGB(30, 24, 4);
    colors[C_TRACK] = RFUI_Color(dark, dark ? RF_ROLE_GROUND : RF_ROLE_SHADOW);
    colors[C_WHITE] = RGB(31, 31, 31);
    LoadPalette(colors, BG_PLTT_ID(0), sizeof(colors));
}

static void Print(const u8 *str, u32 font, u32 x, u32 y, u8 bg, u8 fg)
{
    const u8 colors[3] = {bg, fg, C_SHADOW};
    AddTextPrinterParameterized3(sRFSum->window, font, x, y, colors, TEXT_SKIP_DRAW, str);
}

static void PrintRight(const u8 *str, u32 font, u32 right, u32 y, u8 bg, u8 fg)
{
    s32 width = GetStringWidth(font, str, 0);
    Print(str, font, right > (u32)width ? right - width : 0, y, bg, fg);
}

// Prints in the given width, switching to the narrower font or cutting with "..." if needed.
static void PrintFit(const u8 *str, u32 x, u32 y, u32 width, u8 bg, u8 fg)
{
    const u8 colors[3] = {bg, fg, C_SHADOW};
    RFUI_PrintFitted(sRFSum->window, str, x, y, width, colors);
}

static void PrintNumber(s32 value, u32 digits, u32 right, u32 y, u8 fg)
{
    u8 text[12];
    ConvertIntToDecimalStringN(text, value, STR_CONV_MODE_LEFT_ALIGN, digits);
    PrintRight(text, FONT_SMALL, right, y, C_PANEL, fg);
}

static void Label(const u8 *str, u32 row)
{
    Print(str, FONT_SMALL, LABEL_X, ROW_Y(row), C_PANEL, C_MUTED);
}

static void Value(const u8 *str, u32 row)
{
    PrintFit(str, VALUE_X, ROW_Y(row), VALUE_W, C_PANEL, C_TEXT);
}

static void DrawBar(u32 x, u32 y, u32 width, u32 filled, u8 color)
{
    RFUI_FillRect(sRFSum->window, C_TRACK, x, y, width, 3);
    if (filled)
        RFUI_FillRect(sRFSum->window, color, x, y, min(filled, width), 3);
}

static void DrawTabs(void)
{
    u32 page, x = LEFT_X;

    u32 textWidth = 0, gap;

    // The four tabs share the header width; the spare pixels become equal padding.
    for (page = 0; page < PAGE_COUNT; page++)
        textWidth += GetStringWidth(FONT_SMALL, sPageNames[page], 0);
    gap = (DISPLAY_WIDTH - 2 * LEFT_X - 2 * (PAGE_COUNT - 1) - textWidth) / (2 * PAGE_COUNT);
    for (page = 0; page < PAGE_COUNT; page++)
    {
        u32 width = GetStringWidth(FONT_SMALL, sPageNames[page], 0) + 2 * gap;
        bool32 active = page == sRFSum->page;
        bool32 available = !RFSum_IsEgg() || page == PAGE_DATA;
        u8 fill = active ? C_SELECTED : C_PANEL;

        RFUI_DrawPanel(sRFSum->window, x, 2, width, 15, fill, C_EDGE);
        Print(sPageNames[page], FONT_SMALL, x + gap, 3, fill, active ? C_TEXT : (available ? C_MUTED : C_TRACK));
        x += width + 2;
    }
}

static void DrawFooter(void)
{
    const u8 *hint;

    if (sRFSum->mode == SUMMARY_MODE_SELECT_MOVE && sRFSum->page == PAGE_MOVES)
        hint = COMPOUND_STRING("{A_BUTTON} Olvidar  {B_BUTTON} No aprender  {DPAD_LEFTRIGHT} Páginas");
    else if (sRFSum->page == PAGE_MOVES && sRFSum->movesState == MOVES_SWAP)
        hint = COMPOUND_STRING("{A_BUTTON} Colocar aquí  {B_BUTTON} Cancelar");
    else if (sRFSum->page == PAGE_MOVES && sRFSum->movesState == MOVES_BROWSE)
        hint = sRFSum->lockMoves ? COMPOUND_STRING("{DPAD_UPDOWN} Elegir  {B_BUTTON} Volver")
                                 : COMPOUND_STRING("{DPAD_UPDOWN} Elegir  {A_BUTTON} Mover  {B_BUTTON} Volver");
    else if (sRFSum->page == PAGE_MOVES)
        hint = RFSum_CanRelearn() ? COMPOUND_STRING("{A_BUTTON} Detalles  {START_BUTTON} Recordar  {B_BUTTON} Salir")
                                  : COMPOUND_STRING("{A_BUTTON} Detalles  {DPAD_UPDOWN} Pokémon  {B_BUTTON} Salir");
    else if (sRFSum->page == PAGE_DATA && RFSum_CanRename())
        hint = COMPOUND_STRING("{A_BUTTON} Apodo  {DPAD_LEFTRIGHT} Páginas  {DPAD_UPDOWN} Pokémon  {B_BUTTON} Salir");
    else
        hint = COMPOUND_STRING("{DPAD_LEFTRIGHT} Páginas  {DPAD_UPDOWN} Pokémon  {B_BUTTON} Salir");
    PrintFit(hint, LEFT_X + 2, 148, DISPLAY_WIDTH - 2 * (LEFT_X + 2), C_GROUND, C_TEXT);
}

static void DrawPortrait(void)
{
    u8 text[32];
    struct Pokemon *mon = &sRFSum->mon;
    u32 right = LEFT_X + LEFT_W - PAD, x = LEFT_X + PAD;

    // Rows: number (top, ball sprite on its right), picture, markings sprite, name, level and
    // status, HP figures, HP bar.
    RFUI_DrawPanel(sRFSum->window, LEFT_X, BODY_Y, LEFT_W, BODY_H, C_PANEL, C_EDGE);
    if (RFSum_IsEgg())
    {
        Print(COMPOUND_STRING("HUEVO"), FONT_NORMAL, x, PORTRAIT_NAME_Y, C_PANEL, C_TEXT);
        return;
    }

    StringCopy(text, COMPOUND_STRING("Nº "));
    ConvertIntToDecimalStringN(text + StringLength(text), SpeciesToNationalPokedexNum(RFSum_Species()), STR_CONV_MODE_LEADING_ZEROS, 4);
    Print(text, FONT_SMALL, x, BODY_Y + 3, C_PANEL, C_MUTED);
    if (GetMonData(mon, MON_DATA_IS_SHINY))
        Print(COMPOUND_STRING("{STAR}"), FONT_SMALL, x + 38, BODY_Y + 3, C_PANEL, C_GOLD);

    GetMonData(mon, MON_DATA_NICKNAME, text);
    StringGet_Nickname(text);
    PrintFit(text, x, PORTRAIT_NAME_Y, LEFT_W - 2 * PAD - 8, C_PANEL, C_TEXT);
    switch (GetMonGender(mon))
    {
    case MON_MALE:
        PrintRight(gText_MaleSymbol, FONT_NORMAL, right + 1, PORTRAIT_NAME_Y, C_PANEL, C_BLUE);
        break;
    case MON_FEMALE:
        PrintRight(gText_FemaleSymbol, FONT_NORMAL, right + 1, PORTRAIT_NAME_Y, C_PANEL, C_RED);
        break;
    }

    StringCopy(text, COMPOUND_STRING("Nv. "));
    ConvertIntToDecimalStringN(text + StringLength(text), GetMonData(mon, MON_DATA_LEVEL), STR_CONV_MODE_LEFT_ALIGN, 3);
    Print(text, FONT_SMALL, x, PORTRAIT_NAME_Y + 15, C_PANEL, C_TEXT);
    {
        u32 ailment = GetMonAilment(mon);
        if (ailment != AILMENT_NONE && ailment < ARRAY_COUNT(sAilmentNames))
            PrintRight(sAilmentNames[ailment], FONT_SMALL, right, PORTRAIT_NAME_Y + 15, C_PANEL, ailment == AILMENT_PKRS ? C_ACCENT : C_RED);
        else if (ShouldPokemonShowCuredPokerus(mon))
            PrintRight(COMPOUND_STRING("PKRS"), FONT_SMALL, right, PORTRAIT_NAME_Y + 15, C_PANEL, C_MUTED);
    }

    {
        u32 hp = GetMonData(mon, MON_DATA_HP), maxHp = GetMonData(mon, MON_DATA_MAX_HP);
        u32 width = LEFT_W - 2 * PAD;
        u32 filled = maxHp ? (hp * width + maxHp - 1) / maxHp : 0;
        u8 color = (hp * 2 > maxHp) ? C_GREEN : (hp * 5 > maxHp ? C_YELLOW : C_RED);

        Print(COMPOUND_STRING("PS"), FONT_SMALL, x, PORTRAIT_NAME_Y + 26, C_PANEL, C_MUTED);
        ConvertIntToDecimalStringN(text, hp, STR_CONV_MODE_LEFT_ALIGN, 4);
        StringAppend(text, COMPOUND_STRING("/"));
        ConvertIntToDecimalStringN(text + StringLength(text), maxHp, STR_CONV_MODE_LEFT_ALIGN, 4);
        PrintRight(text, FONT_SMALL, right, PORTRAIT_NAME_Y + 26, C_PANEL, C_TEXT);
        DrawBar(x, PORTRAIT_NAME_Y + 39, width, filled, color);
    }
}

static void DrawNature(u32 row)
{
    u32 nature = GetNature(&sRFSum->mon);
    u32 statNature = GetMonData(&sRFSum->mon, MON_DATA_HIDDEN_NATURE);
    const struct NatureInfo *info = &gNaturesInfo[statNature];

    StringCopy(gStringVar4, sNatureNames[nature]);
    if (statNature != nature)
    {
        StringAppend(gStringVar4, COMPOUND_STRING(" (menta "));
        StringAppend(gStringVar4, sNatureNames[statNature]);
        StringAppend(gStringVar4, COMPOUND_STRING(")"));
    }
    Label(COMPOUND_STRING("Naturaleza"), row);
    Value(gStringVar4, row);
    if (info->statUp == info->statDown)
    {
        Print(COMPOUND_STRING("Neutra"), FONT_SMALL, VALUE_X, ROW_Y(row + 1), C_PANEL, C_MUTED);
        return;
    }
    StringCopy(gStringVar4, COMPOUND_STRING("+"));
    StringAppend(gStringVar4, sStatShortNames[info->statUp]);
    Print(gStringVar4, FONT_SMALL, VALUE_X, ROW_Y(row + 1), C_PANEL, C_RED);
    StringCopy(gStringVar4, COMPOUND_STRING("-"));
    StringAppend(gStringVar4, sStatShortNames[info->statDown]);
    PrintRight(gStringVar4, FONT_SMALL, RIGHT_X + RIGHT_W - PAD, ROW_Y(row + 1), C_PANEL, C_BLUE);
}

// Word-wraps a text (line breaks count as spaces) into `width`, printing up to `maxLines` lines
// from `y`. Only a single word longer than the width falls back to the narrower font.
static void PrintParagraph(const u8 *str, u32 x, u32 y, u32 width, u32 maxLines, u8 fg)
{
    u8 line[80], word[40];
    u32 lineLen = 0, lines = 0;

    line[0] = EOS;
    while (lines < maxLines)
    {
        u32 wordLen = 0;
        while (*str == CHAR_SPACE || *str == CHAR_NEWLINE || *str == CHAR_PROMPT_SCROLL || *str == CHAR_PROMPT_CLEAR)
            str++;
        if (*str == EOS)
            break;
        while (*str != EOS && *str != CHAR_SPACE && *str != CHAR_NEWLINE && *str != CHAR_PROMPT_SCROLL
               && *str != CHAR_PROMPT_CLEAR && wordLen < sizeof(word) - 1)
        {
            // Button icons, extra symbols and control codes keep their argument byte, which
            // may be 0x00, the space character.
            if ((*str == CHAR_KEYPAD_ICON || *str == CHAR_EXTRA_SYMBOL || *str == EXT_CTRL_CODE_BEGIN)
             && str[1] != EOS && wordLen < sizeof(word) - 3)
                word[wordLen++] = *str++;
            word[wordLen++] = *str++;
        }
        word[wordLen] = EOS;
        if (lineLen)
        {
            if (lineLen + 1 + wordLen < sizeof(line))
            {
                line[lineLen] = CHAR_SPACE;
                StringCopy(line + lineLen + 1, word);
                if (GetStringWidth(FONT_SMALL, line, 0) <= (s32)width)
                {
                    lineLen += 1 + wordLen;
                    continue;
                }
            }
            line[lineLen] = EOS;
            PrintFit(line, x, y + LINE * lines++, width, C_PANEL, fg);
            lineLen = 0;
            if (lines >= maxLines)
                return;
        }
        StringCopy(line, word);
        lineLen = wordLen;
    }
    if (lineLen && lines < maxLines)
        PrintFit(line, x, y + LINE * lines, width, C_PANEL, fg);
}

static void DrawDataPage(void)
{
    struct Pokemon *mon = &sRFSum->mon;
    enum Item item = GetMonData(mon, MON_DATA_HELD_ITEM);
    enum Ability ability = GetMonAbility(mon);

    RFUI_DrawPanel(sRFSum->window, RIGHT_X, BODY_Y, RIGHT_W, BODY_H, C_PANEL, C_EDGE);
    if (RFSum_IsEgg())
    {
        u32 friendship = GetMonData(mon, MON_DATA_FRIENDSHIP);
        const u8 *state;

        if (GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG))
            state = COMPOUND_STRING("Tardará mucho en eclosionar.");
        else if (friendship <= 5)
            state = COMPOUND_STRING("¡Se mueve! Está a punto de eclosionar.");
        else if (friendship <= 10)
            state = COMPOUND_STRING("Se oyen ruidos. Eclosionará pronto.");
        else if (friendship <= 40)
            state = COMPOUND_STRING("Aún le falta un poco para eclosionar.");
        else
            state = COMPOUND_STRING("Tardará mucho en eclosionar.");
        Label(COMPOUND_STRING("Estado"), 0);
        PrintParagraph(state, LABEL_X, ROW_Y(1), RIGHT_W - 2 * PAD, 2, C_TEXT);
        Label(COMPOUND_STRING("Origen"), 4);
        PrintParagraph(RFSum_OTMatchesOwner() ? COMPOUND_STRING("Un Huevo que conseguiste tú.")
                                              : COMPOUND_STRING("Un Huevo obtenido en un\nintercambio."),
                       LABEL_X, ROW_Y(5), RIGHT_W - 2 * PAD, 2, C_TEXT);
        return;
    }

    Label(COMPOUND_STRING("Especie"), 0);
    Value(GetSpeciesName(RFSum_Species()), 0);
    Label(COMPOUND_STRING("Tipo"), 1); // icons are sprites
    Label(COMPOUND_STRING("Objeto"), 2);
    Value(item == ITEM_NONE ? COMPOUND_STRING("Ninguno") : GetItemName(item), 2);
    DrawNature(3);
    Label(COMPOUND_STRING("Habilidad"), 5);
    PrintFit(gAbilitiesInfo[ability].name, VALUE_X, ROW_Y(5), RIGHT_X + RIGHT_W - PAD - VALUE_X, C_PANEL, C_GOLD);
    PrintParagraph(gAbilitiesInfo[ability].description, LABEL_X, ROW_Y(6) + 2, RIGHT_W - 2 * PAD, 3, C_TEXT);
}

static void DrawStatsPage(void)
{
    struct Pokemon *mon = &sRFSum->mon;
    const struct NatureInfo *nature = &gNaturesInfo[GetMonData(mon, MON_DATA_HIDDEN_NATURE)];
    enum Species species = RFSum_Species();
    u32 i, baseTotal = 0, ivTotal = 0, evTotal = 0;
    u32 colValue = RIGHT_X + 70, colBase = RIGHT_X + 94, colIv = RIGHT_X + 116, colEv = RIGHT_X + RIGHT_W - PAD;

    RFUI_DrawPanel(sRFSum->window, RIGHT_X, BODY_Y, RIGHT_W, BODY_H, C_PANEL, C_EDGE);
    PrintRight(COMPOUND_STRING("Valor"), FONT_SMALL, colValue, ROW_Y(0), C_PANEL, C_MUTED);
    PrintRight(COMPOUND_STRING("Base"), FONT_SMALL, colBase, ROW_Y(0), C_PANEL, C_MUTED);
    PrintRight(COMPOUND_STRING("IV"), FONT_SMALL, colIv, ROW_Y(0), C_PANEL, C_MUTED);
    PrintRight(COMPOUND_STRING("EV"), FONT_SMALL, colEv, ROW_Y(0), C_PANEL, C_MUTED);

    for (i = 0; i < NUM_STATS; i++)
    {
        u32 stat = sStatOrder[i], y = ROW_Y(i + 1);
        u32 base = GetSpeciesBaseStat(species, stat), iv = RFSum_IV(stat);
        u32 ev = GetMonData(mon, MON_DATA_HP_EV + stat);
        u8 color = C_TEXT;

        if (P_SUMMARY_SCREEN_NATURE_COLORS && nature->statUp != nature->statDown)
        {
            if (stat == nature->statUp)
                color = C_RED;
            else if (stat == nature->statDown)
                color = C_BLUE;
        }
        Print(sStatNames[stat], FONT_SMALL, LABEL_X, y, C_PANEL, color);
        PrintNumber(GetMonData(mon, MON_DATA_MAX_HP + stat), 3, colValue, y, color);
        PrintNumber(base, 3, colBase, y, C_TEXT);
        PrintNumber(iv, 2, colIv, y, iv == MAX_PER_STAT_IVS ? C_GOLD : C_TEXT);
        PrintNumber(ev, 3, colEv, y, ev == MAX_PER_STAT_EVS ? C_GOLD : C_TEXT);
        baseTotal += base;
        ivTotal += iv;
        evTotal += ev;
    }
    Print(COMPOUND_STRING("Total"), FONT_SMALL, LABEL_X, ROW_Y(7), C_PANEL, C_MUTED);
    PrintNumber(baseTotal, 3, colBase, ROW_Y(7), C_GOLD);
    PrintNumber(ivTotal, 3, colIv, ROW_Y(7), C_GOLD);
    PrintNumber(evTotal, 3, colEv, ROW_Y(7), C_GOLD);

    {
        u32 level = GetMonData(mon, MON_DATA_LEVEL), exp = GetMonData(mon, MON_DATA_EXP);
        u32 growth = gSpeciesInfo[species].growthRate, filled = 0, width = RIGHT_W - 2 * PAD;
        u8 text[24];

        Print(COMPOUND_STRING("EXP"), FONT_SMALL, LABEL_X, ROW_Y(8) + 4, C_PANEL, C_MUTED);
        ConvertIntToDecimalStringN(text, exp, STR_CONV_MODE_LEFT_ALIGN, 7);
        Print(text, FONT_SMALL, LABEL_X + 20, ROW_Y(8) + 4, C_PANEL, C_TEXT);
        if (level < MAX_LEVEL)
        {
            u32 low = gExperienceTables[growth][level], high = gExperienceTables[growth][level + 1];
            StringCopy(text, COMPOUND_STRING("Faltan "));
            ConvertIntToDecimalStringN(text + StringLength(text), high - exp, STR_CONV_MODE_LEFT_ALIGN, 7);
            filled = (u64)(exp - low) * width / (high - low);
        }
        else
        {
            StringCopy(text, COMPOUND_STRING("Nivel máximo"));
        }
        PrintRight(text, FONT_SMALL, RIGHT_X + RIGHT_W - PAD, ROW_Y(8) + 4, C_PANEL, C_TEXT);
        DrawBar(LABEL_X, ROW_Y(9) + 5, width, filled, C_ACCENT);
    }
}

static void DrawMoveDetail(void)
{
    u32 x = LABEL_X, y = DETAIL_Y + 4, width = RIGHT_W - 2 * PAD;
    enum Move move;
    u8 text[24];

    RFUI_DrawPanel(sRFSum->window, RIGHT_X, DETAIL_Y, RIGHT_W, BODY_Y + BODY_H - DETAIL_Y, C_PANEL, C_EDGE);
    if (sRFSum->hmWarning)
    {
        PrintParagraph(COMPOUND_STRING("Los movimientos de MO no se\npueden olvidar ahora."), x, y, width, 2, C_RED);
        return;
    }
    if (sRFSum->movesState == MOVES_IDLE && sRFSum->mode != SUMMARY_MODE_SELECT_MOVE)
    {
        PrintParagraph(sRFSum->lockMoves ? COMPOUND_STRING("Pulsa {A_BUTTON} para ver la potencia,\nla precisión y el efecto.")
                                         : COMPOUND_STRING("Pulsa {A_BUTTON} para ver detalles\ny cambiar el orden."),
                       x, y, width, 2, C_MUTED);
        return;
    }
    move = RFSum_RowMove(sRFSum->movesState == MOVES_SWAP ? sRFSum->swapCursor : sRFSum->cursor);
    if (move == MOVE_NONE)
    {
        Print(COMPOUND_STRING("No aprender el movimiento nuevo."), FONT_SMALL, x, y, C_PANEL, C_MUTED);
        return;
    }
    // The category icon (sprite) takes the first 34 pixels of the first line.
    StringCopy(text, COMPOUND_STRING("Pot. "));
    if (GetMovePower(move) <= 1)
        StringAppend(text, COMPOUND_STRING("---"));
    else
        ConvertIntToDecimalStringN(text + StringLength(text), GetMovePower(move), STR_CONV_MODE_LEFT_ALIGN, 3);
    Print(text, FONT_SMALL, x + 36, y, C_PANEL, C_TEXT);
    StringCopy(text, COMPOUND_STRING("Prec. "));
    if (GetMoveAccuracy(move) == 0)
        StringAppend(text, COMPOUND_STRING("---"));
    else
        ConvertIntToDecimalStringN(text + StringLength(text), GetMoveAccuracy(move), STR_CONV_MODE_LEFT_ALIGN, 3);
    PrintRight(text, FONT_SMALL, RIGHT_X + RIGHT_W - PAD, y, C_PANEL, C_TEXT);
    PrintParagraph(GetMoveDescription(move), x, y + LINE, width, 2, C_TEXT);
}

static void DrawMovesPage(void)
{
    u32 row, rows = RFSum_RowCount();
    u8 ppBonuses = GetMonData(&sRFSum->mon, MON_DATA_PP_BONUSES);

    RFUI_DrawPanel(sRFSum->window, RIGHT_X, BODY_Y, RIGHT_W, DETAIL_Y - BODY_Y - 2, C_PANEL, C_EDGE);
    for (row = 0; row < rows; row++)
    {
        enum Move move = RFSum_RowMove(row);
        u32 y = MOVE_ROW_Y(row);
        u8 fill = C_PANEL;
        u8 text[16];

        if (sRFSum->movesState != MOVES_IDLE || sRFSum->mode == SUMMARY_MODE_SELECT_MOVE)
        {
            if (sRFSum->movesState == MOVES_SWAP && row == sRFSum->swapCursor)
                fill = C_SELECTED;
            else if (row == sRFSum->cursor)
                fill = sRFSum->movesState == MOVES_SWAP ? C_SWAP : C_SELECTED;
        }
        if (fill != C_PANEL)
            RFUI_FillRect(sRFSum->window, fill, RIGHT_X + 3, y - 1, RIGHT_W - 6, 16);
        if (row == MAX_MON_MOVES)
            RFUI_FillRect(sRFSum->window, C_EDGE, RIGHT_X + 3, y - 2, RIGHT_W - 6, 1);

        if (move == MOVE_NONE)
        {
            Print(row == MAX_MON_MOVES ? COMPOUND_STRING("(no aprender)") : COMPOUND_STRING("-"),
                  FONT_SMALL, RIGHT_X + 42, y + 2, fill, C_MUTED);
            continue;
        }
        PrintFit(GetMoveName(move), RIGHT_X + 42, y, 72, fill, fill == C_SWAP ? C_WHITE : C_TEXT);
        if (row == MAX_MON_MOVES)
        {
            Print(COMPOUND_STRING("NUEVO"), FONT_SMALL, RIGHT_X + RIGHT_W - PAD - 26, y + 2, fill, C_GOLD);
            continue;
        }
        {
            u32 pp = GetMonData(&sRFSum->mon, MON_DATA_PP1 + row), maxPp = CalculatePPWithBonus(move, ppBonuses, row);
            ConvertIntToDecimalStringN(text, pp, STR_CONV_MODE_LEFT_ALIGN, 2);
            StringAppend(text, COMPOUND_STRING("/"));
            ConvertIntToDecimalStringN(text + StringLength(text), maxPp, STR_CONV_MODE_LEFT_ALIGN, 2);
            PrintRight(text, FONT_SMALL, RIGHT_X + RIGHT_W - PAD, y + 2, fill,
                       pp == 0 ? C_RED : (pp * 4 <= maxPp ? C_GOLD : C_TEXT));
        }
    }
    DrawMoveDetail();
}

// Where and how the Pokémon was obtained, as one sentence for PrintParagraph.
static void BufferMemo(u8 *dest)
{
    u32 location = GetMonData(&sRFSum->mon, MON_DATA_MET_LOCATION);
    u32 level = GetMonData(&sRFSum->mon, MON_DATA_MET_LEVEL);
    u32 game = GetMonData(&sRFSum->mon, MON_DATA_MET_GAME);
    u8 place[32];

    if (InBattleFactory() || InSlateportBattleTent() || RFSum_IsInGamePartnerMon())
    {
        StringCopy(dest, COMPOUND_STRING("Pokémon de alquiler."));
        return;
    }
    if (location < MAPSEC_NONE)
        GetMapNameHandleAquaHideout(place, location);
    else
        StringCopy(place, COMPOUND_STRING("un lugar lejano"));

    if (RFSum_OTMatchesOwner())
    {
        StringCopy(dest, level == 0 ? COMPOUND_STRING("Eclosionó en ") : COMPOUND_STRING("Lo conociste en "));
        StringAppend(dest, place);
        StringAppend(dest, COMPOUND_STRING(" con nivel "));
        ConvertIntToDecimalStringN(dest + StringLength(dest), level == 0 ? EGG_HATCH_LEVEL : level, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(dest, COMPOUND_STRING("."));
    }
    else if (location == METLOC_FATEFUL_ENCOUNTER)
    {
        StringCopy(dest, COMPOUND_STRING("Lo obtuviste en un encuentro fatídico."));
    }
    else if (location != METLOC_IN_GAME_TRADE && game > 0 && game <= VERSION_LEAF_GREEN && location < MAPSEC_NONE)
    {
        StringCopy(dest, COMPOUND_STRING("Probablemente conocido en "));
        StringAppend(dest, place);
        StringAppend(dest, COMPOUND_STRING("."));
    }
    else
    {
        StringCopy(dest, COMPOUND_STRING("Obtenido en un intercambio."));
    }
}

static void DrawMemoPage(void)
{
    struct Pokemon *mon = &sRFSum->mon;
    u8 text[32];
    enum PokeBall ball = GetMonData(mon, MON_DATA_POKEBALL);
    u32 friendship = GetMonData(mon, MON_DATA_FRIENDSHIP);
    u32 barX = VALUE_X + 36, barW = RIGHT_X + RIGHT_W - PAD - barX;

    RFUI_DrawPanel(sRFSum->window, RIGHT_X, BODY_Y, RIGHT_W, BODY_H, C_PANEL, C_EDGE);
    Label(COMPOUND_STRING("EO"), 0);
    GetMonData(mon, MON_DATA_OT_NAME, text);
    ConvertInternationalString(text, GetMonData(mon, MON_DATA_LANGUAGE));
    PrintFit(text, VALUE_X, ROW_Y(0), VALUE_W, C_PANEL, GetMonData(mon, MON_DATA_OT_GENDER) == FEMALE ? C_RED : C_BLUE);

    Label(COMPOUND_STRING("ID"), 1);
    ConvertIntToDecimalStringN(text, GetMonData(mon, MON_DATA_OT_ID) & 0xFFFF, STR_CONV_MODE_LEADING_ZEROS, 5);
    Value(text, 1);
    StringCopy(text, COMPOUND_STRING("Cintas "));
    ConvertIntToDecimalStringN(text + StringLength(text), GetMonData(mon, MON_DATA_RIBBON_COUNT), STR_CONV_MODE_LEFT_ALIGN, 3);
    PrintRight(text, FONT_SMALL, RIGHT_X + RIGHT_W - PAD, ROW_Y(1), C_PANEL, C_MUTED);

    Label(COMPOUND_STRING("Poké Ball"), 2);
    Value(ball < POKEBALL_COUNT ? GetItemName(gPokeBalls[ball].itemId) : COMPOUND_STRING("-"), 2);

    BufferMemo(gStringVar4);
    PrintParagraph(gStringVar4, LABEL_X, ROW_Y(3), TEXT_W, 2, C_TEXT);

    Label(COMPOUND_STRING("Amistad"), 5);
    ConvertIntToDecimalStringN(text, friendship, STR_CONV_MODE_LEFT_ALIGN, 3);
    Value(text, 5);
    DrawBar(barX, ROW_Y(5) + 6, barW, friendship * barW / MAX_FRIENDSHIP, C_RED);

    Label(COMPOUND_STRING("Carácter"), 6);
    PrintParagraph(RFSum_Characteristic(), LABEL_X + 6, ROW_Y(7), TEXT_W - 6, 1, C_GOLD);
    Label(COMPOUND_STRING("P. Oculto"), 8);
    Value(gTypesInfo[RFSum_HiddenPowerType()].name, 8);
    Label(COMPOUND_STRING("Teratipo"), 9);
    Value(gTypesInfo[GetMonData(mon, MON_DATA_TERA_TYPE)].name, 9);
}

static void RFSum_Draw(void)
{
    FillWindowPixelBuffer(sRFSum->window, PIXEL_FILL(C_GROUND));
    DrawTabs();
    DrawPortrait();
    switch (sRFSum->page)
    {
    case PAGE_DATA:
        DrawDataPage();
        break;
    case PAGE_STATS:
        DrawStatsPage();
        break;
    case PAGE_MOVES:
        DrawMovesPage();
        break;
    case PAGE_MEMO:
        DrawMemoPage();
        break;
    }
    DrawFooter();
    CopyWindowToVram(sRFSum->window, COPYWIN_FULL);
    RFSum_UpdateSprites();
}

// ---------------------------------------------------------------------------------------------
// Sprites

static void SetTypeSprite(u32 index, enum Type type, s16 x, s16 y)
{
    struct Sprite *sprite = &gSprites[sRFSum->typeSpriteIds[index]];
    StartSpriteAnim(sprite, type);
    sprite->oam.paletteNum = gTypesInfo[type].palette;
    sprite->x = x + 16;
    sprite->y = y + 8;
    sprite->invisible = FALSE;
}

static void RFSum_UpdateSprites(void)
{
    u32 i;
    bool32 egg = RFSum_IsEgg();
    enum Species species = RFSum_Species();

    for (i = 0; i < ARRAY_COUNT(sRFSum->typeSpriteIds); i++)
        gSprites[sRFSum->typeSpriteIds[i]].invisible = TRUE;
    gSprites[sRFSum->categorySpriteId].invisible = TRUE;

    if (sRFSum->page == PAGE_DATA && !egg)
    {
        SetTypeSprite(0, GetSpeciesType(species, 0), VALUE_X, ROW_Y(1) - 2);
        if (GetSpeciesType(species, 1) != GetSpeciesType(species, 0))
            SetTypeSprite(1, GetSpeciesType(species, 1), VALUE_X + 36, ROW_Y(1) - 2);
    }
    else if (sRFSum->page == PAGE_MOVES)
    {
        enum Move detail = MOVE_NONE;

        for (i = 0; i < RFSum_RowCount(); i++)
        {
            enum Move move = RFSum_RowMove(i);
            if (move != MOVE_NONE)
                SetTypeSprite(2 + i, GetMoveType(move), RIGHT_X + PAD, MOVE_ROW_Y(i) - 1);
        }
        if (!sRFSum->hmWarning && (sRFSum->movesState != MOVES_IDLE || sRFSum->mode == SUMMARY_MODE_SELECT_MOVE))
            detail = RFSum_RowMove(sRFSum->movesState == MOVES_SWAP ? sRFSum->swapCursor : sRFSum->cursor);
        if (detail != MOVE_NONE && GetBattleMoveCategory(detail) != DAMAGE_CATEGORY_NONE)
        {
            struct Sprite *sprite = &gSprites[sRFSum->categorySpriteId];
            StartSpriteAnim(sprite, GetBattleMoveCategory(detail));
            sprite->x = LABEL_X + 16;
            sprite->y = DETAIL_Y + 10;
            sprite->invisible = FALSE;
        }
    }
    if (sRFSum->markings != NULL)
        sRFSum->markings->invisible = egg;
}

static void SpriteCB_RFSumMon(struct Sprite *sprite)
{
    if (!gPaletteFade.active && sprite->data[2] != 1)
    {
        sprite->data[1] = IsMonSpriteNotFlipped(sprite->data[0]);
        if (!RFSum_IsEgg())
            PlayCry_ByMode(sprite->data[0], 0, ShouldPlayNormalMonCry(&sRFSum->mon) ? CRY_MODE_NORMAL : CRY_MODE_WEAK);
        PokemonSummaryDoMonAnimation(sprite, sprite->data[0], RFSum_IsEgg());
    }
}

// Loads the Pokémon picture over several calls; returns the sprite id once it exists.
static u8 RFSum_LoadMonSprite(s16 *state)
{
    enum Species species = RFSum_Species();
    enum Species tag = GetMonData(&sRFSum->mon, MON_DATA_SPECIES_OR_EGG);
    u32 pid = GetMonData(&sRFSum->mon, MON_DATA_PERSONALITY);
    bool32 egg = RFSum_IsEgg();
    u8 spriteId;

    switch (*state)
    {
    case 0:
        HandleLoadSpecialPokePicIsEgg(TRUE, gMonSpritesGfxPtr != NULL
                                              ? gMonSpritesGfxPtr->spritesGfx[B_POSITION_OPPONENT_LEFT]
                                              : MonSpritesGfxManager_GetSpritePtr(MON_SPR_GFX_MANAGER_A, B_POSITION_OPPONENT_LEFT),
                                      species, pid, egg);
        (*state)++;
        return SPRITE_NONE;
    case 1:
        LoadSpritePaletteWithTag(GetMonSpritePalFromSpeciesAndPersonalityIsEgg(species, GetMonData(&sRFSum->mon, MON_DATA_IS_SHINY), pid, egg), tag);
        SetMultiuseSpriteTemplateToPokemon(tag, B_POSITION_OPPONENT_LEFT);
        (*state)++;
        return SPRITE_NONE;
    default:
        spriteId = CreateSprite(&gMultiuseSpriteTemplate, LEFT_X + LEFT_W / 2, PORTRAIT_MON_Y, 5);
        FreeSpriteOamMatrix(&gSprites[spriteId]);
        gSprites[spriteId].data[0] = tag;
        gSprites[spriteId].data[2] = 0;
        gSprites[spriteId].callback = SpriteCB_RFSumMon;
        gSprites[spriteId].oam.priority = 0;
        gSprites[spriteId].hFlip = !IsMonSpriteNotFlipped(tag);
        return spriteId;
    }
}

static void RFSum_CreateBallAndMarkings(void)
{
    enum PokeBall ball = GetMonData(&sRFSum->mon, MON_DATA_POKEBALL);

    if (ball >= POKEBALL_COUNT)
        ball = BALL_POKE;
    LoadBallGfx(ball);
    sRFSum->ballSpriteId = CreateSprite(&gPokeBalls[ball].spriteTemplate, LEFT_X + LEFT_W - PAD - 6, BODY_Y + 9, 0);
    gSprites[sRFSum->ballSpriteId].callback = SpriteCallbackDummy;
    gSprites[sRFSum->ballSpriteId].oam.priority = 0;

    sRFSum->markings = CreateMonMarkingAllCombosSprite(TAG_RFSUM_MARKINGS, TAG_RFSUM_MARKINGS, sMarkingsPal);
    if (sRFSum->markings != NULL)
    {
        StartSpriteAnim(sRFSum->markings, GetMonData(&sRFSum->mon, MON_DATA_MARKINGS));
        sRFSum->markings->x = LEFT_X + LEFT_W / 2;
        sRFSum->markings->y = PORTRAIT_NAME_Y - 5;
        sRFSum->markings->oam.priority = 0;
    }
}

static void RFSum_DestroyMonSprites(void)
{
    if (sAnimDelayTaskId != TASK_NONE)
    {
        DestroyTask(sAnimDelayTaskId);
        sAnimDelayTaskId = TASK_NONE;
    }
    DestroySpriteAndFreeResources(&gSprites[sRFSum->monSpriteId]);
    DestroySpriteAndFreeResources(&gSprites[sRFSum->ballSpriteId]);
    if (sRFSum->markings != NULL)
    {
        DestroySprite(sRFSum->markings);
        FreeSpriteTilesByTag(TAG_RFSUM_MARKINGS);
        FreeSpritePaletteByTag(TAG_RFSUM_MARKINGS);
        sRFSum->markings = NULL;
    }
}

static void RFSum_StopMonAnimation(void)
{
    struct Sprite *sprite = &gSprites[sRFSum->monSpriteId];
    u32 i, base = OBJ_PLTT_ID(sprite->oam.paletteNum);

    sprite->animPaused = TRUE;
    sprite->callback = SpriteCallbackDummy;
    StopPokemonAnimationDelayTask();
    for (i = 0; i < 16; i++)
        gPlttBufferUnfaded[base + i] = gPlttBufferFaded[base + i];
}

// ---------------------------------------------------------------------------------------------
// Setup and teardown

void ShowPokemonSummaryScreen(u8 mode, void *mons, u8 monIndex, u8 maxMonIndex, void (*callback)(void))
{
    sRFSum = AllocZeroed(sizeof(*sRFSum));
    sRFSum->mode = mode;
    if (monIndex == PC_MON_CHOSEN)
    {
        sRFSum->list.boxMons = GetBoxedMonPtr(gSpecialVar_MonBoxId, 0);
        sRFSum->curIndex = gSpecialVar_MonBoxPos;
        sRFSum->maxIndex = IN_BOX_COUNT - 1;
    }
    else
    {
        sRFSum->list.mons = mons;
        sRFSum->curIndex = monIndex;
        sRFSum->maxIndex = maxMonIndex;
    }
    sRFSum->callback = callback;
    if (gInitialSummaryScreenCallback == NULL)
        gInitialSummaryScreenCallback = callback;
    sRFSum->isBoxMon = (mode == SUMMARY_MODE_BOX || monIndex == PC_MON_CHOSEN);
    sRFSum->lockMoves = (mode == SUMMARY_MODE_LOCK_MOVES);
    sRFSum->lockMon = (mode == SUMMARY_MODE_SELECT_MOVE);
    sRFSum->minPage = PAGE_DATA;
    if (mode == SUMMARY_MODE_SELECT_MOVE || mode == SUMMARY_MODE_RELEARNER_BATTLE || mode == SUMMARY_MODE_RELEARNER_CONTEST)
        sRFSum->page = PAGE_MOVES;
    sAnimDelayTaskId = TASK_NONE;
    if (gMonSpritesGfxPtr == NULL)
        CreateMonSpritesGfxManager(MON_SPR_GFX_MANAGER_A, MON_SPR_GFX_MODE_NORMAL);
    if (mode != SUMMARY_MODE_SELECT_MOVE && mode != SUMMARY_MODE_RELEARNER_BATTLE && mode != SUMMARY_MODE_RELEARNER_CONTEST)
        gMoveRelearnerState = MOVE_RELEARNER_LEVEL_UP_MOVES;
    SetMainCallback2(CB2_InitRFSummary);
}

void ShowSelectMovePokemonSummaryScreen(struct Pokemon *mons, u8 monIndex, void (*callback)(void), u16 newMove)
{
    ShowPokemonSummaryScreen(SUMMARY_MODE_SELECT_MOVE, mons, monIndex, gPartiesCount[B_TRAINER_PLAYER] - 1, callback);
    sRFSum->newMove = newMove;
}

u8 GetMoveSlotToReplace(void)
{
    return sMoveSlotToReplace;
}

void SummaryScreen_SetAnimDelayTaskId(u8 taskId)
{
    sAnimDelayTaskId = taskId;
}

static void MainCB2_RFSum(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void VBlankCB_RFSum(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static bool32 RFSum_LoadStep(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankHBlankCallbacksToNull();
        ResetVramOamAndBgCntRegs();
        ClearScheduledBgCopiesToVram();
        ScanlineEffect_Stop();
        ResetPaletteFade();
        gPaletteFade.bufferTransferDisabled = TRUE;
        ResetSpriteData();
        FreeAllSpritePalettes();
        break;
    case 1:
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        SetBgTilemapBuffer(0, AllocZeroed(BG_SCREEN_SIZE));
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        InitWindows((const struct WindowTemplate[]){DUMMY_WIN_TEMPLATE});
        DeactivateAllTextPrinters();
        sRFSum->window = AddWindow(&sCanvasTemplate);
        PutWindowTilemap(sRFSum->window);
        RFSum_LoadPalettes();
        break;
    case 2:
        LoadCompressedSpriteSheet(&gSpriteSheet_MoveTypes);
        LoadPalette(gMoveTypes_Pal, OBJ_PLTT_ID(13), 3 * PLTT_SIZE_4BPP);
        LoadCompressedSpriteSheet(&gSpriteSheet_CategoryIcons);
        LoadSpritePalette(&gSpritePal_CategoryIcons);
        break;
    case 3:
    {
        u32 i;
        for (i = 0; i < ARRAY_COUNT(sRFSum->typeSpriteIds); i++)
        {
            sRFSum->typeSpriteIds[i] = CreateSprite(&gSpriteTemplate_MoveTypes, 0, 0, 2);
            StartSpriteAnim(&gSprites[sRFSum->typeSpriteIds[i]], TYPE_NORMAL);
            gSprites[sRFSum->typeSpriteIds[i]].oam.priority = 0;
            gSprites[sRFSum->typeSpriteIds[i]].invisible = TRUE;
        }
        // Category animation 0 (DAMAGE_CATEGORY_NONE) is a NULL entry: never leave it selected.
        sRFSum->categorySpriteId = CreateSprite(&gSpriteTemplate_CategoryIcons, 0, 0, 2);
        StartSpriteAnim(&gSprites[sRFSum->categorySpriteId], DAMAGE_CATEGORY_STATUS);
        gSprites[sRFSum->categorySpriteId].oam.priority = 0;
        gSprites[sRFSum->categorySpriteId].invisible = TRUE;
        RFSum_CopyMon();
        RFSum_UpdateRelearnable();
        sRFSum->loadState = 0;
        break;
    }
    case 4:
        sRFSum->monSpriteId = RFSum_LoadMonSprite(&sRFSum->loadState);
        if (sRFSum->monSpriteId == SPRITE_NONE)
            return FALSE;
        RFSum_CreateBallAndMarkings();
        break;
    case 5:
        RFSum_Draw();
        CreateTask(sRFSum->mode == SUMMARY_MODE_SELECT_MOVE ? Task_RFSumMoves : Task_RFSumInput, 0);
        ShowBg(0);
        break;
    case 6:
        BlendPalettes(PALETTES_ALL, 16, 0);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        gPaletteFade.bufferTransferDisabled = FALSE;
        break;
    default:
        SetVBlankCallback(VBlankCB_RFSum);
        SetMainCallback2(MainCB2_RFSum);
        return TRUE;
    }
    gMain.state++;
    return FALSE;
}

static void CB2_InitRFSummary(void)
{
    while (!MenuHelpers_ShouldWaitForLinkRecv() && !RFSum_LoadStep() && !MenuHelpers_IsLinkActive())
        ;
}

static void RFSum_BeginClose(u8 taskId)
{
    RFSum_StopMonAnimation();
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_RFSumClose;
}

static void Task_RFSumClose(u8 taskId)
{
    if (MenuHelpers_ShouldWaitForLinkRecv() || gPaletteFade.active)
        return;
    if (sRFSum->callback == gInitialSummaryScreenCallback)
        gInitialSummaryScreenCallback = NULL;
    SetMainCallback2(sRFSum->callback);
    gLastViewedMonIndex = sRFSum->curIndex;
    if (sAnimDelayTaskId != TASK_NONE)
    {
        DestroyTask(sAnimDelayTaskId);
        sAnimDelayTaskId = TASK_NONE;
    }
    ResetSpriteData();
    FreeAllSpritePalettes();
    StopCryAndClearCrySongs();
    m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 0x100);
    if (gMonSpritesGfxPtr == NULL)
        DestroyMonSpritesGfxManager(MON_SPR_GFX_MANAGER_A);
    Free(GetBgTilemapBuffer(0));
    FreeAllWindowBuffers();
    FREE_AND_SET_NULL(sRFSum);
    DestroyTask(taskId);
}

// ---------------------------------------------------------------------------------------------
// Input

static s8 RFSum_NextMon(s8 delta)
{
    s8 index = sRFSum->curIndex;

    if (sRFSum->isBoxMon)
    {
        u8 storageMode = sRFSum->page == PAGE_DATA ? (delta == 1 ? 1 : 3) : (delta == 1 ? 0 : 2);
        return AdvanceStorageMonIndex(sRFSum->list.boxMons, sRFSum->curIndex, sRFSum->maxIndex, storageMode);
    }
    if (IsMultiBattle() == TRUE && !AreMultiPartiesFullTeams())
    {
        s8 order = 0, i;
        for (i = 0; i < PARTY_SIZE; i++)
        {
            if (sMultiBattleOrder[i] == sRFSum->curIndex)
                order = i;
        }
        while (TRUE)
        {
            struct Pokemon *mon;
            order += delta;
            if (order < 0 || order >= PARTY_SIZE)
                return -1;
            mon = &sRFSum->list.mons[sMultiBattleOrder[order]];
            if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE
                && (sRFSum->curIndex != 0 || !GetMonData(mon, MON_DATA_IS_EGG)))
                return sMultiBattleOrder[order];
        }
    }
    do
    {
        index += delta;
        if (index < 0 || index > sRFSum->maxIndex)
            return -1;
    } while (sRFSum->page != PAGE_DATA && GetMonData(&sRFSum->list.mons[index], MON_DATA_IS_EGG));
    return index;
}

static void RFSum_ChangeMon(u8 taskId, s8 delta)
{
    s8 index;

    if (sRFSum->lockMon)
        return;
    index = RFSum_NextMon(delta);
    if (index < 0)
        return;
    PlaySE(SE_SELECT);
    sRFSum->curIndex = index;
    gTasks[taskId].data[0] = 0;
    gTasks[taskId].func = Task_RFSumChangeMon;
}

static void Task_RFSumChangeMon(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (data[0])
    {
    case 0:
        StopCryAndClearCrySongs();
        RFSum_DestroyMonSprites();
        RFSum_CopyMon();
        if (RFSum_IsEgg())
            sRFSum->page = PAGE_DATA;
        if (P_SUMMARY_SCREEN_MOVE_RELEARNER)
        {
            gMoveRelearnerState = MOVE_RELEARNER_LEVEL_UP_MOVES;
            RFSum_UpdateRelearnable();
        }
        data[1] = 0;
        break;
    case 1:
        sRFSum->monSpriteId = RFSum_LoadMonSprite(&data[1]);
        if (sRFSum->monSpriteId == SPRITE_NONE)
            return;
        RFSum_CreateBallAndMarkings();
        break;
    case 2:
        RFSum_Draw();
        break;
    default:
        if (!MenuHelpers_ShouldWaitForLinkRecv())
        {
            data[0] = 0;
            gTasks[taskId].func = Task_RFSumInput;
        }
        return;
    }
    data[0]++;
}

static bool32 RFSum_ChangePage(s8 delta)
{
    s32 page = sRFSum->page + delta;

    if (RFSum_IsEgg() || page < sRFSum->minPage || page >= PAGE_COUNT)
        return FALSE;
    PlaySE(SE_SELECT);
    sRFSum->page = page;
    sRFSum->hmWarning = FALSE;
    RFSum_Draw();
    return TRUE;
}

static void RFSum_SetReturnTarget(void)
{
    if (sRFSum->isBoxMon)
    {
        gSpecialVar_0x8004 = PC_MON_CHOSEN;
        gSpecialVar_MonBoxPos = sRFSum->curIndex;
        gSpecialVar_MonBoxId = StorageGetCurrentBox();
    }
    else
    {
        gSpecialVar_0x8004 = sRFSum->curIndex;
    }
}

static void CB2_RFSumReturnFromNaming(void)
{
    SetBoxMonData(GetSelectedBoxMonFromPcOrParty(), MON_DATA_NICKNAME, gStringVar2);
    ShowPokemonSummaryScreen(SUMMARY_MODE_NORMAL, gParties[B_TRAINER_PLAYER], gSpecialVar_0x8004,
                             gPartiesCount[B_TRAINER_PLAYER] - 1, gInitialSummaryScreenCallback);
}

static void CB2_RFSumRename(void)
{
    ChangePokemonNicknameWithCallback(CB2_RFSumReturnFromNaming);
}

static void Task_RFSumInput(u8 taskId)
{
    if (MenuHelpers_ShouldWaitForLinkRecv() || gPaletteFade.active)
        return;

    if (JOY_NEW(DPAD_UP))
        RFSum_ChangeMon(taskId, -1);
    else if (JOY_NEW(DPAD_DOWN))
        RFSum_ChangeMon(taskId, 1);
    else if (JOY_NEW(DPAD_LEFT) || GetLRKeysPressed() == MENU_L_PRESSED)
        RFSum_ChangePage(-1);
    else if (JOY_NEW(DPAD_RIGHT) || GetLRKeysPressed() == MENU_R_PRESSED)
        RFSum_ChangePage(1);
    else if (JOY_NEW(A_BUTTON))
    {
        if (sRFSum->page == PAGE_DATA && RFSum_CanRename())
        {
            PlaySE(SE_SELECT);
            RFSum_SetReturnTarget();
            sRFSum->callback = CB2_RFSumRename;
            RFSum_BeginClose(taskId);
        }
        else if (sRFSum->page == PAGE_MOVES)
        {
            PlaySE(SE_SELECT);
            sRFSum->cursor = 0;
            sRFSum->movesState = MOVES_BROWSE;
            RFSum_Draw();
            gTasks[taskId].func = Task_RFSumMoves;
        }
    }
    else if (JOY_NEW(START_BUTTON) && sRFSum->page == PAGE_MOVES && RFSum_CanRelearn())
    {
        PlaySE(SE_SELECT);
        RFSum_SetReturnTarget();
        gRelearnMode = RELEARN_MODE_PSS_PAGE_BATTLE_MOVES;
        sRFSum->callback = CB2_InitLearnMove;
        RFSum_BeginClose(taskId);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        RFSum_BeginClose(taskId);
    }
}

static void RFSum_MoveCursor(u8 *cursor, s8 delta)
{
    u32 rows = RFSum_RowCount(), i;
    s32 row = *cursor;

    for (i = 0; i < rows; i++)
    {
        row = (row + delta + rows) % rows;
        if (row == MAX_MON_MOVES || RFSum_RowMove(row) != MOVE_NONE)
            break;
    }
    PlaySE(SE_SELECT);
    *cursor = row;
    sRFSum->hmWarning = FALSE;
    RFSum_Draw();
}

static void RFSum_SwapMoves(u32 a, u32 b)
{
    struct Pokemon *mon = &sRFSum->mon;
    enum Move moveA = GetMonData(mon, MON_DATA_MOVE1 + a), moveB = GetMonData(mon, MON_DATA_MOVE1 + b);
    u8 ppA = GetMonData(mon, MON_DATA_PP1 + a), ppB = GetMonData(mon, MON_DATA_PP1 + b);
    u8 bonuses = GetMonData(mon, MON_DATA_PP_BONUSES);
    u8 bonusA = (bonuses & gPPUpGetMask[a]) >> (a * 2), bonusB = (bonuses & gPPUpGetMask[b]) >> (b * 2);

    bonuses &= ~(gPPUpGetMask[a] | gPPUpGetMask[b]);
    bonuses |= (bonusA << (b * 2)) | (bonusB << (a * 2));
    if (!sRFSum->isBoxMon)
    {
        struct Pokemon *target = &sRFSum->list.mons[sRFSum->curIndex];
        SetMonData(target, MON_DATA_MOVE1 + a, &moveB);
        SetMonData(target, MON_DATA_MOVE1 + b, &moveA);
        SetMonData(target, MON_DATA_PP1 + a, &ppB);
        SetMonData(target, MON_DATA_PP1 + b, &ppA);
        SetMonData(target, MON_DATA_PP_BONUSES, &bonuses);
    }
    else
    {
        struct BoxPokemon *target = &sRFSum->list.boxMons[sRFSum->curIndex];
        SetBoxMonData(target, MON_DATA_MOVE1 + a, &moveB);
        SetBoxMonData(target, MON_DATA_MOVE1 + b, &moveA);
        SetBoxMonData(target, MON_DATA_PP1 + a, &ppB);
        SetBoxMonData(target, MON_DATA_PP1 + b, &ppA);
        SetBoxMonData(target, MON_DATA_PP_BONUSES, &bonuses);
    }
    RFSum_CopyMon();
}

static bool32 RFSum_HasMoreThanOneMove(void)
{
    u32 i;
    for (i = 1; i < MAX_MON_MOVES; i++)
    {
        if (RFSum_RowMove(i) != MOVE_NONE)
            return TRUE;
    }
    return FALSE;
}

static void RFSum_FinishSelectMove(u8 taskId, u8 slot)
{
    sMoveSlotToReplace = slot;
    gSpecialVar_0x8005 = slot;
    gSpecialVar_Result = (slot != MAX_MON_MOVES);
    RFSum_BeginClose(taskId);
}

static void Task_RFSumMoves(u8 taskId)
{
    bool32 selectMode = (sRFSum->mode == SUMMARY_MODE_SELECT_MOVE);

    if (MenuHelpers_ShouldWaitForLinkRecv() || gPaletteFade.active)
        return;

    // Off the moves page (select mode can still read the other pages), only paging and B work.
    if (sRFSum->page != PAGE_MOVES)
    {
        if (JOY_NEW(DPAD_LEFT) || GetLRKeysPressed() == MENU_L_PRESSED)
            RFSum_ChangePage(-1);
        else if (JOY_NEW(DPAD_RIGHT) || GetLRKeysPressed() == MENU_R_PRESSED)
            RFSum_ChangePage(1);
        else if (JOY_NEW(B_BUTTON) && selectMode)
        {
            PlaySE(SE_SELECT);
            RFSum_FinishSelectMove(taskId, MAX_MON_MOVES);
        }
        return;
    }

    if (JOY_NEW(DPAD_UP))
        RFSum_MoveCursor(sRFSum->movesState == MOVES_SWAP ? &sRFSum->swapCursor : &sRFSum->cursor, -1);
    else if (JOY_NEW(DPAD_DOWN))
        RFSum_MoveCursor(sRFSum->movesState == MOVES_SWAP ? &sRFSum->swapCursor : &sRFSum->cursor, 1);
    else if (selectMode && (JOY_NEW(DPAD_LEFT) || GetLRKeysPressed() == MENU_L_PRESSED))
        RFSum_ChangePage(-1);
    else if (selectMode && (JOY_NEW(DPAD_RIGHT) || GetLRKeysPressed() == MENU_R_PRESSED))
        RFSum_ChangePage(1);
    else if (JOY_NEW(A_BUTTON))
    {
        if (selectMode)
        {
            if (sRFSum->cursor == MAX_MON_MOVES || sRFSum->newMove == MOVE_NONE
             || !CannotForgetMove(RFSum_RowMove(sRFSum->cursor)))
            {
                PlaySE(SE_SELECT);
                RFSum_FinishSelectMove(taskId, sRFSum->cursor);
            }
            else
            {
                PlaySE(SE_FAILURE);
                sRFSum->hmWarning = TRUE;
                RFSum_Draw();
            }
        }
        else if (sRFSum->movesState == MOVES_SWAP)
        {
            PlaySE(SE_SELECT);
            if (sRFSum->swapCursor != sRFSum->cursor)
            {
                RFSum_SwapMoves(sRFSum->cursor, sRFSum->swapCursor);
                sRFSum->cursor = sRFSum->swapCursor;
            }
            sRFSum->movesState = MOVES_BROWSE;
            RFSum_Draw();
        }
        else if (sRFSum->lockMoves)
        {
            PlaySE(SE_SELECT);
            sRFSum->movesState = MOVES_IDLE;
            RFSum_Draw();
            gTasks[taskId].func = Task_RFSumInput;
        }
        else if (RFSum_HasMoreThanOneMove())
        {
            PlaySE(SE_SELECT);
            sRFSum->swapCursor = sRFSum->cursor;
            sRFSum->movesState = MOVES_SWAP;
            RFSum_Draw();
        }
        else
        {
            PlaySE(SE_FAILURE);
        }
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (selectMode)
        {
            RFSum_FinishSelectMove(taskId, MAX_MON_MOVES);
        }
        else if (sRFSum->movesState == MOVES_SWAP)
        {
            sRFSum->movesState = MOVES_BROWSE;
            RFSum_Draw();
        }
        else
        {
            sRFSum->movesState = MOVES_IDLE;
            RFSum_Draw();
            gTasks[taskId].func = Task_RFSumInput;
        }
    }
}

#endif // RF_SUMMARY_SCREEN
