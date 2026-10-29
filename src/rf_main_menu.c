#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "region_map.h"
#include "rf_main_menu.h"
#include "rf_quests.h"
#include "rf_ui_theme.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/flags.h"

// Pokémon Ultimate Kanto start menu: panels drawn inside their windows (ember edge on the selected
// one), menu icons, a Continue card with the saved party and progress, and a help line. The menu
// logic, save checks and Mystery Gift handling stay in main_menu.c.

// Text palette 15 roles (main_menu.c loads 10 panel, 11 text, 12 shadow, 1 accent).
#define C_PANEL 10
#define C_TEXT 11
#define C_SHADOW 12
#define C_PINK 7
#define C_BLUE 8
#define C_MUTED 9
#define C_EDGE 13
#define C_EMBER 15

#define WIN_CONTINUE 2
#define WIN_HELP 8
#define CARD_W 208
#define CARD_H 64

static const u8 sColorsText[] = {C_PANEL, C_TEXT, C_SHADOW};
static const u8 sColorsMuted[] = {C_PANEL, C_MUTED, C_SHADOW};
static const u8 sColorsEmber[] = {C_PANEL, C_EMBER, C_SHADOW};
static const u8 sColorsHelp[] = {TEXT_COLOR_TRANSPARENT, C_MUTED, C_SHADOW};

static const u8 sText_Continue[] = _("CONTINUAR");
static const u8 sText_Pokedex[] = _("POKéDEX");
static const u8 sText_Badges[] = _("MEDALLAS");
static const u8 sText_Quests[] = _("MISIONES");
static const u8 sText_Help[] = _("{A_BUTTON} Elegir    {B_BUTTON} Volver al título");
static const u8 sText_Separator[] = _(" · ");
static const u8 sText_Male[] = _("♂");
static const u8 sText_Female[] = _("♀");

// Windows visible for each menu type (see sWindowTemplates_MainMenu), with the icon of each row.
static const u8 sMenuWindows[][6] = {
    [0] = {0, 1, 0xFF},             // no saved game: NEW GAME, OPTION
    [1] = {2, 3, 4, 0xFF},          // saved game: CONTINUE, NEW GAME, OPTION
    [2] = {2, 3, 4, 5, 0xFF},       // + MYSTERY GIFT
    [3] = {2, 3, 4, 5, 6, 0xFF},    // + MYSTERY EVENTS
};
static const u8 sMenuIcons[][6] = {
    [0] = {RF_ICON_PROFILE, RF_ICON_OPTIONS},
    [1] = {RF_ICON_SAVE, RF_ICON_PROFILE, RF_ICON_OPTIONS},
    [2] = {RF_ICON_SAVE, RF_ICON_PROFILE, RF_ICON_KEY, RF_ICON_OPTIONS},
    [3] = {RF_ICON_SAVE, RF_ICON_PROFILE, RF_ICON_KEY, RF_ICON_QUESTS, RF_ICON_OPTIONS},
};

static void LoadColors(void)
{
    static const u16 colors[][2] = {
        {C_PINK, RGB(31, 17, 22)},
        {C_BLUE, RGB(16, 23, 31)},
        {C_MUTED, RGB(22, 20, 20)},
        {C_EDGE, RGB(9, 7, 9)},
        {C_EMBER, RGB(31, 22, 9)},
    };
    u32 i;
    for (i = 0; i < ARRAY_COUNT(colors); i++)
        LoadPalette(&colors[i][1], BG_PLTT_ID(15) + colors[i][0], PLTT_SIZEOF(1));
}

static u32 MenuIndex(u32 menuType)
{
    return menuType < ARRAY_COUNT(sMenuWindows) ? menuType : 0;
}

static void DrawEdge(u8 windowId, bool32 selected)
{
    u32 w = GetWindowAttribute(windowId, WINDOW_WIDTH) * 8;
    u32 h = GetWindowAttribute(windowId, WINDOW_HEIGHT) * 8;
    RFUI_DrawRing(windowId, 0, 0, w, h, selected ? C_EMBER : C_EDGE);
    RFUI_FillRect(windowId, selected ? C_SHADOW : C_PANEL, 1, h - 2, w - 2, 1);
}

static void PrintRight(u8 windowId, u8 font, const u8 *str, u32 right, u32 y, const u8 *colors)
{
    AddTextPrinterParameterized3(windowId, font, right - GetStringWidth(font, str, 0), y, colors, TEXT_SKIP_DRAW, str);
}

static u32 CountBadges(void)
{
    u32 i, count = 0;
    for (i = FLAG_BADGE01_GET; i < FLAG_BADGE01_GET + NUM_BADGES; i++)
        count += FlagGet(i) == TRUE;
    return count;
}

u32 RFMainMenu_CountClaimedQuests(void)
{
    u32 i, count = 0;
    for (i = 0; i < RF_QUEST_COUNT; i++)
        count += RFQuests_GetState(i) == RF_QUEST_CLAIMED;
    return count;
}

static void DrawStat(const u8 *label, u32 value, u32 y)
{
    u8 str[8];
    AddTextPrinterParameterized3(WIN_CONTINUE, FONT_SMALL, 126, y, sColorsMuted, TEXT_SKIP_DRAW, label);
    ConvertIntToDecimalStringN(str, value, STR_CONV_MODE_LEFT_ALIGN, 3);
    PrintRight(WIN_CONTINUE, FONT_SMALL, str, 202, y, sColorsText);
}

static void DrawContinueCard(void)
{
    u8 str[48];
    u8 *end;
    u32 i, badges = CountBadges(), x, y;
    const struct MapHeader *map = Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->location.mapGroup,
                                                                      gSaveBlock1Ptr->location.mapNum);

    FillWindowPixelBuffer(WIN_CONTINUE, PIXEL_FILL(C_PANEL));
    RFUI_DrawIcon(WIN_CONTINUE, RF_ICON_SAVE, 5, 2, C_EMBER, C_SHADOW);
    AddTextPrinterParameterized3(WIN_CONTINUE, FONT_NORMAL, 25, 1, sColorsText, TEXT_SKIP_DRAW, sText_Continue);

    // Where the game was saved and the play time, top right.
    end = GetMapName(str, map->regionMapSectionId, 0);
    end = StringCopy(end, sText_Separator);
    end = ConvertIntToDecimalStringN(end, gSaveBlock2Ptr->playTimeHours, STR_CONV_MODE_LEFT_ALIGN, 3);
    *end++ = CHAR_COLON;
    ConvertIntToDecimalStringN(end, gSaveBlock2Ptr->playTimeMinutes, STR_CONV_MODE_LEADING_ZEROS, 2);
    PrintRight(WIN_CONTINUE, FONT_SMALL, str, 202, 3, sColorsEmber);
    RFUI_FillRect(WIN_CONTINUE, C_EDGE, 6, 20, 196, 1);

    // Player and gender; the party icons are sprites below the name.
    AddTextPrinterParameterized3(WIN_CONTINUE, FONT_NORMAL, 6, 21, sColorsText, TEXT_SKIP_DRAW, gSaveBlock2Ptr->playerName);
    x = 6 + GetStringWidth(FONT_NORMAL, gSaveBlock2Ptr->playerName, 0) + 3;
    {
        const u8 colors[] = {C_PANEL, gSaveBlock2Ptr->playerGender == MALE ? C_BLUE : C_PINK, C_SHADOW};
        AddTextPrinterParameterized3(WIN_CONTINUE, FONT_NORMAL, x, 21,
                                     colors, TEXT_SKIP_DRAW, gSaveBlock2Ptr->playerGender == MALE ? sText_Male : sText_Female);
    }

    // Progress column.
    RFUI_FillRect(WIN_CONTINUE, C_EDGE, 120, 23, 1, 37);
    // Rows close up while the player has no Pokédex yet.
    y = 20;
    if (FlagGet(FLAG_SYS_POKEDEX_GET) == TRUE)
    {
        DrawStat(sText_Pokedex, IsNationalPokedexEnabled() ? GetNationalPokedexCount(FLAG_GET_CAUGHT)
                                                           : GetRegionalPokedexCount(FLAG_GET_CAUGHT), y);
        y += 11;
    }
    DrawStat(sText_Badges, badges, y);
    DrawStat(sText_Quests, RFMainMenu_CountClaimedQuests(), y + 11);
    for (i = 0; i < NUM_BADGES; i++)
        RFUI_FillRect(WIN_CONTINUE, i < badges ? C_EMBER : C_EDGE, 126 + i * 10, 57, 7, 3);
}

static void CreatePartyIcons(void)
{
    u32 i, count = min(gSaveBlock1Ptr->playerPartyCount, PARTY_SIZE);
    s16 left = GetWindowAttribute(WIN_CONTINUE, WINDOW_TILEMAP_LEFT) * 8;
    s16 top = GetWindowAttribute(WIN_CONTINUE, WINDOW_TILEMAP_TOP) * 8;
    LoadMonIconPalettes();
    for (i = 0; i < count; i++)
    {
        struct Pokemon *mon = &gSaveBlock1Ptr->playerParty[i];
        u32 species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);
        u8 spriteId;
        if (species == SPECIES_NONE)
            continue;
        spriteId = CreateMonIcon(species, SpriteCB_MonIcon, left + 14 + i * 19, top + 46, 0, GetMonData(mon, MON_DATA_PERSONALITY));
        if (spriteId < MAX_SPRITES)
            gSprites[spriteId].oam.priority = 0;
    }
}

static void DrawHelp(void)
{
    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    AddTextPrinterParameterized3(WIN_HELP, FONT_SMALL, 0, 2, sColorsHelp, TEXT_SKIP_DRAW, sText_Help);
    PutWindowTilemap(WIN_HELP);
    CopyWindowToVram(WIN_HELP, COPYWIN_FULL);
}

void RFMainMenu_Decorate(u32 menuType)
{
    u32 index = MenuIndex(menuType), i;
    const u8 *windows = sMenuWindows[index];

    LoadColors();
    if (index != 0)
    {
        DrawContinueCard();
        CreatePartyIcons();
    }
    for (i = 0; windows[i] != 0xFF; i++)
    {
        if (windows[i] != WIN_CONTINUE)
            RFUI_DrawIcon(windows[i], sMenuIcons[index][i], 5, 0, C_MUTED, C_SHADOW);
        DrawEdge(windows[i], i == 0);
        PutWindowTilemap(windows[i]);
        CopyWindowToVram(windows[i], COPYWIN_FULL);
    }
    if (index <= 1) // with Mystery Gift rows the help line would overlap the menu
        DrawHelp();
}

void RFMainMenu_Select(u32 menuType, u32 selected)
{
    u32 index = MenuIndex(menuType), i;
    const u8 *windows = sMenuWindows[index];
    for (i = 0; windows[i] != 0xFF; i++)
    {
        DrawEdge(windows[i], i == selected);
        RFUI_DrawIcon(windows[i], sMenuIcons[index][i], 5, windows[i] == WIN_CONTINUE ? 2 : 0,
                      i == selected ? C_EMBER : C_MUTED, C_SHADOW);
        CopyWindowToVram(windows[i], COPYWIN_GFX);
    }
}

// The mountain range behind the menu keeps drifting, as on the title screen.
static void Task_RFMainMenuBackdrop(u8 taskId)
{
    // Register-only scroll; see the title screen for why ChangeBgX is avoided here.
    SetGpuReg(REG_OFFSET_BG2HOFS, ++gTasks[taskId].data[0] / 6);
}

void RFMainMenu_StartBackdrop(void)
{
    if (!FuncIsActiveTask(Task_RFMainMenuBackdrop))
        CreateTask(Task_RFMainMenuBackdrop, 80);
}
