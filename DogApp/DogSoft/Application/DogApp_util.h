#ifndef DOGAPP_UTIL_H
#define DOGAPP_UTIL_H

#include "DogSoft.h"
#include "DogApp.h"
#include <math.h>
#include "arm_math.h"
#include "imu.h"

class jumperLegDual
{
public:
    jumperLegDual(dog_motor_single_t *_leg_l, dog_motor_single_t *_leg_r);

    struct jumpLegInput
    {
        struct
        {
            float dist;
            float theta;
        } pos;

        float Tvel;
        float T;

        float kp;
        float kv;
    };

    void set_input(const jumpLegInput &input);
    void output(void);
    float get_angle(void);

    float get_d(void);

private:
    dog_leg_output_t lo;
    dog_motor_single_t *ml;
    dog_motor_single_t *mr;
};

#endif