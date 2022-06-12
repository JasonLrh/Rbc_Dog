#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#ifndef PI
#define PI					3.14159265358979f
#endif

typedef struct _dog_leg_input_t {
    float dist; // 0~1
    float theta;
} dog_leg_input_t;

// ************************************************************************************************************

#define LEGS_DIST_LR 0.3f
#define LEGS_DIST_FB 0.4f

#define C 0.42f

#define V 0.5f
#define BANDWIDTH 0.75f
#define HEIGH_DIFF 0.04f
#define PARAM_A 7.0


static void simpleLinerWalk_generator(float phrase, dog_leg_input_t * vect){
    // static float C = 0.38;
    // static float V = 1.f;

    while (phrase > 1.0){ // to 0~1
        phrase -= 1.0;
    }

    if (phrase < BANDWIDTH){
        vect->theta = atanf(V/C * (phrase - BANDWIDTH / 2));
        vect->dist = (C - HEIGH_DIFF * sinf(phrase * 2 * PI)) / cosf(vect->theta);

    } else {
        vect->theta = (1 + BANDWIDTH - 2 * phrase) * atanf(V/C * (BANDWIDTH / 2)) / (1.f - BANDWIDTH);

        float m = (( HEIGH_DIFF * sinf(BANDWIDTH * 2 * PI) ) / ( (BANDWIDTH - 1) * PARAM_A ) + 1 + BANDWIDTH) / 2 ;
        float t = C - PARAM_A * (1 - m) * (1 - m);
        float y = PARAM_A * (phrase - m) * (phrase - m) + t;
        vect->dist = ( y ) / cosf(vect->theta);
    }
}


int main(void){

    FILE * fp = fopen("data.txt", "w");
    for (int step = 0; step < 2000; step++){

        // ! 在每一个与地面接触的腿上，角度
    
        dog_leg_input_t vec;


        float t = step / 2000.f;
        simpleLinerWalk_generator(t, &vec);

        fprintf(fp,  "%.3f,%.3f,%.3f,%.3f\n", t, vec.theta, vec.dist, vec.dist * cosf(vec.theta));
    }
}