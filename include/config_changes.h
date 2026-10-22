#ifndef GUARD_CONFIG_CHANGES_H
#define GUARD_CONFIG_CHANGES_H

#include "constants/config_changes.h"
#include "constants/key_system.h"
#include "config/battle.h"
#include "config/pokerus.h"
#include "config/ai.h"

#define UNPACK_CONFIG_STRUCT(_name, _field, _typeMaxValue, ...) INVOKE_WITH_(UNPACK_CONFIG_STRUCT_, _field, UNPACK_B(_typeMaxValue));
#define UNPACK_CONFIG_STRUCT_(_field, _type, ...) _type FIRST(__VA_OPT__(_field:BIT_SIZE(FIRST(__VA_ARGS__)),) _field)

struct ConfigChanges
{
    BATTLE_CONFIG_DEFINITIONS(UNPACK_CONFIG_STRUCT)
    POKEMON_CONFIG_DEFINITIONS(UNPACK_CONFIG_STRUCT)
    AI_CONFIG_DEFINITIONS(UNPACK_CONFIG_STRUCT)
    // Expands to:
    // u32 critChance:4;
    // u32 critMultiplier:4;
    // ...
};

#if TESTING
extern struct ConfigChanges *gConfigChangesTestOverride;
#define GET_CONFIG_VALUE(_field, _default) (gConfigChangesTestOverride == NULL ? (_default) : gConfigChangesTestOverride->_field)
#else
#define GET_CONFIG_VALUE(_field, _default) (_default)
#endif

bool32 KeySystem_IsOn(enum KeySystemKey key);

// Rojo Fuego+ Key System: a key that is turned off swaps the config for its older behavior.
// The tag is a constant at every call site, so configs without a key fold back to their value.
static inline u32 KeySystem_ConfigOverride(enum ConfigTag tag, u32 value)
{
    switch (tag)
    {
    case CONFIG_B_EXP_CATCH:
        return KeySystem_IsOn(KEY_CAPTURE_EXP) ? value : GEN_5;
    case CONFIG_B_SCALED_EXP:
        return KeySystem_IsOn(KEY_SCALED_EXP) ? value : GEN_6;
    case CONFIG_B_PHYSICAL_SPECIAL_SPLIT:
        return KeySystem_IsOn(KEY_PHYSICAL_SPECIAL_SPLIT) ? value : GEN_3;
    case CONFIG_B_CRIT_CHANCE:
    case CONFIG_B_CRIT_MULTIPLIER:
        return KeySystem_IsOn(KEY_MODERN_CRITS) ? value : GEN_5;
    case CONFIG_B_PARALYSIS_SPEED:
    case CONFIG_B_BURN_DAMAGE:
        return KeySystem_IsOn(KEY_MODERN_STATUS) ? value : GEN_6;
    case CONFIG_B_SLEEP_TURNS:
        return KeySystem_IsOn(KEY_MODERN_STATUS) ? value : GEN_3;
    case CONFIG_B_ABILITY_WEATHER:
        return KeySystem_IsOn(KEY_ABILITY_WEATHER_TURNS) ? value : GEN_5;
    default:
        return value;
    }
}

#define UNPACK_BATTLE_CONFIG_GETTER(_name, _field, ...) static inline u32 GetConfig_##_name(void) { return GET_CONFIG_VALUE(_field, KeySystem_ConfigOverride(CONFIG_##_name, _name)); }
#define UNPACK_POKEMON_CONFIG_GETTER(_name, _field, ...) static inline u32 GetConfig_##_name(void) { return GET_CONFIG_VALUE(_field, P_##_name); }

BATTLE_CONFIG_DEFINITIONS(UNPACK_BATTLE_CONFIG_GETTER)
POKEMON_CONFIG_DEFINITIONS(UNPACK_POKEMON_CONFIG_GETTER)
AI_CONFIG_DEFINITIONS(UNPACK_BATTLE_CONFIG_GETTER)

#undef UNPACK_BATTLE_CONFIG_GETTER
#undef UNPACK_POKEMON_CONFIG_GETTER
#undef GET_CONFIG_VALUE

#define GetConfig(name) GetConfig_##name()

ARM_FUNC u32 GetConfigInternal(enum ConfigTag configTag);
void SetConfig(enum ConfigTag configTag, u32 value);

#if TESTING
void TestInitConfigData(void);
void TestFreeConfigData(void);
#endif

#endif // GUARD_CONFIG_CHANGES_H
