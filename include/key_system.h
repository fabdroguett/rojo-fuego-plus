#ifndef GUARD_KEY_SYSTEM_H
#define GUARD_KEY_SYSTEM_H

#include "constants/key_system.h"

bool32 KeySystem_IsOn(enum KeySystemKey key);
void KeySystem_Set(enum KeySystemKey key, bool32 on);
enum KeyDifficulty KeySystem_GetDifficulty(void);
bool32 KeySystem_CanUseFieldMoveWithoutMon(enum Move move);
s32 KeySystem_GetFieldMoveUser(enum Move move);
u32 KeySystem_ScaleTrainerLevel(u32 level);
void KeySystem_OpenMenu(void);
void KeySystem_UpdateHiddenItemSparkle(void);

#endif // GUARD_KEY_SYSTEM_H
