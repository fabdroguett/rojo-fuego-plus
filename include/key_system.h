#ifndef GUARD_KEY_SYSTEM_H
#define GUARD_KEY_SYSTEM_H

#include "constants/key_system.h"

bool32 KeySystem_IsOn(enum KeySystemKey key);
void KeySystem_Set(enum KeySystemKey key, bool32 on);
void KeySystem_OpenMenu(void);

#endif // GUARD_KEY_SYSTEM_H
