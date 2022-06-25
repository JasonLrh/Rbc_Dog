#include "DogCMD.h"

extern FDCAN_HandleTypeDef hfdcan1;
uint8_t tim_queue_enable = 0;
void dogcmd_system(const char * cmd){
    float kp, kv;
    if (cmd[0] != '\0'){
        switch (cmd[0]){
            // case 'C':{
            //     // can
            //     uart_printf("can rx : %d\n", HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0));
            // } break;

            case 'S':{
                // if (HAL_TIM_Base_GetState(&htim6) == HAL_TIM_STATE_BUSY){
                //     HAL_TIM_Base_Stop_IT(&htim6);
                // }
                tim_queue_enable = 0;
                switch (cmd[1])
                {
                    case 'W':
                        // HAL_TIM_Base_Start_IT(&htim6);
                        tim_queue_enable = 1;
                        break;
                    
                    case 'U':
                        sscanf(cmd + 2, "%f,%f", &kp, &kv);
                        dog_body_standup(kp, kv);
                        break;
                    
                    case 'D':
                        dog_body_sitdown();
                        break;
                    
                    // case 'S':
                    //     dog_body_standup(-1, -1); // tim has stoped , just standup here
                    //     break;

                    case 'F':
                        // TODO: change output function
                        break;
                
                default:{
                    ST_LOGE("Invalid CMD");
                } break;
                }

            } break;

            default:{
                ST_LOGE("Invalid CMD");
            } break;
        }
    }else{
        ST_LOGE("system cmd not recognized");
    }
}