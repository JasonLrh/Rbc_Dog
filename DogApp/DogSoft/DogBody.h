#ifndef DOG_BODY_H
#define DOG_BODY_H

#ifdef __cplusplus
extern "C" {
#endif


#include "DogLeg.h"

void dog_body_simpleLinerWalk();

void dog_body_force_stop(void);

void dog_body_standup(float kp, float kv);

void dog_body_sitdown(void);

#ifdef __cplusplus
}
#endif


#endif