#include "DogSoft.h"
#include "DogApp.h"
#include <math.h>
#include "arm_math.h"
#include "imu.h"

class jumperLegDual
{
public:
    jumperLegDual(dog_motor_single_t *_leg_l, dog_motor_single_t *_leg_r)
    {
        ml = _leg_l;
        mr = _leg_r;
    }

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

    void set_input(const jumpLegInput &input)
    {
        float theta = input.pos.theta;
        float d = (LEG_LEN_LOWER + LEG_LEN_UPPER) * input.pos.dist;
        float dig_angle_half = acosf((LEG_LEN_UPPER * LEG_LEN_UPPER - LEG_LEN_LOWER * LEG_LEN_LOWER + d * d) / (2.f * d * LEG_LEN_UPPER));
        for (int i = 0; i < 2; i++)
        {
            lo.m[i].T = input.T;
            lo.m[i].vel = input.Tvel;

            lo.m[i].kp = input.kp;
            lo.m[i].kv = input.kv;

        }
        lo.m[0].pos = - theta + dig_angle_half;
        lo.m[1].pos =   theta + dig_angle_half;
    }

    void output(void)
    {
        dog_leg_set(ml, &lo);
        dog_leg_set(mr, &lo);
    }

    float get_angle(void){
        return (mr[1].p - mr[0].p)/2.f;
        // return mr->p;
    }

private:
    dog_leg_output_t lo;
    dog_motor_single_t *ml;
    dog_motor_single_t *mr;
};

class dogJumper
{
public:
    dogJumper(bool okl){
        
    };

    void update_jump()
    {
        switch (__STATE)
        {
        case J_STATE_PREPARE:{
            for (int i = 0; i < 2; i++){
                leg_input[i].T = 0.f;
                leg_input[i].Tvel = 0.f;
                leg_input[i].kp = 30.f;
                leg_input[i].kv = 2.f;
            }
            leg_input[0].pos.theta = 0.f;
            leg_input[1].pos.theta = __jumper_angle;
            leg_input[0].pos.dist = 0.34f;
            leg_input[1].pos.dist = 0.37f;
            if (DOG_CTRL_PERIOD_ms * __step > 1000){
                __STATE = J_STATE_FRONT_JUMP;

                // front leg
                leg_input[0].T = 0.f;
                leg_input[0].Tvel = 0.f;
                leg_input[0].kp = 36.f; // TODO: jump kp, kv
                leg_input[0].kv = 0.2f;
                leg_input[0].pos.theta = 0.f;
                leg_input[0].pos.dist = 0.6f;
                
                // back leg
                leg_input[1].T = 0.f;
                leg_input[1].Tvel = 0.f;
                leg_input[1].kp = 30.f;
                leg_input[1].kv = 0.8f;
                leg_input[1].pos.theta = 0.f;
                leg_input[1].pos.dist = 0.34f;
            }
        } break;

        case J_STATE_FRONT_JUMP:{

            // TODO : pitch
            leg_input[1].pos.theta = euler[2] + __jumper_angle * 0.8;
            
            if (fabsf(euler[2] - 45 * PI / 180.f) < 3 * PI / 180.f){
                __STATE = J_STATE_BACK_JUMP;

                // front leg
                leg_input[0].T = 0.f;
                leg_input[0].Tvel = 0.f;
                leg_input[0].kp = 6.f;  // TODO: load kp, kv
                leg_input[0].kv = 2.f;
                leg_input[0].pos.theta = 0.f;
                leg_input[0].pos.dist = 0.2f;
                
                // back leg
                leg_input[1].T = 0.f;
                leg_input[1].Tvel = -0.f;
                leg_input[1].kp = 200.f;
                leg_input[1].kv = 0.06f;
                leg_input[1].pos.theta = euler[2] + __jumper_angle;
                leg_input[1].pos.dist = 0.8f;
                // ST_LOGD("valid!");
                __m_cnt = __step;
            }
        } break;

        case J_STATE_BACK_JUMP:{
            
            leg_input[0].pos.theta = euler[2] - __jumper_angle;
            leg_input[1].pos.theta = euler[2];

            if ((__step - __m_cnt)*DOG_CTRL_PERIOD_ms >= 100) {
                __STATE = J_STATE_LOAD;

                // front leg
                leg_input[0].T = 0.f;
                leg_input[0].Tvel = 0.f;
                leg_input[0].kp = 80.f;  // TODO: load kp, kv
                leg_input[0].kv = 2.4f;
                leg_input[0].pos.theta = 0.f;
                leg_input[0].pos.dist = 0.42f;
                
                // back leg
                leg_input[1].T = 0.f;
                leg_input[1].Tvel = 0.f;
                leg_input[1].kp = 25.f;  // TODO: load kp, kv
                leg_input[1].kv = 0.8f;
                leg_input[1].pos.theta = 0.f;
                leg_input[1].pos.dist = 0.42f;
            }

            // if (__step % 200 == 0){
            //     ST_LOGI("%.2f", g[1].get_angle());
            // }
            // if ()
            // __STATE = J_STATE_FRONT_JUMP;
        } break;

        case J_STATE_LOAD:{
            leg_input[0].pos.theta = euler[2] - __jumper_angle;
            leg_input[1].pos.theta = euler[2];
        } break;

        case J_STATE_FINISHED:{

        } break;
        
        default:
            break;
        }
        






        g[0].set_input(leg_input[0]);
        g[1].set_input(leg_input[1]);
        g[0].output();
        g[1].output();
        __step++;
    }

private:
    enum jumpStateMachine
    {
        J_STATE_PREPARE,
        J_STATE_FRONT_JUMP,
        J_STATE_BACK_JUMP,
        J_STATE_LOAD,
        J_STATE_FINISHED
    };

    uint32_t __step = 0;
    uint32_t __m_cnt = 0;
    jumpStateMachine __STATE = J_STATE_PREPARE;
    float __jumper_angle = 30 * PI / 180;
    
    /*
        0:front
        1:back
    */

    jumperLegDual::jumpLegInput leg_input[2];
    jumperLegDual g[2]{
        jumperLegDual(motors.leg.l_f, motors.leg.r_f),
        jumperLegDual(motors.leg.l_b, motors.leg.r_b)};
};

dogJumper jumper(false);

void dogapp_jumper(void)
{
    jumper.update_jump();
}