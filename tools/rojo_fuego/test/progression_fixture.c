// Disposable emulator fixture: never copy into a production build.
#include "global.h"
#include "main.h"
#include "pokemon.h"
#include "event_data.h"
#include "script.h"
#include "overworld.h"
#include "item.h"
#include "rf_traveler.h"
#include "rf_quests.h"
#include "move_relearner.h"
#include "key_system.h"
#include "event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "battle.h"
#include "battle_setup.h"
#include "constants/opponents_frlg.h"
#include "constants/moves.h"
#include "constants/flags.h"
#include "constants/vars.h"

extern const u8 RF_EventScript_Traveler[];
extern const u8 RF_EventScript_ResearchJournalExplain[];
extern const u8 RF_EventScript_Rematch_Brock[];
EWRAM_DATA u32 gRFProgressResult = 0;
EWRAM_DATA u32 gRFProgressStages = 0;
EWRAM_DATA u32 gRFProgressLearned = 0;
EWRAM_DATA u32 gRFProgressRematch = 0;
EWRAM_DATA u32 gRFProgressDebug[6] = {0};

const u32 gRFSaveLayout[] = {
    sizeof(struct SaveBlock1), sizeof(struct SaveBlock2), sizeof(struct SaveBlock3),
    sizeof(struct BoxPokemon), sizeof(struct Pokemon), sizeof(struct PokemonStorage),
    offsetof(struct PokemonStorage, boxes), offsetof(struct PokemonStorage, boxNames),
    offsetof(struct PokemonStorage, boxWallpapers), offsetof(struct PokemonStorage, fusions),
    SECTOR_DATA_SIZE, SAVE_BLOCK_3_CHUNK_SIZE * NUM_SECTORS_PER_SLOT,
    SECTOR_DATA_SIZE * 9,
};

void RFProgress_Traveler(void)
{
    u32 i;
    u16 berry = ITEM_SITRUS_BERRY, hp;
    if (RFQuests_GetState(RF_QUEST_FIELD_SUPPLIES) == RF_QUEST_READY
     && RFQuests_Claim(RF_QUEST_FIELD_SUPPLIES)
     && RFQuests_Accept(RF_QUEST_FIELD_CHALLENGE))
        gRFProgressStages |= 4;
    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_MEWTWO, 30, 31);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_PSYSTRIKE, 0);
    for (i = 1; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_NONE, i);
    CalculatePlayerPartyCount();
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM, &berry);
    hp = GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MAX_HP) / 2 - 1;
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &hp);
    for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; i++)
        FlagClear(i);
    FlagSet(FLAG_BADGE01_GET);
    FlagSet(FLAG_BADGE02_GET);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagClear(FLAG_SYS_GAME_CLEAR);
    FlagClear(FLAG_RF_DAILY_TRAVELER);
    VarSet(VAR_DAYS_FRLG, 1); // single battle, no bag
    RemoveBagItem(ITEM_GREAT_BALL, CountTotalItemQuantityInBag(ITEM_GREAT_BALL));
    Script_RF_TravelerClaimPrize(); // Initialize the appended reward save namespace.
    gSaveBlock3Ptr->rfTravelerPendingPrize = 0;
    SetMainCallback1(NULL);
    SetMainCallback2(CB2_Overworld);
    ScriptContext_SetupScript(RF_EventScript_Traveler);
}

void RFProgress_DisableBerry(void)
{
    KeySystem_Set(KEY_RESTORE_TRAINER_BERRIES, FALSE);
    SetMainCallback2(CB2_Overworld);
}

void RFProgress_Briefing(void)
{
    FlagSet(FLAG_SYS_POKEDEX_GET);
    gSaveBlock3Ptr->rfQuestSaveMagic = 0;
    if (RFQuests_Accept(RF_QUEST_FIELD_BRIEFING))
        gRFProgressStages = 1;
    SetMainCallback1(NULL);
    SetMainCallback2(CB2_Overworld);
    ScriptContext_SetupScript(RF_EventScript_ResearchJournalExplain);
}

void RFProgress_Delivery(void)
{
    if (RFQuests_GetState(RF_QUEST_FIELD_BRIEFING) == RF_QUEST_READY
     && RFQuests_Claim(RF_QUEST_FIELD_BRIEFING)
     && RFQuests_Accept(RF_QUEST_FIELD_SUPPLIES))
        gRFProgressStages |= 2;
    SetMainCallback2(CB2_Overworld);
    ScriptContext_SetupScript(RF_EventScript_ResearchJournalExplain);
}

void RFProgress_Report(void)
{
    if (RFQuests_GetState(RF_QUEST_FIELD_CHALLENGE) == RF_QUEST_READY
     && RFQuests_Claim(RF_QUEST_FIELD_CHALLENGE)
     && RFQuests_Accept(RF_QUEST_FIELD_REPORT))
        gRFProgressStages |= 8;
    SetMainCallback2(CB2_Overworld);
    ScriptContext_SetupScript(RF_EventScript_ResearchJournalExplain);
}

void RFProgress_CheckQuests(void)
{
    if (RFQuests_GetState(RF_QUEST_FIELD_REPORT) == RF_QUEST_READY
     && RFQuests_Claim(RF_QUEST_FIELD_REPORT)
     && CountTotalItemQuantityInBag(ITEM_POTION) == 0
     && CountTotalItemQuantityInBag(ITEM_SUPER_POTION) == 2)
        gRFProgressStages |= 16;
    SetMainCallback2(CB2_Overworld);
}

void RFProgress_Relearner(void)
{
    u32 i;
    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_VILEPLUME, 50, 31);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_GROWL, 0);
    for (i = 1; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_NONE, i);
    CalculatePlayerPartyCount();
    gSpecialVar_0x8004 = 0;
    gRelearnMode = RELEARN_MODE_SCRIPT;
    gMoveRelearnerState = MOVE_RELEARNER_LEVEL_UP_MOVES;
    SetMainCallback1(NULL);
    gMain.savedCallback = CB2_ReturnToField;
    SetMainCallback2(CB2_InitLearnMove);
}

void RFProgress_CheckRelearner(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    gRFProgressLearned = (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_VILEPLUME)
        | ((GetMonData(mon, MON_DATA_MOVE1) == MOVE_GROWL) << 1)
        | ((GetMonData(mon, MON_DATA_MOVE2) != MOVE_NONE) << 2)
        | ((GetMonData(mon, MON_DATA_LEVEL) == 50) << 3);
    // The process ends after this check; no player save is created.
    SetMainCallback2(CB2_Overworld);
}

void RFProgress_Rematch(void)
{
    u32 i;
    u16 testStat = 9999;
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; i++)
        FlagSet(i);
    FlagSet(FLAG_IS_CHAMPION);
    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_MEWTWO, 100, 31);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_SURF, 0);
    for (i = 1; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_NONE, i);
    CalculatePlayerPartyCount();
    // This checks event delivery, not boss balance. Boost only the disposable test party.
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MAX_HP, &testStat);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &testStat);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPATK, &testStat);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_DEF, &testStat);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPDEF, &testStat);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPEED, &testStat);
    RFQuests_Accept(RF_QUEST_REMATCH_BROCK);
    gSelectedObjectEvent = SpawnSpecialObjectEventParameterized(OBJ_EVENT_GFX_SCIENTIST,
        MOVEMENT_TYPE_FACE_DOWN, 250, player->currentCoords.x - 6, player->currentCoords.y - 7,
        player->currentElevation);
    gSpecialVar_LastTalked = 250;
    SetMainCallback2(CB2_Overworld);
    ScriptContext_SetupScript(RF_EventScript_Rematch_Brock);
}

void RFProgress_CheckRematch(void)
{
    gRFProgressDebug[0] = RFQuests_GetState(RF_QUEST_REMATCH_BROCK);
    gRFProgressDebug[1] = gBattleOutcome;
    gRFProgressDebug[2] = gBattleTypeFlags;
    gRFProgressDebug[3] = TRAINER_BATTLE_PARAM.opponentA;
    gRFProgressDebug[4] = TRAINER_RF_REMATCH_BROCK;
    gRFProgressDebug[5] = RFQuests_GetState(RF_QUEST_FIELD_REPORT);
    gRFProgressRematch = RFQuests_GetState(RF_QUEST_REMATCH_BROCK) == RF_QUEST_READY && !gMain.inBattle;
    SetMainCallback2(CB2_Overworld);
}

void RFProgress_CheckTraveler(void)
{
    gRFProgressResult = FlagGet(FLAG_RF_DAILY_TRAVELER)
        | ((CountTotalItemQuantityInBag(ITEM_GREAT_BALL) == 5) << 1)
        | ((!FlagGet(FLAG_RF_INVERSE_BATTLE) && VarGet(VAR_RF_NO_BAG_USE) == 0) << 2)
        | ((!gMain.inBattle) << 3)
        | ((gSaveBlock3Ptr->rfTravelerPendingPrize == 0) << 4)
        | ((GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM)
             == (KeySystem_IsOn(KEY_RESTORE_TRAINER_BERRIES) ? ITEM_SITRUS_BERRY : ITEM_NONE)) << 5);
    SetMainCallback2(CB2_Overworld);
}
