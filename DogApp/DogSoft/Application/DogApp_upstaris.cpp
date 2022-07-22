#include "DogApp_util.h"

extern volatile dog_app_choices app;

// class dogUpper
// {
// public:
//     dogUpper(bool okl){
        
//     };

//     void update_jump()
//     {
//         switch (__STATE)
//         {
//         case J_STATE_PREPARE:{
//             for (int i = 0; i < 2; i++){
//                 leg_input[i].T = 0.f;
//                 leg_input[i].Tvel = 0.f;
//                 leg_input[i].kp = 18.f;
//                 leg_input[i].kv = 1.f;
//                 leg_input[i].pos.theta = __jumper_angle + euler[2];
//                 leg_input[i].pos.dist = 0.34f;
//             }

//             // leg_input[0].pos.theta = __jumper_angle;
//             // leg_input[1].pos.theta = __jumper_angle;
//             // leg_input[0].pos.dist = 0.34f;
//             // leg_input[1].pos.dist = 0.34f;
//             if (DOG_CTRL_PERIOD_ms * __step > 400){
//                 __STATE = J_STATE_JUMP;

//                 for (int i = 0; i < 2; i++){
//                     leg_input[i].T = 0.f;
//                     leg_input[i].Tvel = -2.f;
//                     leg_input[i].kv = 0.2f;
//                     leg_input[i].pos.dist = 0.53f;
//                 }
//                 leg_input[0].pos.theta = __jumper_angle + euler[2];
//                 leg_input[1].pos.theta = __jumper_angle + euler[2];
//                 leg_input[0].kp = 75.f;
//                 leg_input[1].kp = 75.f;
//                 __m_cnt = __step;
//             }
//         } break;

//         case J_STATE_JUMP:{
//             leg_input[0].pos.theta = __jumper_angle + euler[2];
//             leg_input[1].pos.theta = __jumper_angle + euler[2];

//             if (g[0].get_d() > 0.51 && g[1].get_d() > 0.51){
//                 __STATE = J_STATE_LOAD;

//                 for (int i = 0; i < 2; i++){
//                     leg_input[i].T = 0.f;
//                     leg_input[i].Tvel = 0.f;
//                     leg_input[i].kp = 22.f; // TODO: jump kp, kv
//                     leg_input[i].kv = 0.2f;
//                     leg_input[i].pos.theta = euler[2];
//                     leg_input[i].pos.dist = 0.4f;
//                 }
//                 __m_cnt = __step;
//             }
//         } break;

//         case J_STATE_LOAD:{
//             leg_input[0].pos.theta = euler[2];
//             leg_input[1].pos.theta = euler[2];
//             if ((__step - __m_cnt) * DOG_CTRL_PERIOD_ms > 100){ 
//                 __STATE = J_STATE_FINISHED;
//                 for (int i = 0; i < 2; i++){
//                     leg_input[i].T = 0.f;
//                     leg_input[i].Tvel = 0.f;
//                     leg_input[i].kp = 18.f;
//                     leg_input[i].kv = 1.f;
//                     leg_input[i].pos.dist = 0.24f;
//                 }
//                 leg_input[0].pos.theta = euler[2];
//                 leg_input[0].pos.theta = euler[2] - __jumper_angle;
//             }
//         } break;

//         case J_STATE_FINISHED:{
//             leg_input[0].pos.theta = euler[2];
//             leg_input[1].pos.theta = euler[2];

//             __step = 0;
//             __STATE = J_STATE_PREPARE;
//             app = APP_NONE;
//         } break;
        
//         default:
//             break;
//         }

//         g[0].set_input(leg_input[0]);
//         g[1].set_input(leg_input[1]);
//         g[0].output();
//         g[1].output();
//         __step++;
//     }

// private:
//     enum jumpStateMachine
//     {
//         J_STATE_PREPARE,
//         J_STATE_JUMP,
//         J_STATE_LOAD,
//         J_STATE_FINISHED
//     };

//     uint32_t __step = 0;
//     uint32_t __m_cnt = 0;
//     jumpStateMachine __STATE = J_STATE_PREPARE;
//     float __jumper_angle = 15.f * PI / 180.f;
    
//     /*
//         0:front
//         1:back
//     */

//     jumperLegDual::jumpLegInput leg_input[2];
//     jumperLegDual g[2]{
//         jumperLegDual(motors.leg.l_f, motors.leg.r_f),
//         jumperLegDual(motors.leg.l_b, motors.leg.r_b)
//     };
// };


// ! jump as big
// class dogUpper
// {
// public:
//     dogUpper(bool okl){
        
//     };

//     void update_jump()
//     {
//         switch (__STATE)
//         {
//         case J_STATE_PREPARE:{// 准备跳跃
//             for (int i = 0; i < 2; i++){
//                 leg_input[i].T = 0.f;
//                 leg_input[i].Tvel = 0.f;
//                 leg_input[i].kp = 30.f;
//                 leg_input[i].kv = 2.f;
//             }
//             leg_input[0].pos.theta = euler[2];
//             leg_input[1].pos.theta = euler[2] + __jumper_angle;
//             leg_input[0].pos.dist = 0.34f;
//             leg_input[1].pos.dist = 0.37f;
//             if (DOG_CTRL_PERIOD_ms * __step > 400){ // 时间到
//                 __STATE = J_STATE_FRONT_JUMP;

//                 // front leg
//                 leg_input[0].T = 0.f;
//                 leg_input[0].Tvel = -12.f;
//                 leg_input[0].kp = 30.f; // TODO: 前腿跳 身体角度控制
//                 leg_input[0].kv = 0.1f;
//                 leg_input[0].pos.theta = euler[2];
//                 leg_input[0].pos.dist = 0.63f;
                
//                 // back leg
//                 leg_input[1].T = 0.f;
//                 leg_input[1].Tvel = 0.f;
//                 leg_input[1].kp = 30.f;
//                 leg_input[1].kv = 0.8f;
//                 leg_input[1].pos.theta = 0.f;
//                 leg_input[1].pos.dist = 0.34f;
//             }
//         } break;

//         case J_STATE_FRONT_JUMP:{// 前脚起跳

//             // TODO : pitch
//             leg_input[1].pos.theta = euler[2] + __jumper_angle;
            
//             if (euler[2] >  30.f * PI / 180.f){ // 车身角度到位
//                 __STATE = J_STATE_BACK_JUMP;

//                 // front leg
//                 leg_input[0].T = 0.f;
//                 leg_input[0].Tvel = 0.f;
//                 leg_input[0].kp = 6.f;
//                 leg_input[0].kv = 2.f;
//                 leg_input[0].pos.theta = euler[2];
//                 leg_input[0].pos.dist = 0.22f;
                
//                 // back leg
//                 leg_input[1].T = 0.f;
//                 leg_input[1].Tvel = -40.f;
//                 leg_input[1].kp = 60.f; // TODO: 后腿跳 力度控制
//                 leg_input[1].kv = 0.01f;
//                 leg_input[1].pos.dist = 0.7f;
//                 // ST_LOGD("valid!");
//             }
//         } break;

//         case J_STATE_BACK_JUMP:{ // 后腿起跳
            
//             leg_input[0].pos.theta = euler[2];
//             leg_input[1].pos.theta = euler[2] + __jumper_angle;

//             // if ((__step - __m_cnt)*DOG_CTRL_PERIOD_ms >= 60) { // 固定输出时间到位
//             if (g[1].get_d() > 0.68) { // ! 收腿过杆
//                 __STATE = J_STATE_LOAD;

//                 // front leg
//                 leg_input[0].T = 0.f;
//                 leg_input[0].Tvel = 0.f;
//                 leg_input[0].kp = 15.f;  
//                 leg_input[0].kv = 0.2f;
//                 leg_input[0].pos.theta = 0.f;
//                 leg_input[0].pos.dist = 0.20f;
                
//                 // back leg
//                 leg_input[1].T = 0.f;
//                 leg_input[1].Tvel = 0.f;
//                 leg_input[1].kp = 15.f;
//                 leg_input[1].kv = 0.2f;
//                 leg_input[1].pos.theta = 0.f;
//                 leg_input[1].pos.dist = 0.20f;
//                 __m_cnt = __step;
//             }
//         } break;

//         case J_STATE_LOAD:{
//             leg_input[0].pos.theta = euler[2];
//             leg_input[1].pos.theta = euler[2];
//             if ((__step - __m_cnt) * DOG_CTRL_PERIOD_ms > 300){ 
//                 __STATE = J_STATE_FINISHED;
//                 for (int i = 0; i < 2; i++){
//                     leg_input[i].T = 0.f;
//                     leg_input[i].Tvel = 0.f;
//                     leg_input[i].kp = 18.f;
//                     leg_input[i].kv = 1.f;
//                     leg_input[i].pos.dist = 0.48f;
//                 }
//                 leg_input[0].pos.theta = euler[2];
//                 // leg_input[0].pos.theta = euler[2] - __jumper_angle;
//             }
//         } break;

//         case J_STATE_FINISHED:{
//             leg_input[0].pos.theta = euler[2];
//             leg_input[1].pos.theta = euler[2];

//             __step = 0;
//             __STATE = J_STATE_PREPARE;
//             app = APP_NONE;
//         } break;
        
//         default:
//             break;
//         }

//         g[0].set_input(leg_input[0]);
//         g[1].set_input(leg_input[1]);
//         g[0].output();
//         g[1].output();
//         __step++;
//     }

// private:
//     enum jumpStateMachine
//     {
//         J_STATE_PREPARE,
//         J_STATE_FRONT_JUMP,
//         J_STATE_BACK_JUMP,
//         J_STATE_FLY,
//         J_STATE_LOAD,
//         J_STATE_FINISHED,
//     };


//     uint32_t __step = 0;
//     uint32_t __m_cnt = 0;
//     jumpStateMachine __STATE = J_STATE_PREPARE;
//     float __jumper_angle = 36.f * PI / 180.f;
    
//     /*
//         0:front
//         1:back
//     */

//     jumperLegDual::jumpLegInput leg_input[2];
//     jumperLegDual g[2]{
//         jumperLegDual(motors.leg.l_f, motors.leg.r_f),
//         jumperLegDual(motors.leg.l_b, motors.leg.r_b)
//     };
// };

// ! 纵跳
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
                leg_input[i].pos.theta = euler[2];
                leg_input[i].pos.dist = 0.34f;
            }

            if (DOG_CTRL_PERIOD_ms * __step > 400){
                __STATE = J_STATE_JUMP;

                for (int i = 0; i < 2; i++){
                    leg_input[i].T = 0.f;
                    leg_input[i].Tvel = -2.f;
                    leg_input[i].kv = 0.01f;
                    leg_input[i].pos.dist = 0.53f;
                    leg_input[i].pos.theta = __jumper_angle + euler[2];
                }
                leg_input[0].kp = 46.f;
                leg_input[1].kp = 55.f;
                // leg_input[1].pos.theta = __jumper_angle;
                // leg_input[1].kp = 55.f;
                __m_cnt = __step;
            }
        } break;

        case J_STATE_JUMP:{
            leg_input[0].pos.theta = __jumper_angle + euler[2];
            leg_input[1].pos.theta = __jumper_angle + euler[2];

            if (g[0].get_d() > 0.51 && g[1].get_d() > 0.51){
                __STATE = J_STATE_LOAD;

                for (int i = 0; i < 2; i++){
                    leg_input[i].T = 0.f;
                    leg_input[i].Tvel = 0.f;
                    leg_input[i].kp = 28.f; // TODO: jump kp, kv
                    leg_input[i].kv = 0.22f;
                    leg_input[i].pos.theta = euler[2];
                    leg_input[i].pos.dist = 0.34f;
                }
                __m_cnt = __step;
            }
        } break;

        case J_STATE_LOAD:{
            leg_input[0].pos.theta = euler[2];
            leg_input[1].pos.theta = euler[2];
            if ((__step - __m_cnt) * DOG_CTRL_PERIOD_ms > 400){ 
                __STATE = J_STATE_FINISHED;
                for (int i = 0; i < 2; i++){
                    leg_input[i].T = 0.f;
                    leg_input[i].Tvel = 0.f;
                    leg_input[i].kp = 24.f;
                    leg_input[i].kv = 1.0f;
                    leg_input[i].pos.dist = 0.37f;
                    leg_input[i].pos.theta = euler[2];
                }
                // leg_input[0].pos.theta = euler[2];
                // leg_input[0].pos.theta = euler[2] - __jumper_angle;
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
    float __jumper_angle = 15.f * PI / 180.f;
    
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