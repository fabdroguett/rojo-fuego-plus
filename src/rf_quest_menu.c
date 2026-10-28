#include "global.h"
#include "rf_ui_theme.h"
#include "achievements.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "item.h"
#include "list_menu.h"
#include "main.h"
#include "menu.h"
#include "pokemon.h"
#include "rf_quests.h"
#include "rf_story.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/songs.h"

#define QUEST_CHAPTERS RF_QUEST_CHAPTER_COUNT
#define QUEST_ROWS 4
#define sRFQuests RFQuests_GetCatalog()

// A field menu keeps the overworld task/callback ownership unchanged, like the Key System.
// Window tiles occupy [1, 449); standard border tiles start above this range.
static const struct WindowTemplate sQuestWindows[] =
{
    { .bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 28, .height = 2, .paletteNum = 15, .baseBlock = 1 },
    { .bg = 0, .tilemapLeft = 1, .tilemapTop = 4, .width = 28, .height = 8, .paletteNum = 15, .baseBlock = 57 },
    { .bg = 0, .tilemapLeft = 1, .tilemapTop = 13, .width = 28, .height = 6, .paletteNum = 15, .baseBlock = 281 },
};

static const u8 *const sChapterNames[] =
{
    COMPOUND_STRING("MUSEO Y FÓSILES"),
    COMPOUND_STRING("RELIQUIAS DE SETE"),
    COMPOUND_STRING("INVESTIGACIÓN DE OAK"),
    COMPOUND_STRING("REFUGIOS DE KANTO"),
    COMPOUND_STRING("VIDA Y HÁBITATS"),
    COMPOUND_STRING("CUIDADO Y EVOLUCIÓN"),
    COMPOUND_STRING("LEYENDAS DE KANTO"),
    COMPOUND_STRING("LEYENDAS DEL MUNDO"),
    COMPOUND_STRING("ULTRAENTES Y PARADOJAS"),
    COMPOUND_STRING("MEMORIA DE KANTO"),
    COMPOUND_STRING("LA RED Y SUS VOCES"),
    COMPOUND_STRING("UNA PUERTA ABIERTA"),
    COMPOUND_STRING("CUADERNOS DE CAMPO"),
    COMPOUND_STRING("NADIE SE QUEDA ATRÁS"),
    COMPOUND_STRING("CAMINOS QUE SE CRUZAN"),
    COMPOUND_STRING("LO QUE DEVUELVE EL MAR"),
    COMPOUND_STRING("EL CAMINO DE VUELTA"),
};
STATIC_ASSERT(ARRAY_COUNT(sChapterNames) == QUEST_CHAPTERS, RFQuestChapterNames);
static const u8 *const sStatusNames[] =
{
    COMPOUND_STRING("{A_BUTTON} Aceptar"),
    COMPOUND_STRING("En curso"),
    COMPOUND_STRING("{A_BUTTON} Reclamar"),
    COMPOUND_STRING("{A_BUTTON} Desenlace"),
};
static const u8 *const sFilterNames[] = {
    COMPOUND_STRING("{DPAD_LEFTRIGHT} Descubiertas"), COMPOUND_STRING("{DPAD_LEFTRIGHT} Disponibles"),
    COMPOUND_STRING("{DPAD_LEFTRIGHT} En curso"), COMPOUND_STRING("{DPAD_LEFTRIGHT} Listas"),
    COMPOUND_STRING("{DPAD_LEFTRIGHT} Completadas"), COMPOUND_STRING("{DPAD_LEFTRIGHT} Aquí"),
    COMPOUND_STRING("{DPAD_LEFTRIGHT} Fácil"), COMPOUND_STRING("{DPAD_LEFTRIGHT} Media"),
    COMPOUND_STRING("{DPAD_LEFTRIGHT} Alta"),
};
static EWRAM_DATA struct ListMenuItem sQuestList[RF_QUEST_COUNT] = {0};
static EWRAM_DATA u8 sQuestWindowIds[3] = {0};
static EWRAM_DATA u8 sQuestChapter = 0;
static EWRAM_DATA u16 sQuestSelected = 0;
static EWRAM_DATA u16 sQuestListCount = 0;
static EWRAM_DATA u8 sQuestListTask = 0;
static EWRAM_DATA bool8 sQuestMenuOpen = FALSE;
static EWRAM_DATA u8 sQuestFilter = 0;
static EWRAM_DATA u8 sEndingPage = 0;

static void DrawQuestDetails(const u8 *message)
{
    u8 window = sQuestWindowIds[2];
    bool32 unlocked = RFQuests_IsUnlocked(sQuestSelected);
    enum RFQuestState state = RFQuests_GetState(sQuestSelected);
    const u8 *status = sQuestSelected < RF_QUEST_COUNT ? sStatusNames[state] : COMPOUND_STRING("Sin encargos");
    const u8 *body = COMPOUND_STRING("Sin encargos para este filtro.\nIzq./Der.: cambia de filtro.\nL/R: cambia de capítulo.");
    if (sQuestSelected < RF_QUEST_COUNT && (unlocked || state != RF_QUEST_AVAILABLE))
        body = RFStory_Hint(sQuestSelected);
    if (message == NULL && unlocked && state == RF_QUEST_AVAILABLE && RFStory_GiverHint(sQuestSelected) != NULL)
    {
        // Authored scenes are offered in person, so the journal names who gives them.
        status = COMPOUND_STRING("Pídelo en persona");
        StringCopy(gStringVar4, COMPOUND_STRING("Te lo ofrece:\n"));
        StringAppend(gStringVar4, RFStory_GiverHint(sQuestSelected));
        body = gStringVar4;
    }
    if (sEndingPage)
        status = COMPOUND_STRING("{A_BUTTON} Seguir {B_BUTTON} Diario");

    FillWindowPixelBuffer(window, PIXEL_FILL(1));
    RFUI_PrintFitted(window, status, 0, 0, 96, (const u8[]){1,2,3});
    if (!sEndingPage)
        RFUI_PrintFitted(window, sFilterNames[sQuestFilter], 100, 0, 124, (const u8[]){1,2,3});
    RFUI_PrintParagraph(window, message != NULL ? message : body, 0, 12, 224, 3, (const u8[]){1,2,3});
    CopyWindowToVram(window, COPYWIN_GFX);
}

static void QuestMoveCursor(s32 id, bool8 onInit, struct ListMenu *list)
{
    sQuestSelected = id < 0 ? RF_QUEST_NONE : id;
    if (!onInit)
        PlaySE(SE_SELECT);
    DrawQuestDetails(NULL);
}

static void QuestPrintState(u8 window, u32 id, u8 y)
{
    const u8 *mark = COMPOUND_STRING(" ");
    enum RFQuestState state = RFQuests_GetState(id);
    if (id >= RF_QUEST_COUNT)
        return;

    if (state == RF_QUEST_CLAIMED)
        mark = COMPOUND_STRING("OK");
    else if (state == RF_QUEST_READY)
        mark = COMPOUND_STRING("!");
    else if (gSaveBlock3Ptr->rfQuestTracked == id + 1)
        mark = COMPOUND_STRING("▶");
    AddTextPrinterParameterized(window, FONT_SMALL, mark, 203, y, TEXT_SKIP_DRAW, NULL);
}

static void DrawChapterHeader(void)
{
    u32 i, completed = 0;
    u32 total = sQuestListCount == 1 && sQuestList[0].id < 0 ? 0 : sQuestListCount;
    u8 window = sQuestWindowIds[0];
    for (i = 0; i < sQuestListCount; i++)
        if (RFQuests_GetState(sQuestList[i].id) == RF_QUEST_CLAIMED)
            completed++;
    ConvertIntToDecimalStringN(gStringVar1, completed, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(gStringVar1, COMPOUND_STRING("/"));
    ConvertIntToDecimalStringN(gStringVar1 + StringLength(gStringVar1), total, STR_CONV_MODE_LEFT_ALIGN, 3);
    FillWindowPixelBuffer(window, PIXEL_FILL(1));
    RFUI_DrawIcon(window, RF_ICON_QUESTS, 0, 0, 2, 3);
    RFUI_PrintFitted(window, sChapterNames[sQuestChapter], 20, 0, 132, (const u8[]){1,2,3});
    AddTextPrinterParameterized(window, FONT_SMALL, gStringVar1, 155, 0, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(window, FONT_SMALL, COMPOUND_STRING("{L_BUTTON}{R_BUTTON}"), 190, 0, TEXT_SKIP_DRAW, NULL);
    FillWindowPixelRect(window, PIXEL_FILL(3), 20, 13, 204, 2);
    if (sQuestListCount != 0 && completed != 0)
        FillWindowPixelRect(window, PIXEL_FILL(5), 20, 13, 204 * completed / sQuestListCount, 2);
    CopyWindowToVram(window, COPYWIN_FULL);
}

static void BuildQuestList(void)
{
    struct ListMenuTemplate template = {0};
    u32 i;
    u16 trackedRow = 0;

    sQuestListCount = 0;
    for (i = 0; i < RF_QUEST_COUNT; i++)
    {
        if (sRFQuests[i].chapter == sQuestChapter && RFStory_MatchesFilter(i, sQuestFilter))
        {
            sQuestList[sQuestListCount].name = RFQuests_IsUnlocked(i) ? sRFQuests[i].name : COMPOUND_STRING("???");
            if (gSaveBlock3Ptr->rfQuestTracked == i + 1 || sQuestSelected == i)
                trackedRow = sQuestListCount;
            sQuestList[sQuestListCount++].id = i;
        }
    }
    if (sQuestListCount == 0)
    {
        sQuestList[0].name = COMPOUND_STRING("Sin encargos para este filtro");
        sQuestList[0].id = LIST_NOTHING_CHOSEN;
        sQuestListCount = 1;
    }
    DrawChapterHeader();
    FillWindowPixelBuffer(sQuestWindowIds[1], PIXEL_FILL(1));
    template.items = sQuestList;
    template.moveCursorFunc = QuestMoveCursor;
    template.itemPrintFunc = QuestPrintState;
    template.totalItems = sQuestListCount;
    template.maxShowed = min(sQuestListCount, QUEST_ROWS);
    template.windowId = sQuestWindowIds[1];
    template.item_X = 8;
    template.upText_Y = 1;
    template.cursorPal = 2;
    template.fillValue = 1;
    template.cursorShadowPal = 3;
    template.fontId = FONT_NORMAL;
    sQuestListTask = ListMenuInit(&template, trackedRow >= QUEST_ROWS ? trackedRow - QUEST_ROWS + 1 : 0,
        trackedRow >= QUEST_ROWS ? QUEST_ROWS - 1 : trackedRow);
    CopyWindowToVram(sQuestWindowIds[1], COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
}

static void Task_QuestMenu(u8 taskId)
{
    s32 input;

    if (sEndingPage)
    {
        if (JOY_NEW(B_BUTTON))
        {
            sEndingPage = 0;
            DrawQuestDetails(NULL);
        }
        else if (JOY_NEW(A_BUTTON))
        {
            if (RFStory_ReadEnding(sQuestSelected, sEndingPage, gStringVar4, 96))
            {
                sEndingPage++;
                DrawQuestDetails(gStringVar4);
            }
            else
            {
                sEndingPage = 0;
                DrawQuestDetails(NULL);
            }
        }
        return;
    }
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        DestroyListMenuTask(sQuestListTask, NULL, NULL);
        sQuestFilter = (sQuestFilter + (JOY_NEW(DPAD_RIGHT) ? 1 : RF_QUEST_FILTER_COUNT - 1)) % RF_QUEST_FILTER_COUNT;
        PlaySE(SE_SELECT);
        BuildQuestList();
        return;
    }

    if (JOY_NEW(L_BUTTON | R_BUTTON))
    {
        DestroyListMenuTask(sQuestListTask, NULL, NULL);
        sQuestChapter = (sQuestChapter + (JOY_NEW(R_BUTTON) ? 1 : QUEST_CHAPTERS - 1)) % QUEST_CHAPTERS;
        PlaySE(SE_SELECT);
        BuildQuestList();
        return;
    }
    if (JOY_NEW(START_BUTTON) && RFQuests_IsUnlocked(sQuestSelected))
    {
        const struct RFQuest *quest = &sRFQuests[sQuestSelected];
        StringCopy(gStringVar4, COMPOUND_STRING("Premio: "));
        CopyItemName(quest->rewardItem, gStringVar4 + StringLength(gStringVar4));
        StringAppend(gStringVar4, COMPOUND_STRING(" x"));
        ConvertIntToDecimalStringN(gStringVar4 + StringLength(gStringVar4), quest->rewardCount, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringAppend(gStringVar4, COMPOUND_STRING("\nSe entrega una sola vez.\nSELECT: seguir este encargo."));
        DrawQuestDetails(gStringVar4);
        PlaySE(SE_SELECT);
        return;
    }
    if (JOY_NEW(SELECT_BUTTON) && RFQuests_IsUnlocked(sQuestSelected)
     && RFQuests_GetState(sQuestSelected) != RF_QUEST_CLAIMED)
    {
        gSaveBlock3Ptr->rfQuestTracked = gSaveBlock3Ptr->rfQuestTracked == sQuestSelected + 1 ? 0 : sQuestSelected + 1;
        PlaySE(SE_SELECT);
        RedrawListMenu(sQuestListTask);
        return;
    }
    input = ListMenu_ProcessInput(sQuestListTask);
    if (input == LIST_CANCEL)
    {
        u32 i;
        DestroyListMenuTask(sQuestListTask, NULL, NULL);
        for (i = 0; i < ARRAY_COUNT(sQuestWindowIds); i++)
        {
            ClearStdWindowAndFrame(sQuestWindowIds[i], TRUE);
            RemoveWindow(sQuestWindowIds[i]);
        }
        RFUI_EndFieldOverlay();
        ScheduleBgCopyTilemapToVram(0);
        sQuestMenuOpen = FALSE;
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        DestroyTask(taskId);
    }
    else if (input >= 0)
    {
        const struct RFQuest *quest = &sRFQuests[input];
        enum RFQuestState state = RFQuests_GetState(input);

        PlaySE(SE_SELECT);
        if (!RFQuests_IsUnlocked(input) && state != RF_QUEST_CLAIMED)
            return;
        if (state == RF_QUEST_AVAILABLE)
        {
            // The journal can only point to an authored scene; its giver explains it first.
            if (RFStory_GiverHint(input) != NULL)
            {
                PlaySE(SE_FAILURE);
                DrawQuestDetails(NULL);
                return;
            }
            RFQuests_Accept(input);
        }
        else if (state == RF_QUEST_CLAIMED)
        {
            if (RFStory_ReadEnding(input, 0, gStringVar4, 96))
            {
                sEndingPage = 1;
                DrawQuestDetails(gStringVar4);
            }
            return;
        }
        else if (state == RF_QUEST_READY)
        {
            // Add first, mark claimed second. A full bag never loses the reward.
            if (!RFQuests_Claim(input))
            {
                PlaySE(SE_FAILURE);
                DrawQuestDetails(COMPOUND_STRING("No hay espacio en la mochila.\nHaz sitio y reclama el premio aquí."));
                return;
            }
            DestroyListMenuTask(sQuestListTask, NULL, NULL);
            BuildQuestList();
            StringCopy(gStringVar4, COMPOUND_STRING("Premio: "));
            CopyItemName(quest->rewardItem, gStringVar4 + StringLength(gStringVar4));
            StringAppend(gStringVar4, COMPOUND_STRING(" x"));
            ConvertIntToDecimalStringN(gStringVar4 + StringLength(gStringVar4), quest->rewardCount, STR_CONV_MODE_LEFT_ALIGN, 2);
            DrawQuestDetails(gStringVar4);
            PlaySE(SE_EXP_MAX);
            return;
        }
        DestroyListMenuTask(sQuestListTask, NULL, NULL);
        BuildQuestList();
        DrawQuestDetails(NULL);
    }
}

void RFQuests_OpenMenu(void)
{
    u32 i;
    u32 freeTasks = 0;

    if (sQuestMenuOpen)
        return;
    // CreateTask returns 0 when full, so reserve capacity before opening either task.
    for (i = 0; i < NUM_TASKS; i++)
        if (!gTasks[i].isActive)
            freeTasks++;
    if (freeTasks < 3)
    {
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        return;
    }
    Achievements_HideNotification();
    RFQuests_Refresh();
    RFStory_EnsureSave();
    sQuestFilter = 0;
    sEndingPage = 0;
    sQuestChapter = 0;
    if (gSaveBlock3Ptr->rfQuestTracked != 0 && gSaveBlock3Ptr->rfQuestTracked <= RF_QUEST_COUNT)
        sQuestChapter = sRFQuests[gSaveBlock3Ptr->rfQuestTracked - 1].chapter;
    LoadMessageBoxAndBorderGfx();
    for (i = 0; i < ARRAY_COUNT(sQuestWindowIds); i++)
    {
        sQuestWindowIds[i] = AddWindow(&sQuestWindows[i]);
        if (sQuestWindowIds[i] == WINDOW_NONE)
        {
            while (i != 0)
            {
                --i;
                ClearStdWindowAndFrame(sQuestWindowIds[i], TRUE);
                RemoveWindow(sQuestWindowIds[i]);
            }
            ScheduleBgCopyTilemapToVram(0);
            ScriptUnfreezeObjectEvents();
            UnlockPlayerFieldControls();
            return;
        }
        DrawStdWindowFrame(sQuestWindowIds[i], FALSE);
    }
    RFUI_BeginFieldOverlay();
    sQuestMenuOpen = TRUE;
    BuildQuestList();
    CreateTask(Task_QuestMenu, 80);
}
