// Disposable fixture: copy only into an isolated build, never production src/.
#include "global.h"
#include "main.h"
#include "overworld.h"
#include "event_data.h"
#include "key_system.h"
#include "money.h"
#include "menu.h"
#include "start_menu.h"
#include "dexnav.h"
#include "text_window.h"
#include "window.h"
#include "rf_ui_theme.h"
#include "rf_kanto_ui.h"
#include "constants/maps.h"

void RFUiFixture_Start(void);
EWRAM_DATA u32 gRFMainMenuFixtureWindow = 0;

void RFMainMenuFixture_Dark(void)
{
    KeySystem_Set(KEY_DARK_MODE, TRUE);
    SetMoney(&gSaveBlock1Ptr->money, 3080);
    LoadMessageBoxAndBorderGfx();
    RFUiFixture_Start();
}

void RFMainMenuFixture_Light(void)
{
    KeySystem_Set(KEY_DARK_MODE, FALSE);
    SetMoney(&gSaveBlock1Ptr->money, 3080);
    LoadMessageBoxAndBorderGfx();
    RFUiFixture_Start();
}

void RFMainMenuFixture_Zero(void)
{
    RFMainMenuFixture_Dark();
    SetMoney(&gSaveBlock1Ptr->money, 0);
}

void RFMainMenuFixture_Max(void)
{
    RFMainMenuFixture_Light();
    SetMoney(&gSaveBlock1Ptr->money, 999999);
}

void RFMainMenuFixture_Locked(void)
{
    RFMainMenuFixture_Light();
    FlagClear(FLAG_SYS_POKEDEX_GET);
    FlagClear(FLAG_SYS_POKEMON_GET);
    FlagClear(DN_FLAG_DEXNAV_GET);
}

void RFMainMenuFixture_Inspect(void)
{
    gRFMainMenuFixtureWindow = GetStartMenuWindowId();
    SetMainCallback2(CB2_Overworld);
}

void RFMainMenuFixture_Outdoor(void)
{
    RFMainMenuFixture_Dark();
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_ROUTE1);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_ROUTE1);
}

void RFMainMenuFixture_Font(void)
{
    u8 window = GetStartMenuWindowId();
    FillWindowPixelBuffer(window, PIXEL_FILL(1));
    RFKanto_Print(window, COMPOUND_STRING("ÁÉÍÓÚ Ñ ¿¡ 0123456789"), 6, 10, 212, 2);
    RFKanto_Print(window, COMPOUND_STRING("áéíóú ñ ¿Qué tal? ¡Bien!"), 6, 26, 212, 2);
    RFKanto_Print(window, COMPOUND_STRING("POKÉDEX EQUIPO MISIONES"), 6, 42, 212, 2);
    RFKanto_Print(window, COMPOUND_STRING("Il1 O0 68 rn m ¥999999"), 6, 58, 212, 2);
    RFKanto_Print(window, COMPOUND_STRING("Capítulos, hábitat y Pokémon."), 6, 74, 212, 2);
    CopyWindowToVram(window, COPYWIN_GFX);
    SetMainCallback2(CB2_Overworld);
}
