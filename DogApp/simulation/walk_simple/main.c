#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#ifndef PI
#define PI 3.14159265358979f
#endif

FILE *fp;

#define DOG_CTRL_PERIOD_ms 5

typedef struct _dog_motor_output_t
{
    float vel;
    float pos;
    float kv;
    float kp;
    float T;
} dog_motor_output_t;

typedef struct _dog_leg_output_t
{
    dog_motor_output_t m[2];
} dog_leg_output_t;

#define LEGS_DIST_LR 0.3f
#define LEGS_DIST_FB 0.4f

#define LEG_LEN_UPPER 0.15f
#define LEG_LEN_LOWER 0.20f

// ************************************************************************************************************

#define BANDWIDTH 0.75f
#define HEIGH_DIFF 0.01f
#define PARAM_A 9.0

#define H 0.38f
#define V 0.5f

#define PERIOD_CNT 660

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
        lo->m[i].kv = 0.9f;
        lo->m[i].T = 0.f;
    }
    // height += H;

    // vel += V;

    if (phrase < BANDWIDTH){
        theta = atanf(vel/height * (phrase - BANDWIDTH / 2)) ; // TODO : add some offset here
        // dist = (height ) / cosf(theta);
        dist = (height + HEIGH_DIFF * cosf(phrase * 2 * PI / BANDWIDTH)) / cosf(theta);

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

    fprintf(fp, "%.5f,%.5f,%.5f,%.5f,%.5f\n", phrase,  dist * cosf(theta), dist * cosf(theta), theta, d);

}


int main(void)
{

    fp = fopen("data.csv", "w");

    for (uint16_t step = 0; step < PERIOD_CNT; step++)
    {
        dog_leg_output_t out;

        simpleWalk_generator(step / (float)(PERIOD_CNT), H, V, &out);

        // fprintf(fp, "%.3f,%.3f,%.3f,%.3f,%.3f\n", step * 0.1, -out.m[0].pos * 180 / PI, out.m[1].pos * 180 / PI, out.m[0].vel, out.m[1].vel);
    }
}