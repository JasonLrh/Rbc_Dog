#ifndef DOG_APP_H
#define DOG_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DogSoft.h"

typedef enum _dog_app_choices {
    APP_NONE,
    APP_WALK,
    APP_WALK_TEST,
    APP_JUMP,
    APP_UPSTARIS
}dog_app_choices;

void dogapp_walk(void);
void step_param_update(float vel, float height, float baud_of_walk, int period_cnt);

void dogapp_simpleWalk(void);
void dogapp_simpleWalk_fortest(void);
void dogapp_jumper(void);
void dogapp_upstaris(void);

#ifdef __cplusplus
}
#endif


#endif