#include "global.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "item.h"
#include "key_system.h"
#include "list_menu.h"
#include "main.h"
#include "menu.h"
#include "pokemon.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/songs.h"

// Rojo Fuego+ Key System: a menu reached from the Start menu that turns game settings on and off.
// Keys live in gSaveBlock3Ptr->keySystemOff (a set bit means the player turned that key off),
// except Exp. All, which keeps using its own flag so the Exp. Share logic reads it directly.

STATIC_ASSERT(KEY_COUNT <= 32, KeySystemKeysFitInSaveBits);

#define KEY_LIST_ROWS   6
#define KEY_LIST_WIDTH  28
#define KEY_DESC_HEIGHT 4

struct KeyInfo
{
    const u8 *name;
    const u8 *description;
};

static const u8 sText_CaptureExp[] = _("EXP al capturar");
static const u8 sText_CaptureExpDesc[] = _("Capturar da la misma EXP que\nderrotar al Pokémon.");
static const u8 sText_ExpAll[] = _("Exp. All");
static const u8 sText_ExpAllDesc[] = _("Todo el equipo recibe la EXP\ncompleta de cada combate.");
static const u8 sText_ScaledExp[] = _("EXP según nivel");
static const u8 sText_ScaledExpDesc[] = _("Más EXP contra rivales de más nivel\ny menos contra los de menos nivel.");
static const u8 sText_LevelCap[] = _("Límite de nivel");
static const u8 sText_LevelCapDesc[] = _("Tus Pokémon no pasan del nivel\ndel as del próximo líder.");
static const u8 sText_Split[] = _("Físico/especial");
static const u8 sText_SplitDesc[] = _("Cada movimiento es físico o especial\npor sí mismo, no por su tipo.");
static const u8 sText_Crits[] = _("Críticos modernos");
static const u8 sText_CritsDesc[] = _("Los críticos hacen ×1,5 en vez de\n×2 y son algo menos frecuentes.");
static const u8 sText_Status[] = _("Estados modernos");
static const u8 sText_StatusDesc[] = _("Parálisis, sueño y quemaduras\nfuncionan como desde la Gen 7.");
static const u8 sText_Weather[] = _("Clima de 5 turnos");
static const u8 sText_WeatherDesc[] = _("Sequía, Llovizna y similares duran\n5 turnos en vez de todo el combate.");
static const u8 sText_Followers[] = _("Pokémon que te sigue");
static const u8 sText_FollowersDesc[] = _("El primero del equipo camina\ndetrás de ti.");
static const u8 sText_OwEncounters[] = _("Salvajes visibles");
static const u8 sText_OwEncountersDesc[] = _("Los salvajes aparecen en el mapa.\nApagado: encuentros clásicos.");
static const u8 sText_HmFree[] = _("MOs sin enseñar");
static const u8 sText_HmFreeDesc[] = _("Con la medalla y la MO en la mochila\nno hace falta que nadie la sepa.");
static const u8 sText_Autorun[] = _("Correr siempre");
static const u8 sText_AutorunDesc[] = _("Corres sin mantener B.\nMantén B para caminar.");
static const u8 sText_Autosave[] = _("Autoguardado");
static const u8 sText_AutosaveDesc[] = _("Guarda la partida sola cada vez\nque curas en un Centro Pokémon.");
static const u8 sText_Difficulty[] = _("Dificultad");
static const u8 sText_DifficultyDesc[] = _("Fácil baja y Reto sube el nivel\nde los Pokémon de entrenadores.");

static const u8 *const sText_DifficultyNames[KEY_DIFFICULTY_COUNT] =
{
    [KEY_DIFFICULTY_NORMAL]    = COMPOUND_STRING("NORMAL"),
    [KEY_DIFFICULTY_EASY]      = COMPOUND_STRING("FÁCIL"),
    [KEY_DIFFICULTY_CHALLENGE] = COMPOUND_STRING("RETO"),
};

static const struct KeyInfo sKeyInfo[KEY_COUNT] =
{
    [KEY_CAPTURE_EXP]            = {sText_CaptureExp,   sText_CaptureExpDesc},
    [KEY_EXP_ALL]                = {sText_ExpAll,       sText_ExpAllDesc},
    [KEY_SCALED_EXP]             = {sText_ScaledExp,    sText_ScaledExpDesc},
    [KEY_LEVEL_CAP]              = {sText_LevelCap,     sText_LevelCapDesc},
    [KEY_PHYSICAL_SPECIAL_SPLIT] = {sText_Split,        sText_SplitDesc},
    [KEY_MODERN_CRITS]           = {sText_Crits,        sText_CritsDesc},
    [KEY_MODERN_STATUS]          = {sText_Status,       sText_StatusDesc},
    [KEY_ABILITY_WEATHER_TURNS]  = {sText_Weather,      sText_WeatherDesc},
    [KEY_FOLLOWERS]              = {sText_Followers,    sText_FollowersDesc},
    [KEY_OW_ENCOUNTERS]          = {sText_OwEncounters, sText_OwEncountersDesc},
    [KEY_HM_FREE]                = {sText_HmFree,       sText_HmFreeDesc},
    [KEY_AUTORUN]                = {sText_Autorun,      sText_AutorunDesc},
    [KEY_AUTOSAVE]               = {sText_Autosave,     sText_AutosaveDesc},
    [KEY_DIFFICULTY]             = {sText_Difficulty,   sText_DifficultyDesc},
};

static const u8 sText_On[] = _("SÍ");
static const u8 sText_Off[] = _("NO");

static const struct WindowTemplate sKeyListWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = KEY_LIST_WIDTH,
    .height = KEY_LIST_ROWS * 2,
    .paletteNum = 15,
    .baseBlock = 1,
};

static const struct WindowTemplate sKeyDescWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 15,
    .width = KEY_LIST_WIDTH,
    .height = KEY_DESC_HEIGHT,
    .paletteNum = 15,
    .baseBlock = 1 + KEY_LIST_WIDTH * KEY_LIST_ROWS * 2,
};

static EWRAM_DATA struct ListMenuItem sKeyListItems[KEY_COUNT] = {0};

#define tListTaskId   data[0]
#define tListWindowId data[1]
#define tDescWindowId data[2]

static void Task_KeySystemMenu(u8 taskId);
static void KeySystem_PrintValue(u8 windowId, u32 key, u8 y);
static void KeySystem_MoveCursor(s32 key, bool8 onInit, struct ListMenu *list);

bool32 KeySystem_IsOn(enum KeySystemKey key)
{
    if (key == KEY_EXP_ALL)
        return FlagGet(FLAG_EXP_ALL);
    return !(gSaveBlock3Ptr->keySystemOff & (1u << key));
}

void KeySystem_Set(enum KeySystemKey key, bool32 on)
{
    if (key == KEY_EXP_ALL)
    {
        if (on)
            FlagSet(FLAG_EXP_ALL);
        else
            FlagClear(FLAG_EXP_ALL);
    }
    else if (on)
    {
        gSaveBlock3Ptr->keySystemOff &= ~(1u << key);
    }
    else
    {
        gSaveBlock3Ptr->keySystemOff |= (1u << key);
    }
}

enum KeyDifficulty KeySystem_GetDifficulty(void)
{
    return gSaveBlock3Ptr->keySystemDifficulty % KEY_DIFFICULTY_COUNT;
}

// Easy takes 10% off trainer Pokémon levels (at least 1), Challenge adds 10% (at least 2)
u32 KeySystem_ScaleTrainerLevel(u32 level)
{
    switch (KeySystem_GetDifficulty())
    {
    case KEY_DIFFICULTY_EASY:
        return max(1, (s32)level - (s32)max(1, level / 10));
    case KEY_DIFFICULTY_CHALLENGE:
        return min(MAX_LEVEL, level + max(2, level / 10));
    default:
        return level;
    }
}

// HM-free field moves: the badge check stays in the scripts; here the player needs the HM itself
bool32 KeySystem_CanUseFieldMoveWithoutMon(enum Move move)
{
    enum Item item;

    if (!KeySystem_IsOn(KEY_HM_FREE))
        return FALSE;
    for (item = ITEM_HM01; item <= ITEM_HM08; item++)
    {
        if (GetItemTMHMMoveId(item) == move && CheckBagHasItem(item, 1))
            return TRUE;
    }
    return FALSE;
}

// Party slot shown using an HM-free field move: the first Pokémon that could learn it, else the
// first one that is not an Egg. Returns PARTY_SIZE when nobody can stand in.
s32 KeySystem_GetFieldMoveUser(enum Move move)
{
    s32 i, fallback = PARTY_SIZE;

    if (!KeySystem_CanUseFieldMoveWithoutMon(move))
        return PARTY_SIZE;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        enum Species species = GetMonData(mon, MON_DATA_SPECIES);

        if (species == SPECIES_NONE)
            break;
        if (GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        if (CanLearnTeachableMove(species, move))
            return i;
        if (fallback == PARTY_SIZE)
            fallback = i;
    }
    return fallback;
}

static u8 KeySystem_DescWindowId(void)
{
    u8 taskId = FindTaskIdByFunc(Task_KeySystemMenu);
    return (taskId == TASK_NONE) ? WINDOW_NONE : gTasks[taskId].tDescWindowId;
}

// Called by the list menu for every row it draws: prints SÍ/NO aligned to the right edge.
static void KeySystem_PrintValue(u8 windowId, u32 key, u8 y)
{
    const u8 *value;
    u32 x;

    if (key == KEY_DIFFICULTY)
        value = sText_DifficultyNames[KeySystem_GetDifficulty()];
    else
        value = KeySystem_IsOn(key) ? sText_On : sText_Off;
    x = KEY_LIST_WIDTH * 8 - GetStringWidth(FONT_NORMAL, value, 0) - 8;

    AddTextPrinterParameterized(windowId, FONT_NORMAL, value, x, y, TEXT_SKIP_DRAW, NULL);
}

static void KeySystem_MoveCursor(s32 key, bool8 onInit, struct ListMenu *list)
{
    u8 windowId = KeySystem_DescWindowId();

    if (!onInit)
        PlaySE(SE_SELECT);
    if (windowId == WINDOW_NONE)
        return;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(windowId, FONT_NORMAL, sKeyInfo[key].description, 0, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(windowId, COPYWIN_GFX);
}

void KeySystem_OpenMenu(void)
{
    struct ListMenuTemplate template = {0};
    u8 taskId = CreateTask(Task_KeySystemMenu, 80);
    u32 i;

    for (i = 0; i < KEY_COUNT; i++)
    {
        sKeyListItems[i].name = sKeyInfo[i].name;
        sKeyListItems[i].id = i;
    }

    LoadMessageBoxAndBorderGfx();
    gTasks[taskId].tListWindowId = AddWindow(&sKeyListWindowTemplate);
    gTasks[taskId].tDescWindowId = AddWindow(&sKeyDescWindowTemplate);
    DrawStdWindowFrame(gTasks[taskId].tListWindowId, FALSE);
    DrawStdWindowFrame(gTasks[taskId].tDescWindowId, FALSE);

    template.items = sKeyListItems;
    template.moveCursorFunc = KeySystem_MoveCursor;
    template.itemPrintFunc = KeySystem_PrintValue;
    template.totalItems = KEY_COUNT;
    template.maxShowed = KEY_LIST_ROWS;
    template.windowId = gTasks[taskId].tListWindowId;
    template.item_X = 8;
    template.cursor_X = 0;
    template.upText_Y = 1;
    template.cursorPal = 2;
    template.fillValue = 1;
    template.cursorShadowPal = 3;
    template.lettersSpacing = 1;
    template.scrollMultiple = LIST_NO_MULTIPLE_SCROLL;
    template.fontId = FONT_NORMAL;
    gTasks[taskId].tListTaskId = ListMenuInit(&template, 0, 0);

    CopyWindowToVram(gTasks[taskId].tListWindowId, COPYWIN_FULL);
    CopyWindowToVram(gTasks[taskId].tDescWindowId, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
}

static void KeySystem_CloseMenu(u8 taskId)
{
    DestroyListMenuTask(gTasks[taskId].tListTaskId, NULL, NULL);
    ClearStdWindowAndFrame(gTasks[taskId].tListWindowId, TRUE);
    ClearStdWindowAndFrame(gTasks[taskId].tDescWindowId, TRUE);
    RemoveWindow(gTasks[taskId].tListWindowId);
    RemoveWindow(gTasks[taskId].tDescWindowId);
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
    DestroyTask(taskId);
}

static void Task_KeySystemMenu(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    switch (input)
    {
    case LIST_NOTHING_CHOSEN:
        break;
    case LIST_CANCEL:
        PlaySE(SE_SELECT);
        KeySystem_CloseMenu(taskId);
        break;
    default:
        PlaySE(SE_SELECT);
        if (input == KEY_DIFFICULTY)
            gSaveBlock3Ptr->keySystemDifficulty = (KeySystem_GetDifficulty() + 1) % KEY_DIFFICULTY_COUNT;
        else
            KeySystem_Set(input, !KeySystem_IsOn(input));
        RedrawListMenu(gTasks[taskId].tListTaskId);
        break;
    }
}
