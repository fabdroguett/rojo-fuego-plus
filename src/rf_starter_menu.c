#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "key_system.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
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
    COMPOUND_STRING("KANTO"), COMPOUND_STRING("JOHTO"), COMPOUND_STRING("HOENN"),
    COMPOUND_STRING("SINNOH"), COMPOUND_STRING("TESELIA"), COMPOUND_STRING("KALOS"),
    COMPOUND_STRING("ALOLA"), COMPOUND_STRING("GALAR"), COMPOUND_STRING("PALDEA"),
    COMPOUND_STRING("EXTRAS: POTENCIAL"), COMPOUND_STRING("EXTRAS: LEYENDAS"),
    COMPOUND_STRING("EXTRAS: DRAGONES"), COMPOUND_STRING("EXTRAS: NUEVA ERA"),
    COMPOUND_STRING("EXTRAS: COMPAÑEROS"), COMPOUND_STRING("EXTRAS: INSTINTO"),
    COMPOUND_STRING("EXTRAS: CARÁCTER"), COMPOUND_STRING("EXTRAS: SORPRESAS"),
    COMPOUND_STRING("EXTRAS: SINGULARES"),
};
STATIC_ASSERT(ARRAY_COUNT(sStarters) == ARRAY_COUNT(sGroups), RFStarterGroups);

static const struct BgTemplate sBg = {.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31};
static const struct WindowTemplate sWindows[] =
{
    {.bg = 0, .width = 30, .height = 20, .paletteNum = 15, .baseBlock = 1},
    DUMMY_WIN_TEMPLATE,
};
static const u8 sInk[] = {0, 2, 3};
static const u8 sGold[] = {0, 9, 3};
static EWRAM_DATA u8 sGroup = 0;
static EWRAM_DATA u8 sChoice = 0;
static EWRAM_DATA u8 sAbility = 0;
static EWRAM_DATA u16 sPics[3] = {0};
static EWRAM_DATA bool8 sConfirm = FALSE;
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

static void Draw(void)
{
    u32 i;
    u16 species = SelectedSpecies();
    FillWindowPixelBuffer(0, PIXEL_FILL(4));
    RFUI_PrintFitted(0, COMPOUND_STRING("ELIGE TU COMPAÑERO"), 8, 1, 174, sGold);
    RFUI_PrintFitted(0, COMPOUND_STRING("54 / Nv.5"), 182, 1, 52, sInk);
    RFUI_DrawPanel(0, 4, 21, 232, 17, 1, 6);
    RFUI_PrintFitted(0, sGroups[sGroup], 10, 22, 174, sInk);
    ConvertIntToDecimalStringN(gStringVar1, sGroup + 1, STR_CONV_MODE_RIGHT_ALIGN, 2);
    StringAppend(gStringVar1, COMPOUND_STRING("/18"));
    RFUI_PrintFitted(0, gStringVar1, 200, 22, 32, sGold);
    for (i = 0; i < 3; i++)
    {
        u32 x = 4 + i * 79;
        RFUI_DrawPanel(0, x, 41, 74, 76, i == sChoice ? 8 : 1, i == sChoice ? 9 : 6);
        RFUI_PrintFitted(0, GetSpeciesName(sStarters[sGroup][i]), x + 3, 100, 68, i == sChoice ? sGold : sInk);
    }
    StringCopy(gStringVar1, sAbility == 2 ? COMPOUND_STRING("Oculta: ") : COMPOUND_STRING("Normal: "));
    StringAppend(gStringVar1, gAbilitiesInfo[gSpeciesInfo[species].abilities[sAbility]].name);
    RFUI_PrintFitted(0, gStringVar1, 8, 120, 224, sInk);
    RFUI_PrintFitted(0, sConfirm ? COMPOUND_STRING("¿Lo eliges? A: confirmar  B: volver")
        : HasHiddenAbility(species) ? COMPOUND_STRING("SELECT: normal / oculta")
        : COMPOUND_STRING("Esta especie no tiene habilidad oculta."), 8, 133, 224, sGold);
    RFUI_PrintFitted(0, COMPOUND_STRING("L/R: grupo   < >: elegir   A: OK   B: salir"), 8, 146, 224, sInk);
    PutWindowTilemap(0);
    CopyWindowToVram(0, COPYWIN_FULL);
}

static void UpdatePics(void)
{
    u32 i;
    for (i = 0; i < 3; i++)
    {
        if (sPics[i] < MAX_SPRITES)
            FreeAndDestroyMonPicSprite(sPics[i]);
        sPics[i] = CreateMonPicSprite(sStarters[sGroup][i], FALSE, 0, TRUE, 41 + i * 79, 72, i, TAG_NONE);
        if (sPics[i] < MAX_SPRITES)
            gSprites[sPics[i]].oam.priority = 0;
    }
}

static void Task_Exit(u8 taskId)
{
    u32 i;
    if (gPaletteFade.active)
        return;
    for (i = 0; i < 3; i++)
        if (sPics[i] < MAX_SPRITES)
            FreeAndDestroyMonPicSprite(sPics[i]);
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
    if (gPaletteFade.active)
        return;
    if (sConfirm)
    {
        if (JOY_NEW(A_BUTTON))
            Exit(taskId, TRUE);
        else if (JOY_NEW(B_BUTTON))
        {
            sConfirm = FALSE;
            Draw();
        }
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        Exit(taskId, FALSE);
        return;
    }
    if (JOY_NEW(A_BUTTON))
    {
        sConfirm = TRUE;
        PlayCry_Normal(SelectedSpecies(), 0);
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
    u32 i;
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
    RFUI_LoadPalette(15, KeySystem_IsOn(KEY_DARK_MODE));
    LoadPalette(&gPlttBufferUnfaded[BG_PLTT_ID(15) + 4], 0, sizeof(u16));
    sGroup = sChoice = sAbility = 0;
    sConfirm = FALSE;
    sPendingSpecies = SPECIES_NONE;
    for (i = 0; i < 3; i++)
        sPics[i] = SPRITE_NONE;
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
