#include "DogBody.h"
#include <math.h>
#include "arm_math.h"
#include "imu.h"
#define LEGS_DIST_LR 0.3f
#define LEGS_DIST_FB 0.4f

// #include "imu.h"


// void body_move_font(void){
//     dog_leg_input_t vec[4];

// }

#define H 0.42f
#define V 0.6f

#define BANDWIDTH 0.75f
#define HEIGH_DIFF 0.04f
#define PARAM_A 7.0

#define PERIOD_CNT 220

#define LINE_TIME_MS PERIOD_CNT * DOG_CTRL_PERIOD_ms * BANDWIDTH


/*
// height: 0~1 theoreticaly but [0.3~0.5] better
target_pitch: in degree
vel   : 0~1 theoreticaly but [0 ~ 0.8] better

*/
static void simpleLinerWalk_generator(float phrase, dog_leg_input_t * vect, float target_pitch, float tgt_yaw, int l_0_r_1){
    static float tgc_yaw = 0.f;
    float height;
    float vel;

    while (phrase > 1.0){ // to 0~1
        phrase -= 1.0;
    } 

    if ( fabs(phrase - ( BANDWIDTH + ((1-BANDWIDTH) / 2.f) )) < ( 2.1f / PERIOD_CNT ) ){
        tgc_yaw = tgt_yaw;
    }


    height = sinf(target_pitch) * LEGS_DIST_FB / 2.f;
    if (fabsf(height) > 0.15f){
        height = 0.15 * (height > 0.f ? 1.f : -1.f);
    }
    height += H;

    vel = tanf(tgc_yaw * ((l_0_r_1 == 1)? 1.f: -1.f)) * LEGS_DIST_LR * 7.f; // TODO: check the unit of 'V'
    // vel = tgt_yaw * 100; // ! dangerous
    if (fabsf(vel) > 0.3f){
        vel = 0.3 * (vel > 0.f ? 1.f : -1.f);
    }
    vel += V;

    if (phrase < BANDWIDTH){
        vect->theta = atanf(vel/height * (phrase - BANDWIDTH / 2))  + target_pitch; // TODO : add some offset here
        vect->dist = (height - HEIGH_DIFF * sinf(phrase * 2 * PI)) / cosf(vect->theta);

    } else {
        vect->theta = (1 + BANDWIDTH - 2 * phrase) * atanf(vel/height * (BANDWIDTH / 2)) / (1.f - BANDWIDTH);

        float m = (( HEIGH_DIFF * sinf(BANDWIDTH * 2 * PI) ) / ( (BANDWIDTH - 1) * PARAM_A ) + 1 + BANDWIDTH) / 2 ;
        float t = height - PARAM_A * (1 - m) * (1 - m);
        float y = PARAM_A * (phrase - m) * (phrase - m) + t;
        vect->dist = ( y ) / cosf(vect->theta);
    }
}

// 2ms 
// 1s period -> 
extern float target_yaw;
arm_pid_instance_f32 pid_yaw = {
    .Kp = 1.0f,
    .Ki = 0.00f,
};
/*
    1. get lastPack input, cal state
    2. generate step. (independent step)
    3. leg output
*/
void dog_body_simpleLinerWalk(){
    static uint32_t step = 0;
    // static float yaw_intr = 0.f;
    // static float ccg_yaw = 0.f;
    // - >
    // + <

    dog_leg_input_t vec[4];

    float t = step / (float)(PERIOD_CNT);

    float target_pitch = 0.f; // TODO : check direction
    /*
        抬脑袋 ：+
        低头   ：-
    */
    float tgt_yaw = arm_pid_f32(&pid_yaw, target_yaw - yaw); // TODO : check direction
    // float tgt_yaw = 0.f; // TODO : check direction
    /*
        左转   ：+
        右转   ：-
    */

    simpleLinerWalk_generator(t + (0.25 * 0), &(vec[0]), target_pitch, tgt_yaw, 0);
    simpleLinerWalk_generator(t + (0.25 * 1), &(vec[1]), target_pitch, tgt_yaw, 0);
    simpleLinerWalk_generator(t + (0.25 * 2), &(vec[2]), target_pitch, tgt_yaw, 1);
    simpleLinerWalk_generator(t + (0.25 * 3), &(vec[3]), target_pitch, tgt_yaw, 1);

    dog_leg_set_phrase(motors.leg.l_b, &vec[0]);
    dog_leg_set_phrase(motors.leg.l_f, &vec[1]);
    dog_leg_set_phrase(motors.leg.r_b, &vec[2]);
    dog_leg_set_phrase(motors.leg.r_f, &vec[3]);

    step ++;
    // yaw_intr += yaw;
    if (step == PERIOD_CNT){
        step = 0;
        // ccg_yaw = yaw_intr * 180.f / ( (float)(PERIOD_CNT) * PI);
        // uart_printf("yaw:%.2f\n", ccg_yaw);
        
        // yaw_intr = 0.f;
    }
}

void dog_body_standup(float kp, float kv){
    const static dog_leg_input_t vec = {
        .theta = 0.f,
        .dist = 0.30f,
    };

    if (kp > 0.f){
        angle_conf.kp = kp;
    }
    if (kv > 0.f){
        angle_conf.kv = kv;
    }
    angle_conf.t = 0;

    dog_leg_set_phrase(motors.leg.l_f, &vec);
    dog_leg_set_phrase(motors.leg.r_f, &vec);
    dog_leg_set_phrase(motors.leg.r_b, &vec);
    dog_leg_set_phrase(motors.leg.l_b, &vec);
}

void dog_body_sitdown(void){
    for (int i = 0; i < 8; i++){
        dog_motor_set_Control_param(&(motors.raw[i]), - ((float)(motors.raw[i].invers) * (motors.raw[i].zeroPos_offset)), 0.f, 2.f, 1.0f, 0.f);
    }
}

void dog_body_force_stop(void){
    for (int i = 0; i < 8; i++){
        dog_motor_set_Control_param(motors.raw + i, 0.f, 0.f, 0.f, 0.f, 0.f);
    }
    uart_printf("all motor force stoped\n");
}
