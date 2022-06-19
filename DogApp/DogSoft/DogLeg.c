#include "DogLeg.h"
#include <math.h>
#include "arm_math.h"

// in mm
#define LEG_UP 150.f
#define LEG_DOWN 200.f

#define COMPUTE_TEMP(x) (LEG_UP*LEG_UP - LEG_DOWN*LEG_DOWN + x*x)/(2.f * x * LEG_UP)


void dog_leg_set_phrase(dog_motor_single_t * motor, const dog_leg_input_t * input){
    float d = LEG_DOWN - LEG_UP + 2 * input->dist * LEG_UP;

    // check input valid
    if (input->dist < 0.2 || input->dist > 0.9){
        return;
    }
    
    float dig_angle_half = acosf((LEG_UP*LEG_UP - LEG_DOWN*LEG_DOWN + d*d)/(2.f * d * LEG_UP)); // f


    // dig_angle_half *= invers;

    // dog_motor_set_angle(motor + 0, - input->theta + dig_angle_half);
    // dog_motor_set_angle(motor + 1,   input->theta + dig_angle_half);
    dog_motor_set_Control_param(motor + 0, - input->theta + dig_angle_half, 0.f, angle_conf.kp, angle_conf.kv, angle_conf.t);
    dog_motor_set_Control_param(motor + 1,   input->theta + dig_angle_half, 0.f, angle_conf.kp, angle_conf.kv, angle_conf.t);
}

void dog_leg_get_phrase(dog_motor_single_t * motor, dog_leg_input_t * output){ // TODO
    output->theta = ( motor[1].p - motor[0].p ) / 2;
    float dig_angle_half = ( motor[1].p + motor[0].p ) / 2;
    float cosVal = cosf(dig_angle_half); 
    output->dist = LEG_UP * cosVal + sqrtf(LEG_UP * LEG_UP * (cosVal * cosVal + 1) - LEG_DOWN * LEG_DOWN);
}

/* 
speed : m/s
d_limit : 0~1
 */
int dog_leg_set_speed(dog_motor_single_t * motor, const float speed, const float d_limit){
    // float d = LEG_DOWN - LEG_UP + 2 * input->dist * LEG_UP;
    dog_leg_input_t vec;
    dog_leg_get_phrase(motor, &vec);
    if ( (speed > 0.f && vec.dist >= d_limit) || (speed < 0.f && vec.dist <= d_limit) ){
        // TODO : p
        return 1;
    } 
    float d = LEG_DOWN - LEG_UP + 2 * vec.dist * LEG_UP;
    float dig_angle_half = acosf((LEG_UP*LEG_UP - LEG_DOWN*LEG_DOWN + d*d)/(2.f * d * LEG_UP));

    // ( speed / 2 ) / cosf(dig_angle_half);
    
}

/*
out put Force(N)
*/
void dog_leg_set_force(dog_motor_single_t * motor, float F){

}