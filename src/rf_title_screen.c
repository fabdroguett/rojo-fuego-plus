#include "global.h"
#include "berry_fix_program.h"
#include "bg.h"
#include "clear_save_data_menu.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "intro.h"
#include "m4a.h"
#include "main.h"
#include "main_menu.h"
#include "malloc.h"
#include "palette.h"
#include "quickstart.h"
#include "random.h"
#include "reset_rtc_screen.h"
#include "rf_title_screen.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "trig.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// Pokémon Ultimate Kanto title screen: dusk sky, a scrolling mountain range, the logo over a volcano
// whose lava cycles, Charizard and Pikachu at either side, rising embers and a blinking prompt.
// Art: tools/rojo_fuego/make_title_art.py.
#include "data/rf_title_art.h"

#define TAG_EMBER 0x5A10
#define TAG_MON0 0x5A11
#define TAG_MON1 0x5A12
#define EMBER_COUNT 12
#define IDLE_FRAMES (60 * 45)

// Palette slots (see the generator): 0 sky, 1 range, 2 logo/volcano, 3 text.
#define PAL_SKY 0
#define PAL_RANGE 1
#define PAL_FRONT 2
#define PAL_TEXT 3

#define KEYSTROKE_DELSAVE (B_BUTTON | SELECT_BUTTON | DPAD_UP)
#define KEYSTROKE_RESET_RTC (B_BUTTON | SELECT_BUTTON | DPAD_LEFT)
#define KEYSTROKE_BERRY_FIX (B_BUTTON | SELECT_BUTTON)

enum {
    STATE_RUN,
    STATE_CONFIRM,
    STATE_FADE_TO_MENU,
    STATE_FADE_TO_INTRO,
};

static const struct BgTemplate sBgTemplates[] = {
    {.bg = 0, .charBaseIndex = 3, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0},
    {.bg = 1, .charBaseIndex = 2, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 1, .baseTile = 0},
    {.bg = 2, .charBaseIndex = 1, .mapBaseIndex = 29, .screenSize = 0, .paletteMode = 0, .priority = 2, .baseTile = 0},
    {.bg = 3, .charBaseIndex = 0, .mapBaseIndex = 28, .screenSize = 0, .paletteMode = 0, .priority = 3, .baseTile = 0},
};

static const struct OamData sEmberOam = {
    .shape = SPRITE_SHAPE(8x8),
    .size = SPRITE_SIZE(8x8),
    .priority = 1,
};

static const union AnimCmd sEmberAnim[] = {
    ANIMCMD_FRAME(0, 18),
    ANIMCMD_FRAME(1, 18),
    ANIMCMD_FRAME(2, 18),
    ANIMCMD_END,
};
static const union AnimCmd *const sEmberAnims[] = {sEmberAnim};

static void SpriteCB_Ember(struct Sprite *sprite);

// The two title Pokémon: 64x64, alternating their two front frames like a battle idle.
static const struct OamData sMonOam = {
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
    .priority = 1,
};

static const union AnimCmd sMonAnim0[] = {
    ANIMCMD_FRAME(0, 50),
    ANIMCMD_FRAME(64, 14),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sMonAnim1[] = {
    ANIMCMD_FRAME(0, 36),
    ANIMCMD_FRAME(64, 20),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd *const sMonAnims0[] = {sMonAnim0};
static const union AnimCmd *const sMonAnims1[] = {sMonAnim1};

static const struct SpriteTemplate sMonTemplates[] = {
    {.tileTag = TAG_MON0, .paletteTag = TAG_MON0, .oam = &sMonOam, .anims = sMonAnims0, .images = NULL,
     .affineAnims = gDummySpriteAffineAnimTable, .callback = SpriteCallbackDummy},
    {.tileTag = TAG_MON1, .paletteTag = TAG_MON1, .oam = &sMonOam, .anims = sMonAnims1, .images = NULL,
     .affineAnims = gDummySpriteAffineAnimTable, .callback = SpriteCallbackDummy},
};

static const struct SpriteTemplate sEmberTemplate = {
    .tileTag = TAG_EMBER,
    .paletteTag = TAG_EMBER,
    .oam = &sEmberOam,
    .anims = sEmberAnims,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Ember,
};

static EWRAM_DATA u16 sFrame = 0;
static EWRAM_DATA u16 sIdle = 0;

static void CB2_RFTitleRun(void);
static void VBlankCB_RFTitle(void);
static void Task_RFTitle(u8 taskId);

#define sLife data[0]
#define sDrift data[1]
#define sPhase data[2]
#define sSubY data[3]

static void SpawnEmber(bool32 anywhere)
{
    // Most embers leave the crater; some rise from the ridge for depth.
    bool32 crater = (Random() % 4) != 0;
    s16 x = crater ? RF_TITLE_CRATER_X - 8 + Random() % 16 : 8 + Random() % 224;
    s16 y = anywhere ? 40 + Random() % 60 : (crater ? RF_TITLE_CRATER_Y + 2 : 132);
    u8 id = CreateSprite(&sEmberTemplate, x, y, 0);
    if (id == MAX_SPRITES)
        return;
    gSprites[id].sLife = 90 + Random() % 70;
    gSprites[id].sDrift = (Random() % 5) - 2;
    gSprites[id].sPhase = Random() % 64;
    gSprites[id].sSubY = 0;
    StartSpriteAnim(&gSprites[id], 0);
}

static void SpriteCB_Ember(struct Sprite *sprite)
{
    sprite->sSubY += 3;
    sprite->y -= sprite->sSubY >> 2;
    sprite->sSubY &= 3;
    sprite->sPhase++;
    sprite->x2 = (gSineTable[(sprite->sPhase * 4) & 0xFF] * 3) >> 8;
    if ((sprite->sPhase & 7) == 0)
        sprite->x += sprite->sDrift > 0 ? 1 : sprite->sDrift < 0 ? -1 : 0;
    if (--sprite->sLife <= 0 || sprite->y < -8)
    {
        DestroySprite(sprite);
        SpawnEmber(FALSE);
    }
}

#undef sLife
#undef sDrift
#undef sPhase
#undef sSubY

static void ResetGpu(void)
{
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_BLDY, 0);
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    SetGpuReg(REG_OFFSET_BG2HOFS, 0);
    SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    SetGpuReg(REG_OFFSET_BG3HOFS, 0);
    SetGpuReg(REG_OFFSET_BG3VOFS, 0);
}

void CB2_InitTitleScreenRF(void)
{
    switch (gMain.state)
    {
    default:
        gMain.state = 0;
        // fallthrough
    case 0:
        SetVBlankCallback(NULL);
        InitHeap(gHeap, HEAP_SIZE);
        ResetTasks();
        ResetSpriteData();
        FreeAllSpritePalettes();
        ResetPaletteFade();
        ResetGpu();
        DmaFill16(3, 0, (void *)VRAM, VRAM_SIZE);
        DmaFill32(3, 0, (void *)OAM, OAM_SIZE);
        DmaFill16(3, 0, (void *)PLTT, PLTT_SIZE);
        ResetBgsAndClearDma3BusyFlags(FALSE);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        break;
    case 1:
        LoadBgTiles(3, sRFTitleSkyTiles, sizeof(sRFTitleSkyTiles), 0);
        LoadBgTiles(2, sRFTitleRangeTiles, sizeof(sRFTitleRangeTiles), 0);
        LoadBgTiles(1, sRFTitleFrontTiles, sizeof(sRFTitleFrontTiles), 0);
        LoadBgTiles(0, sRFTitleTextTiles, sizeof(sRFTitleTextTiles), 0);
        break;
    case 2:
        LoadBgTilemap(3, sRFTitleSkyMap, sizeof(sRFTitleSkyMap), 0);
        LoadBgTilemap(2, sRFTitleRangeMap, sizeof(sRFTitleRangeMap), 0);
        LoadBgTilemap(1, sRFTitleFrontMap, sizeof(sRFTitleFrontMap), 0);
        LoadBgTilemap(0, sRFTitleTextMap, sizeof(sRFTitleTextMap), 0);
        LoadPalette(sRFTitleSkyPal, BG_PLTT_ID(PAL_SKY), PLTT_SIZE_4BPP);
        LoadPalette(sRFTitleRangePal, BG_PLTT_ID(PAL_RANGE), PLTT_SIZE_4BPP);
        LoadPalette(sRFTitleFrontPal, BG_PLTT_ID(PAL_FRONT), PLTT_SIZE_4BPP);
        LoadPalette(sRFTitleTextPal, BG_PLTT_ID(PAL_TEXT), PLTT_SIZE_4BPP);
        break;
    case 3:
    {
        static const struct SpriteSheet sheet = {sRFTitleEmberTiles, sizeof(sRFTitleEmberTiles), TAG_EMBER};
        static const struct SpritePalette palette = {sRFTitleEmberPal, TAG_EMBER};
        static const struct SpriteSheet monSheets[] = {
            {sRFTitleMon0Tiles, sizeof(sRFTitleMon0Tiles), TAG_MON0},
            {sRFTitleMon1Tiles, sizeof(sRFTitleMon1Tiles), TAG_MON1},
        };
        static const struct SpritePalette monPalettes[] = {
            {sRFTitleMon0Pal, TAG_MON0},
            {sRFTitleMon1Pal, TAG_MON1},
        };
        u32 i;
        LoadSpriteSheet(&sheet);
        LoadSpritePalette(&palette);
        for (i = 0; i < ARRAY_COUNT(sMonTemplates); i++)
        {
            LoadSpriteSheet(&monSheets[i]);
            LoadSpritePalette(&monPalettes[i]);
            // Sprite coordinates are centres; the generator gives top-left corners.
            CreateSprite(&sMonTemplates[i], sRFTitleMonPos[i][0] + 32, sRFTitleMonPos[i][1] + 32, 2);
        }
        for (i = 0; i < EMBER_COUNT; i++)
            SpawnEmber(TRUE);
        break;
    }
    case 4:
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON
                                      | DISPCNT_BG0_ON | DISPCNT_BG1_ON | DISPCNT_BG2_ON | DISPCNT_BG3_ON);
        BeginNormalPaletteFade(PALETTES_ALL, 1, 16, 0, RGB_BLACK);
        sFrame = sIdle = 0;
        CreateTask(Task_RFTitle, 0);
        SetVBlankCallback(VBlankCB_RFTitle);
        SetMainCallback2(CB2_RFTitleRun);
        m4aSongNumStart(MUS_TITLE);
        if (QUICKSTART && QUICKSTART_HUD)
            CreateQuickstartHud();
        return;
    }
    gMain.state++;
}

static void CB2_RFTitleRun(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB_RFTitle(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

// Lava (front colours 12-14) and the twinkling stars (sky colours 13-15) rotate on their own beat;
// the prompt fades in and out by swapping its ink for the ridge colour it sits on.
static void AnimatePalettes(void)
{
    u32 i;
    if ((sFrame & 7) == 0)
    {
        u32 shift = (sFrame >> 3) % 3;
        for (i = 0; i < 3; i++)
        {
            u16 color = sRFTitleFrontPal[12 + (i + shift) % 3];
            gPlttBufferUnfaded[BG_PLTT_ID(PAL_FRONT) + 12 + i] = color;
            if (!gPaletteFade.active)
                gPlttBufferFaded[BG_PLTT_ID(PAL_FRONT) + 12 + i] = color;
        }
    }
    if ((sFrame & 15) == 0)
    {
        u32 shift = (sFrame >> 4) % 3;
        for (i = 0; i < 3; i++)
        {
            u16 color = sRFTitleSkyPal[13 + (i + shift) % 3];
            gPlttBufferUnfaded[BG_PLTT_ID(PAL_SKY) + 13 + i] = color;
            if (!gPaletteFade.active)
                gPlttBufferFaded[BG_PLTT_ID(PAL_SKY) + 13 + i] = color;
        }
    }
    if (!gPaletteFade.active)
    {
        u16 ink = (sFrame & 63) < 40 ? sRFTitleTextPal[1] : sRFTitleTextPal[2];
        gPlttBufferFaded[BG_PLTT_ID(PAL_TEXT) + 1] = gPlttBufferUnfaded[BG_PLTT_ID(PAL_TEXT) + 1] = ink;
    }
}

static void FadeTo(u8 taskId, u32 state, u16 color)
{
    BeginNormalPaletteFade(PALETTES_ALL, 2, 0, 16, color);
    gTasks[taskId].data[0] = state;
}

static void Task_RFTitle(u8 taskId)
{
    s16 *state = &gTasks[taskId].data[0];
    sFrame++;
    // Scroll through the register only: ChangeBgX would leave an offset in the shared BG2 config,
    // which later scenes (the professor's affine BG2) inherit while BG2 is still hidden.
    SetGpuReg(REG_OFFSET_BG2HOFS, sFrame / 6);
    AnimatePalettes();

    switch (*state)
    {
    case STATE_RUN:
        if (gPaletteFade.active)
            break;
        if (JOY_HELD(KEYSTROKE_DELSAVE) == KEYSTROKE_DELSAVE)
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            DestroyTask(taskId);
            SetMainCallback2(CB2_RFTitleToClearSave);
        }
        else if (JOY_HELD(KEYSTROKE_RESET_RTC) == KEYSTROKE_RESET_RTC && CanResetRTC() == TRUE)
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            DestroyTask(taskId);
            SetMainCallback2(CB2_RFTitleToResetRtc);
        }
        else if (JOY_HELD(KEYSTROKE_BERRY_FIX) == KEYSTROKE_BERRY_FIX)
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            DestroyTask(taskId);
            SetMainCallback2(CB2_RFTitleToBerryFix);
        }
        else if (JOY_NEW(A_BUTTON | START_BUTTON))
        {
            PlaySE(SE_M_FLAME_WHEEL);
            gTasks[taskId].data[1] = 0;
            *state = STATE_CONFIRM;
        }
        else if (QUICKSTART && JOY_NEW(SELECT_BUTTON))
        {
            Quickstart();
        }
        else if (++sIdle >= IDLE_FRAMES)
        {
            FadeOutBGM(4);
            FadeTo(taskId, STATE_FADE_TO_INTRO, RGB_BLACK);
        }
        break;
    case STATE_CONFIRM:
        // A short flash of the prompt before the white fade, like striking a match.
        if (++gTasks[taskId].data[1] >= 24)
        {
            FadeOutBGM(4);
            FadeTo(taskId, STATE_FADE_TO_MENU, RGB_WHITE);
        }
        else
        {
            u16 ink = (gTasks[taskId].data[1] & 4) ? sRFTitleTextPal[1] : sRFTitleFrontPal[2];
            gPlttBufferFaded[BG_PLTT_ID(PAL_TEXT) + 1] = gPlttBufferUnfaded[BG_PLTT_ID(PAL_TEXT) + 1] = ink;
        }
        break;
    case STATE_FADE_TO_MENU:
        if (!gPaletteFade.active)
        {
            DestroyTask(taskId);
            SetMainCallback2(CB2_InitMainMenu);
        }
        break;
    case STATE_FADE_TO_INTRO:
        if (!gPaletteFade.active && IsNotWaitingForBGMStop())
        {
            DestroyTask(taskId);
            SetMainCallback2(CB2_InitCopyrightScreenAfterTitleScreen);
        }
        break;
    }
}

// The start menu sits on the same dusk: sky and mountain range on two spare backgrounds, with
// their palettes moved to the requested slots. The tilemaps are written straight to VRAM because a
// queued DMA copy would read the remapped buffer after it is freed.
static void WriteRemappedTilemap(u8 bg, const u16 *map, u32 count, u32 palette)
{
    vu16 *dest = (vu16 *)BG_SCREEN_ADDR(GetBgAttribute(bg, BG_ATTR_MAPBASEINDEX));
    u32 i;
    for (i = 0; i < count; i++)
        dest[i] = (map[i] & 0x0FFF) | (palette << 12);
}

void RFTitle_LoadMenuBackdrop(u8 skyBg, u8 rangeBg, u8 skyPalette, u8 rangePalette)
{
    LoadBgTiles(skyBg, sRFTitleSkyTiles, sizeof(sRFTitleSkyTiles), 0);
    LoadBgTiles(rangeBg, sRFTitleRangeTiles, sizeof(sRFTitleRangeTiles), 0);
    WriteRemappedTilemap(skyBg, sRFTitleSkyMap, ARRAY_COUNT(sRFTitleSkyMap), skyPalette);
    WriteRemappedTilemap(rangeBg, sRFTitleRangeMap, ARRAY_COUNT(sRFTitleRangeMap), rangePalette);
    LoadPalette(sRFTitleSkyPal, BG_PLTT_ID(skyPalette), PLTT_SIZE_4BPP);
    LoadPalette(sRFTitleRangePal, BG_PLTT_ID(rangePalette), PLTT_SIZE_4BPP);
    LoadPalette(&sRFTitleSkyPal[0], BG_PLTT_ID(0), PLTT_SIZEOF(1)); // the backdrop is the night sky
}

void CB2_RFTitleToClearSave(void)
{
    if (!UpdatePaletteFade())
        SetMainCallback2(CB2_InitClearSaveDataScreen);
}

void CB2_RFTitleToResetRtc(void)
{
    if (!UpdatePaletteFade())
    {
        m4aMPlayAllStop();
        SetMainCallback2(CB2_InitResetRtcScreen);
    }
}

void CB2_RFTitleToBerryFix(void)
{
    if (!UpdatePaletteFade())
    {
        m4aMPlayAllStop();
        SetMainCallback2(CB2_InitBerryFixProgram);
    }
}
