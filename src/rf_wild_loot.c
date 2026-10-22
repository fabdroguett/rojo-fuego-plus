#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "item.h"
#include "key_system.h"
#include "pokemon.h"
#include "random.h"
#include "rf_wild_loot.h"

#define RF_LOOT_SAVE_MAGIC 0x4C4F5431

static void EnsureLootSave(void)
{
    if (gSaveBlock3Ptr->rfLootSaveMagic != RF_LOOT_SAVE_MAGIC)
    {
        memset(gSaveBlock3Ptr->rfPendingLoot, 0, sizeof(gSaveBlock3Ptr->rfPendingLoot));
        gSaveBlock3Ptr->rfLootSaveMagic = RF_LOOT_SAVE_MAGIC;
    }
}

static bool32 IsLootItem(enum Item item)
{
    return item != ITEM_NONE && item < ITEMS_COUNT
        && GetItemImportance(item) == 0 && GetItemPocket(item) != POCKET_KEY_ITEMS;
}

u32 RFWildLoot_TryClaim(void)
{
    u32 i, claimed = 0;
    if (!IS_FRLG)
        return 0;
    EnsureLootSave();
    for (i = 0; i < ARRAY_COUNT(gSaveBlock3Ptr->rfPendingLoot); i++)
    {
        enum Item item = gSaveBlock3Ptr->rfPendingLoot[i];
        if (!IsLootItem(item))
            gSaveBlock3Ptr->rfPendingLoot[i] = ITEM_NONE;
        else if (AddBagItem(item, 1))
        {
            gSaveBlock3Ptr->rfPendingLoot[i] = ITEM_NONE;
            claimed++;
        }
    }
    return claimed;
}

enum RFLootResult RFWildLoot_Collect(void)
{
    u32 i, slot;
    enum RFLootResult result = RF_LOOT_NONE;
    const u32 excluded = BATTLE_TYPE_TRAINER | BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED
        | BATTLE_TYPE_RECORDED_LINK | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_SAFARI
        | BATTLE_TYPE_FIRST_BATTLE | BATTLE_TYPE_CATCH_TUTORIAL | BATTLE_TYPE_EREADER_TRAINER
        | BATTLE_TYPE_TRAINER_HILL | BATTLE_TYPE_INGAME_PARTNER;

    if (!IS_FRLG || gBattleOutcome != B_OUTCOME_WON || (gBattleTypeFlags & excluded))
        return RF_LOOT_NONE;
    // Already earned loot remains claimable even after turning off new drops.
    if (RFWildLoot_TryClaim())
        result = RF_LOOT_BAG;
    if (!KeySystem_IsOn(KEY_WILD_LOOT))
        return result;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];
        struct LostItem *lost = GetLostItemState(B_TRAINER_OPPONENT_A, i);
        enum Item item = GetMonData(mon, MON_DATA_HELD_ITEM);
        enum Item none = ITEM_NONE;
        if (!gBattleStruct->partyState[B_TRAINER_OPPONENT_A][i].sentOut
         || GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE
         || GetMonData(mon, MON_DATA_HP) != 0 || !IsLootItem(item)
         || item != lost->originalItem || lost->stolen || lost->wildItemPending
         || gBattleStruct->partyState[B_TRAINER_OPPONENT_A][i].usedHeldItem != ITEM_NONE)
            continue;
        for (slot = 0; slot < ARRAY_COUNT(gSaveBlock3Ptr->rfPendingLoot); slot++)
            if (gSaveBlock3Ptr->rfPendingLoot[slot] == ITEM_NONE)
                break;
        // Reserve capacity before rolling; never discard a successfully awarded drop.
        if (slot == ARRAY_COUNT(gSaveBlock3Ptr->rfPendingLoot) || (Random() % 4) != 0)
            continue;
        if (AddBagItem(item, 1))
        {
            if (result == RF_LOOT_NONE)
                result = RF_LOOT_BAG;
        }
        else
        {
            gSaveBlock3Ptr->rfPendingLoot[slot] = item;
            result = RF_LOOT_PENDING;
        }
        // Ownership transfers once. End-of-battle restoration must not recreate this item.
        SetMonData(mon, MON_DATA_HELD_ITEM, &none);
        lost->originalItem = ITEM_NONE;
        lost->restoreAfterBattle = FALSE;
    }
    return result;
}
