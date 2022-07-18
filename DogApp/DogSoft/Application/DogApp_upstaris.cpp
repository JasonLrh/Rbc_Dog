#include "DogApp_util.h"

extern volatile dog_app_choices app;

class dogUpper
{
public:
    dogUpper(bool okl){
        
    };

    void update_jump()
    {
        switch (__STATE)
        {
        case J_STATE_PREPARE:{
            for (int i = 0; i < 2; i++){
                leg_input[i].T = 0.f;
                leg_input[i].Tvel = 0.f;
                leg_input[i].kp = 18.f;
                leg_input[i].kv = 1.f;
                leg_input[i].pos.theta = __jumper_angle + euler[2];
                leg_input[i].pos.dist = 0.34f;
            }
            // leg_input[0].pos.theta = __jumper_angle;
            // leg_input[1].pos.theta = __jumper_angle;
            // leg_input[0].pos.dist = 0.34f;
            // leg_input[1].pos.dist = 0.34f;
            if (DOG_CTRL_PERIOD_ms * __step > 1000){
                __STATE = J_STATE_JUMP;

                // front leg
                for (int i = 0; i < 2; i++){
                    leg_input[i].T = 0.f;
                    leg_input[i].Tvel = -3.f;
                    leg_input[i].kp = 80.f; // TODO: jump kp, kv
                    leg_input[i].kv = 0.3f;
                    leg_input[i].pos.theta = __jumper_angle + euler[2];
                    leg_input[i].pos.dist = 0.5f;
                }
                __m_cnt = __step;
            }
        } break;

        case J_STATE_JUMP:{
            
            if ((__step - __m_cnt) * DOG_CTRL_PERIOD_ms > 200){
                __STATE = J_STATE_LOAD;

                for (int i = 0; i < 2; i++){
                    leg_input[i].T = 0.f;
                    leg_input[i].Tvel = 0.f;
                    leg_input[i].kp = 22.f; // TODO: jump kp, kv
                    leg_input[i].kv = 0.2f;
                    leg_input[i].pos.theta = euler[2];
                    leg_input[i].pos.dist = 0.4f;
                }
                __m_cnt = __step;
            }
        } break;

        case J_STATE_LOAD:{
            leg_input[0].pos.theta = euler[2];
            leg_input[1].pos.theta = euler[2];
            if ((__step - __m_cnt) * DOG_CTRL_PERIOD_ms > 200){ 
                __STATE = J_STATE_FINISHED;
                for (int i = 0; i < 2; i++){
                    leg_input[i].T = 0.f;
                    leg_input[i].Tvel = 0.f;
                    leg_input[i].kp = 18.f;
                    leg_input[i].kv = 1.f;
                    leg_input[i].pos.theta = euler[2];
                    leg_input[i].pos.dist = 0.34f;
                }
            }
        } break;

        case J_STATE_FINISHED:{
            leg_input[0].pos.theta = euler[2];
            leg_input[1].pos.theta = euler[2];

            __step = 0;
            __STATE = J_STATE_PREPARE;
            app = APP_NONE;
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
        J_STATE_JUMP,
        J_STATE_LOAD,
        J_STATE_FINISHED
    };

    uint32_t __step = 0;
    uint32_t __m_cnt = 0;
    jumpStateMachine __STATE = J_STATE_PREPARE;
    float __jumper_angle = 15 * PI / 180;
    
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

dogUpper upper(false);

void dogapp_upstaris(void)
{
    upper.update_jump();
}