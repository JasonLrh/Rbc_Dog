#ifndef DOG_LEG_H
#define DOG_LEG_H

#include "DogMotor.h"

typedef struct _dog_leg_input_t {
    float dist; // 0~1
    float theta;
} dog_leg_input_t;

/*
position control
*/
void dog_leg_set_phrase(dog_motor_single_t * motor, const dog_leg_input_t * input);

#endif // !DOG_LEG_H