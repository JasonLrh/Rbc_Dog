#include "DogApp.h"


#define H 0.42f
#define HEIGHT H*(LEG_LEN_UPPER + LEG_LEN_LOWER)

#define V 0.6f

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

typedef struct _dog_leg_output_t {
    /* data */
}dog_leg_output_t ;



void dog_leg_set_phrase(dog_motor_single_t * motor, const dog_leg_input_t * input){
    float d = LEG_LEN_LOWER - LEG_LEN_UPPER + 2 * input->dist * LEG_LEN_UPPER;

    // check input valid
    if (input->dist < 0.2 || input->dist > 0.9){
        return;
    }
    
    float dig_angle_half = acosf((LEG_LEN_UPPER*LEG_LEN_UPPER - LEG_LEN_LOWER*LEG_LEN_LOWER + d*d)/(2.f * d * LEG_LEN_UPPER)); // f


    // dig_angle_half *= invers;

    // dog_motor_set_angle(motor + 0, - input->theta + dig_angle_half);
    // dog_motor_set_angle(motor + 1,   input->theta + dig_angle_half);
    // TODO : v : rad/s
    dog_motor_set_Control_param(motor + 0, - input->theta + dig_angle_half, 0.f, angle_conf.kp, angle_conf.kv, angle_conf.t);
    dog_motor_set_Control_param(motor + 1,   input->theta + dig_angle_half, 0.f, angle_conf.kp, angle_conf.kv, angle_conf.t);
}

/*
// height: 0~1 theoreticaly but [0.3~0.5] better
target_pitch: in degree
vel   : 0~1 theoreticaly but [0 ~ 0.8] better

*/
static void simpleLinerWalk_generator(float phrase, dog_leg_input2_t * vect_notuse){
    float height;
    float vel;
    
    dog_leg_input_t vect;

    while (phrase > 1.0){ // to 0~1
        phrase -= 1.0;
    } 

    height = H;

    vel = V;

    if (phrase < BANDWIDTH){
        vect.theta = atanf(vel/height * (phrase - BANDWIDTH / 2)) ; // TODO : add some offset here
        vect.dist = (height - HEIGH_DIFF * sinf(phrase * 2 * PI)) / cosf(vect.theta);

    } else {
        vect.theta = (1 + BANDWIDTH - 2 * phrase) * atanf(vel/height * (BANDWIDTH / 2)) / (1.f - BANDWIDTH);

        float m = (( HEIGH_DIFF * sinf(BANDWIDTH * 2 * PI) ) / ( (BANDWIDTH - 1) * PARAM_A ) + 1 + BANDWIDTH) / 2 ;
        float t = height - PARAM_A * (1 - m) * (1 - m);
        float y = PARAM_A * (phrase - m) * (phrase - m) + t;
        vect.dist = ( y ) / cosf(vect.theta);
    }
}

void dogapp_walk(void){
    static uint32_t step = 0;
    // static float yaw_intr = 0.f;
    // static float ccg_yaw = 0.f;
    // - >
    // + <

    dog_leg_input2_t vec[4];

    float t = step / (float)(PERIOD_CNT);

    /*
        抬脑袋 ：+
        低头   ：-
    */
    // float tgt_yaw = arm_pid_f32(&pid_yaw, target_yaw - yaw); // TODO : check direction
    // float tgt_yaw = 0.f; // TODO : check direction
    /*
        左转   ：+
        右转   ：-
    */

    simpleLinerWalk_generator(t + (0.25 * 0), &(vec[0]));
    simpleLinerWalk_generator(t + (0.25 * 1), &(vec[1]));
    simpleLinerWalk_generator(t + (0.25 * 2), &(vec[2]));
    simpleLinerWalk_generator(t + (0.25 * 3), &(vec[3]));

    // dog_leg_set_phrase(motors.leg.l_b, &vec[0]);
    // dog_leg_set_phrase(motors.leg.l_f, &vec[1]);
    // dog_leg_set_phrase(motors.leg.r_b, &vec[2]);
    // dog_leg_set_phrase(motors.leg.r_f, &vec[3]);

    step ++;
    if (step == PERIOD_CNT){
        step = 0;
    }
}