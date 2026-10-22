#include "global.h"
#include "berry.h"
#include "clock.h"
#include "dewford_trend.h"
#include "event_data.h"
#include "field_specials.h"
#include "field_weather.h"
#include "lottery_corner.h"
#include "main.h"
#include "mass_outbreak.h"
#include "overworld.h"
#include "pokerus.h"
#include "random.h"
#include "rtc.h"
#include "time_events.h"
#include "tv.h"
#include "wallclock.h"
#include "constants/form_change_types.h"
#include "apricorn_tree.h"

static void UpdatePerDay(struct Time *localTime);
static void UpdatePerMinute(struct Time *localTime);

// Rojo Fuego+: in FRLG builds the Emerald var IDs overlap real FRLG vars (VAR_DAYS is the
// Magikarp size record there), so FRLG keeps its day counter in its own var.
#if IS_FRLG
#define VAR_CLOCK_DAYS VAR_DAYS_FRLG
#else
#define VAR_CLOCK_DAYS VAR_DAYS
#endif

void InitTimeBasedEvents(void)
{
    FlagSet(FLAG_SYS_CLOCK_SET);
    RtcCalcLocalTime();
    gSaveBlock2Ptr->lastBerryTreeUpdate = gLocalTime;
    VarSet(VAR_CLOCK_DAYS, gLocalTime.days);
}

void DoTimeBasedEvents(void)
{
    // Rojo Fuego+: FRLG never sets the clock (Emerald does it in the truck), so daily events
    // would never run. Start them the first time the overworld asks.
    if (IS_FRLG && !FlagGet(FLAG_SYS_CLOCK_SET))
        InitTimeBasedEvents();

    if (FlagGet(FLAG_SYS_CLOCK_SET) && !InPokemonCenter())
    {
        RtcCalcLocalTime();
        UpdatePerDay(&gLocalTime);
        UpdatePerMinute(&gLocalTime);
    }
}

void UpdateDailySeed(void)
{
    gSaveBlock1Ptr->dailySeed = Random32();
}

void DoDailyEvents(u32 daysSince)
{
    ClearDailyFlags();
    UpdateDailySeed();
    UpdateMassOutbreakDaysLeft(daysSince);
#if IS_FRLG
    // Rojo Fuego+: only the events that do not touch Emerald-only vars and flags, which share
    // IDs with real FRLG ones.
    TryStartKantoOutbreak();
    UpdatePartyPokerusTime(daysSince);
    UpdateDaysPassedSinceFormChange(daysSince);
    return;
#endif
    UpdateDewfordTrendPerDay(daysSince);
    UpdateTVShowsPerDay(daysSince);
    UpdateWeatherPerDay(daysSince);
    UpdatePartyPokerusTime(daysSince);
    UpdateBirchState(daysSince);
    UpdateFrontierManiac(daysSince);
    UpdateFrontierGambler(daysSince);
    SetShoalItemFlag(daysSince);
    if (!OW_USE_DAILY_SEED_FOR_VANILLA_VARIABLES)
    {
        UpdateMirageRnd(daysSince);
        SetRandomLotteryNumber(daysSince);
    }
    UpdateDaysPassedSinceFormChange(daysSince);
    DailyResetApricornTrees();
}

static void UpdatePerDay(struct Time *localTime)
{
    u16 *days = GetVarPointer(VAR_CLOCK_DAYS);
    u16 daysSince;

    if (*days != localTime->days && *days <= localTime->days)
    {
        daysSince = localTime->days - *days;
        DoDailyEvents(daysSince);
        *days = localTime->days;
    }
}

static void UpdatePerMinute(struct Time *localTime)
{
    struct Time difference;
    int minutes;

    CalcTimeDifference(&difference, &gSaveBlock2Ptr->lastBerryTreeUpdate, localTime);
    minutes = 24 * 60 * difference.days + 60 * difference.hours + difference.minutes;
    if (minutes != 0)
    {
        if (minutes >= 0)
        {
            BerryTreeTimeUpdate(minutes);
            gSaveBlock2Ptr->lastBerryTreeUpdate = *localTime;
        }
    }
}

void FormChangeTimeUpdate()
{
    s32 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        TryFormChange(&gParties[B_TRAINER_PLAYER][i], FORM_CHANGE_TIME_OF_DAY, B_TRAINER_PLAYER);
    }
}

static void ReturnFromStartWallClock(void)
{
    InitTimeBasedEvents();
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

void StartWallClock(void)
{
    SetMainCallback2(CB2_StartWallClock);
    gMain.savedCallback = ReturnFromStartWallClock;
}
