#ifndef GUARD_RF_TRAINING_H
#define GUARD_RF_TRAINING_H

enum RFTrainingMode { RF_TRAIN_EV, RF_TRAIN_IV, RF_TRAIN_MODE_COUNT };
enum RFTrainingAction
{
    RF_TRAIN_MAX, RF_TRAIN_ZERO, RF_TRAIN_PLUS_ONE, RF_TRAIN_MINUS_ONE,
    RF_TRAIN_PLUS_TEN, RF_TRAIN_MINUS_TEN, RF_TRAIN_ACTION_COUNT,
};

bool32 RFTraining_SetValue(u32 slot, u32 stat, u32 mode, u32 value);
void Script_RF_TrainingRead(void);
void Script_RF_TrainingEdit(void);
void Script_RF_TrainingPreview(void);
void Script_RF_TrainingCanEdit(void);

#endif
