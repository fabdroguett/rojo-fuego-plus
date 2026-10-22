#ifndef GUARD_OMNI_RING_H
#define GUARD_OMNI_RING_H

// Rojo Fuego+ Omni Ring: one key item that replaces the Mega Ring, Z-Power Ring, Dynamax Band
// and Tera Orb. It starts with no powers; modules found around the map unlock each mechanic and
// Omni Cells add charges. Tera and Dynamax spend a charge, which a Pokémon Center refills.

#define OMNI_RING_MAX_CHARGES 9
#define POKE_VIAL_MAX_CHARGES 3 // Rojo Fuego+ Botiquín, kept here with the other key item charges

enum OmniMechanic
{
    OMNI_MEGA,
    OMNI_Z_MOVE,
    OMNI_DYNAMAX,
    OMNI_TERA,
    OMNI_MECHANIC_COUNT
};

bool32 OmniRing_HasMechanic(enum OmniMechanic mechanic);
bool32 OmniRing_HasCharge(void);
void OmniRing_UseCharge(void);
void OmniRing_Recharge(void);
bool32 OmniRing_TryInstallItem(enum Item itemId);
void OmniRing_GetStatusString(u8 *dest);

#endif // GUARD_OMNI_RING_H
