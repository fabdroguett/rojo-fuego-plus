#include "global.h"
#include "item.h"
#include "omni_ring.h"
#include "string_util.h"
#include "constants/items.h"

// State lives in SaveBlock3: omniRingMechanics (one bit per OMNI_* mechanic),
// omniRingMaxCharges and omniRingCharges.

static const u8 *const sMechanicNames[OMNI_MECHANIC_COUNT] =
{
    [OMNI_MEGA]    = COMPOUND_STRING("Mega"),
    [OMNI_Z_MOVE]  = COMPOUND_STRING("Z"),
    [OMNI_DYNAMAX] = COMPOUND_STRING("Dinamax"),
    [OMNI_TERA]    = COMPOUND_STRING("Tera"),
};

bool32 OmniRing_HasMechanic(enum OmniMechanic mechanic)
{
    return CheckBagHasItem(ITEM_OMNI_RING, 1) && (gSaveBlock3Ptr->omniRingMechanics & (1u << mechanic));
}

bool32 OmniRing_HasCharge(void)
{
    return gSaveBlock3Ptr->omniRingCharges > 0;
}

void OmniRing_UseCharge(void)
{
    if (gSaveBlock3Ptr->omniRingCharges > 0)
        gSaveBlock3Ptr->omniRingCharges--;
}

void OmniRing_Recharge(void)
{
    gSaveBlock3Ptr->omniRingCharges = gSaveBlock3Ptr->omniRingMaxCharges;
}

static bool32 OmniRing_UnlockMechanic(enum OmniMechanic mechanic)
{
    if (gSaveBlock3Ptr->omniRingMechanics & (1u << mechanic))
        return FALSE;

    gSaveBlock3Ptr->omniRingMechanics |= (1u << mechanic);
    // Tera and Dynamax need charges; the first of them gives the ring its first one
    if ((mechanic == OMNI_DYNAMAX || mechanic == OMNI_TERA) && gSaveBlock3Ptr->omniRingMaxCharges == 0)
        gSaveBlock3Ptr->omniRingMaxCharges = 1;
    OmniRing_Recharge();
    return TRUE;
}

// Returns FALSE when the item would do nothing (mechanic already installed, charges full).
bool32 OmniRing_TryInstallItem(enum Item itemId)
{
    switch (itemId)
    {
    case ITEM_OMNI_MODULE_MEGA:
        return OmniRing_UnlockMechanic(OMNI_MEGA);
    case ITEM_OMNI_MODULE_Z:
        return OmniRing_UnlockMechanic(OMNI_Z_MOVE);
    case ITEM_OMNI_MODULE_DYNAMAX:
        return OmniRing_UnlockMechanic(OMNI_DYNAMAX);
    case ITEM_OMNI_MODULE_TERA:
        return OmniRing_UnlockMechanic(OMNI_TERA);
    case ITEM_OMNI_CELL:
        if (gSaveBlock3Ptr->omniRingMaxCharges >= OMNI_RING_MAX_CHARGES)
            return FALSE;
        gSaveBlock3Ptr->omniRingMaxCharges++;
        OmniRing_Recharge();
        return TRUE;
    default:
        return FALSE;
    }
}

// Text shown when the player uses the ring from the bag, e.g. "Mega Z · Cargas 1/2"
void OmniRing_GetStatusString(u8 *dest)
{
    u32 i;
    bool32 any = FALSE;

    StringCopy(dest, COMPOUND_STRING("Omni Ring: "));
    for (i = 0; i < OMNI_MECHANIC_COUNT; i++)
    {
        if (!(gSaveBlock3Ptr->omniRingMechanics & (1u << i)))
            continue;
        if (any)
            StringAppend(dest, COMPOUND_STRING(", "));
        StringAppend(dest, sMechanicNames[i]);
        any = TRUE;
    }
    if (!any)
        StringAppend(dest, COMPOUND_STRING("sin módulos"));

    StringAppend(dest, COMPOUND_STRING(".\nCargas: "));
    ConvertIntToDecimalStringN(dest + StringLength(dest), gSaveBlock3Ptr->omniRingCharges, STR_CONV_MODE_LEFT_ALIGN, 1);
    StringAppend(dest, COMPOUND_STRING("/"));
    ConvertIntToDecimalStringN(dest + StringLength(dest), gSaveBlock3Ptr->omniRingMaxCharges, STR_CONV_MODE_LEFT_ALIGN, 1);
    StringAppend(dest, COMPOUND_STRING(".{PAUSE_UNTIL_PRESS}"));
}
