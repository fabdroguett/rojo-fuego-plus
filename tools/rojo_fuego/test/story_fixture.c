// Disposable synthetic scenario data. Never compile into a production ROM.
#include "global.h"
#include "rf_story.h"
#include "rf_quests.h"
#include "key_system.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokedex.h"
#include "item.h"
#include "script.h"
#include "save.h"
#include "achievements.h"
#include "constants/species.h"
#include "constants/moves.h"
#include "constants/items.h"
#include "constants/maps.h"

struct RFStoryFixtureActor {u16 map,x,y,lx,ly;u8 local;const u8 *script;};
#include "data/rf_story_fixture_actors.h"
EWRAM_DATA u16 gRFStoryFixtureParams[8] = {0};
EWRAM_DATA u32 gRFStoryFixtureResult[20] = {0};

static void Mark(u32 id,u32 state)
{
    u32 shift=(id%4)*2;
    gSaveBlock3Ptr->rfQuestStates[id/4]=(gSaveBlock3Ptr->rfQuestStates[id/4]&~(3<<shift))|(state<<shift);
}

void RFStoryFixture_Setup(void)
{
    static const u16 flags[]={FLAG_SYS_POKEDEX_GET,FLAG_SYS_POKEMON_GET,FLAG_IS_CHAMPION,FLAG_SYS_GAME_CLEAR,
        FLAG_BADGE01_GET,FLAG_BADGE02_GET,FLAG_BADGE03_GET,FLAG_BADGE04_GET,FLAG_BADGE05_GET,
        FLAG_BADGE06_GET,FLAG_BADGE07_GET,FLAG_BADGE08_GET,FLAG_RESCUED_MR_FUJI,FLAG_RESCUED_LOSTELLE};
    u32 i;
    for(i=0;i<ARRAY_COUNT(flags);i++) FlagSet(flags[i]);
    RFQuests_GetState(0);
    RFStory_EnsureSave();
    ZeroPlayerPartyMons();
    CreateRandomMonWithIVs(&gParties[B_TRAINER_PLAYER][0],SPECIES_MEWTWO,100,31);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0],MOVE_PSYCHIC,0);
    CalculatePlayerPartyCount();
    ClearBag();
    AddBagItem(ITEM_POTION,5);AddBagItem(ITEM_ANTIDOTE,5);AddBagItem(ITEM_POKE_DOLL,2);
    AddBagItem(ITEM_REPEL,3);AddBagItem(ITEM_SUPER_REPEL,5);AddBagItem(ITEM_PEARL,3);
    AddBagItem(ITEM_MOON_STONE,1);
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB,3);
    KeySystem_Set(KEY_DARK_MODE,gRFStoryFixtureParams[3]==0);
    Achievements_HideNotification();
    SetMainCallback1(NULL);SetMainCallback2(CB2_Overworld);
}

void RFStoryFixture_PrepareQuest(void)
{
    u32 id=gRFStoryFixtureParams[0], parent,guard=0;
    if(id<RF_QUEST_COUNT)
    {
        parent=RFQuests_GetCatalog()[id].prerequisite;
        while(parent!=RF_QUEST_NONE && guard++<RF_QUEST_COUNT)
        {
            Mark(parent,RF_QUEST_CLAIMED);
            parent=RFQuests_GetCatalog()[parent].prerequisite;
        }
        Mark(id,RF_QUEST_AVAILABLE);
        if(gRFStoryFixtureParams[2]==1) RFQuests_Accept(id);
        if(gRFStoryFixtureParams[2]==2) Mark(id,RF_QUEST_CLAIMED);
    }
    SetMainCallback2(CB2_Overworld);
}

void RFStoryFixture_WarpActor(void)
{
    u32 actor=gRFStoryFixtureParams[1];
    if(actor<ARRAY_COUNT(sFixtureActors))
    {
        const struct RFStoryFixtureActor *a=&sFixtureActors[actor];
        SetWarpDestination(MAP_GROUP(a->map),MAP_NUM(a->map),-1,a->lx,a->ly);
        SetMainCallback1(NULL);WarpIntoMap();
        // SetWarpDestination takes s8 coordinates: rows past 127 (Kindle Road y=133) wrap
        // negative and the game falls back to the map centre. Apply the full 16-bit tile.
        gSaveBlock1Ptr->location.x=gSaveBlock1Ptr->pos.x=a->lx;
        gSaveBlock1Ptr->location.y=gSaveBlock1Ptr->pos.y=a->ly;
        SetMainCallback2(CB2_LoadMap);
    }
    else SetMainCallback2(CB2_Overworld);
}

void RFStoryFixture_FaceActor(void)
{
    u32 actor=gRFStoryFixtureParams[1];
    if(actor<ARRAY_COUNT(sFixtureActors))
    {
        const struct RFStoryFixtureActor *a=&sFixtureActors[actor];
        enum Direction direction=a->lx<a->x?DIR_EAST:a->lx>a->x?DIR_WEST:a->ly<a->y?DIR_SOUTH:DIR_NORTH;
        ObjectEventTurn(&gObjectEvents[gPlayerAvatar.objectEventId],direction);
    }
    Achievements_HideNotification();
    // Keep field input alive: outdoor maps finish loading before this call, so clearing
    // callback1 here would leave the player unable to talk to the actor.
    SetMainCallback1(CB1_Overworld);SetMainCallback2(CB2_Overworld);
}

void RFStoryFixture_ScriptActor(void)
{
    u8 objectId;
    if(gRFStoryFixtureParams[1]<ARRAY_COUNT(sFixtureActors))
    {
        const struct RFStoryFixtureActor *a=&sFixtureActors[gRFStoryFixtureParams[1]];
        if(!TryGetObjectEventIdByLocalIdAndMap(a->local,MAP_NUM(a->map),MAP_GROUP(a->map),&objectId))
            gSelectedObjectEvent=objectId;
        VarSet(VAR_LAST_TALKED,a->local);
        ScriptContext_SetupScript(sFixtureActors[gRFStoryFixtureParams[1]].script);
    }
    SetMainCallback1(NULL);SetMainCallback2(CB2_Overworld);
}

void RFStoryFixture_Read(void)
{
    u8 objectId=0;
    u32 id=gRFStoryFixtureParams[0], actor=gRFStoryFixtureParams[1];
    gRFStoryFixtureResult[0]=RFQuests_GetState(id);
    gRFStoryFixtureResult[1]=RFStory_ClueCount(id);
    gRFStoryFixtureResult[2]=gSaveBlock3Ptr->rfStoryClues;
    gRFStoryFixtureResult[3]=gSaveBlock3Ptr->rfStoryContractReady;
    gRFStoryFixtureResult[4]=gSaveBlock3Ptr->rfStoryContractClaimed;
    gRFStoryFixtureResult[5]=gSaveBlock3Ptr->rfStoryContractDay;
    gRFStoryFixtureResult[6]=gSaveBlock3Ptr->rfQuestTracked;
    gRFStoryFixtureResult[7]=gSaveBlock3Ptr->rfStorySaveMagic;
    gRFStoryFixtureResult[8]=sizeof(struct SaveBlock3);
    gRFStoryFixtureResult[9]=offsetof(struct SaveBlock3,rfStorySaveMagic);
    gRFStoryFixtureResult[10]=offsetof(struct SaveBlock3,rfQuestStates);
    gRFStoryFixtureResult[11]=gSaveBlock3Ptr->achievementsUnlocked[39]&0xC0;
    gRFStoryFixtureResult[12]=actor<ARRAY_COUNT(sFixtureActors)
        && !TryGetObjectEventIdByLocalIdAndMap(sFixtureActors[actor].local,MAP_NUM(sFixtureActors[actor].map),MAP_GROUP(sFixtureActors[actor].map),&objectId);
    gRFStoryFixtureResult[13]=ArePlayerFieldControlsLocked();
    gRFStoryFixtureResult[14]=RFQuests_GetState(349);
    // Diagnostics for actors that fail to appear: active objects, player tile, engine hiding.
    gRFStoryFixtureResult[16]=0;
    for(objectId=0;objectId<OBJECT_EVENTS_COUNT;objectId++) gRFStoryFixtureResult[16]+=gObjectEvents[objectId].active;
    gRFStoryFixtureResult[17]=(u16)gSaveBlock1Ptr->pos.x|((u32)(u16)gSaveBlock1Ptr->pos.y<<16);
    gRFStoryFixtureResult[18]=gSaveBlock1Ptr->location.mapGroup<<8|gSaveBlock1Ptr->location.mapNum;
    gRFStoryFixtureResult[19]=actor<ARRAY_COUNT(sFixtureActors) && RFStory_IsObjectHidden(sFixtureActors[actor].script);
    SetMainCallback2(CB2_Overworld);
}

void RFStoryFixture_Journal(void)
{
    gSaveBlock3Ptr->rfQuestTracked=gRFStoryFixtureParams[0]+1;
    SetMainCallback1(NULL);SetMainCallback2(CB2_Overworld);
    LockPlayerFieldControls();
    RFQuests_OpenMenu();
}

void RFStoryFixture_CompleteStories(void)
{
    u32 i;
    for(i=350;i<414;i++) Mark(i,RF_QUEST_CLAIMED);
    RFStory_OnClaim(413);
    SetMainCallback2(CB2_Overworld);
}

void RFStoryFixture_SaveRoundTrip(void)
{
    RFStory_EnsureSave();
    Mark(0,RF_QUEST_CLAIMED);Mark(349,RF_QUEST_ACTIVE);
    gSaveBlock3Ptr->rfStoryClues=0x15555;
    gSaveBlock3Ptr->rfStoryContractDay=77;
    gSaveBlock3Ptr->rfStoryContractReady=2;
    gSaveBlock3Ptr->rfStoryContractClaimed=1;
    // Quest extension: decisions, a counter slot and the state bytes of IDs 500-649.
    RFQuests_SetChoice(5);RFQuests_SetChoice(31);
    gSaveBlock3Ptr->rfQuestCounters[2]=0xA9C3;
    gSaveBlock3Ptr->rfQuestStatesExt[0]=0x1B;gSaveBlock3Ptr->rfQuestStatesExt[RF_QUEST_EXT_STATE_BYTES-1]=0x0E;
    gRFStoryFixtureResult[15]=TrySavingData(SAVE_NORMAL);
    if(gRFStoryFixtureResult[15]==SAVE_STATUS_OK)
    {
        u8 shipped[RF_QUEST_STATE_BYTES];
        gSaveBlock3Ptr->rfStoryClues=0;
        gSaveBlock3Ptr->rfStoryContractReady=0;
        Mark(0,0);Mark(349,0);
        gSaveBlock3Ptr->rfQuestChoices=0;gSaveBlock3Ptr->rfQuestCounters[2]=0;
        gSaveBlock3Ptr->rfQuestStatesExt[0]=0;gSaveBlock3Ptr->rfQuestStatesExt[RF_QUEST_EXT_STATE_BYTES-1]=0;
        gRFStoryFixtureResult[15]=LoadGameSave(SAVE_NORMAL)==SAVE_STATUS_OK
            && gSaveBlock3Ptr->rfStoryClues==0x15555
            && gSaveBlock3Ptr->rfStoryContractDay==77
            && gSaveBlock3Ptr->rfStoryContractReady==2
            && gSaveBlock3Ptr->rfStoryContractClaimed==1
            && RFQuests_GetState(0)==RF_QUEST_CLAIMED && RFQuests_GetState(349)==RF_QUEST_ACTIVE
            && gSaveBlock3Ptr->rfQuestChoices==((1u<<5)|(1u<<31))
            && gSaveBlock3Ptr->rfQuestCounters[2]==0xA9C3
            && gSaveBlock3Ptr->rfQuestStatesExt[0]==0x1B
            && gSaveBlock3Ptr->rfQuestStatesExt[RF_QUEST_EXT_STATE_BYTES-1]==0x0E;
        // A save written before the extension: its magic is garbage. Only the extension resets.
        memcpy(shipped,gSaveBlock3Ptr->rfQuestStates,sizeof(shipped));
        gSaveBlock3Ptr->rfQuestExtMagic=0xA5A5A5A5;
        gRFStoryFixtureResult[15]=gRFStoryFixtureResult[15]
            && RFQuests_GetState(0)==RF_QUEST_CLAIMED && RFQuests_GetState(349)==RF_QUEST_ACTIVE
            && memcmp(shipped,gSaveBlock3Ptr->rfQuestStates,sizeof(shipped))==0
            && gSaveBlock3Ptr->rfQuestChoices==0 && gSaveBlock3Ptr->rfQuestCounters[2]==0
            && gSaveBlock3Ptr->rfQuestStatesExt[0]==0
            && gSaveBlock3Ptr->rfStoryClues==0x15555;
    }
    else gRFStoryFixtureResult[15]=0;
    SetMainCallback2(CB2_Overworld);
}
