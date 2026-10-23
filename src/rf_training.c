#include "global.h"
#include "pokemon.h"
#include "main.h"
#include "event_data.h"
#include "rf_training.h"

static bool32 CanEdit(u32 slot, u32 stat, u32 mode)
{
    struct Pokemon *mon;

    if (!IS_FRLG || gMain.inBattle || slot >= PARTY_SIZE || stat >= NUM_STATS || mode >= RF_TRAIN_MODE_COUNT)
        return FALSE;
    mon = &gParties[B_TRAINER_PLAYER][slot];
    return GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE
        && !GetMonData(mon, MON_DATA_IS_EGG) && !GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG);
}

static u32 GetEVTotal(struct Pokemon *mon)
{
    u32 stat, total = 0;
    for (stat = 0; stat < NUM_STATS; stat++)
        total += GetMonData(mon, MON_DATA_HP_EV + stat);
    return total;
}

static u32 GetLimit(struct Pokemon *mon, u32 stat, u32 mode)
{
    u32 others;
    if (mode == RF_TRAIN_IV)
        return MAX_PER_STAT_IVS;
    others = GetEVTotal(mon) - GetMonData(mon, MON_DATA_HP_EV + stat);
    return others >= MAX_TOTAL_EVS ? 0 : min(MAX_PER_STAT_EVS, MAX_TOTAL_EVS - others);
}

bool32 RFTraining_SetValue(u32 slot, u32 stat, u32 mode, u32 value)
{
    struct Pokemon *mon;
    u32 field, off = FALSE;

    if (!CanEdit(slot, stat, mode))
        return FALSE;
    mon = &gParties[B_TRAINER_PLAYER][slot];
    if (value > GetLimit(mon, stat, mode))
        return FALSE;
    if (mode == RF_TRAIN_EV
     && GetEVTotal(mon) - GetMonData(mon, MON_DATA_HP_EV + stat) + value > MAX_TOTAL_EVS)
        return FALSE;
    field = (mode == RF_TRAIN_EV ? MON_DATA_HP_EV : MON_DATA_HP_IV) + stat;
    if (GetMonData(mon, field) == value
     && !(mode == RF_TRAIN_IV && GetMonData(mon, MON_DATA_HYPER_TRAINED_HP + stat)))
        return FALSE;
    SetMonData(mon, field, &value);
    // Editing real IVs must also remove the selected stat's Hyper Training override.
    // Other stats and the original capsule/menta/chapas services remain independent.
    if (mode == RF_TRAIN_IV)
        SetMonData(mon, MON_DATA_HYPER_TRAINED_HP + stat, &off);
    CalculateMonStats(mon);
    return TRUE;
}

void Script_RF_TrainingCanEdit(void)
{
    gSpecialVar_Result = CanEdit(gSpecialVar_0x8004, 0, gSpecialVar_0x8006);
}

// 8004: party slot, 8005: stat, 8006: EV/IV. Outputs 8007: real value, 8008: allowed max,
// 8009: Hyper Training override, 800B: total EV. Reading never changes the Pokemon.
void Script_RF_TrainingRead(void)
{
    u32 slot = gSpecialVar_0x8004, stat = gSpecialVar_0x8005, mode = gSpecialVar_0x8006;
    struct Pokemon *mon;

    gSpecialVar_Result = FALSE;
    if (!CanEdit(slot, stat, mode))
        return;
    mon = &gParties[B_TRAINER_PLAYER][slot];
    gSpecialVar_0x8007 = GetMonData(mon, (mode == RF_TRAIN_EV ? MON_DATA_HP_EV : MON_DATA_HP_IV) + stat);
    gSpecialVar_0x8008 = GetLimit(mon, stat, mode);
    gSpecialVar_0x8009 = GetMonData(mon, MON_DATA_HYPER_TRAINED_HP + stat);
    gSpecialVar_0x800B = GetEVTotal(mon);
    gSpecialVar_Result = TRUE;
}

static bool32 GetTarget(u32 *target)
{
    u32 current, next, limit, action = gSpecialVar_0x800A;
    Script_RF_TrainingRead();
    if (!gSpecialVar_Result || action >= RF_TRAIN_ACTION_COUNT)
    {
        gSpecialVar_Result = FALSE;
        return FALSE;
    }
    current = gSpecialVar_0x8007;
    limit = gSpecialVar_0x8008;
    switch (action)
    {
    case RF_TRAIN_MAX: next = limit; break;
    case RF_TRAIN_ZERO: next = 0; break;
    case RF_TRAIN_PLUS_ONE: next = current >= limit ? current : min(current + 1, limit); break;
    case RF_TRAIN_MINUS_ONE: next = current ? current - 1 : 0; break;
    case RF_TRAIN_PLUS_TEN: next = current >= limit ? current : min(current + 10, limit); break;
    default: next = current > 10 ? current - 10 : 0; break;
    }
    *target = next;
    return TRUE;
}

// 800A selects an action; 8007 previews its result before the confirmation dialog.
void Script_RF_TrainingPreview(void)
{
    u32 value;
    gSpecialVar_Result = GetTarget(&value);
    if (gSpecialVar_Result)
        gSpecialVar_0x8007 = value;
}

void Script_RF_TrainingEdit(void)
{
    u32 value;
    gSpecialVar_Result = FALSE;
    if (GetTarget(&value))
        gSpecialVar_Result = RFTraining_SetValue(gSpecialVar_0x8004, gSpecialVar_0x8005, gSpecialVar_0x8006, value);
}
