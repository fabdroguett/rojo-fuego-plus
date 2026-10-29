#include "global.h"
#include "bg.h"
#include "gpu_regs.h"
#include "palette.h"
#include "rf_intro_art.h"

// Pokémon Ultimate Kanto new-game intro: the professor's lab at dusk and the night tones of the
// controls guide. Art: tools/rojo_fuego/make_intro_art.py.
#include "data/rf_intro_art.h"

// BG1 shares charblock 0 with the 8bpp trainer pictures, which start at VRAM + 0x600, so the lab
// lives in charblock 1, which nothing else in this scene uses.
#define LAB_CHARBASE 1

void RFIntro_LoadLabBackdrop(void)
{
    SetBgAttribute(1, BG_ATTR_CHARBASEINDEX, LAB_CHARBASE);
    ShowBg(1); // writes BG1CNT so the hardware reads the new charblock
    LoadBgTiles(1, sRFIntroLabTiles, sizeof(sRFIntroLabTiles), 0);
    CopyToBgTilemapBuffer(1, sRFIntroLabMap, sizeof(sRFIntroLabMap), 0);
    CopyBgTilemapBufferToVram(1);
    LoadPalette(sRFIntroLabPal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
}

void RFIntro_ResetPictureBg(void)
{
    // The trainer pictures sit on affine BG2 (mode 1), which this scene never fully initialises: it
    // relied on what the FireRed title screen left behind. Set the identity matrix here.
    SetGpuReg(REG_OFFSET_BG2PA, 0x100);
    SetGpuReg(REG_OFFSET_BG2PB, 0);
    SetGpuReg(REG_OFFSET_BG2PC, 0);
    SetGpuReg(REG_OFFSET_BG2PD, 0x100);
}

void RFIntro_LoadGuidePalette(void)
{
    LoadPalette(sRFIntroGuidePal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
    LoadPalette(sRFIntroWelcomePal, BG_PLTT_ID(2), PLTT_SIZE_4BPP); // welcome pages' panels
}
