#include "global.h"
#include "rf_story.h"
#include "rf_quests.h"
#include "event_data.h"
#include "item.h"
#include "main.h"
#include "rtc.h"
#include "string_util.h"
#include "battle.h"
#include "battle_setup.h"
#include "achievements.h"
#include "constants/items.h"
#include "constants/opponents_frlg.h"
#include "constants/characters.h"
#include "constants/maps.h"

#define STORY_MAGIC 0x53545231

enum { RF_STORY_TALK, RF_STORY_DELIVERY, RF_STORY_CLUES, RF_STORY_BATTLE, RF_STORY_RACE, RF_STORY_PUZZLE };
struct RFStoryActor
{
    u16 id, map;
    const u8 *script;
    u16 clueQuest;
    u8 clueBit;
    u16 afterQuest;
    const u8 *greeting;
    const u8 *hint;
};
struct RFStoryQuestion
{
    const u8 *text;
    u8 answer; // expected YES (1) or NO (0)
};
struct RFStoryRace
{
    u16 seconds;
    const u8 *start, *late, *restart;
};
struct RFStoryScene
{
    u8 giver, target, kind;
    u16 trainer;
    u32 clues;
    const u8 *offer;
    const u8 *ending;
    const struct RFStoryQuestion *questions; // RF_STORY_PUZZLE only
    u8 questionCount;
    const struct RFStoryRace *race; // RF_STORY_RACE only
};
struct RFStoryContract
{
    u8 actor;
    u16 unlockQuest, item, quantity, reward, rewardQuantity;
    const u8 *name;
};
#include "data/rf_story.h"

STATIC_ASSERT(ARRAY_COUNT(sScenes) == RF_STORY_COUNT, RFStorySceneCount);
STATIC_ASSERT(ARRAY_COUNT(sActors) == RF_STORY_ACTOR_COUNT, RFStoryActorCount);
static EWRAM_DATA u16 sRaceQuest = 0; // ID + 1; the EWRAM .sbss section must start at zero.
static EWRAM_DATA u32 sRaceStart = 0;
static EWRAM_DATA u8 sPuzzleStep = 0;
static bool32 IsScene(u32 id);
STATIC_ASSERT(ACH_BYTES == 40, RFStoryAchievementSaveOffsetsStable);

void RFStory_EnsureSave(void)
{
    // Append-only extension: never reset or reinterpret the shipped quest slots.
    if (gSaveBlock3Ptr->rfStorySaveMagic != STORY_MAGIC)
    {
        gSaveBlock3Ptr->rfStoryClues = 0;
        gSaveBlock3Ptr->rfStoryContractDay = 0;
        gSaveBlock3Ptr->rfStoryContractReady = 0;
        gSaveBlock3Ptr->rfStoryContractClaimed = 0;
        // Two new achievements fit the unused high bits of the existing 40-byte arrays.
        gSaveBlock3Ptr->achievementsUnlocked[39] &= 0x3F;
        gSaveBlock3Ptr->achievementsClaimed[39] &= 0x3F;
        gSaveBlock3Ptr->rfStorySaveMagic = STORY_MAGIC;
    }
}

void RFStory_OnClaim(u32 quest)
{
    u32 i, finished = 0;
    if (!IsScene(quest))
        return;
    RFStory_EnsureSave();
    for (i = 0; i < 8; i++)
        finished += RFQuests_GetState(RF_STORY_FIRST + i * 8 + 7) == RF_QUEST_CLAIMED;
    if (finished)
        Achievement_Unlock(ACH_RF_STORY_FIRST);
    if (finished == 8)
        Achievement_Unlock(ACH_RF_STORY_ALL);
}

static bool32 IsScene(u32 id)
{
    return id >= RF_STORY_FIRST && id < RF_STORY_FIRST + RF_STORY_COUNT;
}

static bool32 AtActor(u32 actor)
{
    return actor < ARRAY_COUNT(sActors)
        && MAP_GROUP(sActors[actor].map) == gSaveBlock1Ptr->location.mapGroup
        && MAP_NUM(sActors[actor].map) == gSaveBlock1Ptr->location.mapNum;
}

u32 RFStory_TargetMap(u32 quest)
{
    const struct RFQuest *q;
    if (quest >= RF_QUEST_COUNT)
        return 0;
    if (IsScene(quest))
        return sActors[sScenes[quest - RF_STORY_FIRST].target].map;
    q = &RFQuests_GetCatalog()[quest];
    return q->condition == QUEST_VISIT ? q->argument : MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB;
}

u32 RFStory_Difficulty(u32 quest)
{
    const struct RFQuest *q;
    u32 result = 1, depth;
    if (quest >= RF_QUEST_COUNT)
        return 0;
    for (depth = 0; depth < 8 && quest < RF_QUEST_COUNT; depth++)
    {
        q = &RFQuests_GetCatalog()[quest];
        if (q->requiredFlag == FLAG_IS_CHAMPION || q->requiredFlag == FLAG_SYS_GAME_CLEAR
         || q->condition == QUEST_LEGENDS || q->chapter == 1)
            return 3;
        if (q->requiredFlag != 0 || (!IsScene(quest) && q->chapter >= 4))
            result = 2;
        if (!IsScene(quest))
            break;
        quest = q->prerequisite;
    }
    return result;
}

bool32 RFStory_MatchesFilter(u32 quest, u32 filter)
{
    u32 state;
    bool32 unlocked;
    if (quest >= RF_QUEST_COUNT)
        return FALSE;
    state = RFQuests_GetState(quest);
    unlocked = RFQuests_IsUnlocked(quest);
    if (!unlocked && state == RF_QUEST_AVAILABLE)
        return FALSE; // No undiscovered names or future plot rows.
    switch (filter)
    {
    case RF_QUEST_FILTER_AVAILABLE: return unlocked && state == RF_QUEST_AVAILABLE;
    case RF_QUEST_FILTER_ACTIVE: return state == RF_QUEST_ACTIVE;
    case RF_QUEST_FILTER_READY: return state == RF_QUEST_READY;
    case RF_QUEST_FILTER_CLAIMED: return state == RF_QUEST_CLAIMED;
    case RF_QUEST_FILTER_HERE:
        return RFStory_TargetMap(quest) == (u32)(gSaveBlock1Ptr->location.mapNum | (gSaveBlock1Ptr->location.mapGroup << 8));
    case RF_QUEST_FILTER_EASY: return RFStory_Difficulty(quest) == 1;
    case RF_QUEST_FILTER_MEDIUM: return RFStory_Difficulty(quest) == 2;
    case RF_QUEST_FILTER_HARD: return RFStory_Difficulty(quest) == 3;
    }
    return TRUE;
}

const u8 *RFStory_Resolution(u32 quest)
{
    return IsScene(quest) ? sScenes[quest - RF_STORY_FIRST].ending
        : COMPOUND_STRING("El informe está archivado.\nTu trabajo conserva sus resultados\ny no vuelve a entregar premios.");
}

bool32 RFStory_ReadEnding(u32 quest, u32 page, u8 *dest, u32 capacity)
{
    const u8 *text;
    u32 i = 0;
    if (capacity == 0)
        return FALSE;
    dest[0] = EOS;
    if (quest >= RF_QUEST_COUNT || RFQuests_GetState(quest) != RF_QUEST_CLAIMED)
        return FALSE;
    text = RFStory_Resolution(quest);
    while (page && *text != EOS)
        if (*text++ == CHAR_PROMPT_CLEAR)
            page--;
    if (page || *text == EOS)
        return FALSE;
    while (*text != EOS && *text != CHAR_PROMPT_CLEAR && i + 1 < capacity)
        dest[i++] = *text++;
    dest[i] = EOS;
    return TRUE;
}

const u8 *RFStory_GiverHint(u32 quest)
{
    return IsScene(quest) ? sActors[sScenes[quest - RF_STORY_FIRST].giver].hint : NULL;
}

static bool32 RaceRunning(u32 id)
{
    const struct RFStoryRace *race = sScenes[id - RF_STORY_FIRST].race;
    return race != NULL && sRaceQuest == id + 1
        && gMain.vblankCounter1 - sRaceStart <= (u32)race->seconds * 60;
}

const u8 *RFStory_Hint(u32 quest)
{
    u32 i;
    if (!IsScene(quest))
        return quest < RF_QUEST_COUNT ? RFQuests_GetCatalog()[quest].description
            : COMPOUND_STRING("Explora y habla con los vecinos.");
    RFStory_EnsureSave();
    for (i = 0; i < ARRAY_COUNT(sActors); i++)
        if (sActors[i].clueQuest == quest && sActors[i].clueBit < 32
         && !(gSaveBlock3Ptr->rfStoryClues & (1u << sActors[i].clueBit)))
            return sActors[i].hint;
    if (sScenes[quest - RF_STORY_FIRST].kind == RF_STORY_CLUES)
        return sActors[sScenes[quest - RF_STORY_FIRST].target].hint;
    return RFQuests_GetCatalog()[quest].description;
}

bool32 RFStory_IsObjectHidden(const u8 *script)
{
    u32 i;
    if (!IS_FRLG || script == NULL)
        return FALSE;
    for (i = 0; i < ARRAY_COUNT(sActors); i++)
    {
        const struct RFStoryActor *actor = &sActors[i];
        if (actor->script != script)
            continue;
        if (!FlagGet(FLAG_SYS_POKEDEX_GET))
            return TRUE;
        RFStory_EnsureSave();
        if (actor->clueBit < 32)
            return RFQuests_GetState(actor->clueQuest) != RF_QUEST_ACTIVE
                || (gSaveBlock3Ptr->rfStoryClues & (1u << actor->clueBit));
        return actor->afterQuest != RF_QUEST_NONE
            && RFQuests_GetState(actor->afterQuest) != RF_QUEST_CLAIMED;
    }
    return FALSE;
}

bool32 RFStory_RecordClue(u32 actor)
{
    const struct RFStoryActor *a;
    if (!AtActor(actor))
        return FALSE;
    a = &sActors[actor];
    if (a->clueBit >= 32 || RFQuests_GetState(a->clueQuest) != RF_QUEST_ACTIVE)
        return FALSE;
    RFStory_EnsureSave();
    gSaveBlock3Ptr->rfStoryClues |= 1u << a->clueBit;
    return TRUE;
}

u32 RFStory_ClueCount(u32 quest)
{
    u32 bits, count = 0;
    if (!IsScene(quest))
        return 0;
    RFStory_EnsureSave();
    bits = gSaveBlock3Ptr->rfStoryClues & sScenes[quest - RF_STORY_FIRST].clues;
    while (bits)
    {
        count += bits & 1;
        bits >>= 1;
    }
    return count;
}

u32 RFStory_DeliverContract(u32 contract, u32 day)
{
    const struct RFStoryContract *c;
    u32 mask;
    if (contract >= ARRAY_COUNT(sContracts))
        return 0;
    c = &sContracts[contract];
    if (RFQuests_GetState(c->unlockQuest) != RF_QUEST_CLAIMED)
        return 0;
    RFStory_EnsureSave();
    day = min(day, 0xFFFF);
    // Moving the clock backwards cannot replenish contracts. Pending deliveries survive midnight.
    if (day > gSaveBlock3Ptr->rfStoryContractDay)
    {
        gSaveBlock3Ptr->rfStoryContractDay = day;
        gSaveBlock3Ptr->rfStoryContractClaimed = 0;
    }
    mask = 1u << contract;
    if (gSaveBlock3Ptr->rfStoryContractClaimed & mask)
        return 3;
    if (!(gSaveBlock3Ptr->rfStoryContractReady & mask))
    {
        if (!CheckBagHasItem(c->item, c->quantity) || !RemoveBagItem(c->item, c->quantity))
            return 4;
        gSaveBlock3Ptr->rfStoryContractReady |= mask;
    }
    if (!AddBagItem(c->reward, c->rewardQuantity))
        return 1;
    gSaveBlock3Ptr->rfStoryContractReady &= ~mask;
    gSaveBlock3Ptr->rfStoryContractClaimed |= mask;
    return 2;
}

static void FinishScene(u32 id)
{
    RFQuests_NotifyEvent(RF_STORY_EVENT, id);
    if (RFQuests_GetState(id) == RF_QUEST_READY)
    {
        if (RFQuests_Claim(id))
            StringCopy(gStringVar4, RFStory_Resolution(id));
        else
            StringCopy(gStringVar4, COMPOUND_STRING("El informe está completo.\nTu premio sigue pendiente.\pHaz sitio en la mochila y vuelve\no reclámalo en el diario."));
    }
    else
        StringCopy(gStringVar4, COMPOUND_STRING("Aún falta lo indicado en el diario.\nPuedes volver cuando lo tengas.\pNo entregas ni pierdes a ninguno\nde tus Pokémon."));
}

static void PuzzleQuestion(u32 id)
{
    StringCopy(gStringVar4, sScenes[id - RF_STORY_FIRST].questions[sPuzzleStep].text);
}

void Script_RFStoryPrepare(void)
{
    u32 i, actor = gSpecialVar_0x8004;
    u32 state;
    u32 pendingTarget = RF_QUEST_NONE;
    bool32 hasActiveGuidance = FALSE;
    gSpecialVar_Result = 0;
    gSpecialVar_0x8005 = RF_QUEST_NONE;
    gSpecialVar_0x8006 = RF_QUEST_NONE;
    gSpecialVar_0x8007 = 0;
    if (!AtActor(actor))
    {
        StringCopy(gStringVar4, COMPOUND_STRING("Este puesto está en otra zona."));
        return;
    }
    RFStory_EnsureSave();
    StringCopy(gStringVar4, sActors[actor].greeting);
    if (sActors[actor].clueBit < 32)
    {
        if (RFStory_RecordClue(actor))
        {
            StringAppend(gStringVar4, COMPOUND_STRING("\pApunte añadido al cuaderno.\nEl original se queda en su lugar."));
            gSpecialVar_0x8007 = 1;
        }
        return;
    }
    // Reaching the timekeeper wins the race before an unrelated conversation can consume time.
    if (sRaceQuest && IsScene(sRaceQuest - 1))
    {
        u32 id = sRaceQuest - 1;
        if (sScenes[id - RF_STORY_FIRST].target == actor
         && RFQuests_GetState(id) == RF_QUEST_ACTIVE)
        {
            bool32 inTime = RaceRunning(id);
            sRaceQuest = 0;
            gSpecialVar_0x8005 = id;
            if (inTime)
                FinishScene(id);
            else
                StringCopy(gStringVar4, sScenes[id - RF_STORY_FIRST].race->late);
            return;
        }
    }
    for (i = 0; i < RF_STORY_COUNT; i++)
    {
        const struct RFStoryScene *scene = &sScenes[i];
        u32 id = i + RF_STORY_FIRST;
        state = RFQuests_GetState(id);
        if (state == RF_QUEST_READY && (scene->target == actor || scene->giver == actor))
        {
            gSpecialVar_0x8005 = id;
            StringCopy(gStringVar4, COMPOUND_STRING("Tu informe está listo.\n¿Quieres recibir el premio?"));
            gSpecialVar_Result = 2;
            return;
        }
        if (state != RF_QUEST_ACTIVE || scene->target != actor)
            continue;
        // Independent campaigns can share a guide. An unmet older objective must
        // never prevent reporting another campaign or receiving a new commission.
        if (!RFQuests_HasRequirements(id)
         || (scene->kind == RF_STORY_CLUES && (gSaveBlock3Ptr->rfStoryClues & scene->clues) != scene->clues)
         || (scene->kind == RF_STORY_DELIVERY && !CheckBagHasItem(RFQuests_GetCatalog()[id].argument, RFQuests_GetCatalog()[id].alternative))
         || (scene->kind == RF_STORY_RACE && !RaceRunning(id)))
        {
            if (pendingTarget == RF_QUEST_NONE || gSaveBlock3Ptr->rfQuestTracked == id + 1)
                pendingTarget = id;
            continue;
        }
        gSpecialVar_0x8005 = id;
        switch (scene->kind)
        {
        case RF_STORY_DELIVERY:
            StringCopy(gStringVar4, RFQuests_GetCatalog()[id].description);
            StringAppend(gStringVar4, COMPOUND_STRING("\p¿Confirmas la entrega?\nSolo se retira lo solicitado."));
            gSpecialVar_Result = 2;
            return;
        case RF_STORY_CLUES:
            if ((gSaveBlock3Ptr->rfStoryClues & scene->clues) != scene->clues)
            {
                StringCopy(gStringVar4, scene->offer);
                StringAppend(gStringVar4, COMPOUND_STRING("\pSiguiente apunte pendiente:\n"));
                StringAppend(gStringVar4, RFStory_Hint(id));
                return;
            }
            break;
        case RF_STORY_BATTLE:
            StringCopy(gStringVar4, COMPOUND_STRING("¿Preparado para la práctica?\nPuedes volver si no lo estás."));
            gSpecialVar_Result = 4;
            return;
        case RF_STORY_PUZZLE:
            sPuzzleStep = 0;
            PuzzleQuestion(id);
            gSpecialVar_Result = 3;
            return;
        case RF_STORY_RACE:
            if (!RaceRunning(id))
            {
                StringCopy(gStringVar4, scene->race->restart);
                return;
            }
            sRaceQuest = 0;
            break;
        }
        FinishScene(id);
        return;
    }
    for (i = 0; i < RF_STORY_COUNT; i++)
    {
        const struct RFStoryScene *scene = &sScenes[i];
        u32 id = RF_STORY_FIRST + i;
        if (scene->giver != actor || !RFQuests_IsUnlocked(id))
            continue;
        state = RFQuests_GetState(id);
        if (state == RF_QUEST_AVAILABLE)
        {
            gSpecialVar_0x8005 = id;
            StringCopy(gStringVar4, scene->offer);
            StringAppend(gStringVar4, COMPOUND_STRING("\p¿Aceptas el encargo?"));
            gSpecialVar_Result = 1;
            return;
        }
        if (state == RF_QUEST_ACTIVE && scene->kind == RF_STORY_RACE)
        {
            gSpecialVar_0x8005 = id;
            gSpecialVar_0x8007 = 2; // Start after the instructions are dismissed, not while reading.
            StringCopy(gStringVar4, scene->race->start);
            return;
        }
        if (state == RF_QUEST_ACTIVE)
        {
            StringCopy(gStringVar4, RFQuests_GetCatalog()[id].description);
            hasActiveGuidance = TRUE;
        }
    }
    // A recipient remembers the finale too, even if somebody else gave the mission.
    if (!hasActiveGuidance && pendingTarget == RF_QUEST_NONE)
        for (i = 0; i < RF_STORY_COUNT; i++)
            if ((sScenes[i].giver == actor || sScenes[i].target == actor)
             && RFQuests_GetState(RF_STORY_FIRST + i) == RF_QUEST_CLAIMED)
                StringCopy(gStringVar4, sScenes[i].ending);
    if (pendingTarget != RF_QUEST_NONE)
        StringCopy(gStringVar4, RFStory_Hint(pendingTarget));
    for (i = 0; i < ARRAY_COUNT(sContracts); i++)
    {
        const struct RFStoryContract *c = &sContracts[i];
        if (c->actor != actor || RFQuests_GetState(c->unlockQuest) != RF_QUEST_CLAIMED)
            continue;
        gSpecialVar_0x8006 = i;
        StringCopy(gStringVar4, c->name);
        StringAppend(gStringVar4, COMPOUND_STRING("\pEntrega: "));
        CopyItemName(c->item, gStringVar4 + StringLength(gStringVar4));
        StringAppend(gStringVar4, COMPOUND_STRING(" x"));
        ConvertIntToDecimalStringN(gStringVar4 + StringLength(gStringVar4), c->quantity, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringAppend(gStringVar4, COMPOUND_STRING("\pUn premio por día y por puesto.\n¿Entregar o recoger el pendiente?"));
        gSpecialVar_Result = 2;
        return;
    }
}

void Script_RFStoryAfterText(void)
{
    u32 id = gSpecialVar_0x8005;
    if (gSpecialVar_0x8007 == 2 && IsScene(id)
     && sScenes[id - RF_STORY_FIRST].kind == RF_STORY_RACE
     && AtActor(sScenes[id - RF_STORY_FIRST].giver)
     && RFQuests_GetState(id) == RF_QUEST_ACTIVE)
    {
        sRaceQuest = id + 1;
        sRaceStart = gMain.vblankCounter1;
        gSpecialVar_0x8007 = 0;
    }
}

void Script_RFStoryAccept(void)
{
    u32 id = gSpecialVar_0x8005;
    if (!IsScene(id) || !AtActor(sScenes[id - RF_STORY_FIRST].giver) || !RFQuests_Accept(id))
        StringCopy(gStringVar4, COMPOUND_STRING("El encargo ya cambió de estado.\nConsulta el diario."));
    else
    {
        StringCopy(gStringVar4, COMPOUND_STRING("Anotado en el diario.\p"));
        StringAppend(gStringVar4, RFQuests_GetCatalog()[id].description);
    }
}

void Script_RFStoryConfirm(void)
{
    u32 id = gSpecialVar_0x8005;
    u32 contract = gSpecialVar_0x8006;
    if (contract < ARRAY_COUNT(sContracts) && AtActor(sContracts[contract].actor))
    {
        u32 result = RFStory_DeliverContract(contract, gLocalTime.days < 0 ? 0 : gLocalTime.days);
        static const u8 *const messages[] = {
            COMPOUND_STRING("Este encargo no está disponible."),
            COMPOUND_STRING("La entrega está registrada.\nFalta espacio para tu premio.\pSe conserva aunque cambie el día.\nVuelve cuando tengas sitio."),
            COMPOUND_STRING("Entrega y premio registrados.\nGracias por ayudar al puesto."),
            COMPOUND_STRING("Ya recibiste el premio de hoy.\nNo tienes que entregar nada más."),
            COMPOUND_STRING("No tienes lo solicitado.\nNo se ha retirado ningún objeto."),
        };
        StringCopy(gStringVar4, messages[result]);
        return;
    }
    if (!IsScene(id))
        return;
    if (RFQuests_GetState(id) == RF_QUEST_ACTIVE && AtActor(sScenes[id - RF_STORY_FIRST].target)
     && sScenes[id - RF_STORY_FIRST].kind == RF_STORY_DELIVERY && !RFQuests_DeliverItem(id))
    {
        StringCopy(gStringVar4, COMPOUND_STRING("No tienes lo solicitado.\nNo se ha retirado ningún objeto."));
        return;
    }
    if (AtActor(sScenes[id - RF_STORY_FIRST].target) || AtActor(sScenes[id - RF_STORY_FIRST].giver))
        FinishScene(id);
}

void Script_RFStoryPuzzle(void)
{
    u32 id = gSpecialVar_0x8005;
    u32 answer = gSpecialVar_Result;
    const struct RFStoryScene *scene;
    gSpecialVar_Result = 0;
    if (!IsScene(id))
        return;
    scene = &sScenes[id - RF_STORY_FIRST];
    if (scene->kind != RF_STORY_PUZZLE || !AtActor(scene->target) || sPuzzleStep >= scene->questionCount)
        return;
    if (answer != scene->questions[sPuzzleStep].answer)
    {
        sPuzzleStep = 0;
        StringCopy(gStringVar4, COMPOUND_STRING("Ese orden no coincide con la pista.\nPuedes empezar de nuevo.\pEl encargo y tus otros progresos\nse conservan."));
        return;
    }
    if (++sPuzzleStep == scene->questionCount)
    {
        FinishScene(id);
        return;
    }
    PuzzleQuestion(id);
    gSpecialVar_Result = 3;
}

void Script_RFStoryBattleWon(void)
{
    u32 id = gSpecialVar_0x8005;
    // The battle-end adapter records victory before the overworld clears battle flags.
    if (IsScene(id) && AtActor(sScenes[id - RF_STORY_FIRST].target)
     && sScenes[id - RF_STORY_FIRST].kind == RF_STORY_BATTLE
     && RFQuests_GetState(id) == RF_QUEST_READY)
    {
        FinishScene(id);
        return;
    }
    if (!IsScene(id) || !AtActor(sScenes[id - RF_STORY_FIRST].target)
     || sScenes[id - RF_STORY_FIRST].kind != RF_STORY_BATTLE || gBattleOutcome != B_OUTCOME_WON
     || !(gBattleTypeFlags & BATTLE_TYPE_TRAINER)
     || (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED | BATTLE_TYPE_FRONTIER))
     || TRAINER_BATTLE_PARAM.opponentA != sScenes[id - RF_STORY_FIRST].trainer)
    {
        StringCopy(gStringVar4, COMPOUND_STRING("Podemos volver a practicar.\nTu encargo sigue en el diario."));
        return;
    }
    FinishScene(id);
}

u32 RFStory_NotifyTrainerWin(u32 trainer)
{
    u32 i, completed = 0;
    if (gBattleOutcome != B_OUTCOME_WON || !(gBattleTypeFlags & BATTLE_TYPE_TRAINER)
     || (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED | BATTLE_TYPE_FRONTIER)))
        return 0;
    for (i = 0; i < RF_STORY_COUNT; i++)
        if (sScenes[i].kind == RF_STORY_BATTLE && sScenes[i].trainer == trainer
         && AtActor(sScenes[i].target))
            completed += RFQuests_NotifyEvent(RF_STORY_EVENT, RF_STORY_FIRST + i);
    return completed;
}
