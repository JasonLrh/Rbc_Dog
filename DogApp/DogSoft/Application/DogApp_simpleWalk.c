#include "DogSoft.h"
#include <math.h>
#include "arm_math.h"
#include "imu.h"

#define BANDWIDTH 0.75f
#define HEIGH_DIFF 0.01f
#define PARAM_A 11.0

#define H 0.42f
#define V 0.6f

#define PERIOD_CNT 220

static void simpleWalk_generator(float phrase, float height, float vel, dog_leg_output_t * lo){
    // static float tgc_yaw = 0.f;
    // float height;
    // float vel;
    float theta, dist, dig_angle_half, d;

    while (phrase > 1.0){ // to 0~1
        phrase -= 1.0;
    } 

    for (int i = 0; i<2;i++){
        lo->m[i].vel = 0;
        lo->m[i].kp = 45.3f;
        lo->m[i].kv = 0.6f;
        lo->m[i].T = 0.f;
    }
    // height += H;

    // vel += V;

    if (phrase < BANDWIDTH){
        theta = atanf(vel/height * (phrase - BANDWIDTH / 2)) ; // TODO : add some offset here
        dist = (height - HEIGH_DIFF * sinf(phrase * 2 * PI)) / cosf(theta);

    } else {
        theta = (1 + BANDWIDTH - 2 * phrase) * atanf(vel/height * (BANDWIDTH / 2)) / (1.f - BANDWIDTH);

        float mid = (1 + BANDWIDTH) / 2;
        float new_phrase = phrase - mid;
        float t = (height + HEIGH_DIFF) - PARAM_A * (BANDWIDTH - 1) * (BANDWIDTH - 1) / 4 ;
        float y = PARAM_A * new_phrase * new_phrase + t;

        dist = ( y ) / cosf(theta);
    }
    d = LEG_LEN_LOWER - LEG_LEN_UPPER + 2 * dist * LEG_LEN_UPPER;
    dig_angle_half = acosf((LEG_LEN_UPPER*LEG_LEN_UPPER - LEG_LEN_LOWER*LEG_LEN_LOWER + d*d)/(2.f * d * LEG_LEN_UPPER)); // f
    lo->m[0].pos =  - theta + dig_angle_half;
    lo->m[1].pos =    theta + dig_angle_half;
}

void dogapp_simpleWalk(void){
    static uint32_t step = 0;
    // static uint8_t is_start = 0;
    // static float target_yaw;

    // if (is_start == 0){
    //     is_start = 1;
    //     target_yaw = yaw;
    // }

    dog_leg_output_t leg_output[4];
    float t = step / (float)(PERIOD_CNT);

    simpleWalk_generator(t + (0.25 * 0), H + 0.10, V, leg_output + 0);
    simpleWalk_generator(t + (0.25 * 1), H + 0.10, V, leg_output + 1);
    simpleWalk_generator(t + (0.25 * 2), H - 0.10, V, leg_output + 2);
    simpleWalk_generator(t + (0.25 * 3), H - 0.10, V, leg_output + 3);

    dog_leg_set(motors.leg.l_f, leg_output + 0);
    dog_leg_set(motors.leg.r_f, leg_output + 1);
    dog_leg_set(motors.leg.l_b, leg_output + 2);
    dog_leg_set(motors.leg.r_b, leg_output + 3);

    step ++;
}