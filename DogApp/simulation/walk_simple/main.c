#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#ifndef PI
#define PI					3.14159265358979f
#endif

#define DOG_CTRL_PERIOD_ms 2

typedef struct _dog_leg_input_t {
    float dist; // 0~1
    float theta;
} dog_leg_input_t;

// ************************************************************************************************************

#define LEGS_DIST_LR 0.3f
#define LEGS_DIST_FB 0.4f

#define LEGS_DIST_LR 0.3f
#define LEGS_DIST_FB 0.4f

#define LEG_LEN_UPPER 0.15f
#define LEG_LEN_LOWER 0.20f

#define H 0.42f
#define HEIGHT H*(LEG_LEN_UPPER + LEG_LEN_LOWER)

#define V 2.6f

// time ctrl
#define PERIOD_CNT 220
#define BANDWIDTH 0.75f

// curf ctrl
#define HEIGH_DIFF 0.04f
#define PARAM_A 7.0


typedef struct _dog_leg_input2_t {
    float dist; // 0~1
    float theta;
} dog_leg_input2_t;

// static void simpleLinerWalk_generator(float phrase, dog_leg_input_t * vect){
//     // static float C = 0.38;
//     // static float V = 1.f;

//     while (phrase > 1.0){ // to 0~1
//         phrase -= 1.0;
//     }

//     if (phrase < BANDWIDTH){
//         vect->theta = atanf(V/C * (phrase - BANDWIDTH / 2));
//         vect->dist = (C - HEIGH_DIFF * sinf(phrase * 2 * PI)) / cosf(vect->theta);

//     } else {
//         vect->theta = (1 + BANDWIDTH - 2 * phrase) * atanf(V/C * (BANDWIDTH / 2)) / (1.f - BANDWIDTH);

//         float m = (( HEIGH_DIFF * sinf(BANDWIDTH * 2 * PI) ) / ( (BANDWIDTH - 1) * PARAM_A ) + 1 + BANDWIDTH) / 2 ;
//         float t = C - PARAM_A * (1 - m) * (1 - m);
//         float y = PARAM_A * (phrase - m) * (phrase - m) + t;
//         vect->dist = ( y ) / cosf(vect->theta);
//     }
// }


int main(void){

    FILE * fp = fopen("data.txt", "w");
    for (int step = 0; step < PERIOD_CNT; step++){

        // ! 在每一个与地面接触的腿上，角度
    
        // dog_leg_input_t vec;


        float t = (step - PERIOD_CNT*BANDWIDTH/2) * DOG_CTRL_PERIOD_ms / 1000.f;
        float a, a_, b;
        if (step < PERIOD_CNT*BANDWIDTH){
            float n1 = t*V/HEIGHT;
            float n2 = (t*V)*(t*V);
             a = 2 * atan2f((sqrtf(n1*n1 + 1) - 1) , n1);
            if (a > PI){
                a -= PI*2;
            }
            //  b = acosf((LEG_LEN_UPPER*LEG_LEN_UPPER - LEG_LEN_LOWER*LEG_LEN_LOWER + HEIGHT * HEIGHT + n2)/(2 * LEG_LEN_UPPER * sqrtf(n2 + H*H)));
            float d = HEIGHT / cosf(a);
             b = acosf((LEG_LEN_UPPER*LEG_LEN_UPPER - LEG_LEN_LOWER*LEG_LEN_LOWER + d*d)/(2 * LEG_LEN_UPPER * d));
            //  b = acos((HEIGHT*HEIGHT/(cos(2*atan(HEIGHT*(sqrt(1 + V*V*t*t/(HEIGHT*HEIGHT)) - 1)/(V*t)))*cos(2*atan(HEIGHT*(sqrt(1 + V*V*t*t/(HEIGHT*HEIGHT)) - 1)/(V*t)))) - LEG_LEN_LOWER*LEG_LEN_LOWER + LEG_LEN_UPPER*LEG_LEN_UPPER)*cos(2*atan(HEIGHT*(sqrt(1 + V*V*t*t/(HEIGHT*HEIGHT)) - 1)/(V*t)))/(2*HEIGHT*LEG_LEN_UPPER));

            float n3 = sqrtf(n1*n1 + 1) - 1;
            // float a_;
            if (t != 0){
                a_ = 2 * ( V/HEIGHT/sqrtf(n1*n1 + 1) - n3 / (n1 * t)  ) / (1 + (n3*n3)/(n1 * n1));
            }else {
                a_ = 0;
            }
        } else {
            // TODO
            a = b = 0;
        }
        

        // fprintf(fp,  "%.3f,%.3f,%.3f,%.3f\n", t, vec.theta, vec.dist, vec.dist * cosf(vec.theta));
        fprintf(fp,  "%.3f,%.3f,%.3f\n", t*1000, (a+b)*180/PI,  (a-b)*180/PI);
    }
}