#include "global.h"
#include "sprite.h"
#include "rf_bag_art.h"
#include "data/rf_bag_user_art.h"

#define TAG_BAG_USER_ART 0x70E0
#define TAG_BAG_USER_RETURN 0x70E1

enum { ART_HEADER, ART_CATEGORY_FIRST, ART_CURSOR = 8, ART_LEFT, ART_RIGHT,
       ART_UP, ART_DOWN, ART_PREVIEW_RETURN, ART_SPRITE_COUNT };
enum { FRAME_BACKPACK, FRAME_MEDICINE, FRAME_BATTLE, FRAME_EVOLUTION, FRAME_HELD,
       FRAME_TREASURE, FRAME_OTHER, FRAME_BUNDLE, FRAME_LEFT, FRAME_RIGHT,
       FRAME_RETURN, FRAME_CURSOR, FRAME_UP, FRAME_DOWN, FRAME_COUNT };

STATIC_ASSERT(sizeof(sRFBagUserTiles) == FRAME_COUNT * 128, RFBagUserAtlasSize)
STATIC_ASSERT(ARRAY_COUNT(sRFBagUserPaletteGroups) == FRAME_COUNT, RFBagUserPaletteGroups)

static EWRAM_DATA u8 sArtSprites[ART_SPRITE_COUNT] = {0};

static const struct OamData sSmallOam = {
    .shape = SPRITE_SHAPE(16x16), .size = SPRITE_SIZE(16x16),
    .bpp = ST_OAM_4BPP, .priority = 1,
};
static const struct OamData sReturnOam = {
    .shape = SPRITE_SHAPE(32x32), .size = SPRITE_SIZE(32x32),
    .bpp = ST_OAM_4BPP, .priority = 1,
};
#define ART_FRAME(n) {ANIMCMD_FRAME((n) * 4, 0), ANIMCMD_END}
static const union AnimCmd sAnimations[FRAME_COUNT][2] = {
    ART_FRAME(0), ART_FRAME(1), ART_FRAME(2), ART_FRAME(3), ART_FRAME(4),
    ART_FRAME(5), ART_FRAME(6), ART_FRAME(7), ART_FRAME(8), ART_FRAME(9),
    ART_FRAME(10), ART_FRAME(11), ART_FRAME(12), ART_FRAME(13),
};
#undef ART_FRAME
static const union AnimCmd *const sAnimationTable[FRAME_COUNT] = {
    sAnimations[0], sAnimations[1], sAnimations[2], sAnimations[3], sAnimations[4],
    sAnimations[5], sAnimations[6], sAnimations[7], sAnimations[8], sAnimations[9],
    sAnimations[10], sAnimations[11], sAnimations[12], sAnimations[13],
};
static const struct SpriteTemplate sSmallTemplate = {
    .tileTag = TAG_BAG_USER_ART, .paletteTag = TAG_BAG_USER_ART, .oam = &sSmallOam,
    .anims = sAnimationTable, .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};
static const struct SpriteTemplate sReturnTemplate = {
    .tileTag = TAG_BAG_USER_RETURN, .paletteTag = TAG_BAG_USER_ART, .oam = &sReturnOam,
    .anims = gDummySpriteAnimTable, .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static u8 CreateArt(u32 frame, s16 x, s16 y)
{
    u8 id = CreateSprite(&sSmallTemplate, x, y, 0);
    if (id < MAX_SPRITES)
    {
        StartSpriteAnim(&gSprites[id], frame);
        gSprites[id].oam.paletteNum = IndexOfSpritePaletteTag(TAG_BAG_USER_ART + sRFBagUserPaletteGroups[frame]);
    }
    return id;
}

static void ShowArt(u32 slot, bool32 show)
{
    if (sArtSprites[slot] < MAX_SPRITES)
        gSprites[sArtSprites[slot]].invisible = !show;
}

void RFBagArt_Init(void)
{
    static const struct SpriteSheet sheet = {sRFBagUserTiles, sizeof(sRFBagUserTiles), TAG_BAG_USER_ART};
    static const struct SpriteSheet back = {sRFBagUserReturnTiles, sizeof(sRFBagUserReturnTiles), TAG_BAG_USER_RETURN};
    u32 i;
    LoadSpriteSheet(&sheet);
    LoadSpriteSheet(&back);
    for (i = 0; i < ARRAY_COUNT(sRFBagUserPalettes); i++)
    {
        struct SpritePalette palette = {sRFBagUserPalettes[i], TAG_BAG_USER_ART + i};
        LoadSpritePalette(&palette);
    }
    sArtSprites[ART_HEADER] = CreateArt(FRAME_BACKPACK, 13, 8);
    for (i = 0; i < 7; i++)
        sArtSprites[ART_CATEGORY_FIRST + i] = CreateArt(i, 18 + i * 34, 23);
    sArtSprites[ART_CURSOR] = CreateArt(FRAME_CURSOR, 116, 57);
    sArtSprites[ART_LEFT] = CreateArt(FRAME_LEFT, 144, 8);
    sArtSprites[ART_RIGHT] = CreateArt(FRAME_RIGHT, 224, 8);
    sArtSprites[ART_UP] = CreateArt(FRAME_UP, 236, 45);
    sArtSprites[ART_DOWN] = CreateArt(FRAME_DOWN, 236, 142);
    sArtSprites[ART_PREVIEW_RETURN] = CreateSprite(&sReturnTemplate, 40, 73, 0);
    for (i = ART_CURSOR; i < ART_SPRITE_COUNT; i++)
        ShowArt(i, FALSE);
}

void RFBagArt_ShowCategories(bool32 show)
{
    u32 i;
    for (i = 0; i < 7; i++)
        ShowArt(ART_CATEGORY_FIRST + i, show);
}

void RFBagArt_ShowPocketArrows(bool32 show)
{
    ShowArt(ART_LEFT, show);
    ShowArt(ART_RIGHT, show);
}

void RFBagArt_SetCursor(u32 y, bool32 show)
{
    if (sArtSprites[ART_CURSOR] < MAX_SPRITES)
        gSprites[sArtSprites[ART_CURSOR]].y = 48 + y + 8;
    ShowArt(ART_CURSOR, show);
}

void RFBagArt_SetScroll(u32 offset, u32 visible, u32 total)
{
    ShowArt(ART_UP, offset > 0 && total > visible);
    ShowArt(ART_DOWN, offset + visible < total);
}

void RFBagArt_HideTransient(void)
{
    ShowArt(ART_CURSOR, FALSE);
    ShowArt(ART_UP, FALSE);
    ShowArt(ART_DOWN, FALSE);
    ShowArt(ART_PREVIEW_RETURN, FALSE);
}

void RFBagArt_ShowReturn(bool32 show)
{
    ShowArt(ART_PREVIEW_RETURN, show);
}

u8 RFBagArt_CreateReturnRow(s16 x, s16 y)
{
    return CreateArt(FRAME_RETURN, x, y);
}
