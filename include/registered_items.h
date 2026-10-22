#ifndef GUARD_REGISTERED_ITEMS_H
#define GUARD_REGISTERED_ITEMS_H

// Rojo Fuego+: up to four key items registered to SELECT. Slot 0 is the vanilla
// gSaveBlock1Ptr->registeredItem; the rest live in gSaveBlock3Ptr->extraRegisteredItems.
#define EXTRA_REGISTERED_ITEMS 3
#define MAX_REGISTERED_ITEMS   (EXTRA_REGISTERED_ITEMS + 1)

bool32 IsItemRegistered(enum Item itemId);
void ToggleRegisteredItem(enum Item itemId);
u32 GetUsableRegisteredItems(enum Item *items);
void RegisteredItems_OpenMenu(void);

// In item_menu.c: uses a registered item on the field (controls are already locked)
void UseRegisteredItemOnField(enum Item itemId);

#endif // GUARD_REGISTERED_ITEMS_H
