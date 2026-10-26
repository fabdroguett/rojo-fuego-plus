#include "global.h"
#include "rf_ui_theme.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_arena.h"
#include "battle_controllers.h"
#include "battle_dome.h"
#include "battle_interface.h"
#include "battle_message.h"
#include "battle_setup.h"
#include "battle_tv.h"
#include "battle_z_move.h"
#include "battle_gimmick.h"
#include "bg.h"
#include "data.h"
#include "item.h"
#include "item_menu.h"
#include "link.h"
#include "main.h"
#include "m4a.h"
#include "palette.h"
#include "party_menu.h"
#include "pokeball.h"
#include "pokemon.h"
#include "random.h"
#include "recorded_battle.h"
#include "reshow_battle_screen.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "test_runner.h"
#include "text.h"
#include "trainer.h"
#include "util.h"
#include "window.h"
#include "line_break.h"
#include "constants/battle_anim.h"
#include "constants/battle_move_effects.h"
#include "constants/battle_partner.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/party_menu.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/rgb.h"
#include "caps.h"
#include "menu.h"
#include "pokemon_summary_screen.h"
#include "graphics.h"
#include "malloc.h"
#include "decompress.h"
#include "type_icons.h"
#include "pokedex.h"
#include "test/battle.h"
#include "key_system.h"
#include "data/rf_battle_icons.h"

static void PlayerHandleLoadMonSprite(enum BattlerId battler);
static void PlayerHandleDrawTrainerPic(enum BattlerId battler);
static void PlayerHandleTrainerSlide(enum BattlerId battler);
static void PlayerHandleTrainerSlideBack(enum BattlerId battler);
static void PlayerHandlePaletteFade(enum BattlerId battler);
static void PlayerHandlePause(enum BattlerId battler);
static void PlayerHandleChooseAction(enum BattlerId battler);
static void PlayerHandleYesNoBox(enum BattlerId battler);
static void PlayerHandleChooseItem(enum BattlerId battler);
static void PlayerHandleChoosePokemon(enum BattlerId battler);
static void PlayerHandleCmd23(enum BattlerId battler);
static void PlayerHandleStatusXor(enum BattlerId battler);
static void PlayerHandleDMA3Transfer(enum BattlerId battler);
static void PlayerHandlePlayBGM(enum BattlerId battler);
static void PlayerHandleTwoReturnValues(enum BattlerId battler);
static void PlayerHandleChosenMonReturnValue(enum BattlerId battler);
static void PlayerHandleOneReturnValue(enum BattlerId battler);
static void PlayerHandleOneReturnValue_Duplicate(enum BattlerId battler);
static void PlayerHandleIntroTrainerBallThrow(enum BattlerId battler);
static void PlayerHandleDrawPartyStatusSummary(enum BattlerId battler);
static void PlayerHandleEndBounceEffect(enum BattlerId battler);
static void PlayerHandleLinkStandbyMsg(enum BattlerId battler);
static void PlayerHandleResetActionMoveSelection(enum BattlerId battler);
static void PlayerHandleEndLinkBattle(enum BattlerId battler);
static void PlayerHandleBattleDebug(enum BattlerId battler);

static void PlayerBufferRunCommand(enum BattlerId battler);
static void MoveSelectionDisplayPPNumber(enum BattlerId battler);
static void MoveSelectionDisplayPPString(enum BattlerId battler);
static void MoveSelectionDisplayMoveType(enum BattlerId battler);
static void MoveSelectionDisplayMoveNames(enum BattlerId battler);
static enum Type GetMoveMenuDisplayType(enum BattlerId battler, enum Move move);
static void TryMoveSelectionDisplayMoveDescription(enum BattlerId battler);
static void MoveSelectionDisplayMoveDescription(enum BattlerId battler);
static void MoveSelectionDisplayStatStages(enum BattlerId battler);

// Rojo Fuego+: the move description window is showing the stat changes page
static EWRAM_DATA bool8 sShowingStatStages = FALSE;
static void WaitForMonSelection(enum BattlerId battler);
static void CompleteWhenChoseItem(enum BattlerId battler);
static void Task_LaunchLvlUpAnim(u8);
static void Task_PrepareToGiveExpWithExpBar(u8);
static void Task_SetControllerToWaitForString(u8);
static void Task_GiveExpWithExpBar(u8);
static void Task_UpdateLvlInHealthbox(u8);
static void PrintLinkStandbyMsg(void);

static void ReloadMoveNames(enum BattlerId battler);
static u32 CheckTypeEffectiveness(enum BattlerId battlerAtk, enum BattlerId battlerDef);
static u32 CheckTargetTypeEffectiveness(enum BattlerId battler);
static void MoveSelectionDisplayMoveEffectiveness(u32 foeEffectiveness, enum BattlerId battler);

static void (*const sPlayerBufferCommands[CONTROLLER_CMDS_COUNT])(enum BattlerId battler) =
{
    [CONTROLLER_GETMONDATA]               = BtlController_HandleGetMonData,
    [CONTROLLER_GETRAWMONDATA]            = BtlController_HandleGetRawMonData,
    [CONTROLLER_SETMONDATA]               = BtlController_HandleSetMonData,
    [CONTROLLER_SETRAWMONDATA]            = BtlController_HandleSetRawMonData,
    [CONTROLLER_LOADMONSPRITE]            = PlayerHandleLoadMonSprite,
    [CONTROLLER_SWITCHINANIM]             = BtlController_HandleSwitchInAnim,
    [CONTROLLER_RETURNMONTOBALL]          = BtlController_HandleReturnMonToBall,
    [CONTROLLER_DRAWTRAINERPIC]           = PlayerHandleDrawTrainerPic,
    [CONTROLLER_TRAINERSLIDE]             = PlayerHandleTrainerSlide,
    [CONTROLLER_TRAINERSLIDEBACK]         = PlayerHandleTrainerSlideBack,
    [CONTROLLER_FAINTANIMATION]           = BtlController_HandleFaintAnimation,
    [CONTROLLER_PALETTEFADE]              = PlayerHandlePaletteFade,
    [CONTROLLER_BALLTHROWANIM]            = BtlController_HandleBallThrowAnim,
    [CONTROLLER_PAUSE]                    = PlayerHandlePause,
    [CONTROLLER_MOVEANIMATION]            = BtlController_HandleMoveAnimation,
    [CONTROLLER_PRINTSTRING]              = BtlController_HandlePrintString,
    [CONTROLLER_PRINTSTRINGPLAYERONLY]    = BtlController_HandlePrintStringPlayerOnly,
    [CONTROLLER_CHOOSEACTION]             = PlayerHandleChooseAction,
    [CONTROLLER_YESNOBOX]                 = PlayerHandleYesNoBox,
    [CONTROLLER_CHOOSEMOVE]               = PlayerHandleChooseMove,
    [CONTROLLER_OPENBAG]                  = PlayerHandleChooseItem,
    [CONTROLLER_CHOOSEPOKEMON]            = PlayerHandleChoosePokemon,
    [CONTROLLER_23]                       = PlayerHandleCmd23,
    [CONTROLLER_HEALTHBARUPDATE]          = BtlController_HandleHealthBarUpdate,
    [CONTROLLER_EXPUPDATE]                = PlayerHandleExpUpdate,
    [CONTROLLER_STATUSICONUPDATE]         = BtlController_HandleStatusIconUpdate,
    [CONTROLLER_STATUSANIMATION]          = BtlController_HandleStatusAnimation,
    [CONTROLLER_STATUSXOR]                = PlayerHandleStatusXor,
    [CONTROLLER_DATATRANSFER]             = BtlController_Empty,
    [CONTROLLER_DMA3TRANSFER]             = PlayerHandleDMA3Transfer,
    [CONTROLLER_PLAYBGM]                  = PlayerHandlePlayBGM,
    [CONTROLLER_32]                       = BtlController_Empty,
    [CONTROLLER_TWORETURNVALUES]          = PlayerHandleTwoReturnValues,
    [CONTROLLER_CHOSENMONRETURNVALUE]     = PlayerHandleChosenMonReturnValue,
    [CONTROLLER_ONERETURNVALUE]           = PlayerHandleOneReturnValue,
    [CONTROLLER_ONERETURNVALUE_DUPLICATE] = PlayerHandleOneReturnValue_Duplicate,
    [CONTROLLER_HITANIMATION]             = BtlController_HandleHitAnimation,
    [CONTROLLER_CANTSWITCH]               = BtlController_Empty,
    [CONTROLLER_PLAYSE]                   = BtlController_HandlePlaySE,
    [CONTROLLER_PLAYFANFAREORBGM]         = BtlController_HandlePlayFanfareOrBGM,
    [CONTROLLER_FAINTINGCRY]              = BtlController_HandleFaintingCry,
    [CONTROLLER_INTROSLIDE]               = BtlController_HandleIntroSlide,
    [CONTROLLER_INTROTRAINERBALLTHROW]    = PlayerHandleIntroTrainerBallThrow,
    [CONTROLLER_DRAWPARTYSTATUSSUMMARY]   = PlayerHandleDrawPartyStatusSummary,
    [CONTROLLER_HIDEPARTYSTATUSSUMMARY]   = BtlController_HandleHidePartyStatusSummary,
    [CONTROLLER_ENDBOUNCE]                = PlayerHandleEndBounceEffect,
    [CONTROLLER_SPRITEINVISIBILITY]       = BtlController_HandleSpriteInvisibility,
    [CONTROLLER_BATTLEANIMATION]          = BtlController_HandleBattleAnimation,
    [CONTROLLER_LINKSTANDBYMSG]           = PlayerHandleLinkStandbyMsg,
    [CONTROLLER_RESETACTIONMOVESELECTION] = PlayerHandleResetActionMoveSelection,
    [CONTROLLER_ENDLINKBATTLE]            = PlayerHandleEndLinkBattle,
    [CONTROLLER_DEBUGMENU]                = PlayerHandleBattleDebug,
    [CONTROLLER_TERMINATOR_NOP]           = BtlController_TerminatorNop
};

void SetControllerToPlayer(enum BattlerId battler)
{
    gBattlerBattleController[battler] = BATTLE_CONTROLLER_PLAYER;
    gBattlerControllerEndFuncs[battler] = PlayerBufferExecCompleted;
    gBattlerControllerFuncs[battler] = PlayerBufferRunCommand;
    gDoingBattleAnim = FALSE;
    gPlayerDpadHoldFrames = 0;
}

void PlayerBufferExecCompleted(enum BattlerId battler)
{
    gBattlerControllerFuncs[battler] = PlayerBufferRunCommand;
    if (gBattleTypeFlags & BATTLE_TYPE_LINK)
    {
        u8 playerId = GetMultiplayerId();

        PrepareBufferDataTransferLink(battler, B_COMM_CONTROLLER_IS_DONE, 4, &playerId);
        gBattleResources->bufferA[battler][0] = CONTROLLER_TERMINATOR_NOP;
    }
    else
    {
        MarkBattleControllerIdleOnLocal(battler);
    }
}

static void PlayerBufferRunCommand(enum BattlerId battler)
{
    if (IsBattleControllerActiveOnLocal(battler))
    {
        if (gBattleResources->bufferA[battler][0] < ARRAY_COUNT(sPlayerBufferCommands))
            sPlayerBufferCommands[gBattleResources->bufferA[battler][0]](battler);
        else
            BtlController_Complete(battler);
    }
}

static void CompleteOnBattlerSpritePosX_0(enum BattlerId battler)
{
    if (gSprites[gBattlerSpriteIds[battler]].x2 == 0)
        BtlController_Complete(battler);
}

static enum Item GetPrevBall(enum Item ballId)
{
    s32 i;
    enum PokeBall index = ItemIdToBallId(ballId);
    enum Item newBall = ITEM_NONE;

    for (i = 0; i < POKEBALL_COUNT; i++)
    {
        index--;
        if (index == -1)
            index = POKEBALL_COUNT - 1;
        newBall = gPokeBalls[index].itemId;
        if (CheckBagHasItem(newBall, 1))
            return newBall;
    }
    return ballId;
}

static enum Item GetNextBall(enum Item ballId)
{
    s32 i;
    enum PokeBall index = ItemIdToBallId(ballId);
    enum Item newBall = ITEM_NONE;

    for (i = 0; i < POKEBALL_COUNT; i++)
    {
        index++;
        if (index == POKEBALL_COUNT)
            index = 0;
        newBall = gPokeBalls[index].itemId;
        if (CheckBagHasItem(newBall, 1))
            return newBall;
    }
    return ballId;
}

static void HandleInputChooseAction(enum BattlerId battler)
{
    enum Item itemId = gBattleResources->bufferA[battler][2] | (gBattleResources->bufferA[battler][3] << 8);

    DoBounceEffect(battler, BOUNCE_HEALTHBOX, 7, 1);
    DoBounceEffect(battler, BOUNCE_MON, 7, 1);

    if (JOY_REPEAT(DPAD_ANY) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_L_EQUALS_A)
        gPlayerDpadHoldFrames++;
    else
        gPlayerDpadHoldFrames = 0;

    if (B_LAST_USED_BALL == TRUE && B_LAST_USED_BALL_CYCLE == TRUE
    && !(B_LAST_USED_BALL_BUTTON == L_BUTTON && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_L_EQUALS_A))
    {
        if (!gLastUsedBallMenuPresent)
        {
            gBattleStruct->ackBallUseBtn = FALSE;
        }
        else if (JOY_NEW(B_LAST_USED_BALL_BUTTON))
        {
            gBattleStruct->ackBallUseBtn = TRUE;
            gBattleStruct->ballSwapped = FALSE;
            ArrowsChangeColorLastBallCycle(TRUE);
        }

        if (gBattleStruct->ackBallUseBtn)
        {
            if (JOY_HELD(B_LAST_USED_BALL_BUTTON) && (JOY_NEW(DPAD_DOWN) || JOY_NEW(DPAD_RIGHT)))
            {
                bool32 sameBall = FALSE;
                u32 nextBall = GetNextBall(gBallToDisplay);
                gBattleStruct->ballSwapped = TRUE;
                if (gBallToDisplay == nextBall)
                    sameBall = TRUE;
                else
                    gBallToDisplay = nextBall;
                SwapBallToDisplay(sameBall);
                PlaySE(SE_SELECT);
            }
            else if (JOY_HELD(B_LAST_USED_BALL_BUTTON) && (JOY_NEW(DPAD_UP) || JOY_NEW(DPAD_LEFT)))
            {
                bool32 sameBall = FALSE;
                u32 prevBall = GetPrevBall(gBallToDisplay);
                gBattleStruct->ballSwapped = TRUE;
                if (gBallToDisplay == prevBall)
                    sameBall = TRUE;
                else
                    gBallToDisplay = prevBall;
                SwapBallToDisplay(sameBall);
                PlaySE(SE_SELECT);
            }
            else if (JOY_NEW(B_BUTTON) || (!JOY_HELD(B_LAST_USED_BALL_BUTTON) && gBattleStruct->ballSwapped))
            {
                gBattleStruct->ackBallUseBtn = FALSE;
                gBattleStruct->ballSwapped = FALSE;
                ArrowsChangeColorLastBallCycle(FALSE);
            }
            else if (!JOY_HELD(B_LAST_USED_BALL_BUTTON) && CanThrowLastUsedBall())
            {
                gBattleStruct->ackBallUseBtn = FALSE;
                PlaySE(SE_SELECT);
                ArrowsChangeColorLastBallCycle(FALSE);
                TryHideLastUsedBall();
                BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_THROW_BALL, 0);
                BtlController_Complete(battler);
            }
            return;
        }
    }

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        TryHideLastUsedBall();

        switch (gActionSelectionCursor[battler])
        {
        case 0: // Top left
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_USE_MOVE, 0);
            break;
        case 1: // Top right
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_USE_ITEM, 0);
            break;
        case 2: // Bottom left
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_SWITCH, 0);
            break;
        case 3: // Bottom right
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_RUN, 0);
            break;
        }
        BtlController_Complete(battler);
    }
    else if (JOY_NEW(DPAD_LEFT))
    {
        if (gActionSelectionCursor[battler] & 1) // if is B_ACTION_USE_ITEM or B_ACTION_RUN
        {
            PlaySE(SE_SELECT);
            ActionSelectionDestroyCursorAt(gActionSelectionCursor[battler]);
            gActionSelectionCursor[battler] ^= 1;
            ActionSelectionCreateCursorAt(gActionSelectionCursor[battler], 0);
        }
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        if (!(gActionSelectionCursor[battler] & 1)) // if is B_ACTION_USE_MOVE or B_ACTION_SWITCH
        {
            PlaySE(SE_SELECT);
            ActionSelectionDestroyCursorAt(gActionSelectionCursor[battler]);
            gActionSelectionCursor[battler] ^= 1;
            ActionSelectionCreateCursorAt(gActionSelectionCursor[battler], 0);
        }
    }
    else if (JOY_NEW(DPAD_UP))
    {
        if (gActionSelectionCursor[battler] & 2) // if is B_ACTION_SWITCH or B_ACTION_RUN
        {
            PlaySE(SE_SELECT);
            ActionSelectionDestroyCursorAt(gActionSelectionCursor[battler]);
            gActionSelectionCursor[battler] ^= 2;
            ActionSelectionCreateCursorAt(gActionSelectionCursor[battler], 0);
        }
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (!(gActionSelectionCursor[battler] & 2)) // if is B_ACTION_USE_MOVE or B_ACTION_USE_ITEM
        {
            PlaySE(SE_SELECT);
            ActionSelectionDestroyCursorAt(gActionSelectionCursor[battler]);
            gActionSelectionCursor[battler] ^= 2;
            ActionSelectionCreateCursorAt(gActionSelectionCursor[battler], 0);
        }
    }
    else if (JOY_NEW(B_BUTTON) || gPlayerDpadHoldFrames > 59)
    {
        if (IsDoubleBattle()
         && GetBattlerPosition(battler) == B_POSITION_PLAYER_RIGHT
         && !(gAbsentBattlerFlags & (1u << GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)))
         && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
        {
            // Return item to bag if partner had selected one (if consumable).
            if (gBattleResources->bufferA[battler][1] == B_ACTION_USE_ITEM && GetItemConsumability(itemId))
            {
                AddBagItem(itemId, 1);
            }
            PlaySE(SE_SELECT);
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_CANCEL_PARTNER, 0);
            BtlController_Complete(battler);
        }
        else if (B_QUICK_MOVE_CURSOR_TO_RUN)
        {
            if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER)) // If wild battle, pressing B moves cursor to "Run".
            {
                PlaySE(SE_SELECT);
                ActionSelectionDestroyCursorAt(gActionSelectionCursor[battler]);
                gActionSelectionCursor[battler] = 3;
                ActionSelectionCreateCursorAt(gActionSelectionCursor[battler], 0);
            }
        }
    }
    else if (JOY_NEW(START_BUTTON))
    {
        SwapHpBarsWithHpText();
    }
    else if (DEBUG_BATTLE_MENU == TRUE && JOY_NEW(SELECT_BUTTON))
    {
        BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_DEBUG, 0);
        BtlController_Complete(battler);
    }
    else if (B_LAST_USED_BALL == TRUE && B_LAST_USED_BALL_CYCLE == FALSE
             && JOY_NEW(B_LAST_USED_BALL_BUTTON) && CanThrowLastUsedBall())
    {
        PlaySE(SE_SELECT);
        TryHideLastUsedBall();
        BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_THROW_BALL, 0);
        BtlController_Complete(battler);
    }
}

void HandleInputChooseTarget(enum BattlerId battler)
{
    enum BattlerId i;
    static const enum BattlerPosition identities[MAX_BATTLERS_COUNT] =
    {
        B_POSITION_PLAYER_LEFT,
        B_POSITION_PLAYER_RIGHT,
        B_POSITION_OPPONENT_RIGHT,
        B_POSITION_OPPONENT_LEFT,
    };
    enum Move move = GetMonData(GetBattlerMon(battler), MON_DATA_MOVE1 + gMoveSelectionCursor[battler]);
    enum MoveTarget moveTarget = GetBattlerMoveSelectionTargetType(battler, move);

    DoBounceEffect(gMultiUsePlayerCursor, BOUNCE_HEALTHBOX, 15, 1);
    for (i = 0; i < gBattlersCount; i++)
    {
        if (i != gMultiUsePlayerCursor)
            EndBounceEffect(i, BOUNCE_HEALTHBOX);
    }

    if (JOY_HELD(DPAD_ANY) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_L_EQUALS_A)
        gPlayerDpadHoldFrames++;
    else
        gPlayerDpadHoldFrames = 0;

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCB_HideAsMoveTarget;
        if (gBattleStruct->gimmick.playerSelect)
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | RET_GIMMICK | (gMultiUsePlayerCursor << 8));
        else
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | (gMultiUsePlayerCursor << 8));
        EndBounceEffect(gMultiUsePlayerCursor, BOUNCE_HEALTHBOX);
        TryHideLastUsedBall();
        HideGimmickTriggerSprite();
        BtlController_Complete(battler);
    }
    else if (JOY_NEW(B_BUTTON) || gPlayerDpadHoldFrames > 59)
    {
        PlaySE(SE_SELECT);
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCB_HideAsMoveTarget;
        gBattlerControllerFuncs[battler] = HandleInputChooseMove;
        if (gBattleStruct->gimmick.playerSelect == 1 && gBattleStruct->gimmick.usableGimmick[battler] == GIMMICK_Z_MOVE)
        {
            gBattleStruct->gimmick.playerSelect = 0;
            gBattleStruct->zmove.viewing = TRUE;
            ReloadMoveNames(battler);
        }
        TryToAddMoveInfoWindow();
        DoBounceEffect(battler, BOUNCE_HEALTHBOX, 7, 1);
        DoBounceEffect(battler, BOUNCE_MON, 7, 1);
        EndBounceEffect(gMultiUsePlayerCursor, BOUNCE_HEALTHBOX);
    }
    else if (JOY_NEW(DPAD_LEFT | DPAD_UP))
    {
        PlaySE(SE_SELECT);
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCB_HideAsMoveTarget;

        if (moveTarget == TARGET_USER_OR_ALLY)
        {
            gMultiUsePlayerCursor ^= BIT_FLANK;
        }
        else
        {
            bool32 validTarget = FALSE;
            do
            {
                enum BattlerPosition currSelIdentity = GetBattlerPosition(gMultiUsePlayerCursor);

                for (i = 0; i < MAX_BATTLERS_COUNT; i++)
                {
                    if (currSelIdentity == identities[i])
                        break;
                }
                do
                {
                    if (i == 0)
                        i = MAX_BATTLERS_COUNT - 1;
                    else
                        i--;
                    gMultiUsePlayerCursor = GetBattlerAtPosition(identities[i]);
                } while (gMultiUsePlayerCursor >= gBattlersCount);

                switch (GetBattlerPosition(gMultiUsePlayerCursor))
                {
                case B_POSITION_PLAYER_LEFT:
                case B_POSITION_PLAYER_RIGHT:
                    if (battler != gMultiUsePlayerCursor)
                        validTarget = TRUE;
                    break;
                case B_POSITION_OPPONENT_LEFT:
                case B_POSITION_OPPONENT_RIGHT:
                    validTarget = TRUE;
                    break;
                default:
                    break;
                }

                if (!CanTargetBattler(battler, gMultiUsePlayerCursor, move)
                 || (moveTarget == TARGET_OPPONENT && IsOnPlayerSide(gMultiUsePlayerCursor)))
                    validTarget = FALSE;

                if (B_SHOW_EFFECTIVENESS && validTarget)
                    MoveSelectionDisplayMoveEffectiveness(CheckTypeEffectiveness(battler, gMultiUsePlayerCursor), battler);

            } while (!validTarget);
        }
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCB_ShowAsMoveTarget;
    }
    else if (JOY_NEW(DPAD_RIGHT | DPAD_DOWN))
    {
        PlaySE(SE_SELECT);
        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCB_HideAsMoveTarget;

        if (moveTarget == TARGET_USER_OR_ALLY)
        {
            gMultiUsePlayerCursor ^= BIT_FLANK;
        }
        else
        {
            do
            {
                enum BattlerPosition currSelIdentity = GetBattlerPosition(gMultiUsePlayerCursor);

                for (i = 0; i < MAX_BATTLERS_COUNT; i++)
                {
                    if (currSelIdentity == identities[i])
                        break;
                }
                do
                {
                    if (++i > 3)
                        i = 0;
                    gMultiUsePlayerCursor = GetBattlerAtPosition(identities[i]);
                } while (gMultiUsePlayerCursor == gBattlersCount);

                i = 0;
                switch (GetBattlerPosition(gMultiUsePlayerCursor))
                {
                case B_POSITION_PLAYER_LEFT:
                case B_POSITION_PLAYER_RIGHT:
                    if (battler != gMultiUsePlayerCursor)
                        i++;
                    break;
                case B_POSITION_OPPONENT_LEFT:
                case B_POSITION_OPPONENT_RIGHT:
                    i++;
                    break;
                default:
                    break;
                }
                if (B_SHOW_EFFECTIVENESS)
                    MoveSelectionDisplayMoveEffectiveness(CheckTypeEffectiveness(battler, gMultiUsePlayerCursor), battler);

                if (!CanTargetBattler(battler, gMultiUsePlayerCursor, move)
                 || (moveTarget == TARGET_OPPONENT && IsOnPlayerSide(gMultiUsePlayerCursor)))
                    i = 0;
            } while (i == 0);
        }

        gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCB_ShowAsMoveTarget;
    }
}

static void HideAllTargets(void)
{
    for (enum BattlerId i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        if (ShouldHideBattler(i))
        {
            gSprites[gBattlerSpriteIds[i]].callback = SpriteCB_HideAsMoveTarget;
            EndBounceEffect(i, BOUNCE_HEALTHBOX);
        }
    }
}

void HandleInputShowEntireFieldTargets(enum BattlerId battler)
{
    if (JOY_HELD(DPAD_ANY) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_L_EQUALS_A)
        gPlayerDpadHoldFrames++;
    else
        gPlayerDpadHoldFrames = 0;

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        HideAllTargets();
        if (gBattleStruct->gimmick.playerSelect)
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | RET_GIMMICK | (gMultiUsePlayerCursor << 8));
        else
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | (gMultiUsePlayerCursor << 8));
        HideGimmickTriggerSprite();
        BtlController_Complete(battler);
    }
    else if (JOY_NEW(B_BUTTON) || gPlayerDpadHoldFrames > 59)
    {
        PlaySE(SE_SELECT);
        HideAllTargets();
        gBattlerControllerFuncs[battler] = HandleInputChooseMove;
        DoBounceEffect(battler, BOUNCE_HEALTHBOX, 7, 1);
        DoBounceEffect(battler, BOUNCE_MON, 7, 1);
    }
}

void HandleInputShowTargets(enum BattlerId battler)
{
    if (JOY_HELD(DPAD_ANY) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_L_EQUALS_A)
        gPlayerDpadHoldFrames++;
    else
        gPlayerDpadHoldFrames = 0;

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        HideAllTargets();
        if (gBattleStruct->gimmick.playerSelect)
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | RET_GIMMICK | (gMultiUsePlayerCursor << 8));
        else
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | (gMultiUsePlayerCursor << 8));
        HideGimmickTriggerSprite();
        TryHideLastUsedBall();
        BtlController_Complete(battler);
    }
    else if (JOY_NEW(B_BUTTON) || gPlayerDpadHoldFrames > 59)
    {
        PlaySE(SE_SELECT);
        HideAllTargets();
        gBattlerControllerFuncs[battler] = HandleInputChooseMove;
        DoBounceEffect(battler, BOUNCE_HEALTHBOX, 7, 1);
        DoBounceEffect(battler, BOUNCE_MON, 7, 1);
    }
}

static void TryShowAsTarget(enum BattlerId battler)
{
    if (IsBattlerAlive(battler))
    {
        DoBounceEffect(battler, BOUNCE_HEALTHBOX, 15, 1);
        gSprites[gBattlerSpriteIds[battler]].callback = SpriteCB_ShowAsMoveTarget;
    }
}

static bool32 CanSelectBattler(enum MoveTarget target)
{
    switch (target)
    {
    case TARGET_RANDOM:
    case TARGET_BOTH:
    case TARGET_DEPENDS:
    case TARGET_FOES_AND_ALLY:
    case TARGET_OPPONENTS_FIELD:
    case TARGET_USER:
    case TARGET_ALLY:
    case TARGET_USER_OR_ALLY:
    case TARGET_USER_AND_ALLY:
        return TRUE;
    default:
        break;
    }

    return FALSE;
}

void HandleInputChooseMove(enum BattlerId battler)
{
    u32 canSelectTarget = 0;
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);

    if (JOY_HELD(DPAD_ANY) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_L_EQUALS_A)
        gPlayerDpadHoldFrames++;
    else
        gPlayerDpadHoldFrames = 0;

    if (JOY_NEW(A_BUTTON) && !gBattleStruct->descriptionSubmenu)
    {
        TryToHideMoveInfoWindow();
        PlaySE(SE_SELECT);

        enum MoveTarget moveTarget = GetBattlerMoveSelectionTargetType(battler, moveInfo->moves[gMoveSelectionCursor[battler]]);
        bool32 isUserOrAlly = moveTarget == TARGET_USER || moveTarget == TARGET_USER_OR_ALLY || moveTarget == TARGET_USER_AND_ALLY;

        if (gBattleStruct->zmove.viewing)
        {
            gBattleStruct->zmove.viewing = FALSE;
            if (GetMoveCategory(moveInfo->moves[gMoveSelectionCursor[battler]]) != DAMAGE_CATEGORY_STATUS)
                moveTarget = TARGET_SELECTED;  //damaging z moves always have selected target
        }

        // Status moves turn into Max Guard when Dynamaxed, targets user.
        if (GetActiveGimmick(battler) == GIMMICK_DYNAMAX || IsGimmickSelected(battler, GIMMICK_DYNAMAX))
            moveTarget = GetMoveTarget(GetMaxMove(battler, moveInfo->moves[gMoveSelectionCursor[battler]]));

        gMultiUsePlayerCursor = GetDefaultSelectionTarget(battler, moveTarget);
        
        enum BattlerId partner = GetPartnerBattler(battler);

        if (gBattleResources->bufferA[battler][1]) // a double battle
        {
            if (!CanSelectBattler(moveTarget))
                canSelectTarget = 1; // either selected or user
            if (moveTarget == TARGET_USER_OR_ALLY && IsBattlerAlive(partner))
                canSelectTarget = 1;

            if (moveInfo->currentPP[gMoveSelectionCursor[battler]] == 0)
            {
                canSelectTarget = 0;
            }
            else if (isUserOrAlly && !IsBattlerAlive(partner))
            {
                gMultiUsePlayerCursor = battler;
                canSelectTarget = 0;
            }

            if (B_SHOW_TARGETS == TRUE)
            {
                // Show all available targets for multi-target moves
                if (moveTarget == TARGET_ALL_BATTLERS || moveTarget == TARGET_FIELD)
                {
                    for (enum BattlerId i = 0; i < gBattlersCount; i++)
                        TryShowAsTarget(i);

                    canSelectTarget = 3;
                }
                else if (IsSpreadMove(moveTarget) || moveTarget == TARGET_OPPONENTS_FIELD || moveTarget == TARGET_USER_AND_ALLY)
                {
                    TryShowAsTarget(gMultiUsePlayerCursor);
                    TryShowAsTarget(GetPartnerBattler(gMultiUsePlayerCursor));
                    if (moveTarget == TARGET_FOES_AND_ALLY)
                        TryShowAsTarget(GetPartnerBattler(battler));
                    canSelectTarget = 2;
                }
            }
        }

        switch (canSelectTarget)
        {
        case 0:
        default:
            if (gBattleStruct->gimmick.playerSelect)
                BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | RET_GIMMICK | (gMultiUsePlayerCursor << 8));
            else
                BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, gMoveSelectionCursor[battler] | (gMultiUsePlayerCursor << 8));
            HideGimmickTriggerSprite();
            TryHideLastUsedBall();
            BtlController_Complete(battler);
            break;
        case 1:
            gBattlerControllerFuncs[battler] = HandleInputChooseTarget;

            if (moveTarget == TARGET_USER || moveTarget == TARGET_USER_OR_ALLY)
                gMultiUsePlayerCursor = battler;
            else if (!IsBattlerAlive(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT)))
                gMultiUsePlayerCursor = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
            else
                gMultiUsePlayerCursor = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
            if (B_SHOW_EFFECTIVENESS)
                MoveSelectionDisplayMoveEffectiveness(CheckTypeEffectiveness(battler, gMultiUsePlayerCursor), battler);

            gSprites[gBattlerSpriteIds[gMultiUsePlayerCursor]].callback = SpriteCB_ShowAsMoveTarget;
            break;
        case 2:
            gBattlerControllerFuncs[battler] = HandleInputShowTargets;
            break;
        case 3: // Entire field
            gBattlerControllerFuncs[battler] = HandleInputShowEntireFieldTargets;
            break;
        }
    }
    else if ((JOY_NEW(B_BUTTON) || gPlayerDpadHoldFrames > 59)  && !gBattleStruct->descriptionSubmenu)
    {
        PlaySE(SE_SELECT);
        gBattleStruct->gimmick.playerSelect = FALSE;
        if (gBattleStruct->zmove.viewing)
        {
            ReloadMoveNames(battler);
            ChangeGimmickTriggerSprite(gBattleStruct->gimmick.triggerSpriteId, gBattleStruct->gimmick.playerSelect);
        }
        else
        {
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, 0xFFFF);
            HideGimmickTriggerSprite();
            BtlController_Complete(battler);
            TryToHideMoveInfoWindow();
        }
    }
    else if (JOY_NEW(DPAD_LEFT) && !gBattleStruct->zmove.viewing)
    {
        if (gMoveSelectionCursor[battler] & 1)
        {
            MoveSelectionDestroyCursorAt(gMoveSelectionCursor[battler]);
            gMoveSelectionCursor[battler] ^= 1;
            PlaySE(SE_SELECT);
            MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
            if (B_SHOW_EFFECTIVENESS)
                MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
            MoveSelectionDisplayPPNumber(battler);
            MoveSelectionDisplayMoveType(battler);
            TryMoveSelectionDisplayMoveDescription(battler);
            TryChangeZTrigger(battler, gMoveSelectionCursor[battler]);
        }
    }
    else if (JOY_NEW(DPAD_RIGHT) && !gBattleStruct->zmove.viewing)
    {
        if (!(gMoveSelectionCursor[battler] & 1)
         && (gMoveSelectionCursor[battler] ^ 1) < gNumberOfMovesToChoose)
        {
            MoveSelectionDestroyCursorAt(gMoveSelectionCursor[battler]);
            gMoveSelectionCursor[battler] ^= 1;
            PlaySE(SE_SELECT);
            MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
            if (B_SHOW_EFFECTIVENESS)
                MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
            MoveSelectionDisplayPPNumber(battler);
            MoveSelectionDisplayMoveType(battler);
            TryMoveSelectionDisplayMoveDescription(battler);
            TryChangeZTrigger(battler, gMoveSelectionCursor[battler]);
        }
    }
    else if (JOY_NEW(DPAD_UP) && !gBattleStruct->zmove.viewing)
    {
        if (gMoveSelectionCursor[battler] & 2)
        {
            MoveSelectionDestroyCursorAt(gMoveSelectionCursor[battler]);
            gMoveSelectionCursor[battler] ^= 2;
            PlaySE(SE_SELECT);
            MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
            if (B_SHOW_EFFECTIVENESS)
                MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
            MoveSelectionDisplayPPNumber(battler);
            MoveSelectionDisplayMoveType(battler);
            TryMoveSelectionDisplayMoveDescription(battler);
            TryChangeZTrigger(battler, gMoveSelectionCursor[battler]);
        }
    }
    else if (JOY_NEW(DPAD_DOWN) && !gBattleStruct->zmove.viewing)
    {
        if (!(gMoveSelectionCursor[battler] & 2)
         && (gMoveSelectionCursor[battler] ^ 2) < gNumberOfMovesToChoose)
        {
            MoveSelectionDestroyCursorAt(gMoveSelectionCursor[battler]);
            gMoveSelectionCursor[battler] ^= 2;
            PlaySE(SE_SELECT);
            MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
            if (B_SHOW_EFFECTIVENESS)
                MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
            MoveSelectionDisplayPPNumber(battler);
            MoveSelectionDisplayMoveType(battler);
            TryMoveSelectionDisplayMoveDescription(battler);
            TryChangeZTrigger(battler, gMoveSelectionCursor[battler]);
        }
    }
    else if (B_MOVE_REARRANGEMENT_IN_BATTLE < GEN_4 && JOY_NEW(SELECT_BUTTON) && !gBattleStruct->zmove.viewing && !gBattleStruct->descriptionSubmenu)
    {
        if (gNumberOfMovesToChoose > 1 && !(gBattleTypeFlags & BATTLE_TYPE_LINK))
        {
            MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 29);

            if (gMoveSelectionCursor[battler] != 0)
                gMultiUsePlayerCursor = 0;
            else
                gMultiUsePlayerCursor = gMoveSelectionCursor[battler] + 1;

            MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 27);
            BattlePutTextOnWindow(gText_BattleSwitchWhich, B_WIN_SWITCH_PROMPT);
            gBattlerControllerFuncs[battler] = HandleMoveSwitching;
        }
    }
    else if (gBattleStruct->descriptionSubmenu)
    {
        if (JOY_NEW(SELECT_BUTTON))
        {
            // Rojo Fuego+: SELECT switches between the move description and the stat changes
            PlaySE(SE_SELECT);
            sShowingStatStages ^= TRUE;
            if (sShowingStatStages)
                MoveSelectionDisplayStatStages(battler);
            else
                MoveSelectionDisplayMoveDescription(battler);
        }
        else if (JOY_NEW(B_MOVE_DESCRIPTION_BUTTON) || JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON))
        {
            gBattleStruct->descriptionSubmenu = FALSE;
            sShowingStatStages = FALSE;
            if (gCategoryIconSpriteId != 0xFF)
            {
                DestroySprite(&gSprites[gCategoryIconSpriteId]);
                gCategoryIconSpriteId = 0xFF;
            }

            FillWindowPixelBuffer(B_WIN_MOVE_DESCRIPTION, PIXEL_FILL(0));
            ClearStdWindowAndFrame(B_WIN_MOVE_DESCRIPTION, FALSE);
            CopyWindowToVram(B_WIN_MOVE_DESCRIPTION, COPYWIN_GFX);
            PlaySE(SE_SELECT);
            if (B_SHOW_EFFECTIVENESS)
                MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
            MoveSelectionDisplayPPNumber(battler);
            MoveSelectionDisplayMoveType(battler);
        }
    }
    else if (JOY_NEW(B_MOVE_DESCRIPTION_BUTTON) &&
        !(B_MOVE_DESCRIPTION_BUTTON == L_BUTTON && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_L_EQUALS_A))
    {
        gBattleStruct->descriptionSubmenu = TRUE;
        TryMoveSelectionDisplayMoveDescription(battler);
    }
    else if (JOY_NEW(START_BUTTON))
    {
        if (gBattleStruct->gimmick.usableGimmick[battler] != GIMMICK_NONE
            && !HasTrainerUsedGimmick(battler, gBattleStruct->gimmick.usableGimmick[battler])
            && !(gBattleStruct->gimmick.usableGimmick[battler] == GIMMICK_Z_MOVE
                 && GetUsableZMove(battler, moveInfo->moves[gMoveSelectionCursor[battler]]) == MOVE_NONE))
        {
            gBattleStruct->gimmick.playerSelect ^= 1;
            ReloadMoveNames(battler);
            ChangeGimmickTriggerSprite(gBattleStruct->gimmick.triggerSpriteId, gBattleStruct->gimmick.playerSelect);
            PlaySE(SE_SELECT);
        }
    }
}

static void ReloadMoveNames(enum BattlerId battler)
{
    if (gBattleStruct->zmove.viable && !gBattleStruct->zmove.viewing)
    {
        struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
        MoveSelectionDisplayZMove(GetUsableZMove(battler, moveInfo->moves[gMoveSelectionCursor[battler]]), battler);
    }
    else
    {
        gBattleStruct->zmove.viewing = FALSE;
        MoveSelectionDestroyCursorAt(battler);
        MoveSelectionDisplayMoveNames(battler);
        MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
        if (B_SHOW_EFFECTIVENESS)
            MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
        MoveSelectionDisplayPPNumber(battler);
        MoveSelectionDisplayMoveType(battler);
    }
}

static u32 UNUSED HandleMoveInputUnused(enum BattlerId battler)
{
    u32 var = 0;

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        var = 1;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        gBattle_BG0_X = 0;
        gBattle_BG0_Y = DISPLAY_HEIGHT * 2;
        var = 0xFF;
    }
    if (JOY_NEW(DPAD_LEFT) && gMoveSelectionCursor[battler] & 1)
    {
        MoveSelectionDestroyCursorAt(gMoveSelectionCursor[battler]);
        gMoveSelectionCursor[battler] ^= 1;
        PlaySE(SE_SELECT);
        MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
    }
    if (JOY_NEW(DPAD_RIGHT) && !(gMoveSelectionCursor[battler] & 1)
        && (gMoveSelectionCursor[battler] ^ 1) < gNumberOfMovesToChoose)
    {
        MoveSelectionDestroyCursorAt(gMoveSelectionCursor[battler]);
        gMoveSelectionCursor[battler] ^= 1;
        PlaySE(SE_SELECT);
        MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
    }
    if (JOY_NEW(DPAD_UP) && gMoveSelectionCursor[battler] & 2)
    {
        MoveSelectionDestroyCursorAt(gMoveSelectionCursor[battler]);
        gMoveSelectionCursor[battler] ^= 2;
        PlaySE(SE_SELECT);
        MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
    }
    if (JOY_NEW(DPAD_DOWN) && !(gMoveSelectionCursor[battler] & 2)
        && (gMoveSelectionCursor[battler] ^ 2) < gNumberOfMovesToChoose)
    {
        MoveSelectionDestroyCursorAt(gMoveSelectionCursor[battler]);
        gMoveSelectionCursor[battler] ^= 2;
        PlaySE(SE_SELECT);
        MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
    }

    return var;
}

void HandleMoveSwitching(enum BattlerId battler)
{
    u8 perMovePPBonuses[MAX_MON_MOVES];
    struct ChooseMoveStruct moveStruct;
    u8 totalPPBonuses;

    if (JOY_NEW(A_BUTTON | SELECT_BUTTON))
    {
        struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
        PlaySE(SE_SELECT);

        if (gMoveSelectionCursor[battler] != gMultiUsePlayerCursor)
        {
            struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
            s32 i;

            // swap moves and pp
            i = moveInfo->moves[gMoveSelectionCursor[battler]];
            moveInfo->moves[gMoveSelectionCursor[battler]] = moveInfo->moves[gMultiUsePlayerCursor];
            moveInfo->moves[gMultiUsePlayerCursor] = i;

            i = moveInfo->currentPP[gMoveSelectionCursor[battler]];
            moveInfo->currentPP[gMoveSelectionCursor[battler]] = moveInfo->currentPP[gMultiUsePlayerCursor];
            moveInfo->currentPP[gMultiUsePlayerCursor] = i;

            i = moveInfo->maxPP[gMoveSelectionCursor[battler]];
            moveInfo->maxPP[gMoveSelectionCursor[battler]] = moveInfo->maxPP[gMultiUsePlayerCursor];
            moveInfo->maxPP[gMultiUsePlayerCursor] = i;

            if (gBattleMons[battler].volatiles.mimickedMoves & (1u << gMoveSelectionCursor[battler]))
            {
                gBattleMons[battler].volatiles.mimickedMoves &= ~(1u << gMoveSelectionCursor[battler]);
                gBattleMons[battler].volatiles.mimickedMoves |= 1u << gMultiUsePlayerCursor;
            }

            MoveSelectionDisplayMoveNames(battler);

            for (i = 0; i < MAX_MON_MOVES; i++)
                perMovePPBonuses[i] = (gBattleMons[battler].ppBonuses & (3 << (i * 2))) >> (i * 2);

            totalPPBonuses = perMovePPBonuses[gMoveSelectionCursor[battler]];
            perMovePPBonuses[gMoveSelectionCursor[battler]] = perMovePPBonuses[gMultiUsePlayerCursor];
            perMovePPBonuses[gMultiUsePlayerCursor] = totalPPBonuses;

            totalPPBonuses = 0;
            for (i = 0; i < MAX_MON_MOVES; i++)
                totalPPBonuses |= perMovePPBonuses[i] << (i * 2);

            gBattleMons[battler].ppBonuses = totalPPBonuses;

            for (i = 0; i < MAX_MON_MOVES; i++)
            {
                gBattleMons[battler].moves[i] = moveInfo->moves[i];
                gBattleMons[battler].pp[i] = moveInfo->currentPP[i];
            }

            if (!(gBattleMons[battler].volatiles.transformed))
            {
                for (i = 0; i < MAX_MON_MOVES; i++)
                {
                    moveStruct.moves[i] = GetMonData(GetBattlerMon(battler), MON_DATA_MOVE1 + i);
                    moveStruct.currentPP[i] = GetMonData(GetBattlerMon(battler), MON_DATA_PP1 + i);
                }

                totalPPBonuses = GetMonData(GetBattlerMon(battler), MON_DATA_PP_BONUSES);
                for (i = 0; i < MAX_MON_MOVES; i++)
                    perMovePPBonuses[i] = (totalPPBonuses & (3 << (i * 2))) >> (i * 2);

                i = moveStruct.moves[gMoveSelectionCursor[battler]];
                moveStruct.moves[gMoveSelectionCursor[battler]] = moveStruct.moves[gMultiUsePlayerCursor];
                moveStruct.moves[gMultiUsePlayerCursor] = i;

                i = moveStruct.currentPP[gMoveSelectionCursor[battler]];
                moveStruct.currentPP[gMoveSelectionCursor[battler]] = moveStruct.currentPP[gMultiUsePlayerCursor];
                moveStruct.currentPP[gMultiUsePlayerCursor] = i;

                totalPPBonuses = perMovePPBonuses[gMoveSelectionCursor[battler]];
                perMovePPBonuses[gMoveSelectionCursor[battler]] = perMovePPBonuses[gMultiUsePlayerCursor];
                perMovePPBonuses[gMultiUsePlayerCursor] = totalPPBonuses;

                totalPPBonuses = 0;
                for (i = 0; i < MAX_MON_MOVES; i++)
                    totalPPBonuses |= perMovePPBonuses[i] << (i * 2);

                for (i = 0; i < MAX_MON_MOVES; i++)
                {
                    SetMonData(GetBattlerMon(battler), MON_DATA_MOVE1 + i, &moveStruct.moves[i]);
                    SetMonData(GetBattlerMon(battler), MON_DATA_PP1 + i, &moveStruct.currentPP[i]);
                }

                SetMonData(GetBattlerMon(battler), MON_DATA_PP_BONUSES, &totalPPBonuses);
            }
        }

        if (IS_FRLG && gBattleTypeFlags & BATTLE_TYPE_FIRST_BATTLE)
            gBattlerControllerFuncs[battler] = OakOldManHandleInputChooseMove;
        else
            gBattlerControllerFuncs[battler] = HandleInputChooseMove;
        gMoveSelectionCursor[battler] = gMultiUsePlayerCursor;
        MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
        if (B_SHOW_EFFECTIVENESS)
            MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
        else
            MoveSelectionDisplayPPString(battler);
        MoveSelectionDisplayPPNumber(battler);
        MoveSelectionDisplayMoveType(battler);
        AssignUsableZMoves(battler, moveInfo->moves);
    }
    else if (JOY_NEW(B_BUTTON | SELECT_BUTTON))
    {
        PlaySE(SE_SELECT);
        MoveSelectionDestroyCursorAt(gMultiUsePlayerCursor);
        MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);

        if (gBattleTypeFlags & BATTLE_TYPE_FIRST_BATTLE)
            gBattlerControllerFuncs[battler] = OakOldManHandleInputChooseMove;
        else
            gBattlerControllerFuncs[battler] = HandleInputChooseMove;

        if (B_SHOW_EFFECTIVENESS)
            MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
        else
            MoveSelectionDisplayPPString(battler);
        MoveSelectionDisplayPPNumber(battler);
        MoveSelectionDisplayMoveType(battler);
    }
    else if (JOY_NEW(DPAD_LEFT))
    {
        if (gMultiUsePlayerCursor & 1)
        {
            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 29);
            else
                MoveSelectionDestroyCursorAt(gMultiUsePlayerCursor);

            gMultiUsePlayerCursor ^= 1;
            PlaySE(SE_SELECT);

            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 0);
            else
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 27);
        }
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        if (!(gMultiUsePlayerCursor & 1) && (gMultiUsePlayerCursor ^ 1) < gNumberOfMovesToChoose)
        {
            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 29);
            else
                MoveSelectionDestroyCursorAt(gMultiUsePlayerCursor);

            gMultiUsePlayerCursor ^= 1;
            PlaySE(SE_SELECT);

            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 0);
            else
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 27);
        }
    }
    else if (JOY_NEW(DPAD_UP))
    {
        if (gMultiUsePlayerCursor & 2)
        {
            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 29);
            else
                MoveSelectionDestroyCursorAt(gMultiUsePlayerCursor);

            gMultiUsePlayerCursor ^= 2;
            PlaySE(SE_SELECT);

            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 0);
            else
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 27);
        }
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (!(gMultiUsePlayerCursor & 2) && (gMultiUsePlayerCursor ^ 2) < gNumberOfMovesToChoose)
        {
            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 29);
            else
                MoveSelectionDestroyCursorAt(gMultiUsePlayerCursor);

            gMultiUsePlayerCursor ^= 2;
            PlaySE(SE_SELECT);

            if (gMultiUsePlayerCursor == gMoveSelectionCursor[battler])
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 0);
            else
                MoveSelectionCreateCursorAt(gMultiUsePlayerCursor, 27);
        }
    }
}

static void SetLinkBattleEndCallbacks(enum BattlerId battler)
{
    if (gWirelessCommType == 0)
    {
        if (!gReceivedRemoteLinkPlayers)
        {
            m4aSongNumStop(SE_LOW_HEALTH);
            gMain.inBattle = FALSE;
            gMain.callback1 = gPreBattleCallback1;
            SetMainCallback2(CB2_InitEndLinkBattle);
            if (gBattleOutcome == B_OUTCOME_WON)
                TryPutLinkBattleTvShowOnAir();
            FreeAllWindowBuffers();
        }
    }
    else
    {
        if (IsLinkTaskFinished())
        {
            m4aSongNumStop(SE_LOW_HEALTH);
            gMain.inBattle = FALSE;
            gMain.callback1 = gPreBattleCallback1;
            SetMainCallback2(CB2_InitEndLinkBattle);
            if (gBattleOutcome == B_OUTCOME_WON)
                TryPutLinkBattleTvShowOnAir();
            FreeAllWindowBuffers();
        }
    }
}

// Despite handling link battles separately, this is only ever used by link battles
void SetBattleEndCallbacks(enum BattlerId battler)
{
    if (!gPaletteFade.active)
    {
        if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        {
            if (IsLinkTaskFinished())
            {
                if (gWirelessCommType == 0)
                    SetCloseLinkCallback();
                else
                    SetLinkStandbyCallback();

                gBattlerControllerFuncs[battler] = SetLinkBattleEndCallbacks;
            }
        }
        else
        {
            m4aSongNumStop(SE_LOW_HEALTH);
            gMain.inBattle = FALSE;
            gMain.callback1 = gPreBattleCallback1;
            SetMainCallback2(gMain.savedCallback);
        }
    }
}

static void Intro_WaitForShinyAnimAndHealthbox(enum BattlerId battler)
{
    bool8 healthboxAnimDone = FALSE;

    // Check if healthbox has finished sliding in
    if (TwoPlayerIntroMons(battler) && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
    {
        if (gSprites[gHealthboxSpriteIds[battler]].callback == SpriteCallbackDummy
         && gSprites[gHealthboxSpriteIds[GetPartnerBattler(battler)]].callback == SpriteCallbackDummy)
            healthboxAnimDone = TRUE;
    }
    else
    {
        if (gSprites[gHealthboxSpriteIds[battler]].callback == SpriteCallbackDummy)
            healthboxAnimDone = TRUE;
    }

    // If healthbox and shiny anim are done
    if (healthboxAnimDone && gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim
        && gBattleSpritesDataPtr->healthBoxesData[GetPartnerBattler(battler)].finishedShinyMonAnim)
    {
        // Reset shiny anim (even if it didn't occur)
        gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[GetPartnerBattler(battler)].triedShinyMonAnim = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[GetPartnerBattler(battler)].finishedShinyMonAnim = FALSE;
        FreeShinyStars();

        HandleLowHpMusicChange(GetBattlerMon(battler), battler);

        if (TwoPlayerIntroMons(battler))
            HandleLowHpMusicChange(GetBattlerMon(GetPartnerBattler(battler)), GetPartnerBattler(battler));

        gBattleSpritesDataPtr->healthBoxesData[battler].introEndDelay = 3;
        gBattlerControllerFuncs[battler] = BtlController_Intro_DelayAndEnd;
    }
}

static void Intro_TryShinyAnimShowHealthbox(enum BattlerId battler)
{
    bool32 bgmRestored = FALSE;
    bool32 battlerAnimsDone = FALSE;

    // Start shiny animation if applicable for 1st Pokémon
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim
     && !gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive)
        TryShinyAnimation(battler, GetBattlerMon(battler));

    // Start shiny animation if applicable for 2nd Pokémon
    if (!gBattleSpritesDataPtr->healthBoxesData[GetPartnerBattler(battler)].triedShinyMonAnim
     && !gBattleSpritesDataPtr->healthBoxesData[GetPartnerBattler(battler)].ballAnimActive)
        TryShinyAnimation(GetPartnerBattler(battler), GetBattlerMon(GetPartnerBattler(battler)));

    // Show healthbox after ball anim
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive
     && !gBattleSpritesDataPtr->healthBoxesData[GetPartnerBattler(battler)].ballAnimActive)
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[battler].healthboxSlideInStarted)
        {
            if (TwoPlayerIntroMons(battler) && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
            {
                UpdateHealthboxAttribute(gHealthboxSpriteIds[GetPartnerBattler(battler)], GetBattlerMon(GetPartnerBattler(battler)), HEALTHBOX_ALL);
                StartHealthboxSlideIn(GetPartnerBattler(battler));
                SetHealthboxSpriteVisible(gHealthboxSpriteIds[GetPartnerBattler(battler)]);
            }
            UpdateHealthboxAttribute(gHealthboxSpriteIds[battler], GetBattlerMon(battler), HEALTHBOX_ALL);
            StartHealthboxSlideIn(battler);
            SetHealthboxSpriteVisible(gHealthboxSpriteIds[battler]);
        }
        gBattleSpritesDataPtr->healthBoxesData[battler].healthboxSlideInStarted = TRUE;
    }

    // Restore bgm after cry has played and healthbox anim is started
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].waitForCry
        && gBattleSpritesDataPtr->healthBoxesData[battler].healthboxSlideInStarted
        && !gBattleSpritesDataPtr->healthBoxesData[GetPartnerBattler(battler)].waitForCry
        && !IsCryPlayingOrClearCrySongs())
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[battler].bgmRestored)
        {
            if (gBattleTypeFlags & BATTLE_TYPE_MULTI && gBattleTypeFlags & BATTLE_TYPE_LINK)
                m4aMPlayContinue(&gMPlayInfo_BGM);
            else
                m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 0x100);
        }
        gBattleSpritesDataPtr->healthBoxesData[battler].bgmRestored = TRUE;
        bgmRestored = TRUE;
    }

    // Wait for battler anims
    if (TwoPlayerIntroMons(battler) && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
    {
        if (gSprites[gBattleControllerData[battler]].callback == SpriteCallbackDummy
            && gSprites[gBattlerSpriteIds[battler]].callback == SpriteCallbackDummy
            && gSprites[gBattleControllerData[GetPartnerBattler(battler)]].callback == SpriteCallbackDummy
            && gSprites[gBattlerSpriteIds[GetPartnerBattler(battler)]].callback == SpriteCallbackDummy)
        {
            battlerAnimsDone = TRUE;
        }
    }
    else
    {
        if (gSprites[gBattleControllerData[battler]].callback == SpriteCallbackDummy
            && gSprites[gBattlerSpriteIds[battler]].callback == SpriteCallbackDummy)
        {
            battlerAnimsDone = TRUE;
        }
    }

    // Clean up
    if (bgmRestored && battlerAnimsDone)
    {
        if (TwoPlayerIntroMons(battler) && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
            DestroySprite(&gSprites[gBattleControllerData[GetPartnerBattler(battler)]]);
        DestroySprite(&gSprites[gBattleControllerData[battler]]);

        gBattleSpritesDataPtr->animationData->introAnimActive = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[battler].bgmRestored = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[battler].healthboxSlideInStarted = FALSE;

        gBattlerControllerFuncs[battler] = Intro_WaitForShinyAnimAndHealthbox;
    }
}

void Task_PlayerController_RestoreBgmAfterCry(u8 taskId)
{
    if (!IsCryPlayingOrClearCrySongs())
    {
        m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 0x100);
        DestroyTask(taskId);
    }
}

#define tExpTask_monId          data[0]
#define tExpTask_battler        data[2]
#define tExpTask_gainedExp_1    data[3]
#define tExpTask_gainedExp_2    data[4] // Stored as two half-words containing a word.
#define tExpTask_frames         data[10]

static void DynamaxModifyHPLevelUp(struct Pokemon *mon, enum BattlerId battler, u32 oldMaxHP)
{
    ApplyDynamaxHPMultiplier(mon);
    gBattleScripting.levelUpHP = GetMonData(mon, MON_DATA_MAX_HP) - oldMaxHP; // overwrite levelUpHP since it overflows
    gBattleMons[battler].hp += gBattleScripting.levelUpHP;
    SetMonData(mon, MON_DATA_HP, &gBattleMons[battler].hp);
}

static s32 GetTaskExpValue(u8 taskId)
{
    return (u16)(gTasks[taskId].tExpTask_gainedExp_1) | (gTasks[taskId].tExpTask_gainedExp_2 << 16);
}

static void Task_GiveExpToMon(u8 taskId)
{
    u32 monId = (u8)(gTasks[taskId].tExpTask_monId);
    enum BattlerId battler = gTasks[taskId].tExpTask_battler;
    s32 gainedExp = GetTaskExpValue(taskId);

    if (GetBattlerCoordsIndex(battler) == BATTLE_COORDS_DOUBLES || monId != gBattlerPartyIndexes[battler]) // Give exp without moving the expbar.
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][monId];
        enum Species species = GetMonData(mon, MON_DATA_SPECIES);
        u8 level = GetMonData(mon, MON_DATA_LEVEL);
        u32 currExp = GetMonData(mon, MON_DATA_EXP);
        u32 nextLvlExp = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1];
        u32 expAfterGain = currExp + gainedExp;
        u32 oldMaxHP = GetMonData(mon, MON_DATA_MAX_HP);

        if (expAfterGain >= nextLvlExp)
        {
            SetMonData(mon, MON_DATA_EXP, (B_LEVEL_UP_NOTIFICATION >= GEN_9) ? &expAfterGain : &nextLvlExp);

            CalculateMonStats(mon);

            // Reapply Dynamax HP multiplier after stats are recalculated.
            if (GetActiveGimmick(battler) == GIMMICK_DYNAMAX && monId == gBattlerPartyIndexes[battler])
                DynamaxModifyHPLevelUp(mon, battler, oldMaxHP);

            gainedExp -= nextLvlExp - currExp;
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, RET_VALUE_LEVELED_UP, (B_LEVEL_UP_NOTIFICATION >= GEN_9) ? 0 : gainedExp);

            if (IsDoubleBattle() == TRUE
             && (monId == gBattlerPartyIndexes[battler] || monId == gBattlerPartyIndexes[GetPartnerBattler(battler)]))
                gTasks[taskId].func = Task_LaunchLvlUpAnim;
            else
                gTasks[taskId].func = Task_SetControllerToWaitForString;
        }
        else
        {
            currExp += gainedExp;
            SetMonData(mon, MON_DATA_EXP, &currExp);
            gBattlerControllerFuncs[battler] = Controller_WaitForString;
            DestroyTask(taskId);
        }
    }
    else
    {
        gTasks[taskId].func = Task_PrepareToGiveExpWithExpBar;
    }
}

static void Task_PrepareToGiveExpWithExpBar(u8 taskId)
{
    u8 monIndex = gTasks[taskId].tExpTask_monId;
    s32 gainedExp = GetTaskExpValue(taskId);
    enum BattlerId battler = gTasks[taskId].tExpTask_battler;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][monIndex];
    u8 level = GetMonData(mon, MON_DATA_LEVEL);
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    u32 exp = GetMonData(mon, MON_DATA_EXP);
    u32 currLvlExp = gExperienceTables[gSpeciesInfo[species].growthRate][level];
    u32 expToNextLvl;

    exp -= currLvlExp;
    expToNextLvl = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1] - currLvlExp;
    SetBattleBarStruct(battler, gHealthboxSpriteIds[battler], expToNextLvl, exp, -gainedExp);
    TestRunner_Battle_RecordExp(battler, exp, -gainedExp);
    PlaySE(SE_EXP);
    gTasks[taskId].func = Task_GiveExpWithExpBar;
}

static void Task_GiveExpWithExpBar(u8 taskId)
{
    u32 level, expAfterGain;
    enum Species species;
    u32 oldMaxHP;
    s32 currExp, expOnNextLvl, newExpPoints;

    if (gTasks[taskId].tExpTask_frames < 13)
    {
        gTasks[taskId].tExpTask_frames++;
    }
    else
    {
        u8 monId = gTasks[taskId].tExpTask_monId;
        s32 gainedExp = GetTaskExpValue(taskId);
        enum BattlerId battler = gTasks[taskId].tExpTask_battler;
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][monId];

        newExpPoints = MoveBattleBar(battler, gHealthboxSpriteIds[battler], EXP_BAR, 0);
        SetHealthboxSpriteVisible(gHealthboxSpriteIds[battler]);
        if (newExpPoints == -1) // The bar has been filled with given exp points.
        {
            m4aSongNumStop(SE_EXP);
            level = GetMonData(mon, MON_DATA_LEVEL);
            currExp = GetMonData(mon, MON_DATA_EXP);
            species = GetMonData(mon, MON_DATA_SPECIES);
            oldMaxHP = GetMonData(mon, MON_DATA_MAX_HP);
            expOnNextLvl = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1];

            expAfterGain = currExp + gainedExp;
            if (expAfterGain >= expOnNextLvl)
            {
                if (B_LEVEL_UP_NOTIFICATION >= GEN_9)
                    SetMonData(mon, MON_DATA_EXP, &expAfterGain);
                else
                    SetMonData(mon, MON_DATA_EXP, &expOnNextLvl);

                CalculateMonStats(mon);

                // Reapply Dynamax HP multiplier after stats are recalculated.
                if (GetActiveGimmick(battler) == GIMMICK_DYNAMAX && monId == gBattlerPartyIndexes[battler])
                    DynamaxModifyHPLevelUp(mon, battler, oldMaxHP);

                gainedExp -= expOnNextLvl - currExp;
                BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, RET_VALUE_LEVELED_UP, (B_LEVEL_UP_NOTIFICATION >= GEN_9) ? 0 : gainedExp);
                gTasks[taskId].func = Task_LaunchLvlUpAnim;
            }
            else
            {
                currExp += gainedExp;
                SetMonData(mon, MON_DATA_EXP, &currExp);
                gBattlerControllerFuncs[battler] = Controller_WaitForString;
                DestroyTask(taskId);
            }
        }
    }
}

static void Task_LaunchLvlUpAnim(u8 taskId)
{
    enum BattlerId battler = gTasks[taskId].tExpTask_battler;
    u8 monIndex = gTasks[taskId].tExpTask_monId;

    if (IsDoubleBattle() == TRUE && monIndex == gBattlerPartyIndexes[GetPartnerBattler(battler)])
        battler ^= BIT_FLANK;

    InitAndLaunchSpecialAnimation(battler, battler, battler, B_ANIM_LVL_UP);
    gTasks[taskId].func = Task_UpdateLvlInHealthbox;
}

static void Task_UpdateLvlInHealthbox(u8 taskId)
{
    enum BattlerId battler = gTasks[taskId].tExpTask_battler;

    if (!gBattleSpritesDataPtr->healthBoxesData[battler].specialAnimActive)
    {
        u8 monIndex = gTasks[taskId].tExpTask_monId;

        if (IsDoubleBattle() == TRUE && monIndex == gBattlerPartyIndexes[GetPartnerBattler(battler)])
            UpdateHealthboxAttribute(gHealthboxSpriteIds[GetPartnerBattler(battler)], &gParties[B_TRAINER_PLAYER][monIndex], HEALTHBOX_ALL);
        else
            UpdateHealthboxAttribute(gHealthboxSpriteIds[battler], &gParties[B_TRAINER_PLAYER][monIndex], HEALTHBOX_ALL);

        gTasks[taskId].func = Task_SetControllerToWaitForString;
    }
}

static void Task_SetControllerToWaitForString(u8 taskId)
{
    enum BattlerId battler = gTasks[taskId].tExpTask_battler;
    gBattlerControllerFuncs[battler] = Controller_WaitForString;
    DestroyTask(taskId);
}

static void OpenPartyMenuToChooseMon(enum BattlerId battler)
{
    if (!gPaletteFade.active)
    {
        u8 caseId;

        gBattlerControllerFuncs[battler] = WaitForMonSelection;
        caseId = gTasks[gBattleControllerData[battler]].data[0];
        DestroyTask(gBattleControllerData[battler]);
        CloseMainBattleScreen();
        OpenPartyMenuInBattle(caseId);
    }
}

static void WaitForMonSelection(enum BattlerId battler)
{
    if (gMain.callback2 == BattleMainCB2 && !gPaletteFade.active)
    {
        if (gPartyMenuUseExitCallback == TRUE)
            BtlController_EmitChosenMonReturnValue(battler, B_COMM_TO_ENGINE, gSelectedMonPartyId, gBattlePartyCurrentOrder);
        else
            BtlController_EmitChosenMonReturnValue(battler, B_COMM_TO_ENGINE, PARTY_SIZE, NULL);

        if (gBattleResources->bufferA[battler][1] == PARTY_ACTION_SEND_OUT)
            PrintLinkStandbyMsg();

        BtlController_Complete(battler);
    }
}

static void OpenBagAndChooseItem(enum BattlerId battler)
{
    if (!gPaletteFade.active)
    {
        gBattlerControllerFuncs[battler] = CompleteWhenChoseItem;
        ReshowBattleScreenDummy();
        CloseMainBattleScreen();
        if (gBattleStruct->victoryCatchState == VICTORY_CATCH_OPEN_BAG)
            CB2_ChooseBall();
        else
            CB2_BagMenuFromBattle();
    }
}

static void CompleteWhenChoseItem(enum BattlerId battler)
{
    if (gMain.callback2 == BattleMainCB2 && !gPaletteFade.active)
    {
        BtlController_EmitOneReturnValue(battler, B_COMM_TO_ENGINE, gSpecialVar_ItemId);
        BtlController_Complete(battler);
    }
}

static void PlayerHandleYesNoInput(enum BattlerId battler)
{
    if (JOY_NEW(DPAD_UP) && gMultiUsePlayerCursor != 0)
    {
        PlaySE(SE_SELECT);
        BattleDestroyYesNoCursorAt(gMultiUsePlayerCursor);
        gMultiUsePlayerCursor = 0;
        BattleCreateYesNoCursorAt(0);
    }
    if (JOY_NEW(DPAD_DOWN) && gMultiUsePlayerCursor == 0)
    {
        PlaySE(SE_SELECT);
        BattleDestroyYesNoCursorAt(gMultiUsePlayerCursor);
        gMultiUsePlayerCursor = 1;
        BattleCreateYesNoCursorAt(1);
    }
    if (JOY_NEW(A_BUTTON))
    {
        HandleBattleWindow(YESNOBOX_X_Y, WINDOW_CLEAR);
        PlaySE(SE_SELECT);

        if (gMultiUsePlayerCursor != 0)
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_UNK_14, 0);
        else
            BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_NOTHING_FAINTED, 0);

        BtlController_Complete(battler);
    }
    if (JOY_NEW(B_BUTTON))
    {
        HandleBattleWindow(YESNOBOX_X_Y, WINDOW_CLEAR);
        PlaySE(SE_SELECT);
        BtlController_Complete(battler);
    }
}

// Rojo Fuego+ battle menus. Each action/move card owns a BG palette (12-15) so its colour icon
// keeps its own ramp; the prompt and the move info card share palette 10. Selecting a card only
// rewrites its palette (mint fill, gold cursor), so moving the cursor never redraws tiles.
#define RF_BATTLE_PANEL_PAL     10
#define RF_BATTLE_CELL_PAL(i)   (12 + (i))
#define RF_CELL_OUTSIDE         9   // textbox frame colour around the rounded cards
#define RF_CELL_EDGE            6
#define RF_CELL_CURSOR          7
#define RF_CELL_FILL            14
#define RF_CELL_HEIGHT          16
#define RF_CELL_TEXT_X          20
#define RF_MOVE_CELL_WIDTH      (10 * TILE_WIDTH)
#define RF_ACTION_CELL_WIDTH    (8 * TILE_WIDTH)
#define RF_ACTION_MENU_LEFT     13
#define RF_ACTION_MENU_TOP      35

enum
{
    RF_CELL_IDLE,
    RF_CELL_SELECTED,
    RF_CELL_HELD, // move picked up for rearranging
};

static const u8 sBattleCellTextColors[] = {TEXT_COLOR_TRANSPARENT, TEXT_DYNAMIC_COLOR_4, TEXT_DYNAMIC_COLOR_6};
static u8 sBattleMoveCellIcons[MAX_MON_MOVES];
static u8 sBattleActionCellIcons[4];

static void LoadBattleCellPalette(u32 bank, u32 icon, u32 state)
{
    u16 colors[16];

    CpuCopy16(&gPlttBufferUnfaded[BG_PLTT_ID(state == RF_CELL_SELECTED ? RF_UI_BATTLE_SELECTED_PAL : 5)], colors, sizeof(colors));
    colors[RF_CELL_OUTSIDE] = gPlttBufferUnfaded[BG_PLTT_ID(0) + RF_CELL_OUTSIDE];
    colors[RF_CELL_EDGE] = state == RF_CELL_HELD ? RF_UI_GOLD
                         : (KeySystem_IsOn(KEY_DARK_MODE) ? RF_UI_NIGHT_EDGE : RF_UI_DAY_EDGE);
    colors[RF_CELL_CURSOR] = state == RF_CELL_IDLE ? colors[RF_CELL_FILL] : RF_UI_GOLD;
    if (icon >= RF_BATTLE_ICON_COUNT)
        icon = TYPE_NORMAL;
    CpuCopy16(sRFBattleIconRamps[icon], &colors[1], sizeof(sRFBattleIconRamps[icon]));
    LoadPalette(colors, BG_PLTT_ID(bank), sizeof(colors));
}

// Rounded card on a filled interior, leaving a 1px frame-coloured gap on its right and bottom so
// neighbouring cards never merge into a double line.
static void DrawBattleCell(u8 window, u32 x, u32 y, u32 width, u32 height)
{
    u32 right = x + width - 2, bottom = y + height - 2;

    RFUI_FillRect(window, RF_CELL_FILL, x + 1, y + 1, width - 3, height - 3);
    RFUI_FillRect(window, RF_CELL_OUTSIDE, x + width - 1, y, 1, height);
    RFUI_FillRect(window, RF_CELL_OUTSIDE, x, y + height - 1, width, 1);
    RFUI_FillRect(window, RF_CELL_OUTSIDE, x, y, 2, 1);
    RFUI_FillRect(window, RF_CELL_OUTSIDE, x, y + 1, 1, 1);
    RFUI_FillRect(window, RF_CELL_OUTSIDE, right - 1, y, 2, 1);
    RFUI_FillRect(window, RF_CELL_OUTSIDE, right, y + 1, 1, 1);
    RFUI_FillRect(window, RF_CELL_OUTSIDE, x, bottom, 2, 1);
    RFUI_FillRect(window, RF_CELL_OUTSIDE, x, bottom - 1, 1, 1);
    RFUI_FillRect(window, RF_CELL_OUTSIDE, right - 1, bottom, 2, 1);
    RFUI_FillRect(window, RF_CELL_OUTSIDE, right, bottom - 1, 1, 1);
    RFUI_DrawRing(window, x, y, width - 1, height - 1, RF_CELL_EDGE);
}

// Cursor arrow (invisible while idle: its colour equals the fill) and the 12x12 colour icon.
static void DrawBattleCellIcon(u8 window, u32 icon, u32 x, u32 y)
{
    static const u8 sCursorWidths[] = {1, 2, 3, 2, 1};
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sCursorWidths); i++)
        RFUI_FillRect(window, RF_CELL_CURSOR, x + 2, y + 5 + i, sCursorWidths[i], 1);
    if (icon >= RF_BATTLE_ICON_COUNT)
        icon = TYPE_NORMAL;
    BlitBitmapRectToWindow(window, sRFBattleIconGfx, 0, icon * 16, 16, RF_BATTLE_ICON_COUNT * 16, x + 4, y, 16, 16);
}

// Prints a one-line label, switching to a narrower font and then an ellipsis when it does not fit.
static void PrintBattleCellLabel(u8 window, const u8 *text, u32 x, u32 y, u32 maxWidth, u32 font)
{
    u8 buffer[40];
    u32 length;

    if (GetStringWidth(font, text, 0) > maxWidth)
        font = FONT_NARROWER;
    if (GetStringWidth(font, text, 0) > maxWidth)
    {
        StringCopyN(buffer, text, sizeof(buffer) - 2);
        buffer[sizeof(buffer) - 2] = EOS;
        length = StringLength(buffer);
        while (length > 0 && GetStringWidth(font, buffer, 0) + GetStringWidth(font, COMPOUND_STRING("…"), 0) > maxWidth)
            buffer[--length] = EOS;
        buffer[length++] = CHAR_ELLIPSIS;
        buffer[length] = EOS;
        text = buffer;
    }
    AddTextPrinterParameterized3(window, font, x, y, sBattleCellTextColors, TEXT_SKIP_DRAW, text);
}

void DrawBattleActionMenu(const u8 *const *labels, const u8 *icons)
{
    u16 *tilemap = GetBgTilemapBuffer(0);
    u32 i, row, col;

    FillWindowPixelBuffer(B_WIN_ACTION_MENU, PIXEL_FILL(RF_CELL_OUTSIDE));
    for (i = 0; i < ARRAY_COUNT(sBattleActionCellIcons); i++)
    {
        u32 x = RF_ACTION_CELL_WIDTH * (i & 1), y = RF_CELL_HEIGHT * (i >> 1);

        sBattleActionCellIcons[i] = icons[i];
        DrawBattleCell(B_WIN_ACTION_MENU, x, y, RF_ACTION_CELL_WIDTH, RF_CELL_HEIGHT);
        DrawBattleCellIcon(B_WIN_ACTION_MENU, icons[i], x, y);
        PrintBattleCellLabel(B_WIN_ACTION_MENU, labels[i], x + RF_CELL_TEXT_X, y + 1,
                             RF_ACTION_CELL_WIDTH - RF_CELL_TEXT_X - 3, FONT_NORMAL);
        LoadBattleCellPalette(RF_BATTLE_CELL_PAL(i), icons[i], RF_CELL_IDLE);
    }
    PutWindowTilemap(B_WIN_ACTION_MENU);
    // One window, four palettes: retint each card's 8x2 tiles.
    for (i = 0; i < ARRAY_COUNT(sBattleActionCellIcons); i++)
    {
        for (row = 0; row < 2; row++)
        {
            for (col = 0; col < RF_ACTION_CELL_WIDTH / TILE_WIDTH; col++)
            {
                u32 index = (RF_ACTION_MENU_TOP + 2 * (i >> 1) + row) * 32
                          + RF_ACTION_MENU_LEFT + (RF_ACTION_CELL_WIDTH / TILE_WIDTH) * (i & 1) + col;
                tilemap[index] = (tilemap[index] & 0x0FFF) | (RF_BATTLE_CELL_PAL(i) << 12);
            }
        }
    }
    CopyWindowToVram(B_WIN_ACTION_MENU, COPYWIN_FULL);
}

static const u8 *const sBattleActionLabels[] =
{
    COMPOUND_STRING("Luchar"),
    COMPOUND_STRING("Bolsa"),
    COMPOUND_STRING("Equipo"),
    COMPOUND_STRING("Huir"),
};
static const u8 sBattleActionIcons[] = {TYPE_FIGHTING, RF_BATTLE_ICON_BAG, RF_BATTLE_ICON_BALL, RF_BATTLE_ICON_RUN};

void DrawStandardBattleActionMenu(void)
{
    DrawBattleActionMenu(sBattleActionLabels, sBattleActionIcons);
}

static const u8 *const sSafariActionLabels[] =
{
    COMPOUND_STRING("Bola"),
    COMPOUND_STRING("Cebo"),
    COMPOUND_STRING("Roca"),
    COMPOUND_STRING("Huir"),
};
static const u8 sSafariActionIcons[] = {RF_BATTLE_ICON_BALL, TYPE_GRASS, TYPE_ROCK, RF_BATTLE_ICON_RUN};

void DrawSafariBattleActionMenu(void)
{
    DrawBattleActionMenu(sSafariActionLabels, sSafariActionIcons);
}

// "¿Qué hará / X?" card to the left of the command grid.
void DrawBattleActionPrompt(const u8 *text)
{
    u32 width = WindowWidthPx(B_WIN_ACTION_PROMPT), height = 4 * TILE_HEIGHT;
    u32 font = GetFontIdToFit(text, FONT_NORMAL, 0, width - 9);

    FillWindowPixelBuffer(B_WIN_ACTION_PROMPT, PIXEL_FILL(RF_CELL_OUTSIDE));
    DrawBattleCell(B_WIN_ACTION_PROMPT, 0, 0, width, height);
    AddTextPrinterParameterized3(B_WIN_ACTION_PROMPT, font, 5, 1, sBattleCellTextColors, TEXT_SKIP_DRAW, text);
    LoadBattleCellPalette(RF_BATTLE_PANEL_PAL, TYPE_NORMAL, RF_CELL_IDLE);
    PutWindowTilemap(B_WIN_ACTION_PROMPT);
    CopyWindowToVram(B_WIN_ACTION_PROMPT, COPYWIN_FULL);
}

static void MoveSelectionDisplayMoveNames(enum BattlerId battler)
{
    s32 i;
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
    gNumberOfMovesToChoose = 0;

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        u32 windowId = i + B_WIN_MOVE_NAME_1;
        enum Type type = TYPE_MYSTERY;

        FillWindowPixelBuffer(windowId, PIXEL_FILL(RF_CELL_OUTSIDE));
        DrawBattleCell(windowId, 0, 0, RF_MOVE_CELL_WIDTH, RF_CELL_HEIGHT);
        if (moveInfo->moves[i] != MOVE_NONE)
        {
            if (IsGimmickSelected(battler, GIMMICK_DYNAMAX) || GetActiveGimmick(battler) == GIMMICK_DYNAMAX)
                StringCopy(gDisplayedStringBattle, GetMoveName(GetMaxMove(battler, moveInfo->moves[i])));
            else
                StringCopy(gDisplayedStringBattle, GetMoveName(moveInfo->moves[i]));
            type = GetMoveMenuDisplayType(battler, moveInfo->moves[i]);
            DrawBattleCellIcon(windowId, type, 0, 0);
            // The right edge stays free for the effectiveness marker of the selected move.
            PrintBattleCellLabel(windowId, gDisplayedStringBattle, RF_CELL_TEXT_X, 1,
                                 RF_MOVE_CELL_WIDTH - RF_CELL_TEXT_X - 11, FONT_NARROW);
            gNumberOfMovesToChoose++;
        }
        sBattleMoveCellIcons[i] = type;
        LoadBattleCellPalette(RF_BATTLE_CELL_PAL(i), type, RF_CELL_IDLE);
        PutWindowTilemap(windowId);
        CopyWindowToVram(windowId, COPYWIN_FULL);
    }
}

// Info card beside the move grid: PP of the selected move and a tag in its type colours.
static void MoveSelectionDisplayMoveInfo(enum BattlerId battler)
{
    static const u8 sPPColors[] = {TEXT_COLOR_TRANSPARENT, 12, 11};
    static const u8 sTagColors[] = {TEXT_COLOR_TRANSPARENT, 5, 1};
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
    enum Move move = moveInfo->moves[gMoveSelectionCursor[battler]];
    enum Type type = GetMoveMenuDisplayType(battler, move);
    u32 width = WindowWidthPx(B_WIN_PP), font, textWidth;
    u8 ppText[8], *ppEnd;

    if (type <= TYPE_NONE || type >= NUMBER_OF_MON_TYPES)
        type = TYPE_MYSTERY;
    SetPPNumbersPaletteInMoveSelection(battler);
    LoadBattleCellPalette(RF_BATTLE_PANEL_PAL, type, RF_CELL_IDLE);
    FillWindowPixelBuffer(B_WIN_PP, PIXEL_FILL(RF_CELL_OUTSIDE));
    DrawBattleCell(B_WIN_PP, 0, 0, width, 4 * TILE_HEIGHT);

    AddTextPrinterParameterized3(B_WIN_PP, FONT_NARROW, 5, 1, sBattleCellTextColors, TEXT_SKIP_DRAW, gText_MoveInterfacePP);
    if (gBattleResources->bufferA[battler][2] != TRUE)
    {
        ppEnd = ConvertIntToDecimalStringN(ppText, moveInfo->currentPP[gMoveSelectionCursor[battler]], STR_CONV_MODE_LEFT_ALIGN, 2);
        *(ppEnd++) = CHAR_SLASH;
        ConvertIntToDecimalStringN(ppEnd, moveInfo->maxPP[gMoveSelectionCursor[battler]], STR_CONV_MODE_LEFT_ALIGN, 2);
        textWidth = GetStringWidth(FONT_NARROW, ppText, 0);
        AddTextPrinterParameterized3(B_WIN_PP, FONT_NARROW, width - 6 - textWidth, 1, sPPColors, TEXT_SKIP_DRAW, ppText);
    }

    // Type tag: rounded pill in the type's base colour, its highlight as ink.
    RFUI_FillRect(B_WIN_PP, 3, 5, 16, width - 12, 11);
    RFUI_FillRect(B_WIN_PP, 3, 4, 17, width - 10, 9);
    RFUI_FillRect(B_WIN_PP, 2, 5, 26, width - 12, 1);
    font = GetFontIdToFit(gTypesInfo[type].name, FONT_SMALL, 0, width - 14);
    textWidth = GetStringWidth(font, gTypesInfo[type].name, 0);
    AddTextPrinterParameterized3(B_WIN_PP, font, (width - textWidth) / 2 - 1, 15, sTagColors, TEXT_SKIP_DRAW, gTypesInfo[type].name);

    PutWindowTilemap(B_WIN_PP);
    CopyWindowToVram(B_WIN_PP, COPYWIN_FULL);
}

static void MoveSelectionDisplayPPString(enum BattlerId battler)
{
    // "PP" is part of the info card drawn by MoveSelectionDisplayMoveInfo.
    (void)battler;
}

static void MoveSelectionDisplayPPNumber(enum BattlerId battler)
{
    MoveSelectionDisplayMoveInfo(battler);
}

static enum Type GetMoveMenuDisplayType(enum BattlerId battler, enum Move move)
{
    enum Species speciesId = gBattleMons[battler].species;
    enum Type type = GetMoveType(move);
    enum BattleMoveEffects effect = GetMoveEffect(move);

    if (effect == EFFECT_TERA_BLAST)
    {
        if (IsGimmickSelected(battler, GIMMICK_TERA) || GetActiveGimmick(battler) == GIMMICK_TERA)
            type = GetBattlerTeraType(battler);
    }
    else if (effect == EFFECT_IVY_CUDGEL)
    {
        if (speciesId == SPECIES_OGERPON_WELLSPRING || speciesId == SPECIES_OGERPON_WELLSPRING_TERA
         || speciesId == SPECIES_OGERPON_HEARTHFLAME || speciesId == SPECIES_OGERPON_HEARTHFLAME_TERA
         || speciesId == SPECIES_OGERPON_CORNERSTONE || speciesId == SPECIES_OGERPON_CORNERSTONE_TERA)
            type = GetSpeciesType(speciesId, 1);
    }
    else if (GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS
             && (GetActiveGimmick(battler) == GIMMICK_DYNAMAX || IsGimmickSelected(battler, GIMMICK_DYNAMAX)))
    {
        type = TYPE_NORMAL; // Max Guard is always a Normal-type move
    }
    else if (effect == EFFECT_TERA_STARSTORM)
    {
        if (speciesId == SPECIES_TERAPAGOS_STELLAR
        || (IsGimmickSelected(battler, GIMMICK_TERA) && speciesId == SPECIES_TERAPAGOS_TERASTAL))
            type = TYPE_STELLAR;
    }
    else if (P_SHOW_DYNAMIC_TYPES) // Non-vanilla changes to battle UI showing dynamic types
    {
        struct Pokemon *mon = GetBattlerMon(battler);
        type = CheckDynamicMoveType(mon, move, battler, MON_IN_BATTLE);
    }
    return type;
}

static void MoveSelectionDisplayMoveType(enum BattlerId battler)
{
    // The type tag lives in the info card, refreshed together with the PP.
    (void)battler;
}

// Rojo Fuego+: appends "At+2 Ve-1 ..." for every stat stage that is not neutral
static void AppendStatStages(u8 *dest, enum BattlerId battler)
{
    static const u8 *const sStatNames[NUM_BATTLE_STATS] =
    {
        [STAT_HP]      = COMPOUND_STRING("PS"),
        [STAT_ATK]     = COMPOUND_STRING("At"),
        [STAT_DEF]     = COMPOUND_STRING("Df"),
        [STAT_SPEED]   = COMPOUND_STRING("Ve"),
        [STAT_SPATK]   = COMPOUND_STRING("AE"),
        [STAT_SPDEF]   = COMPOUND_STRING("DE"),
        [STAT_ACC]     = COMPOUND_STRING("Pr"),
        [STAT_EVASION] = COMPOUND_STRING("Ev"),
    };
    u32 stat;
    bool32 any = FALSE;

    for (stat = STAT_ATK; stat < NUM_BATTLE_STATS; stat++)
    {
        s32 stage = gBattleMons[battler].statStages[stat] - DEFAULT_STAT_STAGE;
        if (stage == 0)
            continue;
        StringAppend(dest, sStatNames[stat]);
        StringAppend(dest, stage > 0 ? COMPOUND_STRING("+") : COMPOUND_STRING("-"));
        ConvertIntToDecimalStringN(dest + StringLength(dest), abs(stage), STR_CONV_MODE_LEFT_ALIGN, 1);
        StringAppend(dest, COMPOUND_STRING(" "));
        any = TRUE;
    }
    if (!any)
        StringAppend(dest, COMPOUND_STRING("sin cambios"));
}

static void MoveSelectionDisplayStatStages(enum BattlerId battler)
{
    enum BattlerId foe;

    if (gCategoryIconSpriteId != 0xFF)
    {
        DestroySprite(&gSprites[gCategoryIconSpriteId]);
        gCategoryIconSpriteId = 0xFF;
    }
    LoadMessageBoxAndBorderGfx();
    DrawStdWindowFrame(B_WIN_MOVE_DESCRIPTION, FALSE);

    StringCopy(gDisplayedStringBattle, COMPOUND_STRING("TÚ: "));
    AppendStatStages(gDisplayedStringBattle, battler);
    StringAppend(gDisplayedStringBattle, gText_NewLine);
    StringAppend(gDisplayedStringBattle, COMPOUND_STRING("RIVAL: "));
    foe = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
    if (!IsBattlerAlive(foe) && IsDoubleBattle())
        foe = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
    AppendStatStages(gDisplayedStringBattle, foe);
    if (IsDoubleBattle() && IsBattlerAlive(GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT))
     && foe != GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT))
    {
        StringAppend(gDisplayedStringBattle, COMPOUND_STRING("· "));
        AppendStatStages(gDisplayedStringBattle, GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT));
    }
    BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MOVE_DESCRIPTION);
    CopyWindowToVram(B_WIN_MOVE_DESCRIPTION, COPYWIN_FULL);
}

static void TryMoveSelectionDisplayMoveDescription(enum BattlerId battler)
{
    if (!B_SHOW_MOVE_DESCRIPTION)
        return;

    if (gBattleStruct->descriptionSubmenu)
        MoveSelectionDisplayMoveDescription(battler);
}

static void MoveSelectionDisplayMoveDescription(enum BattlerId battler)
{
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct*)(&gBattleResources->bufferA[battler][4]);
    enum Move move = moveInfo->moves[gMoveSelectionCursor[battler]];
    u16 pwr = GetMovePower(move);
    u16 acc = GetMoveAccuracy(move);
    enum DamageCategory cat = GetBattleMoveCategory(move);

    if (GetActiveGimmick(battler) == GIMMICK_DYNAMAX || IsGimmickSelected(battler, GIMMICK_DYNAMAX))
    {
        pwr = GetMaxMovePower(move, move);
        move = GetMaxMove(battler, move);
        acc = 0;
    }

    u8 pwr_num[3], acc_num[3];
    u8 cat_desc[7] = _("CAT: ");
    u8 pwr_desc[7] = _("PWR: ");
    u8 acc_desc[7] = _("ACC: ");
    u8 cat_start[] = _("{CLEAR_TO 3}");
    u8 pwr_start[] = _("{CLEAR_TO 56}");
    u8 acc_start[] = _("{CLEAR_TO 108}");
    LoadMessageBoxAndBorderGfx();
    DrawStdWindowFrame(B_WIN_MOVE_DESCRIPTION, FALSE);
    if (pwr < 2)
        StringCopy(pwr_num, gText_BattleSwitchWhich5);
    else
        ConvertIntToDecimalStringN(pwr_num, pwr, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (acc < 2)
        StringCopy(acc_num, gText_BattleSwitchWhich5);
    else
        ConvertIntToDecimalStringN(acc_num, acc, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringCopy(gDisplayedStringBattle, cat_start);
    StringAppend(gDisplayedStringBattle, cat_desc);
    StringAppend(gDisplayedStringBattle, pwr_start);
    StringAppend(gDisplayedStringBattle, pwr_desc);
    StringAppend(gDisplayedStringBattle, pwr_num);
    StringAppend(gDisplayedStringBattle, acc_start);
    StringAppend(gDisplayedStringBattle, acc_desc);
    StringAppend(gDisplayedStringBattle, acc_num);
    u8 *descStart = StringAppend(gDisplayedStringBattle, gText_NewLine);
    u8 *descEnd = StringAppend(gDisplayedStringBattle, GetMoveDescription(move));
    WrapFontIdToFit(descStart, descEnd, FONT_NORMAL, WindowWidthPx(B_WIN_MOVE_DESCRIPTION));
    BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MOVE_DESCRIPTION);

    if (gCategoryIconSpriteId == 0xFF)
        gCategoryIconSpriteId = CreateSprite(&gSpriteTemplate_CategoryIcons, 38, 64, 1);

    StartSpriteAnim(&gSprites[gCategoryIconSpriteId], cat);

    CopyWindowToVram(B_WIN_MOVE_DESCRIPTION, COPYWIN_FULL);
}

// Rojo Fuego+: the selected action/move is a gold cell (arrow column + label), as in the start
// menu. Only the palette bits of the BG0 tilemap change; the printed text stays as it is.
void MoveSelectionCreateCursorAt(u8 cursorPosition, u8 baseTileNum)
{
    if (cursorPosition >= MAX_MON_MOVES)
        return;
    // baseTileNum 29 marks the move picked up for rearranging; 0 and 27 are the live cursor.
    LoadBattleCellPalette(RF_BATTLE_CELL_PAL(cursorPosition), sBattleMoveCellIcons[cursorPosition],
                          baseTileNum == 29 ? RF_CELL_HELD : RF_CELL_SELECTED);
}

void MoveSelectionDestroyCursorAt(u8 cursorPosition)
{
    if (cursorPosition >= MAX_MON_MOVES)
        return;
    LoadBattleCellPalette(RF_BATTLE_CELL_PAL(cursorPosition), sBattleMoveCellIcons[cursorPosition], RF_CELL_IDLE);
}

void ActionSelectionCreateCursorAt(u8 cursorPosition, u8 baseTileNum)
{
    if (cursorPosition >= ARRAY_COUNT(sBattleActionCellIcons))
        return;
    LoadBattleCellPalette(RF_BATTLE_CELL_PAL(cursorPosition), sBattleActionCellIcons[cursorPosition], RF_CELL_SELECTED);
}

void ActionSelectionDestroyCursorAt(u8 cursorPosition)
{
    if (cursorPosition >= ARRAY_COUNT(sBattleActionCellIcons))
        return;
    LoadBattleCellPalette(RF_BATTLE_CELL_PAL(cursorPosition), sBattleActionCellIcons[cursorPosition], RF_CELL_IDLE);
}

void CB2_SetUpReshowBattleScreenAfterMenu(void)
{
    SetMainCallback2(ReshowBattleScreenAfterMenu);
}

void CB2_SetUpReshowBattleScreenAfterMenu2(void)
{
    SetMainCallback2(ReshowBattleScreenAfterMenu);
}

static void PrintLinkStandbyMsg(void)
{
    if (gBattleTypeFlags & BATTLE_TYPE_LINK)
    {
        gBattle_BG0_X = 0;
        gBattle_BG0_Y = 0;
        BattlePutTextOnWindow(gText_LinkStandby, B_WIN_MSG);
    }
}

static void PlayerHandleLoadMonSprite(enum BattlerId battler)
{
    BattleLoadMonSpriteGfx(GetBattlerMon(battler), battler);
    gSprites[gBattlerSpriteIds[battler]].oam.paletteNum = battler;
    gBattlerControllerFuncs[battler] = CompleteOnBattlerSpritePosX_0;
}

enum TrainerPicID LinkPlayerGetTrainerPicId(u32 multiplayerId)
{
    u8 gender = gLinkPlayers[multiplayerId].gender;
    enum GameVersion version = gLinkPlayers[multiplayerId].version & 0xFF;

    return GetPlayerTrainerPic(gender, version);
}

static enum TrainerPicID PlayerGetTrainerBackPicId(void)
{
    enum TrainerPicID trainerPicId;

    if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        trainerPicId = LinkPlayerGetTrainerPicId(GetMultiplayerId());
    else
        trainerPicId = GetPlayerTrainerPic(gSaveBlock2Ptr->playerGender, GAME_VERSION);

    return trainerPicId;
}

// In emerald it's possible to have a tag battle in the battle frontier facilities with AI
// which use the front sprite for both the player and the partner as opposed to any other battles (including the one with Steven)
// that use an animated back pic.
static void PlayerHandleDrawTrainerPic(enum BattlerId battler)
{
    bool32 isFrontPic;
    s16 xPos, yPos;
    enum TrainerPicID trainerPicId;

    if (TESTING)
    {
        trainerPicId = TRAINER_PIC_BRENDAN;
        if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER)
            xPos = 32;
        else
            xPos = 80;
        yPos = (8 - GetTrainerBackPicCoords(trainerPicId)->size) * 4 + 80;
    }
    else
    {
        trainerPicId = PlayerGetTrainerBackPicId();

        if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
        {
            if ((GetBattlerPosition(battler) & BIT_FLANK) != B_FLANK_LEFT) // Second mon, on the right.
                xPos = 90;
            else // First mon, on the left.
                xPos = 32;

            if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER && gPartnerTrainerId < TRAINER_PARTNER(PARTNER_NONE))
            {
                xPos = 90;
                yPos = 80;
            }
            else
            {
                yPos = (8 - GetTrainerBackPicCoords(trainerPicId)->size) * 4 + 80;
            }
        }
        else
        {
            xPos = 80;
            yPos = (8 - GetTrainerBackPicCoords(trainerPicId)->size) * 4 + 80;
        }
    }

    // Use front pic table for any tag battles unless your partner is Steven or a custom partner.
    if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER && gPartnerTrainerId < TRAINER_PARTNER(PARTNER_NONE))
    {
        trainerPicId = PlayerGenderToFrontTrainerPicId(gSaveBlock2Ptr->playerGender);
        isFrontPic = TRUE;
    }
    else // Use back pic in all the other usual circumstances.
    {
        isFrontPic = FALSE;
    }

    BtlController_HandleDrawTrainerPic(battler, trainerPicId, isFrontPic, xPos, yPos, -1);
}

static void PlayerHandleTrainerSlide(enum BattlerId battler)
{
    enum TrainerPicID trainerPicId = PlayerGetTrainerBackPicId();
    BtlController_HandleTrainerSlide(battler, trainerPicId);
}

static void PlayerHandleTrainerSlideBack(enum BattlerId battler)
{
    BtlController_HandleTrainerSlideBack(battler, 50, TRUE);
}

static void PlayerHandlePaletteFade(enum BattlerId battler)
{
    BeginNormalPaletteFade(PALETTES_ALL, 2, 0, 16, RGB_BLACK);
    BtlController_Complete(battler);
}

static void PlayerHandlePause(enum BattlerId battler)
{
    u8 timer = gBattleResources->bufferA[battler][1];

    while (timer != 0)
        timer--;

    BtlController_Complete(battler);
}

static void HandleChooseActionAfterDma3(enum BattlerId battler)
{
    if (!IsDma3ManagerBusyWithBgCopy())
    {
        gBattle_BG0_X = 0;
        gBattle_BG0_Y = DISPLAY_HEIGHT;
        if (gBattleStruct->aiDelayTimer != 0)
        {
            if (DEBUG_AI_DELAY_TIMER)
            {
                static const u8 sFramesText[] = _(" frames thinking\n");
                static const u8 sCyclesText[] = _(" cycles");
                ConvertIntToDecimalStringN(gDisplayedStringBattle, gBattleStruct->aiDelayFrames, STR_CONV_MODE_RIGHT_ALIGN, 3);
                u8* end = StringAppend(gDisplayedStringBattle, sFramesText);
                ConvertIntToDecimalStringN(end, gBattleStruct->aiDelayCycles, STR_CONV_MODE_RIGHT_ALIGN, 8);
                // Clear old result once read out
                gBattleStruct->aiDelayCycles = 0;
                StringAppend(gDisplayedStringBattle, sCyclesText);
                BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_ACTION_PROMPT);
            }
            gBattleStruct->aiDelayTimer = 0;
            gBattleStruct->aiDelayFrames = 0;
        }
        gBattlerControllerFuncs[battler] = HandleInputChooseAction;
    }
}

static void PlayerHandleChooseAction(enum BattlerId battler)
{
    s32 i;

    gBattlerControllerFuncs[battler] = HandleChooseActionAfterDma3;
    BattleTv_ClearExplosionFaintCause();
    DrawStandardBattleActionMenu();

    for (i = 0; i < 4; i++)
        ActionSelectionDestroyCursorAt(i);

    TryRestoreLastUsedBall();
    ActionSelectionCreateCursorAt(gActionSelectionCursor[battler], 0);
    PREPARE_MON_NICK_BUFFER(gBattleTextBuff1, battler, gBattlerPartyIndexes[battler]);
    BattleStringExpandPlaceholdersToDisplayedString(gText_WhatWillPkmnDo);

    enum BattlerId partner = GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT);
    if (B_SHOW_PARTNER_TARGET && gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER && IsBattlerAlive(partner))
    {
        StringCopy(gStringVar1, COMPOUND_STRING("Compañero:\n"));
        enum Move move = GetBattlerChosenMove(partner);
        StringAppend(gStringVar1, GetMoveName(move));
        enum MoveTarget moveTarget = GetBattlerMoveSelectionTargetType(partner, move);
        if (moveTarget == TARGET_SELECTED || moveTarget == TARGET_SMART)
        {
            if (gAiBattleData->chosenTarget[partner] == B_POSITION_OPPONENT_LEFT)
                StringAppend(gStringVar1, COMPOUND_STRING(" -{UP_ARROW}"));
            else if (gAiBattleData->chosenTarget[partner] == B_POSITION_OPPONENT_RIGHT)
                StringAppend(gStringVar1, COMPOUND_STRING(" {UP_ARROW}-"));
            else if (gAiBattleData->chosenTarget[partner] == B_POSITION_PLAYER_LEFT)
                StringAppend(gStringVar1, COMPOUND_STRING(" {DOWN_ARROW}-"));
            else if (gAiBattleData->chosenTarget[partner] == B_POSITION_PLAYER_RIGHT)
                StringAppend(gStringVar1, COMPOUND_STRING(" -{DOWN_ARROW}"));
        }
        else if (moveTarget == TARGET_USER_AND_ALLY)
        {
            StringAppend(gStringVar1, COMPOUND_STRING(" {DOWN_ARROW}{DOWN_ARROW}"));
        }
        else if (moveTarget == TARGET_BOTH)
        {
            StringAppend(gStringVar1, COMPOUND_STRING(" {UP_ARROW}{UP_ARROW}"));
        }
        else if (moveTarget == TARGET_FOES_AND_ALLY)
        {
            StringAppend(gStringVar1, COMPOUND_STRING(" {V_D_ARROW}{UP_ARROW}"));
        }
        else if (moveTarget == TARGET_ALL_BATTLERS || moveTarget == TARGET_FIELD)
        {
            StringAppend(gStringVar1, COMPOUND_STRING(" {V_D_ARROW}{V_D_ARROW}"));
        }
        DrawBattleActionPrompt(gStringVar1);
    }
    else
    {
        DrawBattleActionPrompt(gDisplayedStringBattle);
    }
}

static void PlayerHandleYesNoBox(enum BattlerId battler)
{
    if (IsOnPlayerSide(battler))
    {
        HandleBattleWindow(YESNOBOX_X_Y, 0);
        BattlePutTextOnWindow(gText_BattleYesNoChoice, B_WIN_YESNO);
        gMultiUsePlayerCursor = 1;
        BattleCreateYesNoCursorAt(1);
        gBattlerControllerFuncs[battler] = PlayerHandleYesNoInput;
    }
    else
    {
        BtlController_Complete(battler);
    }
}

void HandleChooseMoveAfterDma3(enum BattlerId battler)
{
    if (!IsDma3ManagerBusyWithBgCopy())
    {
        gBattle_BG0_X = 0;
        gBattle_BG0_Y = DISPLAY_HEIGHT * 2;
        gBattlerControllerFuncs[battler] = HandleInputChooseMove;
    }
}

// arenaMindPoints is used here as a placeholder for a timer.

static void PlayerChooseMoveInBattlePalace(enum BattlerId battler)
{
    if (--gBattleStruct->arenaMindPoints[battler] == 0)
    {
        gBattlePalaceMoveSelectionRngValue = gRngValue;
        BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, ChooseMoveAndTargetInBattlePalace(battler));
        BtlController_Complete(battler);
    }
}

void PlayerHandleChooseMove(enum BattlerId battler)
{
    if (gBattleTypeFlags & BATTLE_TYPE_PALACE)
    {
        gBattleStruct->arenaMindPoints[battler] = 8;
        gBattlerControllerFuncs[battler] = PlayerChooseMoveInBattlePalace;
    }
    else
    {
        struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);

        InitMoveSelectionsVarsAndStrings(battler);
        gBattleStruct->gimmick.playerSelect = FALSE;
        TryToAddMoveInfoWindow();

        AssignUsableZMoves(battler, moveInfo->moves);
        gBattleStruct->zmove.viable = (gBattleStruct->zmove.possibleZMoves[battler] & (1u << gMoveSelectionCursor[battler])) != 0;

        if (!IsGimmickTriggerSpriteActive())
            gBattleStruct->gimmick.triggerSpriteId = 0xFF;
        else if (!IsGimmickTriggerSpriteMatchingBattler(battler))
            DestroyGimmickTriggerSprite();
        if (!(gBattleStruct->gimmick.usableGimmick[battler] == GIMMICK_Z_MOVE && !gBattleStruct->zmove.viable))
            CreateGimmickTriggerSprite(battler);

        gBattlerControllerFuncs[battler] = HandleChooseMoveAfterDma3;
    }
}

void InitMoveSelectionsVarsAndStrings(enum BattlerId battler)
{
    LoadTypeIcons(battler);
    MoveSelectionDisplayMoveNames(battler);
    gMultiUsePlayerCursor = 0xFF;
    MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
    if (B_SHOW_EFFECTIVENESS)
        MoveSelectionDisplayMoveEffectiveness(CheckTargetTypeEffectiveness(battler), battler);
    else
        MoveSelectionDisplayPPString(battler);
    MoveSelectionDisplayPPNumber(battler);
    MoveSelectionDisplayMoveType(battler);
}

static void PlayerHandleChooseItem(enum BattlerId battler)
{
    s32 i;

    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
    gBattlerControllerFuncs[battler] = OpenBagAndChooseItem;
    gBattlerInMenuId = battler;

    for (i = 0; i < ARRAY_COUNT(gBattlePartyCurrentOrder); i++)
        gBattlePartyCurrentOrder[i] = gBattleResources->bufferA[battler][1 + i];
}

static void PlayerHandleChoosePokemon(enum BattlerId battler)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(gBattlePartyCurrentOrder); i++)
        gBattlePartyCurrentOrder[i] = gBattleResources->bufferA[battler][4 + i];

    memcpy(gBattleStruct->battlerPartyOrders[battler], gBattlePartyCurrentOrder, sizeof(gBattlePartyCurrentOrder));

    if (gBattleTypeFlags & BATTLE_TYPE_ARENA && gBattleResources->bufferA[battler][1] != PARTY_ACTION_CANT_SWITCH
        && gBattleResources->bufferA[battler][1] != PARTY_ACTION_CHOOSE_FAINTED_MON
        && gBattleResources->bufferA[battler][1] != PARTY_ACTION_SEND_MON_TO_BOX)
    {
        BtlController_EmitChosenMonReturnValue(battler, B_COMM_TO_ENGINE, gBattlerPartyIndexes[battler] + 1, gBattlePartyCurrentOrder);
        BtlController_Complete(battler);
    }
    else
    {
        gBattleControllerData[battler] = CreateTask(TaskDummy, 0xFF);
        gTasks[gBattleControllerData[battler]].data[0] = gBattleResources->bufferA[battler][1];
        *(&gBattleStruct->battlerPreventingSwitchout) = gBattleResources->bufferA[battler][8];
        *(&gBattleStruct->prevSelectedPartySlot) = gBattleResources->bufferA[battler][2];
        *(&gBattleStruct->abilityPreventingSwitchout) = (gBattleResources->bufferA[battler][3] & 0xFF) | (gBattleResources->bufferA[battler][7] << 8);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
        gBattlerControllerFuncs[battler] = OpenPartyMenuToChooseMon;
        gBattlerInMenuId = battler;
    }
}

static void PlayerHandleCmd23(enum BattlerId battler)
{
    BattleStopLowHpSound();
    BeginNormalPaletteFade(PALETTES_ALL, 2, 0, 16, RGB_BLACK);
    BtlController_Complete(battler);
}

void PlayerHandleExpUpdate(enum BattlerId battler)
{
    u8 monId = gBattleResources->bufferA[battler][1];
    s32 taskId, expPointsToGive;

    if (GetMonData(&gParties[B_TRAINER_PLAYER][monId], MON_DATA_LEVEL) >= MAX_LEVEL)
    {
        BtlController_Complete(battler);
    }
    else
    {
        LoadBattleBarGfx(1);
        expPointsToGive = T1_READ_32(&gBattleResources->bufferA[battler][2]);
        taskId = CreateTask(Task_GiveExpToMon, 10);
        gTasks[taskId].tExpTask_monId = monId;
        gTasks[taskId].tExpTask_gainedExp_1 = expPointsToGive;
        gTasks[taskId].tExpTask_gainedExp_2 = expPointsToGive >> 16;
        gTasks[taskId].tExpTask_battler = battler;
        gBattlerControllerFuncs[battler] = BattleControllerDummy;
    }
}

#undef tExpTask_monId
#undef tExpTask_battler
#undef tExpTask_gainedExp_1
#undef tExpTask_gainedExp_2
#undef tExpTask_frames

static void PlayerHandleStatusXor(enum BattlerId battler)
{
    u32 val = GetMonData(GetBattlerMon(battler), MON_DATA_STATUS) ^ gBattleResources->bufferA[battler][1];

    SetMonData(GetBattlerMon(battler), MON_DATA_STATUS, &val);
    BtlController_Complete(battler);
}

static void PlayerHandleDMA3Transfer(enum BattlerId battler)
{
    u32 dstArg = gBattleResources->bufferA[battler][1]
            | (gBattleResources->bufferA[battler][2] << 8)
            | (gBattleResources->bufferA[battler][3] << 16)
            | (gBattleResources->bufferA[battler][4] << 24);
    u16 sizeArg = gBattleResources->bufferA[battler][5] | (gBattleResources->bufferA[battler][6] << 8);

    DmaCopyLarge16(3, &gBattleResources->bufferA[battler][7], (void *)dstArg, sizeArg, 0x1000);
    BtlController_Complete(battler);
}

static void PlayerHandlePlayBGM(enum BattlerId battler)
{
    PlayBGM(gBattleResources->bufferA[battler][1] | (gBattleResources->bufferA[battler][2] << 8));
    BtlController_Complete(battler);
}

static void PlayerHandleTwoReturnValues(enum BattlerId battler)
{
    BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, B_ACTION_USE_MOVE, 0);
    BtlController_Complete(battler);
}

static void PlayerHandleChosenMonReturnValue(enum BattlerId battler)
{
    BtlController_EmitChosenMonReturnValue(battler, B_COMM_TO_ENGINE, 0, NULL);
    BtlController_Complete(battler);
}

static void PlayerHandleOneReturnValue(enum BattlerId battler)
{
    BtlController_EmitOneReturnValue(battler, B_COMM_TO_ENGINE, 0);
    BtlController_Complete(battler);
}

static void PlayerHandleOneReturnValue_Duplicate(enum BattlerId battler)
{
    BtlController_EmitOneReturnValue_Duplicate(battler, B_COMM_TO_ENGINE, 0);
    BtlController_Complete(battler);
}

static void PlayerHandleIntroTrainerBallThrow(enum BattlerId battler)
{
    enum TrainerPicID trainerPicID = PlayerGetTrainerBackPicId();
    const u16 *trainerPal = GetTrainerBackPicPalette(trainerPicID);
    BtlController_HandleIntroTrainerBallThrow(battler, 0xD6F8, trainerPal, 31, Intro_TryShinyAnimShowHealthbox);
}

static void PlayerHandleDrawPartyStatusSummary(enum BattlerId battler)
{
    BtlController_HandleDrawPartyStatusSummary(battler, B_SIDE_PLAYER, TRUE);
}

static void PlayerHandleEndBounceEffect(enum BattlerId battler)
{
    EndBounceEffect(battler, BOUNCE_HEALTHBOX);
    EndBounceEffect(battler, BOUNCE_MON);
    BtlController_Complete(battler);
}

static void PlayerHandleLinkStandbyMsg(enum BattlerId battler)
{
    RecordedBattle_RecordAllBattlerData(&gBattleResources->bufferA[battler][2]);
    switch (gBattleResources->bufferA[battler][1])
    {
    case LINK_STANDBY_MSG_STOP_BOUNCE:
        PrintLinkStandbyMsg();
        // fall through
    case LINK_STANDBY_STOP_BOUNCE_ONLY:
        EndBounceEffect(battler, BOUNCE_HEALTHBOX);
        EndBounceEffect(battler, BOUNCE_MON);
        break;
    case LINK_STANDBY_MSG_ONLY:
        PrintLinkStandbyMsg();
        break;
    }
    BtlController_Complete(battler);
}

static void PlayerHandleResetActionMoveSelection(enum BattlerId battler)
{
    switch (gBattleResources->bufferA[battler][1])
    {
    case RESET_ACTION_MOVE_SELECTION:
        gActionSelectionCursor[battler] = 0;
        gMoveSelectionCursor[battler] = 0;
        break;
    case RESET_ACTION_SELECTION:
        gActionSelectionCursor[battler] = 0;
        break;
    case RESET_MOVE_SELECTION:
        gMoveSelectionCursor[battler] = 0;
        break;
    }
    BtlController_Complete(battler);
}

static void PlayerHandleEndLinkBattle(enum BattlerId battler)
{
    RecordedBattle_RecordAllBattlerData(&gBattleResources->bufferA[battler][4]);
    gBattleOutcome = gBattleResources->bufferA[battler][1];
    gSaveBlock2Ptr->frontier.disableRecordBattle = gBattleResources->bufferA[battler][2];
    FadeOutMapMusic(5);
    BeginFastPaletteFade(3);
    BtlController_Complete(battler);
    gBattlerControllerFuncs[battler] = SetBattleEndCallbacks;
}

static void Controller_WaitForDebug(enum BattlerId battler)
{
    if (gMain.callback2 == BattleMainCB2 && !gPaletteFade.active)
    {
        BtlController_Complete(battler);
    }
}

static void PlayerHandleBattleDebug(enum BattlerId battler)
{
    BeginNormalPaletteFade(-1, 0, 0, 0x10, 0);
    SetMainCallback2(CB2_BattleDebugMenu);
    gBattlerControllerFuncs[battler] = Controller_WaitForDebug;
}

// Order based numerically, with EFFECTIVENESS_CANNOT_VIEW at 0 to always prioritize any other effectiveness during comparison
enum
{
    EFFECTIVENESS_CANNOT_VIEW,
    EFFECTIVENESS_NO_EFFECT,
    EFFECTIVENESS_MOSTLY_INEFFECTIVE,
    EFFECTIVENESS_NOT_VERY_EFFECTIVE,
    EFFECTIVENESS_NORMAL,
    EFFECTIVENESS_SUPER_EFFECTIVE,
    EFFECTIVENESS_EXTREMELY_EFFECTIVE,
};

static bool32 ShouldShowTypeEffectiveness(u32 targetId)
{
    if (IsGhostBattleWithoutScope())
        return FALSE;

    if (B_SHOW_EFFECTIVENESS == SHOW_EFFECTIVENESS_CAUGHT)
        return GetSetPokedexFlag(SpeciesToNationalPokedexNum(gBattleMons[targetId].species), FLAG_GET_CAUGHT);

    if (B_SHOW_EFFECTIVENESS == SHOW_EFFECTIVENESS_SEEN)
        return GetSetPokedexFlag(SpeciesToNationalPokedexNum(gBattleMons[targetId].species), FLAG_GET_SEEN);

    return TRUE;
}

static u32 CheckTypeEffectiveness(enum BattlerId battlerAtk, enum BattlerId battlerDef)
{
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battlerAtk][4]);
    struct DamageContext ctx = {0};
    ctx.battlerAtk = battlerAtk;
    ctx.battlerDef = battlerDef;
    ctx.move = moveInfo->moves[gMoveSelectionCursor[battlerAtk]];
    ctx.moveType = CheckDynamicMoveType(GetBattlerMon(battlerAtk), ctx.move, battlerAtk, MON_IN_BATTLE);
    ctx.updateFlags = FALSE;
    ctx.weather = GetWeather();
    ctx.terrain = gFieldTimers.terrain;
    ctx.abilities[ctx.battlerAtk] = GetBattlerAbility(battlerAtk);
    ctx.abilities[ctx.battlerDef] = GetBattlerAbility(battlerDef);
    ctx.holdEffects[ctx.battlerAtk] = GetBattlerHoldEffect(battlerAtk);
    ctx.holdEffects[ctx.battlerDef] = GetBattlerHoldEffect(battlerDef);

    uq4_12_t modifier = CalcTypeEffectivenessMultiplier(&ctx);

    if (!ShouldShowTypeEffectiveness(battlerDef))
        return EFFECTIVENESS_CANNOT_VIEW;

    if (modifier == UQ_4_12(0.0))
        return EFFECTIVENESS_NO_EFFECT; // No effect
    else if (modifier <= UQ_4_12(0.25))
        return EFFECTIVENESS_MOSTLY_INEFFECTIVE; // Mostly ineffective
    else if (modifier <= UQ_4_12(0.5))
        return EFFECTIVENESS_NOT_VERY_EFFECTIVE; // Not very effective
    else if (modifier >= UQ_4_12(4.0))
        return EFFECTIVENESS_EXTREMELY_EFFECTIVE; // Extremely effective
    else if (modifier >= UQ_4_12(2.0))
        return EFFECTIVENESS_SUPER_EFFECTIVE; // Super effective
    return EFFECTIVENESS_NORMAL; // Normal effectiveness
}

static u32 CheckTargetTypeEffectiveness(enum BattlerId battler)
{
    enum BattlerId battlerFoe = GetOppositeBattler(battler);
    u32 foeEffectiveness = CheckTypeEffectiveness(battler, battlerFoe);

    if (IsDoubleBattle())
    {
        enum BattlerId partnerFoe = GetPartnerBattler(battlerFoe);
        u32 partnerFoeEffectiveness = CheckTypeEffectiveness(battler, partnerFoe);
        if (!IsBattlerAlive(battlerFoe))
            return partnerFoeEffectiveness;
        if (IsBattlerAlive(battlerFoe) && IsBattlerAlive(partnerFoe)
         && partnerFoeEffectiveness > foeEffectiveness)
            return partnerFoeEffectiveness;
    }
    return foeEffectiveness; // fallthrough for any other circumstance
}

static void MoveSelectionDisplayMoveEffectiveness(u32 foeEffectiveness, enum BattlerId battler)
{
    static const u8 sNormalGlyph[] = {0x6, 0x9, 0x9, 0x6};
    static const u8 sStrongGlyph[] = {0x2, 0x7, 0x7, 0x2};
    static const u8 sWeakGlyph[] = {0x7, 0x5, 0x2, 0x0};
    static const u8 sIneffectiveGlyph[] = {0x5, 0x2, 0x2, 0x5};
    const u8 *glyph = NULL;
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
    u32 i, row, col;
    u8 windowId;

    if (!IsBattleMoveStatus(moveInfo->moves[gMoveSelectionCursor[battler]]))
    {
        switch (foeEffectiveness)
        {
        case EFFECTIVENESS_EXTREMELY_EFFECTIVE:
        case EFFECTIVENESS_SUPER_EFFECTIVE:     glyph = sStrongGlyph; break;
        case EFFECTIVENESS_NOT_VERY_EFFECTIVE:
        case EFFECTIVENESS_MOSTLY_INEFFECTIVE:  glyph = sWeakGlyph; break;
        case EFFECTIVENESS_NO_EFFECT:           glyph = sIneffectiveGlyph; break;
        case EFFECTIVENESS_NORMAL:              glyph = sNormalGlyph; break;
        case EFFECTIVENESS_CANNOT_VIEW:
        default:                                glyph = NULL; break;
        }
    }

    // The right edge of each move card is reserved for the selected move's effectiveness.
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        windowId = i + B_WIN_MOVE_NAME_1;
        RFUI_FillRect(windowId, RF_CELL_FILL, RF_MOVE_CELL_WIDTH - 10, 4, 6, 8);
        if (i == gMoveSelectionCursor[battler] && glyph != NULL)
        {
            for (row = 0; row < 4; row++)
            {
                for (col = 0; col < 4; col++)
                {
                    if (glyph[row] & (0x8 >> col))
                        RFUI_FillRect(windowId, 13, RF_MOVE_CELL_WIDTH - 9 + col, 5 + row, 1, 1);
                }
            }
        }
        CopyWindowToVram(windowId, COPYWIN_GFX);
    }
}
