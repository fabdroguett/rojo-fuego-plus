#include "global.h"
#include "achievements.h"
#include "event_object_lock.h"
#include "item.h"
#include "list_menu.h"
#include "menu.h"
#include "registered_items.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/songs.h"

// Rojo Fuego+: SELECT with several registered key items opens a small list to pick one.

static u16 *GetSlot(u32 slot)
{
    if (slot == 0)
        return &gSaveBlock1Ptr->registeredItem;
    return &gSaveBlock3Ptr->extraRegisteredItems[slot - 1];
}

bool32 IsItemRegistered(enum Item itemId)
{
    u32 slot;

    if (itemId == ITEM_NONE)
        return FALSE;
    for (slot = 0; slot < MAX_REGISTERED_ITEMS; slot++)
    {
        if (*GetSlot(slot) == itemId)
            return TRUE;
    }
    return FALSE;
}

// Registers the item in the first free slot, or unregisters it if it was registered.
// With every slot taken, the oldest extra item makes room.
void ToggleRegisteredItem(enum Item itemId)
{
    u32 slot;

    for (slot = 0; slot < MAX_REGISTERED_ITEMS; slot++)
    {
        if (*GetSlot(slot) == itemId)
        {
            *GetSlot(slot) = ITEM_NONE;
            return;
        }
    }
    for (slot = 0; slot < MAX_REGISTERED_ITEMS; slot++)
    {
        if (*GetSlot(slot) == ITEM_NONE)
        {
            *GetSlot(slot) = itemId;
            return;
        }
    }
    for (slot = 1; slot < MAX_REGISTERED_ITEMS - 1; slot++)
        *GetSlot(slot) = *GetSlot(slot + 1);
    *GetSlot(MAX_REGISTERED_ITEMS - 1) = itemId;
}

// Fills items with the registered items still in the bag, forgetting the others
u32 GetUsableRegisteredItems(enum Item *items)
{
    u32 slot, count = 0;

    for (slot = 0; slot < MAX_REGISTERED_ITEMS; slot++)
    {
        u16 *registered = GetSlot(slot);
        if (*registered == ITEM_NONE)
            continue;
        if (!CheckBagHasItem(*registered, 1))
        {
            *registered = ITEM_NONE;
            continue;
        }
        items[count++] = *registered;
    }
    return count;
}

//// Selection menu ////

static const struct WindowTemplate sRegisteredItemsWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 18,
    .tilemapTop = 1,
    .width = 11,
    .height = MAX_REGISTERED_ITEMS * 2,
    .paletteNum = 15,
    .baseBlock = 1,
};

static EWRAM_DATA struct ListMenuItem sRegisteredListItems[MAX_REGISTERED_ITEMS] = {0};
static EWRAM_DATA u8 sRegisteredItemNames[MAX_REGISTERED_ITEMS][ITEM_NAME_LENGTH + 1] = {0};

#define tListTaskId data[0]
#define tWindowId   data[1]

static void CloseRegisteredItemsMenu(u8 taskId)
{
    DestroyListMenuTask(gTasks[taskId].tListTaskId, NULL, NULL);
    ClearStdWindowAndFrame(gTasks[taskId].tWindowId, TRUE);
    RemoveWindow(gTasks[taskId].tWindowId);
    DestroyTask(taskId);
}

static void Task_RegisteredItemsMenu(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    switch (input)
    {
    case LIST_NOTHING_CHOSEN:
        break;
    case LIST_CANCEL:
        PlaySE(SE_SELECT);
        CloseRegisteredItemsMenu(taskId);
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        break;
    default:
        PlaySE(SE_SELECT);
        CloseRegisteredItemsMenu(taskId);
        UseRegisteredItemOnField(input);
        break;
    }
}

void RegisteredItems_OpenMenu(void)
{
    struct ListMenuTemplate template = {0};
    enum Item items[MAX_REGISTERED_ITEMS];
    u32 i, count = GetUsableRegisteredItems(items);
    u8 taskId = CreateTask(Task_RegisteredItemsMenu, 80);

    Achievements_HideNotification(); // it uses the same part of the screen
    for (i = 0; i < count; i++)
    {
        CopyItemName(items[i], sRegisteredItemNames[i]);
        sRegisteredListItems[i].name = sRegisteredItemNames[i];
        sRegisteredListItems[i].id = items[i];
    }

    LoadMessageBoxAndBorderGfx();
    gTasks[taskId].tWindowId = AddWindow(&sRegisteredItemsWindowTemplate);
    DrawStdWindowFrame(gTasks[taskId].tWindowId, FALSE);
    template.items = sRegisteredListItems;
    template.totalItems = count;
    template.maxShowed = count;
    template.windowId = gTasks[taskId].tWindowId;
    template.item_X = 8;
    template.upText_Y = 1;
    template.cursorPal = 2;
    template.fillValue = 1;
    template.cursorShadowPal = 3;
    template.lettersSpacing = 1;
    template.scrollMultiple = LIST_NO_MULTIPLE_SCROLL;
    template.fontId = FONT_NORMAL;
    gTasks[taskId].tListTaskId = ListMenuInit(&template, 0, 0);
    CopyWindowToVram(gTasks[taskId].tWindowId, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
}
