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
#define HEIGH_DIFF 0.0f
#define PARAM_A 9.0

// #define H 0.37795f
#define H 0.45f
#define V 0.5f

#define PERIOD_CNT 200

static void simpleWalk_generator(float phrase, float height, float vel, dog_leg_output_t * lo){
    // static float tgc_yaw = 0.f;
    // float height;
    // float vel;
    float theta, dist, dig_angle_half, d;
    float __phrase_raw = phrase;

    

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
        float half_line_len = vel * (LEG_LEN_UPPER + LEG_LEN_LOWER) * BANDWIDTH / 2 ;
        float h_to_line = height * (LEG_LEN_UPPER + LEG_LEN_LOWER);

        float tuo_a = 5.f / 4.f;
        float tuo_b = 2.f / 5.f;
        float tuo_dist= sqrtf( 1 - (1 / tuo_a) * (1 / tuo_a) ) * tuo_b * half_line_len;
        float tuo_off = - h_to_line + tuo_dist;

        float edge_angle = atan2f(tuo_dist, half_line_len);
        

        float tuo_time = - edge_angle + (phrase - BANDWIDTH) * (PI + 2 * edge_angle) / (1 - BANDWIDTH);

        float x = tuo_a * half_line_len * cosf(tuo_time);
        float y = tuo_b * half_line_len * sin(tuo_time) + tuo_off;

        dist = sqrtf(x*x + y*y) / (LEG_LEN_LOWER + LEG_LEN_UPPER);

        theta =  atan2f(y, x) + PI / 2;


        // theta = (1 + BANDWIDTH - 2 * phrase) * atanf(vel/height * (BANDWIDTH / 2)) / (1.f - BANDWIDTH);

        // float mid = (1 + BANDWIDTH) / 2;
        // float new_phrase = phrase - mid;
        // float t = (height + HEIGH_DIFF) - PARAM_A * (BANDWIDTH - 1) * (BANDWIDTH - 1) / 4 ;
        // float y = PARAM_A * new_phrase * new_phrase + t;
        // dist = ( y ) / cosf(theta);
    }
    // d = LEG_LEN_LOWER - LEG_LEN_UPPER + 2 * dist * LEG_LEN_UPPER;
    d = (LEG_LEN_LOWER + LEG_LEN_UPPER) * dist ;

    dig_angle_half = acosf((LEG_LEN_UPPER*LEG_LEN_UPPER - LEG_LEN_LOWER*LEG_LEN_LOWER + d*d)/(2.f * d * LEG_LEN_UPPER)); // f
    lo->m[0].pos =  - theta + dig_angle_half;
    lo->m[1].pos =    theta + dig_angle_half;

    fprintf(fp, "%.5f,%.5f,%.5f,%.5f,%.5f\n", __phrase_raw, - lo->m[0].pos, lo->m[1].pos, d, theta);
    // fprintf(fp, "%.5f,%.5f,%.5f,%.5f,%.5f\n", __phrase_raw, d, theta, dist * cosf(theta), 0.f);
    // fprintf(fp, "%.5f,%.5f,%.5f,%.5f,%.5f\n", __phrase_raw, dist * cosf(theta), dist * cosf(theta), dist * cosf(theta), 0.f);

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