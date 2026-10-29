#include "global.h"
#include "rf_naming_screen.h"
#include "battle_main.h"
#include "bg.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "naming_screen.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "rf_ui_theme.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// Layout (tiles): header 0-3, page tabs 4-5, keys 6-15 on the left, BORRAR/ESPACIO/LISTO on the right,
// help bar 18-19. Everything is drawn into BG0 windows over a dark backdrop.
enum { WIN_HEADER, WIN_TABS, WIN_KEYS, WIN_BUTTONS, WIN_HELP };
enum { PAGE_UPPER, PAGE_LOWER, PAGE_SYMBOLS, PAGE_COUNT };
enum { BUTTON_DELETE, BUTTON_SPACE, BUTTON_DONE, BUTTON_COUNT };

#define KEY_COLS 9
#define KEY_ROWS 4
#define BUTTON_COL KEY_COLS
#define KEY_W 16
#define KEY_H 20
#define MAX_TEXT 16

// Palette 15: 1 panel, 2 text, 3 shadow, 4 key face, 5 edge, 6 ember, 7 ink on ember, 8 muted.
enum { C_CLEAR, C_PANEL, C_TEXT, C_SHADOW, C_KEY, C_EDGE, C_EMBER, C_INK, C_MUTED };
static const u16 sPalette[16] = {
    RF_UI_NIGHT_GROUND, RF_UI_NIGHT_PANEL, RF_UI_NIGHT_TEXT, RF_UI_NIGHT_SHADOW, RF_UI_NIGHT_PANEL_HI,
    RF_UI_NIGHT_EDGE, RF_UI_BATTLE_SELECTED, RF_UI_NIGHT_GROUND, RF_UI_NIGHT_MUTED,
};
static const u8 sTextColors[] = {TEXT_COLOR_TRANSPARENT, C_TEXT, C_SHADOW};
static const u8 sMutedColors[] = {TEXT_COLOR_TRANSPARENT, C_MUTED, C_SHADOW};
static const u8 sEmberColors[] = {TEXT_COLOR_TRANSPARENT, C_EMBER, C_SHADOW};
static const u8 sInkColors[] = {TEXT_COLOR_TRANSPARENT, C_INK, C_EMBER};

static const struct BgTemplate sBgTemplates[] = {
    {.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 1, .baseTile = 0},
};
static const struct WindowTemplate sWindowTemplates[] = {
    [WIN_HEADER]  = {.bg = 0, .tilemapLeft = 0,  .tilemapTop = 0,  .width = 30, .height = 4,  .paletteNum = 15, .baseBlock = 1},
    [WIN_TABS]    = {.bg = 0, .tilemapLeft = 1,  .tilemapTop = 4,  .width = 19, .height = 2,  .paletteNum = 15, .baseBlock = 121},
    [WIN_KEYS]    = {.bg = 0, .tilemapLeft = 1,  .tilemapTop = 6,  .width = 19, .height = 10, .paletteNum = 15, .baseBlock = 159},
    [WIN_BUTTONS] = {.bg = 0, .tilemapLeft = 21, .tilemapTop = 4,  .width = 8,  .height = 12, .paletteNum = 15, .baseBlock = 349},
    [WIN_HELP]    = {.bg = 0, .tilemapLeft = 0,  .tilemapTop = 18, .width = 30, .height = 2,  .paletteNum = 15, .baseBlock = 445},
    DUMMY_WIN_TEMPLATE,
};

static const u8 sKeys[PAGE_COUNT][KEY_ROWS][KEY_COLS + 1] = {
    [PAGE_UPPER] = {
        _("ABCDEFGHI"), _("JKLMNÑOPQ"), _("RSTUVWXYZ"), _(".,-!?¡¿♂♀"),
    },
    [PAGE_LOWER] = {
        _("abcdefghi"), _("jklmnñopq"), _("rstuvwxyz"), _(".,-!?¡¿♂♀"),
    },
    [PAGE_SYMBOLS] = {
        _("012345678"), _("9ÁÉÍÓÚÜáé"), _("íóúü/:;()"), _("&%+=“”‘’…"),
    },
};
static const u8 *const sPageLabels[PAGE_COUNT] = {COMPOUND_STRING("ABC"), COMPOUND_STRING("abc"), COMPOUND_STRING("ÁÉ¿")};
static const u8 *const sButtonLabels[BUTTON_COUNT] = {COMPOUND_STRING("BORRAR"), COMPOUND_STRING("ESPACIO"), COMPOUND_STRING("LISTO")};
static const u8 sButtonY[BUTTON_COUNT] = {16, 38, 60};
static const u8 sButtonH[BUTTON_COUNT] = {20, 20, 32};
static const u8 sText_Help[] = _("{A_BUTTON}Escribir  {B_BUTTON}Borrar  {SELECT_BUTTON}Página  {START_BUTTON}Listo");
static const u8 sText_TitlePlayer[] = _("¿Cómo te llamas?");
static const u8 sText_TitleRival[] = _("¿Cómo se llama tu rival?");
static const u8 sText_TitleBox[] = _("¿Nombre de la caja?");
static const u8 sText_TitleMon[] = _("¿Mote para {STR_VAR_1}?");
static const u8 sText_Counter[] = _("{STR_VAR_1}/{STR_VAR_2}");

struct RFNaming
{
    MainCallback returnCallback;
    u8 *dest;
    u8 text[MAX_TEXT + 1];
    u32 monPersonality;
    u16 monSpeciesOrGender;
    u16 monGender;
    u8 templateNum;
    u8 maxChars;
    u8 length;
    u8 page;
    u8 cursorX;
    u8 cursorY;
    u8 state;
};

static EWRAM_DATA struct RFNaming *sNaming = NULL;

static void CB2_RFNamingInit(void);
static void CB2_RFNaming(void);
static void VBlankCB_RFNaming(void);
static void Task_RFNaming(u8 taskId);
static void DrawAll(void);

bool32 RFNaming_Start(u8 templateNum, u8 *destBuffer, u16 monSpeciesOrPlayerGender, u16 monGender,
                      u32 monPersonality, MainCallback returnCallback)
{
    sNaming = AllocZeroed(sizeof(*sNaming));
    if (sNaming == NULL)
        return FALSE;
    sNaming->templateNum = templateNum;
    sNaming->dest = destBuffer;
    sNaming->monSpeciesOrGender = monSpeciesOrPlayerGender;
    sNaming->monGender = monGender;
    sNaming->monPersonality = monPersonality;
    sNaming->returnCallback = returnCallback;
    switch (templateNum)
    {
    case NAMING_SCREEN_BOX:        sNaming->maxChars = BOX_NAME_LENGTH; break;
    case NAMING_SCREEN_CAUGHT_MON:
    case NAMING_SCREEN_NICKNAME:   sNaming->maxChars = POKEMON_NAME_LENGTH; break;
    default:                       sNaming->maxChars = PLAYER_NAME_LENGTH; break;
    }
    if (sNaming->maxChars > MAX_TEXT)
        sNaming->maxChars = MAX_TEXT;
    sNaming->text[0] = EOS;
    if (templateNum == NAMING_SCREEN_PLAYER)
        StartTimer1();
    gMain.state = 0;
    SetMainCallback2(CB2_RFNamingInit);
    return TRUE;
}

static void CB2_RFNamingInit(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetHBlankCallback(NULL);
        ScanlineEffect_Stop();
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        DmaFillLarge16(3, 0, (void *)VRAM, VRAM_SIZE, 0x1000);
        DmaClear32(3, (void *)OAM, OAM_SIZE);
        DmaClear16(3, (void *)PLTT, PLTT_SIZE);
        ResetPaletteFade();
        ResetSpriteData();
        FreeAllSpritePalettes();
        ResetTasks();
        gMain.state++;
        break;
    case 1:
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        InitWindows(sWindowTemplates);
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BG0HOFS, 0);
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        LoadPalette(sPalette, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
        LoadPalette(sPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 2:
    {
        u8 spriteId = NamingScreen_CreatePortrait(sNaming->templateNum, sNaming->monSpeciesOrGender,
                                                  sNaming->monPersonality, 20, 17);
        if (spriteId != MAX_SPRITES)
            gSprites[spriteId].oam.priority = 0;
        DrawAll();
        gMain.state++;
        break;
    }
    case 3:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        SetVBlankCallback(VBlankCB_RFNaming);
        CreateTask(Task_RFNaming, 0);
        SetMainCallback2(CB2_RFNaming);
        break;
    }
}

static void CB2_RFNaming(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB_RFNaming(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void Print(u8 window, u8 font, const u8 *text, u32 x, u32 y, const u8 *colors)
{
    AddTextPrinterParameterized4(window, font, x, y, 0, 0, colors, TEXT_SKIP_DRAW, text);
}

static void DrawHeader(void)
{
    u32 i, x = 40;
    const u8 *title;

    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(C_PANEL));
    RFUI_FillRect(WIN_HEADER, C_EDGE, 0, 31, 240, 1);
    RFUI_FillRect(WIN_HEADER, C_KEY, 4, 2, 30, 28); // portrait plate behind the sprite
    switch (sNaming->templateNum)
    {
    case NAMING_SCREEN_RIVAL: title = sText_TitleRival; break;
    case NAMING_SCREEN_BOX:   title = sText_TitleBox; break;
    case NAMING_SCREEN_CAUGHT_MON:
    case NAMING_SCREEN_NICKNAME:
        StringCopy(gStringVar1, GetSpeciesName(sNaming->monSpeciesOrGender));
        StringExpandPlaceholders(gStringVar4, sText_TitleMon);
        title = gStringVar4;
        break;
    default: title = sText_TitlePlayer; break;
    }
    Print(WIN_HEADER, FONT_SMALL, title, x, 2, sMutedColors);
    // One slot per letter: the typed text over a dim underline, the next slot marked in ember.
    for (i = 0; i < sNaming->maxChars; i++)
        RFUI_FillRect(WIN_HEADER, i == sNaming->length ? C_EMBER : C_EDGE, x + i * 9, 27, 7, i == sNaming->length ? 2 : 1);
    for (i = 0; i < sNaming->length; i++)
    {
        u8 ch[2] = {sNaming->text[i], EOS};
        Print(WIN_HEADER, FONT_NORMAL, ch, x + i * 9 + (7 - GetStringWidth(FONT_NORMAL, ch, 0)) / 2, 12, sTextColors);
    }
    ConvertIntToDecimalStringN(gStringVar1, sNaming->length, STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar2, sNaming->maxChars, STR_CONV_MODE_LEFT_ALIGN, 2);
    StringExpandPlaceholders(gStringVar4, sText_Counter);
    Print(WIN_HEADER, FONT_NORMAL, gStringVar4, 234 - GetStringWidth(FONT_NORMAL, gStringVar4, 0), 12,
          sNaming->length == sNaming->maxChars ? sEmberColors : sMutedColors);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void DrawTabs(void)
{
    u32 i;

    FillWindowPixelBuffer(WIN_TABS, PIXEL_FILL(C_CLEAR));
    for (i = 0; i < PAGE_COUNT; i++)
    {
        u32 x = 4 + i * 50, w = 46;
        bool32 active = (i == sNaming->page);

        RFUI_FillRect(WIN_TABS, active ? C_KEY : C_PANEL, x, 1, w, 14);
        if (active)
            RFUI_FillRect(WIN_TABS, C_EMBER, x, 14, w, 2);
        Print(WIN_TABS, FONT_NORMAL, sPageLabels[i], x + (w - GetStringWidth(FONT_NORMAL, sPageLabels[i], 0)) / 2, 1,
              active ? sEmberColors : sMutedColors);
    }
    CopyWindowToVram(WIN_TABS, COPYWIN_FULL);
}

static void DrawKeys(void)
{
    u32 row, col;

    FillWindowPixelBuffer(WIN_KEYS, PIXEL_FILL(C_PANEL));
    for (row = 0; row < KEY_ROWS; row++)
    {
        for (col = 0; col < KEY_COLS; col++)
        {
            u32 x = 4 + col * KEY_W, y = row * KEY_H;
            bool32 selected = (sNaming->cursorX == col && sNaming->cursorY == row);
            u8 ch[2] = {sKeys[sNaming->page][row][col], EOS};

            RFUI_FillRect(WIN_KEYS, selected ? C_EMBER : C_KEY, x + 1, y + 2, KEY_W - 2, KEY_H - 4);
            Print(WIN_KEYS, FONT_NORMAL, ch, x + (KEY_W - GetStringWidth(FONT_NORMAL, ch, 0)) / 2, y + 3,
                  selected ? sInkColors : sTextColors);
        }
    }
    CopyWindowToVram(WIN_KEYS, COPYWIN_FULL);
}

static u32 SelectedButton(void)
{
    if (sNaming->cursorX != BUTTON_COL)
        return BUTTON_COUNT;
    return sNaming->cursorY < BUTTON_DONE ? sNaming->cursorY : BUTTON_DONE;
}

static void DrawButtons(void)
{
    u32 i;

    FillWindowPixelBuffer(WIN_BUTTONS, PIXEL_FILL(C_CLEAR));
    for (i = 0; i < BUTTON_COUNT; i++)
    {
        bool32 selected = (SelectedButton() == i);
        bool32 done = (i == BUTTON_DONE);
        u32 y = sButtonY[i], h = sButtonH[i];

        if (selected)
            RFUI_FillRect(WIN_BUTTONS, done ? C_TEXT : C_EMBER, 0, y - 1, 64, h + 2);
        RFUI_FillRect(WIN_BUTTONS, done ? C_EMBER : C_KEY, 1, y, 62, h);
        Print(WIN_BUTTONS, FONT_NORMAL, sButtonLabels[i], (64 - GetStringWidth(FONT_NORMAL, sButtonLabels[i], 0)) / 2,
              y + (h - 14) / 2, done ? sInkColors : (selected ? sEmberColors : sTextColors));
    }
    CopyWindowToVram(WIN_BUTTONS, COPYWIN_FULL);
}

static void DrawHelp(void)
{
    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(C_PANEL));
    RFUI_FillRect(WIN_HELP, C_EDGE, 0, 0, 240, 1);
    Print(WIN_HELP, FONT_SMALL, sText_Help, 6, 3, sMutedColors);
    CopyWindowToVram(WIN_HELP, COPYWIN_FULL);
}

static void DrawAll(void)
{
    u32 i;

    for (i = 0; i <= WIN_HELP; i++)
        PutWindowTilemap(i);
    DrawHeader();
    DrawTabs();
    DrawKeys();
    DrawButtons();
    DrawHelp();
    CopyBgTilemapBufferToVram(0);
}

static void AddChar(u8 ch)
{
    if (sNaming->length >= sNaming->maxChars)
        return;
    sNaming->text[sNaming->length++] = ch;
    sNaming->text[sNaming->length] = EOS;
    // After the first capital letter, carry on in lower case (as the original keyboard does).
    if (sNaming->length == 1 && sNaming->page == PAGE_UPPER && ch != CHAR_SPACE)
        sNaming->page = PAGE_LOWER;
    if (sNaming->length == sNaming->maxChars)
    {
        sNaming->cursorX = BUTTON_COL;
        sNaming->cursorY = BUTTON_DONE;
    }
}

static void DeleteChar(void)
{
    if (sNaming->length == 0)
        return;
    sNaming->text[--sNaming->length] = EOS;
}

static void SaveText(void)
{
    u32 i;

    for (i = 0; i < sNaming->length; i++)
    {
        if (sNaming->text[i] != CHAR_SPACE)
        {
            StringCopyN(sNaming->dest, sNaming->text, sNaming->maxChars + 1);
            break;
        }
    }
}

static void Exit(u8 taskId)
{
    MainCallback next = sNaming->returnCallback;

    if (sNaming->templateNum == NAMING_SCREEN_PLAYER)
        SeedRngAndSetTrainerId();
    if (sNaming->templateNum == NAMING_SCREEN_CAUGHT_MON && CalculatePlayerPartyCount() < PARTY_SIZE)
        next = BattleMainCB2;
    DestroyTask(taskId);
    FreeAllWindowBuffers();
    FREE_AND_SET_NULL(sNaming);
    SetMainCallback2(next);
}

static void Task_RFNaming(u8 taskId)
{
    bool32 redraw = FALSE;

    if (gPaletteFade.active)
        return;
    if (sNaming->state == 1)
    {
        Exit(taskId);
        return;
    }
    if (JOY_REPEAT(DPAD_LEFT))
    {
        sNaming->cursorX = sNaming->cursorX == 0 ? BUTTON_COL : sNaming->cursorX - 1;
        redraw = TRUE;
    }
    else if (JOY_REPEAT(DPAD_RIGHT))
    {
        sNaming->cursorX = sNaming->cursorX == BUTTON_COL ? 0 : sNaming->cursorX + 1;
        redraw = TRUE;
    }
    else if (JOY_REPEAT(DPAD_UP))
    {
        if (sNaming->cursorX == BUTTON_COL)
            sNaming->cursorY = SelectedButton() == 0 ? BUTTON_DONE : SelectedButton() - 1;
        else
            sNaming->cursorY = sNaming->cursorY == 0 ? KEY_ROWS - 1 : sNaming->cursorY - 1;
        redraw = TRUE;
    }
    else if (JOY_REPEAT(DPAD_DOWN))
    {
        if (sNaming->cursorX == BUTTON_COL)
            sNaming->cursorY = SelectedButton() == BUTTON_DONE ? 0 : SelectedButton() + 1;
        else
            sNaming->cursorY = sNaming->cursorY == KEY_ROWS - 1 ? 0 : sNaming->cursorY + 1;
        redraw = TRUE;
    }
    else if (JOY_NEW(SELECT_BUTTON | R_BUTTON))
    {
        sNaming->page = (sNaming->page + 1) % PAGE_COUNT;
        redraw = TRUE;
    }
    else if (JOY_NEW(L_BUTTON))
    {
        sNaming->page = (sNaming->page + PAGE_COUNT - 1) % PAGE_COUNT;
        redraw = TRUE;
    }
    else if (JOY_NEW(START_BUTTON))
    {
        sNaming->cursorX = BUTTON_COL;
        sNaming->cursorY = BUTTON_DONE;
        redraw = TRUE;
    }
    else if (JOY_NEW(B_BUTTON))
    {
        DeleteChar();
        redraw = TRUE;
    }
    else if (JOY_NEW(A_BUTTON))
    {
        switch (SelectedButton())
        {
        case BUTTON_DELETE: DeleteChar(); break;
        case BUTTON_SPACE:  AddChar(CHAR_SPACE); break;
        case BUTTON_DONE:
            PlaySE(SE_SELECT);
            SaveText();
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sNaming->state = 1;
            return;
        default:
            AddChar(sKeys[sNaming->page][sNaming->cursorY][sNaming->cursorX]);
            break;
        }
        redraw = TRUE;
    }
    if (redraw)
    {
        PlaySE(SE_SELECT);
        DrawHeader();
        DrawTabs();
        DrawKeys();
        DrawButtons();
    }
}
