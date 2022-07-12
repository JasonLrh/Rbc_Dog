#include "DogLeg.h"
#include "DogSoft.h"
#include <math.h>
#include "arm_math.h"


#define COMPUTE_TEMP(x) (LEG_LEN_UPPER*LEG_LEN_UPPER - LEG_LEN_LOWER*LEG_LEN_LOWER + x*x)/(2.f * x * LEG_LEN_UPPER)

void dog_leg_set(dog_motor_single_t * motor, const dog_leg_output_t * input){
    dog_motor_set(motor + 0, (input->m) + 0);
    dog_motor_set(motor + 1, (input->m) + 1);
}

void dog_leg_set_phrase(dog_motor_single_t * motor, const dog_leg_input_t * input){
    // float d = LEG_LEN_LOWER - LEG_LEN_UPPER + 2 * input->dist * LEG_LEN_UPPER;
    float d = (LEG_LEN_LOWER + LEG_LEN_UPPER) * input->dist ;
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

void dog_leg_get_phrase(dog_motor_single_t * motor, dog_leg_input_t * output){ // TODO
    output->theta = ( motor[1].p - motor[0].p ) / 2;
    float dig_angle_half = ( motor[1].p + motor[0].p ) / 2;
    float cosVal = cosf(dig_angle_half); 
    output->dist = LEG_LEN_UPPER * cosVal + sqrtf(LEG_LEN_UPPER * LEG_LEN_UPPER * (cosVal * cosVal + 1) - LEG_LEN_LOWER * LEG_LEN_LOWER);
}

/* 
speed : m/s
d_limit : 0~1
 */
int dog_leg_set_speed(dog_motor_single_t * motor, const float speed, const float d_limit){
    // float d = LEG_LEN_LOWER - LEG_LEN_UPPER + 2 * input->dist * LEG_LEN_UPPER;
    dog_leg_input_t vec;
    dog_leg_get_phrase(motor, &vec);
    if ( (speed > 0.f && vec.dist >= d_limit) || (speed < 0.f && vec.dist <= d_limit) ){
        // TODO : p
        return 1;
    } 
    // float d = LEG_LEN_LOWER - LEG_LEN_UPPER + 2 * vec.dist * LEG_LEN_UPPER;
    // float dig_angle_half = acosf((LEG_LEN_UPPER*LEG_LEN_UPPER - LEG_LEN_LOWER*LEG_LEN_LOWER + d*d)/(2.f * d * LEG_LEN_UPPER));

    // ( speed / 2 ) / cosf(dig_angle_half);
    return -1;   
}

/*
out put Force(N)
*/
void dog_leg_set_force(dog_motor_single_t * motor, float F){

}