#ifndef GUARD_KEY_SYSTEM_H
#define GUARD_KEY_SYSTEM_H

#include "constants/key_system.h"

bool32 KeySystem_IsOn(enum KeySystemKey key);
void KeySystem_Set(enum KeySystemKey key, bool32 on);
enum KeyDifficulty KeySystem_GetDifficulty(void);
u32 KeySystem_ScaleTrainerLevel(u32 level);
void KeySystem_OpenMenu(void);

#endif // GUARD_KEY_SYSTEM_H
