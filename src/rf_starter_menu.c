#include "global.h"
#include "battle_main.h"
#include "bg.h"
#include "decompress.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "key_system.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_summary_screen.h"
#include "rf_ui_theme.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "trainer_pokemon_sprites.h"
#include "window.h"
#include "constants/songs.h"

// Grass / fire / water order. Extras retain the same three rival story branches.
static const u16 sStarters[][3] =
{
    {SPECIES_BULBASAUR, SPECIES_CHARMANDER, SPECIES_SQUIRTLE},
    {SPECIES_CHIKORITA, SPECIES_CYNDAQUIL, SPECIES_TOTODILE},
    {SPECIES_TREECKO, SPECIES_TORCHIC, SPECIES_MUDKIP},
    {SPECIES_TURTWIG, SPECIES_CHIMCHAR, SPECIES_PIPLUP},
    {SPECIES_SNIVY, SPECIES_TEPIG, SPECIES_OSHAWOTT},
    {SPECIES_CHESPIN, SPECIES_FENNEKIN, SPECIES_FROAKIE},
    {SPECIES_ROWLET, SPECIES_LITTEN, SPECIES_POPPLIO},
    {SPECIES_GROOKEY, SPECIES_SCORBUNNY, SPECIES_SOBBLE},
    {SPECIES_SPRIGATITO, SPECIES_FUECOCO, SPECIES_QUAXLY},
    {SPECIES_BELDUM, SPECIES_RIOLU, SPECIES_GIBLE},
    {SPECIES_AXEW, SPECIES_DRATINI, SPECIES_LARVITAR},
    {SPECIES_BAGON, SPECIES_DEINO, SPECIES_GOOMY},
    {SPECIES_JANGMO_O, SPECIES_DREEPY, SPECIES_FRIGIBAX},
    {SPECIES_EEVEE, SPECIES_RALTS, SPECIES_SHINX},
    {SPECIES_ROCKRUFF, SPECIES_ZORUA, SPECIES_ABSOL},
    {SPECIES_TOXEL, SPECIES_CHARCADET, SPECIES_PAWNIARD},
    {SPECIES_MAREEP, SPECIES_TRAPINCH, SPECIES_NOIBAT},
    {SPECIES_TOGEPI, SPECIES_MIMIKYU, SPECIES_DURALUDON},
};
static const u8 *const sGroups[] =
{
    COMPOUND_STRING("KANTO · GEN. I"), COMPOUND_STRING("JOHTO · GEN. II"),
    COMPOUND_STRING("HOENN · GEN. III"), COMPOUND_STRING("SINNOH · GEN. IV"),
    COMPOUND_STRING("TESELIA · GEN. V"), COMPOUND_STRING("KALOS · GEN. VI"),
    COMPOUND_STRING("ALOLA · GEN. VII"), COMPOUND_STRING("GALAR · GEN. VIII"),
    COMPOUND_STRING("PALDEA · GEN. IX"),
    COMPOUND_STRING("EXTRAS: POTENCIAL"), COMPOUND_STRING("EXTRAS: LEYENDAS"),
    COMPOUND_STRING("EXTRAS: DRAGONES"), COMPOUND_STRING("EXTRAS: NUEVA ERA"),
    COMPOUND_STRING("EXTRAS: COMPAÑEROS"), COMPOUND_STRING("EXTRAS: INSTINTO"),
    COMPOUND_STRING("EXTRAS: CARÁCTER"), COMPOUND_STRING("EXTRAS: SORPRESAS"),
    COMPOUND_STRING("EXTRAS: SINGULARES"),
};
STATIC_ASSERT(ARRAY_COUNT(sStarters) == ARRAY_COUNT(sGroups), RFStarterGroups);

// 240x160 canvas: header + group pager, three cards, ability row and footer.
#define CARD_X(i)   (4 + (i) * 79)
#define CARD_Y      21
#define CARD_W      74
#define CARD_H      100
#define PIC_Y       55
#define INFO_Y      122
#define BOB_PERIOD  16

static const struct BgTemplate sBg = {.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31};
static const struct WindowTemplate sWindows[] =
{
    {.bg = 0, .width = 30, .height = 20, .paletteNum = 15, .baseBlock = 1},
    DUMMY_WIN_TEMPLATE,
};
// RFUI_LoadPalette roles: 1 panel, 2 text, 3 shadow, 5 muted, 6 edge, 8 selected, 9 gold.
static const u8 sInk[] = {0, 2, 3};
static const u8 sGold[] = {0, 9, 3};
static const u8 sMuted[] = {0, 5, 3};
static const u8 sCardInk[] = {1, 2, 3};
static const u8 sCardGold[] = {8, 9, 3};
static const u8 sChipOff[] = {1, 5, 3};
static const u8 sChipNone[] = {1, 6, 1};
static const u8 sModalInk[] = {8, 2, 3};
static EWRAM_DATA u8 sGroup = 0;
static EWRAM_DATA u8 sChoice = 0;
static EWRAM_DATA u8 sAbility = 0;
static EWRAM_DATA u16 sPics[3] = {0};
static EWRAM_DATA u8 sTypeIcons[3][2] = {0};
static EWRAM_DATA bool8 sConfirm = FALSE;
static EWRAM_DATA bool8 sDark = FALSE;
static EWRAM_DATA bool8 sFocusApplied = FALSE;
static EWRAM_DATA u16 sPendingSpecies = SPECIES_NONE;
static EWRAM_DATA u8 sPendingAbility = 0;

static u16 SelectedSpecies(void)
{
    return sStarters[sGroup][sChoice];
}

static bool32 HasHiddenAbility(u16 species)
{
    return gSpeciesInfo[species].abilities[2] != ABILITY_NONE
        && gSpeciesInfo[species].abilities[2] != gSpeciesInfo[species].abilities[0]
        && gSpeciesInfo[species].abilities[2] != gSpeciesInfo[species].abilities[1];
}

static void PrintCentered(const u8 *text, u32 x, u32 y, u32 width, const u8 *colors)
{
    u32 font = GetStringWidth(FONT_SMALL, text, 0) > width ? FONT_SMALL_NARROWER : FONT_SMALL;
    u32 textWidth = GetStringWidth(font, text, 0);
    if (textWidth > width)
        RFUI_PrintFitted(0, text, x, y, width, colors);
    else
        AddTextPrinterParameterized3(0, font, x + (width - textWidth) / 2, y, colors, TEXT_SKIP_DRAW, text);
}

static void DrawChip(const u8 *label, u32 x, u32 width, bool32 active, bool32 available)
{
    if (active)
        RFUI_DrawPanel(0, x, INFO_Y, width, 13, 8, 9);
    else
        RFUI_DrawPanel(0, x, INFO_Y, width, 13, 1, 6);
    PrintCentered(label, x, INFO_Y, width, active ? sCardGold : available ? sChipOff : sChipNone);
}

static void DrawHeader(void)
{
    u32 i;
    PrintCentered(sGroups[sGroup], 20, 0, 200, sGold);
    AddTextPrinterParameterized3(0, FONT_SMALL, 4, 0, sMuted, TEXT_SKIP_DRAW, COMPOUND_STRING("{L_BUTTON}"));
    AddTextPrinterParameterized3(0, FONT_SMALL, 222, 0, sMuted, TEXT_SKIP_DRAW, COMPOUND_STRING("{R_BUTTON}"));
    // Pager: one segment per group. The current one is gold and taller, so position is not colour-only.
    for (i = 0; i < ARRAY_COUNT(sStarters); i++)
    {
        if (i == sGroup)
            RFUI_FillRect(0, 9, 4 + i * 13, 14, 11, 4);
        else
            RFUI_FillRect(0, 6, 4 + i * 13, 15, 11, 2);
    }
}

static void DrawCards(void)
{
    u32 i;
    for (i = 0; i < 3; i++)
    {
        bool32 selected = (i == sChoice);
        RFUI_DrawPanel(0, CARD_X(i), CARD_Y, CARD_W, CARD_H, selected ? 8 : 1, selected ? 9 : 6);
        if (selected) // double gold ring: focus reads by weight as well as colour
            RFUI_DrawRing(0, CARD_X(i) + 1, CARD_Y + 1, CARD_W - 2, CARD_H - 2, 9);
        PrintCentered(GetSpeciesName(sStarters[sGroup][i]), CARD_X(i) + 3, CARD_Y + 66, CARD_W - 6,
            selected ? sCardGold : sCardInk);
    }
}

static void DrawAbilityRow(void)
{
    u16 species = SelectedSpecies();
    bool32 hidden = HasHiddenAbility(species);
    u16 ability = gSpeciesInfo[species].abilities[sAbility];

    DrawChip(COMPOUND_STRING("NORMAL"), 4, 40, sAbility != 2, TRUE);
    DrawChip(COMPOUND_STRING("OCULTA"), 46, 40, sAbility == 2, hidden);
    RFUI_PrintFitted(0, gAbilitiesInfo[ability].name, 92, INFO_Y, 144, sInk);
    RFUI_PrintFitted(0, hidden ? gAbilitiesInfo[ability].description
        : COMPOUND_STRING("Sin habilidad oculta: conserva la suya."), 6, INFO_Y + 13, 230, sMuted);
    PrintCentered(hidden
        ? COMPOUND_STRING("{DPAD_LEFTRIGHT} Elegir  {SELECT_BUTTON} Habilidad  {A_BUTTON} OK  {B_BUTTON} Salir")
        : COMPOUND_STRING("{DPAD_LEFTRIGHT} Elegir  {A_BUTTON} OK  {B_BUTTON} Salir"), 4, INFO_Y + 25, 232, sMuted);
}

static void DrawConfirm(void)
{
    u16 species = SelectedSpecies();
    RFUI_DrawPanel(0, 4, INFO_Y, 232, 38, 8, 9);
    StringCopy(gStringVar1, COMPOUND_STRING("¿Eliges a "));
    StringAppend(gStringVar1, GetSpeciesName(species));
    StringAppend(gStringVar1, COMPOUND_STRING("?"));
    PrintCentered(gStringVar1, 8, INFO_Y + 1, 224, sCardGold);
    StringCopy(gStringVar1, sAbility == 2 ? COMPOUND_STRING("Habilidad oculta: ") : COMPOUND_STRING("Habilidad: "));
    StringAppend(gStringVar1, gAbilitiesInfo[gSpeciesInfo[species].abilities[sAbility]].name);
    StringAppend(gStringVar1, COMPOUND_STRING(" · Nv. 5"));
    PrintCentered(gStringVar1, 8, INFO_Y + 12, 224, sModalInk);
    PrintCentered(COMPOUND_STRING("{A_BUTTON} Sí, es mi compañero    {B_BUTTON} Volver"), 8, INFO_Y + 24, 224, sCardGold);
}

static void Draw(void)
{
    FillWindowPixelBuffer(0, PIXEL_FILL(4));
    DrawHeader();
    DrawCards();
    if (sConfirm)
        DrawConfirm();
    else
        DrawAbilityRow();
    PutWindowTilemap(0);
    CopyWindowToVram(0, COPYWIN_FULL);
}

// Unselected companions recede toward the card colour; the focused one keeps full colour.
static void ApplyFocus(void)
{
    u32 i, mask = 0;
    u16 panel = sDark ? RF_UI_NIGHT_PANEL : RF_UI_DAY_PANEL;
    for (i = 0; i < 3; i++)
        if (i != sChoice)
            mask |= 1u << (16 + i);
    BlendPalettes(1u << (16 + sChoice), 0, panel);
    BlendPalettes(mask, sConfirm ? 11 : 6, panel);
    sFocusApplied = TRUE;
}

static void UpdateTypeIcons(void)
{
    u32 i, j;
    for (i = 0; i < 3; i++)
    {
        u16 species = sStarters[sGroup][i];
        enum Type types[2] = {GetSpeciesType(species, 0), GetSpeciesType(species, 1)};
        bool32 dual = types[0] != types[1];
        for (j = 0; j < 2; j++)
        {
            struct Sprite *sprite;
            if (sTypeIcons[i][j] >= MAX_SPRITES)
                continue;
            sprite = &gSprites[sTypeIcons[i][j]];
            sprite->invisible = (j == 1 && !dual);
            StartSpriteAnim(sprite, types[j]);
            sprite->oam.paletteNum = gTypesInfo[types[j]].palette;
            sprite->x = CARD_X(i) + CARD_W / 2 + (dual ? (j == 0 ? -17 : 17) : 0);
            sprite->y = CARD_Y + 87;
        }
    }
}

static void UpdatePics(void)
{
    u32 i;
    for (i = 0; i < 3; i++)
    {
        if (sPics[i] < MAX_SPRITES)
            FreeAndDestroyMonPicSprite(sPics[i]);
        sPics[i] = CreateMonPicSprite(sStarters[sGroup][i], FALSE, 0, TRUE, CARD_X(i) + CARD_W / 2, PIC_Y, i, TAG_NONE);
        if (sPics[i] < MAX_SPRITES)
            gSprites[sPics[i]].oam.priority = 0;
    }
    UpdateTypeIcons();
    sFocusApplied = FALSE;
}

static void AnimateFocus(u8 taskId)
{
    u32 i;
    s16 timer = ++gTasks[taskId].data[0];
    for (i = 0; i < 3; i++)
    {
        if (sPics[i] >= MAX_SPRITES)
            continue;
        // Idle hop for the focused companion; it stays raised while confirming.
        if (i != sChoice)
            gSprites[sPics[i]].y2 = 0;
        else if (sConfirm)
            gSprites[sPics[i]].y2 = -3;
        else
            gSprites[sPics[i]].y2 = ((timer / BOB_PERIOD) & 1) ? -2 : 0;
    }
}

static void Task_Exit(u8 taskId)
{
    u32 i, j;
    if (gPaletteFade.active)
        return;
    for (i = 0; i < 3; i++)
    {
        if (sPics[i] < MAX_SPRITES)
            FreeAndDestroyMonPicSprite(sPics[i]);
        for (j = 0; j < 2; j++)
            if (sTypeIcons[i][j] < MAX_SPRITES)
                DestroySprite(&gSprites[sTypeIcons[i][j]]);
    }
    FreeSpriteTilesByTag(gSpriteTemplate_MoveTypes.tileTag);
    FreeAllWindowBuffers();
    DestroyTask(taskId);
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

static void Exit(u8 taskId, bool32 accepted)
{
    static const u8 storyBranches[] = {0, 2, 1};
    sPendingSpecies = accepted ? SelectedSpecies() : SPECIES_NONE;
    sPendingAbility = sAbility;
    gSpecialVar_Result = sPendingSpecies;
    gSpecialVar_0x8005 = storyBranches[sChoice];
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_Exit;
}

static void Task_Input(u8 taskId)
{
    AnimateFocus(taskId);
    if (gPaletteFade.active)
        return;
    if (!sFocusApplied)
        ApplyFocus();
    if (sConfirm)
    {
        if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            Exit(taskId, TRUE);
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sConfirm = FALSE;
            Draw();
            ApplyFocus();
        }
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        Exit(taskId, FALSE);
        return;
    }
    if (JOY_NEW(A_BUTTON))
    {
        sConfirm = TRUE;
        PlayCry_Normal(SelectedSpecies(), 0);
        Draw();
        ApplyFocus();
        return;
    }
    else if (JOY_NEW(L_BUTTON | R_BUTTON))
    {
        sGroup = (sGroup + (JOY_NEW(R_BUTTON) ? 1 : ARRAY_COUNT(sStarters) - 1)) % ARRAY_COUNT(sStarters);
        sAbility = 0;
        UpdatePics();
    }
    else if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        sChoice = (sChoice + (JOY_NEW(DPAD_RIGHT) ? 1 : 2)) % 3;
        sAbility = 0;
        sFocusApplied = FALSE;
    }
    else if (JOY_NEW(SELECT_BUTTON) && HasHiddenAbility(SelectedSpecies()))
        sAbility = sAbility == 2 ? 0 : 2;
    else
        return;
    PlaySE(SE_SELECT);
    Draw();
}

static void VBlank(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void Main(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

void CB2_RF_StarterMenu(void)
{
    u32 i, j;
    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_WIN0H, 0);
    SetGpuReg(REG_OFFSET_WIN0V, 0);
    ScanlineEffect_Stop();
    ResetTasks();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetAllPicSprites();
    ResetPaletteFade();
    DmaFill16(3, 0, VRAM, VRAM_SIZE);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, &sBg, 1);
    ChangeBgX(0, 0, BG_COORD_SET);
    ChangeBgY(0, 0, BG_COORD_SET);
    InitWindows(sWindows);
    DeactivateAllTextPrinters();
    sDark = KeySystem_IsOn(KEY_DARK_MODE);
    RFUI_LoadPalette(15, sDark);
    LoadPalette(&gPlttBufferUnfaded[BG_PLTT_ID(15) + 4], 0, sizeof(u16));
    LoadCompressedSpriteSheet(&gSpriteSheet_MoveTypes);
    LoadPalette(gMoveTypes_Pal, OBJ_PLTT_ID(13), 3 * PLTT_SIZE_4BPP);
    sGroup = sChoice = sAbility = 0;
    sConfirm = FALSE;
    sPendingSpecies = SPECIES_NONE;
    for (i = 0; i < 3; i++)
    {
        sPics[i] = SPRITE_NONE;
        for (j = 0; j < 2; j++)
        {
            sTypeIcons[i][j] = CreateSprite(&gSpriteTemplate_MoveTypes, 0, 0, 0);
            if (sTypeIcons[i][j] < MAX_SPRITES)
                gSprites[sTypeIcons[i][j]].oam.priority = 0;
        }
    }
    UpdatePics();
    Draw();
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    CreateTask(Task_Input, 0);
    ShowBg(0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP | DISPCNT_BG0_ON);
    SetVBlankCallback(VBlank);
    SetMainCallback2(Main);
}

void Script_RF_ChooseStarter(void)
{
    SetMainCallback2(CB2_RF_StarterMenu);
}

// Called immediately after givemon, before nickname or rival scripts can alter context.
void Script_RF_ApplyStarterAbility(void)
{
    if (sPendingSpecies != SPECIES_NONE && gPartiesCount[B_TRAINER_PLAYER] != 0)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gPartiesCount[B_TRAINER_PLAYER] - 1];
        if (GetMonData(mon, MON_DATA_SPECIES) == sPendingSpecies)
            SetMonData(mon, MON_DATA_ABILITY_NUM, &sPendingAbility);
    }
    sPendingSpecies = SPECIES_NONE;
}
