#include "DogApp_util.h"

extern volatile dog_app_choices app;

#define JUMP_ANGLE euler[2] + __jumper_angle 

jumperLegDual::jumperLegDual(dog_motor_single_t *_leg_l, dog_motor_single_t *_leg_r){
    ml = _leg_l;
    mr = _leg_r;
}

void jumperLegDual::set_input(const jumpLegInput &input)
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

void jumperLegDual::output(void)
{
    dog_leg_set(ml, &lo);
    dog_leg_set(mr, &lo);
}

float jumperLegDual::get_angle(void){
    return (mr[1].p - mr[0].p)/2.f;
}

float jumperLegDual::get_d(void){
    float t = (mr[1].p + mr[0].p)/2.f;
    float ret0 = LEG_LEN_UPPER * cosf(t);
    float ret1 = sqrtf(ret0 * ret0 - LEG_LEN_UPPER * LEG_LEN_UPPER + LEG_LEN_LOWER * LEG_LEN_LOWER);
    return (ret0 + ret1) / (LEG_LEN_LOWER + LEG_LEN_UPPER);
}

class dogJumper
{
public:
    dogJumper(bool okl){
        
    };

    void update_jump()
    {
        switch (__STATE)
        {
        case J_STATE_PREPARE:{// 准备跳跃
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
            if (DOG_CTRL_PERIOD_ms * __step > 400){ // 时间到
                __STATE = J_STATE_FRONT_JUMP;

                // front leg
                leg_input[0].T = 0.f;
                leg_input[0].Tvel = -12.f;
                leg_input[0].kp = 75.f; // TODO: 前腿跳 身体角度控制
                leg_input[0].kv = 0.1f;
                leg_input[0].pos.theta = 0.f;
                leg_input[0].pos.dist = 0.63f;
                
                // back leg
                leg_input[1].T = 0.f;
                leg_input[1].Tvel = 0.f;
                leg_input[1].kp = 30.f;
                leg_input[1].kv = 0.8f;
                leg_input[1].pos.theta = 0.f;
                leg_input[1].pos.dist = 0.34f;
            }
        } break;

        case J_STATE_FRONT_JUMP:{// 前脚起跳

            // TODO : pitch
            leg_input[1].pos.theta = JUMP_ANGLE;
            
            if (euler[2] >  PI / 2.f - __jumper_angle){ // 车身角度到位
                __STATE = J_STATE_BACK_JUMP;
                // __STATE = J_STATE_FINISHED;

                // front leg
                leg_input[0].T = 0.f;
                leg_input[0].Tvel = 0.f;
                leg_input[0].kp = 6.f;
                leg_input[0].kv = 1.2f;
                leg_input[0].pos.theta = 0.f;
                leg_input[0].pos.dist = 0.18f;
                
                // back leg
                leg_input[1].T = 0.f;
                leg_input[1].Tvel = -45.f;
                leg_input[1].kp = 490.f; // TODO: 后腿跳 力度控制
                leg_input[1].kv = 0.01f;
                leg_input[1].pos.dist = 0.89f; // ! 改长
                // ST_LOGD("valid!");
            }
        } break;

        case J_STATE_BACK_JUMP:{ // 后腿起跳
            
            leg_input[0].pos.theta = euler[2] - __jumper_angle;
            leg_input[1].pos.theta = JUMP_ANGLE;

            // if ((__step - __m_cnt)*DOG_CTRL_PERIOD_ms >= 60) { // 固定输出时间到位
            if (g[1].get_d() > 0.87) { // ! 收腿过杆
                __STATE = J_STATE_FLY;

                // front leg
                leg_input[0].T = 0.f;
                leg_input[0].Tvel = 0.f;
                leg_input[0].kp = 15.f;  
                leg_input[0].kv = 0.8f;
                leg_input[0].pos.theta = 0.f;
                leg_input[0].pos.dist = 0.22f;
                
                // back leg
                leg_input[1].T = 0.f;
                leg_input[1].Tvel = 0.f;
                leg_input[1].kp = 32.f;
                leg_input[1].kv = 0.2f;
                leg_input[1].pos.theta = 0.f;
                leg_input[1].pos.dist = 0.22f;
                __m_cnt = __step;
            }
        } break;

        case J_STATE_FLY:{ // 飞空
            leg_input[0].pos.theta = euler[2] - __jumper_angle;
            leg_input[1].pos.theta = euler[2];
            // if (euler[2] < - 8.f * PI / 180.f){
            if ((__step - __m_cnt) * DOG_CTRL_PERIOD_ms > 500){
                __STATE = J_STATE_LOAD;

                // front leg
                leg_input[0].T = 0.f;
                leg_input[0].Tvel = 0.f;
                leg_input[0].kp = 80.f;  // TODO: load kp, kv
                leg_input[0].kv = 2.4f;
                leg_input[0].pos.theta = euler[2] - __jumper_angle;
                leg_input[0].pos.dist = 0.48f;
                
                // back leg
                leg_input[1].T = 0.f;
                leg_input[1].Tvel = 0.f;
                leg_input[1].kp = 32.f;  // TODO: load kp, kv
                leg_input[1].kv = 0.8f;
                leg_input[1].pos.theta = euler[2];
                leg_input[1].pos.dist = 0.34f;
                __m_cnt = __step;
            }
        } break;

        case J_STATE_LOAD:{ // 降落准备
            leg_input[0].pos.theta = euler[2] - __jumper_angle;
            leg_input[1].pos.theta = euler[2];
            // if (euler[2] > - 8.f * PI / 180.f){
            if ((__step - __m_cnt) * DOG_CTRL_PERIOD_ms > 200){
                __STATE = J_STATE_FINISHED;

                leg_input[1].kp = 80.f;  // TODO: load kp, kv
                leg_input[1].kv = 2.4f;
                leg_input[1].pos.theta = euler[2];
                leg_input[1].pos.dist = 0.40f;
            }
        } break;

        case J_STATE_FINISHED:{ // 完成
            leg_input[0].pos.theta = euler[2];
            leg_input[1].pos.theta = euler[2];

            __step = 0;
            __STATE = J_STATE_PREPARE;
            app = APP_NONE;

        } break;

        // case J_STATE_TEST:{
        //     leg_input[1].pos.theta = euler[2] + __jumper_angle * 0.8;
        //     if (__last_euler > euler[2]){
        //         ST_LOGI("%.2f", __last_euler * 180.f / PI);
        //     }
        //     __last_euler = euler[2];

        //     if ((__step - __m_cnt)*DOG_CTRL_PERIOD_ms >= 500){
        //         __STATE = J_STATE_FINISHED;
        //     }
        // } break;
        
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
        J_STATE_FLY,
        J_STATE_LOAD,
        J_STATE_FINISHED,

        // J_STATE_TEST
    };

    uint32_t __step = 0;
    uint32_t __m_cnt = 0;
    jumpStateMachine __STATE = J_STATE_PREPARE;
    float __jumper_angle = 30.f * PI / 180.f;

    float __last_euler = 0.f;
    
    /*
        0:front
        1:back
    */

    jumperLegDual::jumpLegInput leg_input[2];
    jumperLegDual g[2]{
        jumperLegDual(motors.leg.l_f, motors.leg.r_f),
        jumperLegDual(motors.leg.l_b, motors.leg.r_b)
    };
};

dogJumper jumper(false);

void dogapp_jumper(void)
{
    jumper.update_jump();
}