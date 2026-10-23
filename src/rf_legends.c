#include "global.h"
#include "event_data.h"
#include "rf_legends.h"
#include "rf_quests.h"

// Rojo Fuego+ legendary sagas (tools/rojo_fuego/legends.py). Each legend is an overworld object
// that only spawns while its saga mission is active and until it has been caught.

#define LEGEND_SAVE_MAGIC 0x4C474E44

#include "data/rf_legends.h"

STATIC_ASSERT(RF_LEGEND_COUNT <= RF_LEGEND_CAPACITY, RFLegendSaveCapacity);

static void EnsureLegendSave(void)
{
    // Older saves end before these fields; their padding is not assumed to be zero.
    if (gSaveBlock3Ptr->rfLegendSaveMagic != LEGEND_SAVE_MAGIC)
    {
        memset(gSaveBlock3Ptr->rfLegendCaught, 0, sizeof(gSaveBlock3Ptr->rfLegendCaught));
        gSaveBlock3Ptr->rfLegendSaveMagic = LEGEND_SAVE_MAGIC;
    }
}

static bool32 IsLegendCaught(u32 id)
{
    EnsureLegendSave();
    return (gSaveBlock3Ptr->rfLegendCaught[id / 8] >> (id % 8)) & 1;
}

bool32 RFLegends_IsObjectHidden(const u8 *script)
{
    u32 i;

    if (script == NULL)
        return FALSE;
    for (i = 0; i < RF_LEGEND_COUNT; i++)
    {
        if (sRFLegends[i].script == script)
            return IsLegendCaught(i) || RFQuests_GetState(sRFLegends[i].quest) != RF_QUEST_ACTIVE;
    }
    return FALSE;
}

bool32 RFLegends_IsSagaCaught(u32 saga)
{
    u32 i, quest = RF_QUEST_LEGEND_00 + saga, found = 0;

    for (i = 0; i < RF_LEGEND_COUNT; i++)
    {
        if (sRFLegends[i].quest != quest)
            continue;
        if (!IsLegendCaught(i))
            return FALSE;
        found++;
    }
    return found != 0;
}

// VAR_0x8004: legend ID. Called only after a capture, never after a defeat or escape.
void Script_RF_LegendCaught(void)
{
    u32 id = gSpecialVar_0x8004;

    if (id >= RF_LEGEND_COUNT)
        return;
    EnsureLegendSave();
    gSaveBlock3Ptr->rfLegendCaught[id / 8] |= 1 << (id % 8);
    RFQuests_Refresh();
}
