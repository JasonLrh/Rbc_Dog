#ifndef DOG_SOFT_H
#define DOG_SOFT_H

#ifdef __cplusplus
extern "C" {
#endif


#include "DogBody.h"
#include "CommandSystem/DogCMD.h"

#define LEGS_DIST_LR 0.3f
#define LEGS_DIST_FB 0.4f

#define LEG_LEN_UPPER 0.15f
#define LEG_LEN_LOWER 0.20f

#define DOG_CTRL_PERIOD_ms 5
#define DOG_CTRL_FREQ_Hz 1000/(float)DOG_CTRL_PERIOD_ms

#ifdef __cplusplus
}
#endif

#endif // !DOG_SOFT