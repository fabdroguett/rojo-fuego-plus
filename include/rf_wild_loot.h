#ifndef GUARD_RF_WILD_LOOT_H
#define GUARD_RF_WILD_LOOT_H

enum RFLootResult { RF_LOOT_NONE, RF_LOOT_BAG, RF_LOOT_PENDING };
u32 RFWildLoot_TryClaim(void);
enum RFLootResult RFWildLoot_Collect(void);

#endif
